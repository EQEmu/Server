/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2006  EQEMu Development Team (http://eqemulator.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../common/debug.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>

#include "../common/rulesys.h"
#include "../common/MiscFunctions.h"
#include "zone_profile.h"
#include "map.h"
#include "zone.h"
#include "pathing.h"
#ifdef _WINDOWS
#define snprintf	_snprintf
#endif


extern Zone* zone;

#define FEAR_PATHING_DEBUG


//this is called whenever we are damaged to process possible fleeing
void Mob::CheckFlee() {
	//if were allready fleeing, dont need to check more...
	if(flee_mode && curfp)
		return;
	
	//dont bother if we are immune to fleeing
	if(SpecAttacks[IMMUNE_FLEEING] || spellbonuses.ImmuneToFlee)
		return;

	if(!flee_timer.Check())
		return;	//only do all this stuff every little while, since
				//its not essential that we start running RIGHT away
	
	//see if were possibly hurt enough
	float ratio = GetHPRatio();
	if(ratio >= RuleI(Combat, FleeHPRatio))
		return;
	
	//we might be hurt enough, check con now..
	Mob *hate_top = GetHateTop();
	if(!hate_top) {
		//this should never happen...
		StartFleeing();
		return;
	}
	
	float other_ratio = hate_top->GetHPRatio();
	if(other_ratio < 20) {
		//our hate top is almost dead too... stay and fight
		return;
	}
	
	//base our flee ratio on our con. this is how the 
	//attacker sees the mob, since this is all we can observe
	uint32 con = GetLevelCon(hate_top->GetLevel(), GetLevel());
	float run_ratio;
	switch(con) {
		//these values are not 100% researched
		case CON_GREEN:
			run_ratio = RuleI(Combat, FleeHPRatio);
			break;
		case CON_LIGHTBLUE:
			run_ratio = RuleI(Combat, FleeHPRatio) * 8 / 10;
			break;
		case CON_BLUE:
			run_ratio = RuleI(Combat, FleeHPRatio) * 6 / 10;
			break;
		default:
			run_ratio = RuleI(Combat, FleeHPRatio) * 4 / 10;
			break;
	}
	if(ratio < run_ratio)
	{
		if( RuleB(Combat, FleeIfNotAlone) 
		  || ( !RuleB(Combat, FleeIfNotAlone) 
		    && (entity_list.GetHatedCount(hate_top, this) == 0)))
			StartFleeing();

	}
}


void Mob::ProcessFlee() {

	//Stop fleeing if effect is applied after they start to run. 
	//When ImmuneToFlee effect fades it will turn fear back on and check if it can still flee. 
	if(flee_mode && (SpecAttacks[IMMUNE_FLEEING] || spellbonuses.ImmuneToFlee) && !spellbonuses.IsFeared){
		curfp = false;
		return;
	}

	//see if we are still dying, if so, do nothing
	if(GetHPRatio() < (float)RuleI(Combat, FleeHPRatio))
		return;
	
	//we are not dying anymore... see what we do next
	
	flee_mode = false;
	
	//see if we are legitimately feared now
	if(!spellbonuses.IsFeared) {
		//not feared... were done...
		curfp = false;
		return;
	}
}

float Mob::GetFearSpeed() {
    if(flee_mode) {
        //we know ratio < FLEE_HP_RATIO
        float speed = GetRunspeed();
        float ratio = GetHPRatio();

		// mob's movement will halt with a decent snare at HP specified by rule.
		if (ratio <= RuleI(Combat, FleeSnareHPRatio) && GetSnaredAmount() > 40) {
				return 0.0001f;
		}

		if (ratio < FLEE_HP_MINSPEED)
			ratio = FLEE_HP_MINSPEED;

        speed = speed * 0.5 * ratio / 100;
 
        return(speed);
    }
    return(GetRunspeed());
}

void Mob::CalculateNewFearpoint()
{
	if(RuleB(Pathing, Fear) && zone->pathing)
	{
		int Node = zone->pathing->GetRandomPathNode();
	
		VERTEX Loc = zone->pathing->GetPathNodeCoordinates(Node);

		++Loc.z;

		VERTEX CurrentPosition(GetX(), GetY(), GetZ());

		list<int> Route = zone->pathing->FindRoute(CurrentPosition, Loc);

		if(Route.size() > 0)
		{
			fear_walkto_x = Loc.x;
			fear_walkto_y = Loc.y;
			fear_walkto_z = Loc.z;
			curfp = true;

			mlog(PATHING__DEBUG, "Feared to node %i (%8.3f, %8.3f, %8.3f)", Node, Loc.x, Loc.y, Loc.z);
			return;
		}

		mlog(PATHING__DEBUG, "No path found to selected node. Falling through to old fear point selection.");
	}

	int loop = 0;
	float ranx, rany, ranz;
	curfp = false;
	while (loop < 100) //Max 100 tries
	{
		int ran = 250 - (loop*2);
		loop++;
		ranx = GetX()+MakeRandomInt(0, ran-1)-MakeRandomInt(0, ran-1);
		rany = GetY()+MakeRandomInt(0, ran-1)-MakeRandomInt(0, ran-1);
		ranz = FindGroundZ(ranx,rany);
		if (ranz == -999999)
			continue;
		float fdist = ranz - GetZ();
		if (fdist >= -12 && fdist <= 12 && CheckCoordLosNoZLeaps(GetX(),GetY(),GetZ(),ranx,rany,ranz))
		{
			curfp = true;
			break;
		}
	}
	if (curfp)
	{
		fear_walkto_x = ranx;
		fear_walkto_y = rany;
		fear_walkto_z = ranz;
	}
	else //Break fear
	{
		BuffFadeByEffect(SE_Fear);
	}
}

//we need to start acting scared...
//old fear function, kept for ref.
/*void Mob::SetFeared(Mob *caster, uint32 duration, bool flee) {
	//special args to stop fear
	if(caster == nullptr && duration == 0) {
		fear_state = fearStateNotFeared;
#ifdef FLEE_HP_RATIO
		flee_mode = false;
#endif
		safe_delete(fear_path_state);
		return;
	}
	
	flee_mode = flee;
	
	//fear dosent work without at least maps
	if(zone->zonemap == nullptr) {
		fear_state = fearStateStuck;
		return;	//just stand there
	}
	
	//if we are allready feared, and we are on a fear grid..
	//then just stay happy on the grid...
	if(fear_path_state != nullptr) {
		if(fear_state != fearStateGrid) {
			LogFile->write(EQEMuLog::Debug, "Umm... %s has a fear path state, but is not in a grid state. Wtf?", GetName());
			fear_state = fearStateGrid;
		}
		return;
	}
	
	//try to run straight away from the caster
	VERTEX hit, fear_vector;
	if(FearTryStraight(caster, duration, flee, hit, fear_vector)) {
		return;
	}
		
	//OK, so if we just run, we are going to hit something...
	//now we have to think a little more.
	
	//first, try to find a fear node that we can see.
	if(zone->pathing != nullptr) {
		fear_path_state = new MobFearState();
		if(zone->pathing->FindNearestFear(fear_path_state, GetX(), GetY(), GetZ())) {
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing Start: found path, moving from (%.2f, %.2f, %.2f) to path node  (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), fear_path_state->x, fear_path_state->y, fear_path_state->z);
#endif
			//we found a fear node... were on our way..
			cur_wp_x = fear_path_state->x;
			cur_wp_y = fear_path_state->y;
			cur_wp_z = fear_path_state->z;
			fear_state = fearStateGrid;
			return;
		}
		
		//we have failed to find a path, so we dont need this..
		safe_delete(fear_path_state);
	}
	
	//if we cannot just run, and we cannot see any paths, then
	//we will give one last ditch effort to find a legit path. We 
	//will run as far as we can away from the player, and hope we 
	//can see a path from there if not, we will start breaking rules
	
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing Start: Hope run from (%.2f, %.2f, %.2f), hit at (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), hit.x, hit.y, hit.z);
#endif
	//use the hit point - a little + a little Z as the first waypoint.
	cur_wp_x = hit.x - fear_vector.x * 2;
	cur_wp_y = hit.y - fear_vector.y * 2;
	cur_wp_z = GetZ();
	fear_state = fearStateRunning;
}
//old fear function, kept for ref.
bool Mob::FearTryStraight(Mob *caster, uint32 duration, bool flee, VERTEX &hit, VERTEX &fear_vector) {
	//gotta have somebody to run from
	if(caster == nullptr)
		return(false);
	
	//our goal is to run along this vector...
	fear_vector.x = GetX() - caster->GetX();
	fear_vector.y = GetY() - caster->GetY();
	fear_vector.z = 0;	//I dont see any reason to use Z
	float mag = sqrtf(fear_vector.x*fear_vector.x + fear_vector.y*fear_vector.y);
	fear_vector.x /= mag;
	fear_vector.y /= mag;
	
	//now see if we can just run without hitting anything...
	VERTEX start, end;
	start.x = GetX();
	start.y = GetY();
	start.z = GetZ() + 5.0;	//raise up a little over small bumps
	
	//distance moved per movement tic.
	float distance = NPC_SPEED_MULTIPLIER * GetFearSpeed();
	//times number of movement tics in the spell.
	distance *= float(duration) / float(AImovement_duration);
	
	end.x = start.x + fear_vector.x * distance;
	end.y = start.y + fear_vector.y * distance;
	end.z = start.z;
	
	if(!zone->zonemap->LineIntersectsZone(start, end, 0.5, &hit, nullptr)) {
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing Start: can run entire vector from (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), end.x, end.y, end.z);
#endif
		//no hit, we can run this whole vector.
		cur_wp_x = end.x;
		cur_wp_y = end.y;
		cur_wp_z = GetZ();
		fear_state = fearStateRunningForever;
		return(true);	//were done, nothing difficult needed.
	}
	
	return(false);
}

//old fear function, kept for ref.
void Mob::CalculateFearPosition() {
	if(zone->zonemap == nullptr || fear_state == fearStateStuck) {
		return;	//just stand there
	}
	
	//This is the entire movement section, right here:
	if (cur_wp_x != GetX() && cur_wp_y != GetY()) {
		// not at waypoint yet, so keep moving
		CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetFearSpeed(), true); 
		return;
	}	
	
	
	
	//we have reached our waypoint, now what?
	//figure out a new waypoint to run at...
	
	if(fear_state == fearStateRunningForever) {
		if(flee_mode) {
			//a fleeing mob may run away again
			VERTEX hit, fear_vector;
			if(FearTryStraight(GetHateTop(), FLEE_RUN_DURATION, true, hit, fear_vector))
				return;	//we are running again
			//else, we need to find a grid, so act like we were on a hope run
			fear_state = fearStateRunning;
		}
#ifndef FORCE_FEAR_TO_RUN
		else {
			//we were supposed to run forever, but we did not...
			//should re-fear ourself or something??
			fear_state = fearStateStuck;
			return;
		}
#endif
	}
	
	//first see if we are on a path. if so our life is easy
	if(fear_state == fearStateGrid && fear_path_state) {
		//assume that we have zone->pathing since we got to this state.
		if(!zone->pathing->NextFearPath(fear_path_state)) {
			//this is bad, we were on a path and now its giving us
			//an error... we dont have a good way to deal with this
			fear_state = fearStateStuck;
			return;
		}
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: on path, moving from (%.2f, %.2f, %.2f) to path node  (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), fear_path_state->x, fear_path_state->y, fear_path_state->z);
#endif
		//we found a fear node... were on our way..
		cur_wp_x = fear_path_state->x;
		cur_wp_y = fear_path_state->y;
		cur_wp_z = fear_path_state->z;
		
		CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetFearSpeed(), true);
		return;
	}
	
	//the only valid state left is fearStateRunning, where we try to
	//find a grid once we reach our waypoint, which we have..
	if(fear_state != fearStateRunning) {
		//wtf... unknown state
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: Reached our fear waypoint, but we are in an unknown state %d... stopping.", fear_state);
		fear_state = fearStateStuck;
		return;
	}
	
	//we wanted to try to find a waypoint now, so lets try..
	if(zone->pathing != nullptr) {
		fear_path_state = new MobFearState();
		
		if(zone->pathing->FindNearestFear(fear_path_state, GetX(), GetY(), GetZ())) {
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: ran to find path, moving from (%.2f, %.2f, %.2f) to path node  (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), fear_path_state->x, fear_path_state->y, fear_path_state->z);
#endif
			//we found a fear node... were on our way..
			cur_wp_x = fear_path_state->x;
			cur_wp_y = fear_path_state->y;
			cur_wp_z = fear_path_state->z;
			fear_state = fearStateGrid;
			CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetFearSpeed(), true);
			return;
		}
	
		//if we get here... all valid methods have failed

#ifdef FORCE_FEAR_TO_RUN		
		//ok, now we start making shit up

		//for now, we will limit our bullshitting to ignoring LOS
		//when finding a pathing node, we SHOULD always get something..
		//do not force a path if we are fleeing
		if(!flee_mode && zone->pathing->FindNearestFear(fear_path_state, GetX(), GetY(), GetZ(), false)) {
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: Bullshit Path from (%.2f, %.2f, %.2f) to path node  (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), fear_path_state->x, fear_path_state->y, fear_path_state->z);
#endif
			//we found a fear node... were on our way..
			cur_wp_x = fear_path_state->x;
			cur_wp_y = fear_path_state->y;
			cur_wp_z = fear_path_state->z;
			fear_state = fearStateGrid;
			CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetFearSpeed(), true);
			return;
		}
#endif	//FORCE_FEAR_TO_RUN
		
		//we have failed to find a path once again, so we dont need this..
		safe_delete(fear_path_state);
	}
	
	//if we get HERE... then NOTHING worked... just stick
	fear_state = fearStateStuck;

	//end of function, everything else is #ifdef'd out
//}

//I dont wanna get rid of this right now because it was a lot of hard
//work to write... but it dosent work reliably, so oh well..
#ifdef OLD_FEAR_PATHING	
	/*
		The idea...
		
		try to run along fear vector.
		If we can see along it, run
		otherwise, try to walk up a hill along the same vector
		then try to move along a wall along largest component of FV
		if cant move, change stae to stuck.
		
		once we know a place to run, use the waypoint code to do it
		then if combat ends, we will reach the waypoint and
		
		
	*/
	/*
	//first try our original fear vector again...
	VERTEX start, end, hit, normalhit;
	start.x = GetX() - fear_vector.x * 0.4;
	start.y = GetY() - fear_vector.y * 0.4;
	start.z = GetZ() + 6.0;	//raise up a little over small bumps
	
	end.x = start.x + fear_vector.x * 10;
	end.y = start.y + fear_vector.y * 10;
	end.z = start.z;
	
	if(!zone->zonemap->LineIntersectsZone(start, end, 0.5, &normalhit, nullptr)) {
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: From (%.2f, %.2f, %.2f) normal run to (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), end.x, end.y, end.z);
#endif
		//we can run along this vector without hitting anything...
		cur_wp_x = end.x;
		cur_wp_y = end.y;
		cur_wp_z = end.z - 6.0;
		CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetRunspeed(), true); 
		return;
	}
	//see if we can make ANY useful progress along that vector
	
	//first, adjust normalhit to back up a little bit
	//so we dont run through the wall
	normalhit.x -= 0.4 * fear_vector.x;
	normalhit.y -= 0.4 * fear_vector.y;
	
	float xd = normalhit.x - start.x;
	if(xd < 0)
		xd = 0 - xd;
	float yd = normalhit.y - start.y;
	if(yd < 0)
		yd = 0 - yd;
	
	//this 2 is arbitrary
	if((xd+yd) > 2.0) {
#ifdef FEAR_PATHING_DEBUG
	LogFile->write(EQEMuLog::Debug, "Fear Pathing: From (%.2f, %.2f, %.2f) small run to (%.2f, %.2f, %.2f)",
		GetX(), GetY(), GetZ(), cur_wp_x, cur_wp_y, cur_wp_z);
#endif
		cur_wp_x = normalhit.x;
		cur_wp_y = normalhit.y;
		cur_wp_z = GetZ();
		
		//try and fix up the Z coord if possible
		//not sure if this is worth it, since it prolly isnt up much
		
		NodeRef c = zone->zonemap->SeekNode(zone->zonemap->GetRoot(), end.x, end.y);
		if(c != NODE_NONE) {
			cur_wp_z = zone->zonemap->FindBestZ(c, end, &hit, nullptr);
			if(cur_wp_z < start.z)
				cur_wp_z = end.z;	//revert on error
		}
		
		CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetRunspeed(), true); 
		return;
	}
	
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: From (%.2f, %.2f, %.2f) normal hit at (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), normalhit.x, normalhit.y, normalhit.z);
#endif
	
	//if we get here, we cannot run along our normal vector...
	//try up hill first
	
	/*
	while this uphill stuff works great in outdoor zones,
	it totally breaks dungeons...
	
	float speed = GetRunspeed();
	end.x = start.x + fear_vector.x * speed;
	end.y = start.y + fear_vector.y * speed;
	end.z = start.z + speed + speed;
	
	if(!zone->zonemap->LineIntersectsZone(start, end, 0.5, &hit, nullptr)) {
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: From (%.2f, %.2f, %.2f) up hill run to (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), end.x, end.y, end.z);
#endif
		//we can run along this vector without hitting anything...
		cur_wp_x = end.x - 0.4 * fear_vector.x;
		cur_wp_y = end.y - 0.4 * fear_vector.y;
		cur_wp_z = end.z;
		
		//try and fix up the Z coord if possible
		//not sure if this is worth it, since it prolly isnt up much
		
		NodeRef c = zone->zonemap->SeekNode(zone->zonemap->GetRoot(), end.x, end.y);
		if(c != NODE_NONE) {
			cur_wp_z = zone->zonemap->FindBestZ(c, end, &hit, nullptr);
			if(cur_wp_z < start.z)
				cur_wp_z = end.z;	//revert on error
		}
		
		
		CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetRunspeed(), true); 
		return;
	}
	*/
	/*
	//cant run along our vector at all....
	//one last ditch effort... try to move to the side a little
	//along the minor component of the fear vector.
	//try it in one direction first...
	if(fear_vector.x < fear_vector.y) {
		end.x = start.x + fear_vector.x * 3;
		end.y = start.y;
	} else {
		end.x = start.x;
		end.y = start.y + fear_vector.y * 3;
	}
	end.z = start.z + 3;	//a little lift as always
	
	if(!zone->zonemap->LineIntersectsZone(start, end, 0.5, &hit, nullptr)) {
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: From (%.2f, %.2f, %.2f) strafe 1 to (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), end.x, end.y, end.z);
#endif
		//we can run along this vector without hitting anything...
		cur_wp_x = end.x - 0.4 * fear_vector.x;
		cur_wp_y = end.y - 0.4 * fear_vector.y;
		cur_wp_z = end.z - 3;
		CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetRunspeed(), true); 
		return;
	}
	
	//now the other...
	if(fear_vector.x < fear_vector.y) {
		end.x = start.x + fear_vector.x * 3;
		end.y = start.y;
	} else {
		end.x = start.x;
		end.y = start.y + fear_vector.y * 3;
	}
	end.z = start.z + 3;	//a little lift as always
	
	if(!zone->zonemap->LineIntersectsZone(start, end, 0.5, &hit, nullptr)) {
#ifdef FEAR_PATHING_DEBUG
		LogFile->write(EQEMuLog::Debug, "Fear Pathing: From (%.2f, %.2f, %.2f) strafe 2 to (%.2f, %.2f, %.2f)",
			GetX(), GetY(), GetZ(), end.x, end.y, end.z);
#endif
		//we can run along this vector without hitting anything...
		cur_wp_x = end.x - 0.4 * fear_vector.x;
		cur_wp_y = end.y - 0.4 * fear_vector.y;
		cur_wp_z = end.z - 3;
		CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, GetRunspeed(), true); 
		return;
	}
	
	//if we get here... we have wasted enough CPU cycles
	//just call it quits on fear pathing...
	
	//send them to normalhit and then stop
	cur_wp_x = normalhit.x;
	cur_wp_y = normalhit.y;
	cur_wp_z = GetZ();
	fear_state = fearStateRunningToStick;
#ifdef FEAR_PATHING_DEBUG
	LogFile->write(EQEMuLog::Debug, "Fear Pathing: From (%.2f, %.2f, %.2f) final move to (%.2f, %.2f, %.2f)",
		GetX(), GetY(), GetZ(), normalhit.x, normalhit.y, normalhit.z);
#endif
#endif	//OLD_FEAR_PATHING
}*/














