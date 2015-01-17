/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/debug.h"
#ifdef _EQDEBUG
#include <iostream>
#endif

#include "../common/features.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/misc_functions.h"

#include "map.h"
#include "npc.h"
#include "quest_parser_collection.h"
#include "water_map.h"

#include <math.h>
#include <stdlib.h>

struct wp_distance
{
	float dist;
	int index;
};

static inline float ABS(float x) {
	if(x < 0)
		return(-x);
	return(x);
}

void NPC::AI_SetRoambox(float iDist, float iRoamDist, uint32 iDelay, uint32 iMinDelay) {
	AI_SetRoambox(iDist, GetX()+iRoamDist, GetX()-iRoamDist, GetY()+iRoamDist, GetY()-iRoamDist, iDelay, iMinDelay);
}

void NPC::AI_SetRoambox(float iDist, float iMaxX, float iMinX, float iMaxY, float iMinY, uint32 iDelay, uint32 iMinDelay) {
	roambox_distance = iDist;
	roambox_max_x = iMaxX;
	roambox_min_x = iMinX;
	roambox_max_y = iMaxY;
	roambox_min_y = iMinY;
	roambox_movingto_x = roambox_max_x + 1; // this will trigger a recalc
	roambox_delay = iDelay;
	roambox_min_delay = iMinDelay;
}

void NPC::DisplayWaypointInfo(Client *c) {

	c->Message(0, "Mob is on grid %d, in spawn group %d, on waypoint %d/%d",
		GetGrid(),
		GetSp2(),
		GetCurWp(),
		GetMaxWp() );


	std::vector<wplist>::iterator cur, end;
	cur = Waypoints.begin();
	end = Waypoints.end();
	for(; cur != end; ++cur) {
		c->Message(0,"Waypoint %d: (%.2f,%.2f,%.2f,%.2f) pause %d",
				cur->index,
				cur->x,
				cur->y,
				cur->z,
				cur->heading,
				cur->pause );
	}
}

void NPC::StopWandering()
{	// stops a mob from wandering, takes him off grid and sends him back to spawn point
	roamer=false;
	CastToNPC()->SetGrid(0);
	SendPosition();
	mlog(QUESTS__PATHING, "Stop Wandering requested.");
	return;
}

void NPC::ResumeWandering()
{	// causes wandering to continue - overrides waypoint pause timer and PauseWandering()
	if(!IsNPC())
		return;
	if (GetGrid() != 0)
	{
		if (GetGrid() < 0)
		{	// we were paused by a quest
			AIwalking_timer->Disable();
			SetGrid( 0 - GetGrid());
			if (cur_wp==-1)
			{	// got here by a MoveTo()
				cur_wp=save_wp;
				UpdateWaypoint(cur_wp);	// have him head to last destination from here
			}
			mlog(QUESTS__PATHING, "Resume Wandering requested. Grid %d, wp %d", GetGrid(), cur_wp);
		}
		else if (AIwalking_timer->Enabled())
		{	// we are at a waypoint paused normally
			mlog(QUESTS__PATHING, "Resume Wandering on timed pause. Grid %d, wp %d", GetGrid(), cur_wp);
			AIwalking_timer->Trigger();	// disable timer to end pause now
		}
		else
		{
			LogFile->write(EQEmuLog::Error, "NPC not paused - can't resume wandering: %lu", (unsigned long)GetNPCTypeID());
			return;
		}

		if (m_CurrentWayPoint.m_X == GetX() && m_CurrentWayPoint.m_Y == GetY())
		{	// are we we at a waypoint? if so, trigger event and start to next
			char temp[100];
			itoa(cur_wp,temp,10);	//do this before updating to next waypoint
			CalculateNewWaypoint();
			SetAppearance(eaStanding, false);
			parse->EventNPC(EVENT_WAYPOINT_DEPART, this, nullptr, temp, 0);
		}	// if not currently at a waypoint, we continue on to the one we were headed to before the stop
	}
	else
	{
		LogFile->write(EQEmuLog::Error, "NPC not on grid - can't resume wandering: %lu", (unsigned long)GetNPCTypeID());
	}
	return;
}

void NPC::PauseWandering(int pausetime)
{	// causes wandering to stop but is resumable
	// 0 pausetime means pause until resumed
	// otherwise automatically resume when time is up
	if (GetGrid() != 0)
	{
		DistractedFromGrid = true;
		mlog(QUESTS__PATHING, "Paused Wandering requested. Grid %d. Resuming in %d ms (0=not until told)", GetGrid(), pausetime);
		SendPosition();
		if (pausetime<1)
		{	// negative grid number stops him dead in his tracks until ResumeWandering()
			SetGrid( 0 - GetGrid());
		}
		else
		{	// specified waiting time, he'll resume after that
			AIwalking_timer->Start(pausetime*1000); // set the timer
		}
	} else {
		LogFile->write(EQEmuLog::Error, "NPC not on grid - can't pause wandering: %lu", (unsigned long)GetNPCTypeID());
	}
	return;
}

void NPC::MoveTo(const xyz_heading& position, bool saveguardspot)
{	// makes mob walk to specified location
	if (IsNPC() && GetGrid() != 0)
	{	// he is on a grid
		if (GetGrid() < 0)
		{	// currently stopped by a quest command
			SetGrid( 0 - GetGrid());	// get him moving again
			mlog(AI__WAYPOINTS, "MoveTo during quest wandering. Canceling quest wandering and going back to grid %d when MoveTo is done.", GetGrid());
		}
		AIwalking_timer->Disable();	// disable timer in case he is paused at a wp
		if (cur_wp>=0)
		{	// we've not already done a MoveTo()
			save_wp=cur_wp;	// save the current waypoint
			cur_wp=-1;		// flag this move as quest controlled
		}
		mlog(AI__WAYPOINTS, "MoveTo %s, pausing regular grid wandering. Grid %d, save_wp %d",to_string(static_cast<xyz_location>(position)).c_str(), -GetGrid(), save_wp);
	}
	else
	{	// not on a grid
		roamer=true;
		save_wp=0;
		cur_wp=-2;		// flag as quest controlled w/no grid
		mlog(AI__WAYPOINTS, "MoveTo %s without a grid.", to_string(static_cast<xyz_location>(position)).c_str());
	}
	if (saveguardspot)
	{
        m_GuardPoint = position;

		if(m_GuardPoint.m_Heading == 0)
			m_GuardPoint.m_Heading  = 0.0001;		//hack to make IsGuarding simpler

		if(m_GuardPoint.m_Heading  == -1)
			m_GuardPoint.m_Heading  = this->CalculateHeadingToTarget(position.m_X, position.m_Y);

		mlog(AI__WAYPOINTS, "Setting guard position to %s", to_string(static_cast<xyz_location>(m_GuardPoint)).c_str());
	}

    m_CurrentWayPoint = position;
	cur_wp_pause = 0;
	pLastFightingDelayMoving = 0;
	if(AIwalking_timer->Enabled())
		AIwalking_timer->Start(100);
}

void NPC::UpdateWaypoint(int wp_index)
{
	if(wp_index >= static_cast<int>(Waypoints.size())) {
		mlog(AI__WAYPOINTS, "Update to waypoint %d failed. Not found.", wp_index);
		return;
	}
	std::vector<wplist>::iterator cur;
	cur = Waypoints.begin();
	cur += wp_index;

    m_CurrentWayPoint = xyz_heading(cur->x, cur->y, cur->z, cur->heading);
	cur_wp_pause = cur->pause;
	mlog(AI__WAYPOINTS, "Next waypoint %d: (%.3f, %.3f, %.3f, %.3f)", wp_index, m_CurrentWayPoint.m_X, m_CurrentWayPoint.m_Y, m_CurrentWayPoint.m_Z, m_CurrentWayPoint.m_Heading);

	//fix up pathing Z
	if(zone->HasMap() && RuleB(Map, FixPathingZAtWaypoints))
	{

		if(!RuleB(Watermap, CheckForWaterAtWaypoints) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(m_CurrentWayPoint)))
		{
			Map::Vertex dest(m_CurrentWayPoint.m_X, m_CurrentWayPoint.m_Y, m_CurrentWayPoint.m_Z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr);

			if( (newz > -2000) && ABS(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaWaypoint))
				m_CurrentWayPoint.m_Z = newz + 1;
		}
	}

}

void NPC::CalculateNewWaypoint()
{
	int old_wp = cur_wp;
	bool reached_end = false;
	bool reached_beginning = false;
	if (cur_wp == max_wp)
		reached_end = true;
	if (cur_wp == 0)
		reached_beginning = true;

	switch(wandertype)
	{
	case 0: //circle
	{
		if (reached_end)
			cur_wp = 0;
		else
			cur_wp = cur_wp + 1;
		break;
	}
	case 1: //10 closest
	{
		std::list<wplist> closest;
		GetClosestWaypoint(closest, 10, GetPosition());
		std::list<wplist>::iterator iter = closest.begin();
		if(closest.size() != 0)
		{
			iter = closest.begin();
			std::advance(iter, zone->random.Int(0, closest.size() - 1));
			cur_wp = (*iter).index;
		}

		break;
	}
	case 2: //random
	{
		cur_wp = zone->random.Int(0, Waypoints.size() - 1);
		if(cur_wp == old_wp)
		{
			if(cur_wp == (Waypoints.size() - 1))
			{
				if(cur_wp > 0)
				{
					cur_wp--;
				}
			}
			else if(cur_wp == 0)
			{
				if((Waypoints.size() - 1) > 0)
				{
					cur_wp++;
				}
			}
		}

		break;
	}
	case 3: //patrol
	{
		if(reached_end)
			patrol = 1;
		else if(reached_beginning)
			patrol = 0;
		if(patrol == 1)
			cur_wp = cur_wp - 1;
		else
			cur_wp = cur_wp + 1;

		break;
	}
	case 4: //goto the end and depop with spawn timer
	case 6: //goto the end and depop without spawn timer
	{
		cur_wp = cur_wp + 1;
		break;
	}
	case 5: //pick random closest 5 and pick one that's in sight
	{
		std::list<wplist> closest;
		GetClosestWaypoint(closest, 5, GetPosition());

		std::list<wplist>::iterator iter = closest.begin();
		while(iter != closest.end())
		{
			if(CheckLosFN((*iter).x, (*iter).y, (*iter).z, GetSize()))
			{
				++iter;
			}
			else
			{
				iter = closest.erase(iter);
			}
		}

		if(closest.size() != 0)
		{
			iter = closest.begin();
			std::advance(iter, zone->random.Int(0, closest.size() - 1));
			cur_wp = (*iter).index;
		}
		break;
	}
	}

	tar_ndx = 52;

	// Preserve waypoint setting for quest controlled NPCs
	if (cur_wp < 0)
		cur_wp = old_wp;

	// Check to see if we need to update the waypoint.
	if (cur_wp != old_wp)
		UpdateWaypoint(cur_wp);
}

bool wp_distance_pred(const wp_distance& left, const wp_distance& right)
{
	return left.dist < right.dist;
}

void NPC::GetClosestWaypoint(std::list<wplist> &wp_list, int count, const xyz_location& location)
{
	wp_list.clear();
	if(Waypoints.size() <= count)
	{
		for(int i = 0; i < Waypoints.size(); ++i)
		{
			wp_list.push_back(Waypoints[i]);
		}
		return;
	}

	std::list<wp_distance> distances;
	for(int i = 0; i < Waypoints.size(); ++i)
	{
		float cur_x = (Waypoints[i].x - location.m_X);
		cur_x *= cur_x;
		float cur_y = (Waypoints[i].y - location.m_Y);
		cur_y *= cur_y;
		float cur_z = (Waypoints[i].z - location.m_Z);
		cur_z *= cur_z;
		float cur_dist = cur_x + cur_y + cur_z;
		wp_distance w_dist;
		w_dist.dist = cur_dist;
		w_dist.index = i;
		distances.push_back(w_dist);
	}
	distances.sort([](const wp_distance& a, const wp_distance& b) {
		return a.dist < b.dist;
	});

	std::list<wp_distance>::iterator iter = distances.begin();
	for(int i = 0; i < count; ++i)
	{
		wp_list.push_back(Waypoints[(*iter).index]);
		++iter;
	}
}

void NPC::SetWaypointPause()
{
	//Declare time to wait on current WP

	if (cur_wp_pause == 0) {
		AIwalking_timer->Start(100);
	}
	else
	{

		switch (pausetype)
		{
			case 0: //Random Half
				AIwalking_timer->Start((cur_wp_pause - zone->random.Int(0, cur_wp_pause-1)/2)*1000);
				break;
			case 1: //Full
				AIwalking_timer->Start(cur_wp_pause*1000);
				break;
			case 2: //Random Full
				AIwalking_timer->Start(zone->random.Int(0, cur_wp_pause-1)*1000);
				break;
		}
	}
}

void NPC::SaveGuardSpot(bool iClearGuardSpot) {
	if (iClearGuardSpot) {
		mlog(AI__WAYPOINTS, "Clearing guard order.");
		m_GuardPoint = xyz_heading(0, 0, 0, 0);
	}
	else {
        m_GuardPoint = m_Position;

		if(m_GuardPoint.m_Heading == 0)
			m_GuardPoint.m_Heading = 0.0001;		//hack to make IsGuarding simpler
		mlog(AI__WAYPOINTS, "Setting guard position to %s", to_string(static_cast<xyz_location>(m_GuardPoint)).c_str());
	}
}

void NPC::NextGuardPosition() {
	if (!CalculateNewPosition2(m_GuardPoint.m_X, m_GuardPoint.m_Y, m_GuardPoint.m_Z, GetMovespeed())) {
		SetHeading(m_GuardPoint.m_Heading);
		mlog(AI__WAYPOINTS, "Unable to move to next guard position. Probably rooted.");
	}
	else if((m_Position.m_X == m_GuardPoint.m_X) && (m_Position.m_Y == m_GuardPoint.m_Y) && (m_Position.m_Z == m_GuardPoint.m_Z))
	{
		if(moved)
		{
			moved=false;
			SetMoving(false);
			SendPosition();
		}
	}
}

/*
// we need this for charmed NPCs
void Mob::SaveSpawnSpot() {
	spawn_x = x_pos;
	spawn_y = y_pos;
	spawn_z = z_pos;
	spawn_heading = heading;
}*/




/*float Mob::CalculateDistanceToNextWaypoint() {
	return CalculateDistance(cur_wp_x, cur_wp_y, cur_wp_z);
}*/

float Mob::CalculateDistance(float x, float y, float z) {
	return (float)sqrtf( ((m_Position.m_X-x)*(m_Position.m_X-x)) + ((m_Position.m_Y-y)*(m_Position.m_Y-y)) + ((m_Position.m_Z-z)*(m_Position.m_Z-z)) );
}

/*
uint8 NPC::CalculateHeadingToNextWaypoint() {
	return CalculateHeadingToTarget(cur_wp_x, cur_wp_y);
}
*/
float Mob::CalculateHeadingToTarget(float in_x, float in_y) {
	float angle;

	if (in_x-m_Position.m_X > 0)
		angle = - 90 + atan((float)(in_y-m_Position.m_Y) / (float)(in_x-m_Position.m_X)) * 180 / M_PI;
	else if (in_x-m_Position.m_X < 0)
		angle = + 90 + atan((float)(in_y-m_Position.m_Y) / (float)(in_x-m_Position.m_X)) * 180 / M_PI;
	else // Added?
	{
		if (in_y-m_Position.m_Y > 0)
			angle = 0;
		else
			angle = 180;
	}
	if (angle < 0)
		angle += 360;
	if (angle > 360)
		angle -= 360;
	return (256*(360-angle)/360.0f);
}

bool Mob::MakeNewPositionAndSendUpdate(float x, float y, float z, float speed, bool checkZ) {
	if(GetID()==0)
		return true;

	if ((m_Position.m_X-x == 0) && (m_Position.m_Y-y == 0)) {//spawn is at target coords
		if(m_Position.m_Z-z != 0) {
			m_Position.m_Z = z;
			mlog(AI__WAYPOINTS, "Calc Position2 (%.3f, %.3f, %.3f): Jumping pure Z.", x, y, z);
			return true;
		}
		mlog(AI__WAYPOINTS, "Calc Position2 (%.3f, %.3f, %.3f) inWater=%d: We are there.", x, y, z, inWater);
		return false;
	}
	else if ((ABS(m_Position.m_X - x) < 0.1) && (ABS(m_Position.m_Y - y) < 0.1))
	{
		mlog(AI__WAYPOINTS, "Calc Position2 (%.3f, %.3f, %.3f): X/Y difference <0.1, Jumping to target.", x, y, z);

		if(IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), x, y, z);
		}

		m_Position.m_X = x;
		m_Position.m_Y = y;
		m_Position.m_Z = z;
		return true;
	}

	int compare_steps = IsBoat() ? 1 : 20;
	if(tar_ndx < compare_steps && m_TargetLocation.m_X==x && m_TargetLocation.m_Y==y) {

		float new_x = m_Position.m_X + m_TargetV.m_X*tar_vector;
		float new_y = m_Position.m_Y + m_TargetV.m_Y*tar_vector;
		float new_z = m_Position.m_Z + m_TargetV.m_Z*tar_vector;
		if(IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
		}

		m_Position.m_X = new_x;
		m_Position.m_Y = new_y;
		m_Position.m_Z = new_z;

		mlog(AI__WAYPOINTS, "Calculating new position2 to (%.3f, %.3f, %.3f), old vector (%.3f, %.3f, %.3f)", x, y, z, m_TargetV.m_X, m_TargetV.m_Y, m_TargetV.m_Z);

		uint8 NPCFlyMode = 0;

		if(IsNPC()) {
			if(CastToNPC()->GetFlyMode() == 1 || CastToNPC()->GetFlyMode() == 2)
				NPCFlyMode = 1;
		}

		//fix up pathing Z
		if(!NPCFlyMode && checkZ && zone->HasMap() && RuleB(Map, FixPathingZWhenMoving))
		{
			if(!RuleB(Watermap, CheckForWaterWhenMoving) || !zone->HasWaterMap() ||
				(zone->HasWaterMap() && !zone->watermap->InWater(m_Position.m_X)))
			{
				Map::Vertex dest(m_Position.m_X, m_Position.m_Y, m_Position.m_Z);

				float newz = zone->zonemap->FindBestZ(dest, nullptr) + 2.0f;

				mlog(AI__WAYPOINTS, "BestZ returned %4.3f at %4.3f, %4.3f, %4.3f", newz,m_Position.m_X,m_Position.m_Y,m_Position.m_Z);

				if( (newz > -2000) && ABS(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaMoving)) // Sanity check.
				{
					if((ABS(x - m_Position.m_X) < 0.5) && (ABS(y - m_Position.m_Y) < 0.5))
					{
						if(ABS(z-m_Position.m_Z) <= RuleR(Map, FixPathingZMaxDeltaMoving))
							m_Position.m_Z = z;
						else
							m_Position.m_Z = newz + 1;
					}
					else
						m_Position.m_Z = newz + 1;
				}
			}
		}

		tar_ndx++;
		return true;
	}


	if (tar_ndx>50) {
		tar_ndx--;
	} else {
		tar_ndx=0;
	}
	m_TargetLocation = xyz_location(x, y, z);

	float nx = this->m_Position.m_X;
	float ny = this->m_Position.m_Y;
	float nz = this->m_Position.m_Z;
//	float nh = this->heading;

	m_TargetV.m_X = x - nx;
	m_TargetV.m_Y = y - ny;
	m_TargetV.m_Z = z - nz;

	//pRunAnimSpeed = (int8)(speed*NPC_RUNANIM_RATIO);
	//speed *= NPC_SPEED_MULTIPLIER;

	mlog(AI__WAYPOINTS, "Calculating new position2 to (%.3f, %.3f, %.3f), new vector (%.3f, %.3f, %.3f) rate %.3f, RAS %d", x, y, z, m_TargetV.m_X, m_TargetV.m_Y, m_TargetV.m_Z, speed, pRunAnimSpeed);

	// --------------------------------------------------------------------------
	// 2: get unit vector
	// --------------------------------------------------------------------------
	float mag = sqrtf (m_TargetV.m_X*m_TargetV.m_X + m_TargetV.m_Y*m_TargetV.m_Y + m_TargetV.m_Z*m_TargetV.m_Z);
	tar_vector = speed / mag;

// mob move fix
	int numsteps = (int) ( mag * 20 / speed) + 1;


// mob move fix

	if (numsteps<20)
	{
		if (numsteps>1)
		{
			tar_vector=1.0f	;
			m_TargetV.m_X = m_TargetV.m_X/numsteps;
			m_TargetV.m_Y = m_TargetV.m_Y/numsteps;
			m_TargetV.m_Z = m_TargetV.m_Z/numsteps;

			float new_x = m_Position.m_X + m_TargetV.m_X;
			float new_y = m_Position.m_Y + m_TargetV.m_Y;
			float new_z = m_Position.m_Z + m_TargetV.m_Z;
			if(IsNPC()) {
				entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
			}

			m_Position.m_X = new_x;
			m_Position.m_Y = new_y;
			m_Position.m_Z = new_z;
			m_Position.m_Heading = CalculateHeadingToTarget(x, y);
			tar_ndx=22-numsteps;
			mlog(AI__WAYPOINTS, "Next position2 (%.3f, %.3f, %.3f) (%d steps)", m_Position.m_X, m_Position.m_Y, m_Position.m_Z, numsteps);
		}
		else
		{
			if(IsNPC()) {
				entity_list.ProcessMove(CastToNPC(), x, y, z);
			}

			m_Position.m_X = x;
			m_Position.m_Y = y;
			m_Position.m_Z = z;

			mlog(AI__WAYPOINTS, "Only a single step to get there... jumping.");

		}
	}

	else {
		tar_vector/=20;

		float new_x = m_Position.m_X + m_TargetV.m_X*tar_vector;
		float new_y = m_Position.m_Y + m_TargetV.m_Y*tar_vector;
		float new_z = m_Position.m_Z + m_TargetV.m_Z*tar_vector;
		if(IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
		}

		m_Position.m_X = new_x;
		m_Position.m_Y = new_y;
		m_Position.m_Z = new_z;
		m_Position.m_Heading = CalculateHeadingToTarget(x, y);
		mlog(AI__WAYPOINTS, "Next position2 (%.3f, %.3f, %.3f) (%d steps)", m_Position.m_X, m_Position.m_Y, m_Position.m_Z, numsteps);
	}

	uint8 NPCFlyMode = 0;

	if(IsNPC()) {
		if(CastToNPC()->GetFlyMode() == 1 || CastToNPC()->GetFlyMode() == 2)
			NPCFlyMode = 1;
	}

	//fix up pathing Z
	if(!NPCFlyMode && checkZ && zone->HasMap() && RuleB(Map, FixPathingZWhenMoving)) {

		if(!RuleB(Watermap, CheckForWaterWhenMoving) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(m_Position)))
		{
			Map::Vertex dest(m_Position.m_X, m_Position.m_Y, m_Position.m_Z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr);

			mlog(AI__WAYPOINTS, "BestZ returned %4.3f at %4.3f, %4.3f, %4.3f", newz,m_Position.m_X,m_Position.m_Y,m_Position.m_Z);

			if( (newz > -2000) && ABS(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaMoving)) // Sanity check.
			{
				if(ABS(x - m_Position.m_X) < 0.5 && ABS(y - m_Position.m_Y) < 0.5)
				{
					if(ABS(z - m_Position.m_Z) <= RuleR(Map, FixPathingZMaxDeltaMoving))
						m_Position.m_Z = z;
					else
						m_Position.m_Z = newz + 1;
				}
				else
					m_Position.m_Z = newz+1;
				}
		}
	}

	SetMoving(true);
	moved=true;

    m_Delta = xyz_heading(m_Position.m_X - nx, m_Position.m_Y - ny, m_Position.m_Z - nz, 0.0f);

	if (IsClient())
		SendPosUpdate(1);
	else
		SendPosUpdate();

	SetAppearance(eaStanding, false);
	pLastChange = Timer::GetCurrentTime();
	return true;
}

bool Mob::CalculateNewPosition2(float x, float y, float z, float speed, bool checkZ) {
	if(IsNPC() || IsClient() || IsPet()) {
		pRunAnimSpeed = (int8)(speed*NPC_RUNANIM_RATIO);
		speed *= NPC_SPEED_MULTIPLIER;
	}

	return MakeNewPositionAndSendUpdate(x, y, z, speed, checkZ);
}

bool Mob::CalculateNewPosition(float x, float y, float z, float speed, bool checkZ) {
	if(GetID()==0)
		return true;

	float nx = m_Position.m_X;
	float ny = m_Position.m_Y;
	float nz = m_Position.m_Z;

	// if NPC is rooted
	if (speed == 0.0) {
		SetHeading(CalculateHeadingToTarget(x, y));
		if(moved){
			SendPosition();
			SetMoving(false);
			moved=false;
		}
		SetRunAnimSpeed(0);
		mlog(AI__WAYPOINTS, "Rooted while calculating new position to (%.3f, %.3f, %.3f)", x, y, z);
		return true;
	}

	float old_test_vector=test_vector;
	m_TargetV.m_X = x - nx;
	m_TargetV.m_Y = y - ny;
	m_TargetV.m_Z = z - nz;

	if (m_TargetV.m_X == 0 && m_TargetV.m_Y == 0)
		return false;
	pRunAnimSpeed = (uint8)(speed*NPC_RUNANIM_RATIO);
	speed *= NPC_SPEED_MULTIPLIER;

	mlog(AI__WAYPOINTS, "Calculating new position to (%.3f, %.3f, %.3f) vector (%.3f, %.3f, %.3f) rate %.3f RAS %d", x, y, z, m_TargetV.m_X, m_TargetV.m_Y, m_TargetV.m_Z, speed, pRunAnimSpeed);

	// --------------------------------------------------------------------------
	// 2: get unit vector
	// --------------------------------------------------------------------------
	test_vector=sqrtf (x*x + y*y + z*z);
	tar_vector = speed / sqrtf (m_TargetV.m_X*m_TargetV.m_X + m_TargetV.m_Y*m_TargetV.m_Y + m_TargetV.m_Z*m_TargetV.m_Z);
	m_Position.m_Heading = CalculateHeadingToTarget(x, y);

	if (tar_vector >= 1.0) {
		if(IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), x, y, z);
		}

		m_Position.m_X = x;
		m_Position.m_Y = y;
		m_Position.m_Z = z;
		mlog(AI__WAYPOINTS, "Close enough, jumping to waypoint");
	}
	else {
		float new_x = m_Position.m_X + m_TargetV.m_X*tar_vector;
		float new_y = m_Position.m_Y + m_TargetV.m_Y*tar_vector;
		float new_z = m_Position.m_Z + m_TargetV.m_Z*tar_vector;
		if(IsNPC()) {
			entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
		}

		m_Position.m_X = new_x;
		m_Position.m_Y = new_y;
		m_Position.m_Z = new_z;
		mlog(AI__WAYPOINTS, "Next position (%.3f, %.3f, %.3f)", m_Position.m_X, m_Position.m_Y, m_Position.m_Z);
	}

	uint8 NPCFlyMode = 0;

	if(IsNPC()) {
		if(CastToNPC()->GetFlyMode() == 1 || CastToNPC()->GetFlyMode() == 2)
			NPCFlyMode = 1;
	}

	//fix up pathing Z
	if(!NPCFlyMode && checkZ && zone->HasMap() && RuleB(Map, FixPathingZWhenMoving))
	{
		if(!RuleB(Watermap, CheckForWaterWhenMoving) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(m_Position)))
		{
			Map::Vertex dest(m_Position.m_X, m_Position.m_Y, m_Position.m_Z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr) + 2.0f;

			mlog(AI__WAYPOINTS, "BestZ returned %4.3f at %4.3f, %4.3f, %4.3f", newz,m_Position.m_X,m_Position.m_Y,m_Position.m_Z);

			if( (newz > -2000) && ABS(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaMoving)) // Sanity check.
			{
				if(ABS(x - m_Position.m_X) < 0.5 && ABS(y - m_Position.m_Y) < 0.5)
				{
					if(ABS(z - m_Position.m_Z) <= RuleR(Map, FixPathingZMaxDeltaMoving))
						m_Position.m_Z = z;
					else
						m_Position.m_Z = newz + 1;
				}
				else
					m_Position.m_Z = newz+1;
			}
		}
	}

	//OP_MobUpdate
	if((old_test_vector!=test_vector) || tar_ndx>20){ //send update
		tar_ndx=0;
		this->SetMoving(true);
		moved=true;
		m_Delta = xyz_heading(m_Position.m_X - nx, m_Position.m_Y - ny, m_Position.m_Z - nz, 0.0f);
		SendPosUpdate();
	}
	tar_ndx++;

	// now get new heading
	SetAppearance(eaStanding, false); // make sure they're standing
	pLastChange = Timer::GetCurrentTime();
	return true;
}

void NPC::AssignWaypoints(int32 grid) {
	if(grid == 0)
		return;		//grid ID 0 not supported

	if(grid < 0) {
		// Allow setting negative grid values for pausing pathing
		this->CastToNPC()->SetGrid(grid);
		return;
	}

	Waypoints.clear();
    roamer = false;

	// Retrieve the wander and pause types for this grid
	std::string query = StringFormat("SELECT `type`, `type2` FROM `grid` WHERE `id` = %i AND `zoneid` = %i", grid, zone->GetZoneID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
        LogFile->write(EQEmuLog::Error, "MySQL Error while trying to assign grid %u to mob %s: %s", grid, name, results.ErrorMessage().c_str());
        return;
	}

	if (results.RowCount() == 0)
        return;

	auto row = results.begin();

    wandertype = atoi(row[0]);
    pausetype = atoi(row[1]);


    this->CastToNPC()->SetGrid(grid);	// Assign grid number

    // Retrieve all waypoints for this grid
    query = StringFormat("SELECT `x`,`y`,`z`,`pause`,`heading` "
                        "FROM grid_entries WHERE `gridid` = %i AND `zoneid` = %i "
                        "ORDER BY `number`", grid, zone->GetZoneID());
    results = database.QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEmuLog::Error, "MySQL Error while trying to assign waypoints from grid %u to mob %s: %s", grid, name, results.ErrorMessage().c_str());
        return;
    }

    roamer = true;
    max_wp = 0;	// Initialize it; will increment it for each waypoint successfully added to the list

    for (auto row = results.begin(); row != results.end(); ++row, ++max_wp)
    {
        wplist newwp;
        newwp.index = max_wp;
        newwp.x = atof(row[0]);
        newwp.y = atof(row[1]);
        newwp.z = atof(row[2]);

        if(zone->HasMap() && RuleB(Map, FixPathingZWhenLoading) )
        {
            auto positon = xyz_location(newwp.x,newwp.y,newwp.z);
            if(!RuleB(Watermap, CheckWaypointsInWaterWhenLoading) || !zone->HasWaterMap() ||
                (zone->HasWaterMap() && !zone->watermap->InWater(positon)))
            {
                Map::Vertex dest(newwp.x, newwp.y, newwp.z);

                float newz = zone->zonemap->FindBestZ(dest, nullptr);

                if( (newz > -2000) && ABS(newz-dest.z) < RuleR(Map, FixPathingZMaxDeltaLoading))
                    newwp.z = newz + 1;
            }
        }

        newwp.pause = atoi(row[3]);
        newwp.heading = atof(row[4]);
        Waypoints.push_back(newwp);
    }

	if(Waypoints.size() < 2) {
		roamer = false;
	}

    UpdateWaypoint(0);
    SetWaypointPause();

    if (wandertype == 1 || wandertype == 2 || wandertype == 5)
        CalculateNewWaypoint();

}

void Mob::SendTo(float new_x, float new_y, float new_z) {
	if(IsNPC()) {
		entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
	}

	m_Position.m_X = new_x;
	m_Position.m_Y = new_y;
	m_Position.m_Z = new_z;
	mlog(AI__WAYPOINTS, "Sent To (%.3f, %.3f, %.3f)", new_x, new_y, new_z);

	if(flymode == FlyMode1)
		return;

	//fix up pathing Z, this shouldent be needed IF our waypoints
	//are corrected instead
	if(zone->HasMap() && RuleB(Map, FixPathingZOnSendTo) )
	{
		if(!RuleB(Watermap, CheckForWaterOnSendTo) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(m_Position)))
		{
			Map::Vertex dest(m_Position.m_X, m_Position.m_Y, m_Position.m_Z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr);

			mlog(AI__WAYPOINTS, "BestZ returned %4.3f at %4.3f, %4.3f, %4.3f", newz,m_Position.m_X,m_Position.m_Y,m_Position.m_Z);

			if( (newz > -2000) && ABS(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaSendTo)) // Sanity check.
				m_Position.m_Z = newz + 1;
		}
	}
	else
		m_Position.m_Z += 0.1;
}

void Mob::SendToFixZ(float new_x, float new_y, float new_z) {
	if(IsNPC()) {
		entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z + 0.1);
	}

	m_Position.m_X = new_x;
	m_Position.m_Y = new_y;
	m_Position.m_Z = new_z + 0.1;

	//fix up pathing Z, this shouldent be needed IF our waypoints
	//are corrected instead

	if(zone->HasMap() && RuleB(Map, FixPathingZOnSendTo))
	{
		if(!RuleB(Watermap, CheckForWaterOnSendTo) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(m_Position)))
		{
			Map::Vertex dest(m_Position.m_X, m_Position.m_Y, m_Position.m_Z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr);

			mlog(AI__WAYPOINTS, "BestZ returned %4.3f at %4.3f, %4.3f, %4.3f", newz,m_Position.m_X,m_Position.m_Y,m_Position.m_Z);

			if( (newz > -2000) && ABS(newz-dest.z) < RuleR(Map, FixPathingZMaxDeltaSendTo)) // Sanity check.
				m_Position.m_Z = newz + 1;
		}
	}
}

int	ZoneDatabase::GetHighestGrid(uint32 zoneid) {

	std::string query = StringFormat("SELECT COALESCE(MAX(id), 0) FROM grid WHERE zoneid = %i", zoneid);
	auto results = QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEmuLog::Error, "Error in GetHighestGrid query '%s': %s", query.c_str(), results.ErrorMessage().c_str());
        return 0;
    }

	if (results.RowCount() != 1)
        return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint8 ZoneDatabase::GetGridType2(uint32 grid, uint16 zoneid) {

	int type2 = 0;
	std::string query = StringFormat("SELECT type2 FROM grid WHERE id = %i AND zoneid = %i", grid, zoneid);
	auto results = QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEmuLog::Error, "Error in GetGridType2 query '%s': %s", query.c_str(), results.ErrorMessage().c_str());
        return 0;
    }

    if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();

	return atoi(row[0]);
}

bool ZoneDatabase::GetWaypoints(uint32 grid, uint16 zoneid, uint32 num, wplist* wp) {

    if (wp == nullptr)
        return false;

	std::string query = StringFormat("SELECT x, y, z, pause, heading FROM grid_entries "
                                    "WHERE gridid = %i AND number = %i AND zoneid = %i", grid, num, zoneid);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEmuLog::Error, "Error in GetWaypoints query '%s': %s", query.c_str(), results.ErrorMessage().c_str());
        return false;
    }

	if (results.RowCount() != 1)
        return false;

    auto row = results.begin();

    wp->x = atof(row[0]);
    wp->y = atof(row[1]);
    wp->z = atof(row[2]);
    wp->pause = atoi(row[3]);
    wp->heading = atof(row[4]);

    return true;
}

void ZoneDatabase::AssignGrid(Client *client, const xy_location& location, uint32 grid)
{
	int matches = 0, fuzzy = 0, spawn2id = 0;

	// looks like most of the stuff in spawn2 is straight integers
	// so let's try that first
	std::string query = StringFormat("SELECT id, x, y FROM spawn2 WHERE zone = '%s' AND x = %i AND y = %i",
                                    zone->GetShortName(), (int)location.m_X, (int)location.m_Y);
    auto results = QueryDatabase(query);
	if(!results.Success()) {
		LogFile->write(EQEmuLog::Error, "Error querying spawn2 '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
		return;
	}

// how much it's allowed to be off by
#define _GASSIGN_TOLERANCE	1.0
	if (results.RowCount() == 0)	// try a fuzzy match if that didn't find it
	{
        query = StringFormat("SELECT id,x,y FROM spawn2 WHERE zone='%s' AND "
                            "ABS( ABS(x) - ABS(%f) ) < %f AND "
                            "ABS( ABS(y) - ABS(%f) ) < %f",
                            zone->GetShortName(), location.m_X, _GASSIGN_TOLERANCE, location.m_Y, _GASSIGN_TOLERANCE);
        results = QueryDatabase(query);
		if (!results.Success()) {
			LogFile->write(EQEmuLog::Error, "Error querying fuzzy spawn2 '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
			return;
		}

		fuzzy = 1;
		matches = results.RowCount();
	}

    if (matches == 0)
	{
        client->Message(0, "ERROR: Unable to assign grid - can't find it in spawn2");
        return;
    }

    if(matches > 1)
	{
		client->Message(0, "ERROR: Unable to assign grid - multiple spawn2 rows match");
		return;
	}

    auto row = results.begin();

    spawn2id = atoi(row[0]);
    xy_location dbLocation = xy_location(atof(row[1]), atof(row[2]));

	query = StringFormat("UPDATE spawn2 SET pathgrid = %d WHERE id = %d", grid, spawn2id);
	results = QueryDatabase(query);
	if (!results.Success())
	{
		LogFile->write(EQEmuLog::Error, "Error updating spawn2 '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
		return;
    }

    if (results.RowsAffected() != 1)
	{
        client->Message(0, "ERROR: found spawn2 id %d but the update query failed", spawn2id);
        return;
    }

    if(client)
        client->LogSQL(query.c_str());

	if (!fuzzy)
	{
        client->Message(0, "Grid assign: spawn2 id = %d updated - exact match", spawn2id);
        return;
    }

    float difference = sqrtf(pow(fabs(location.m_X - dbLocation.m_X) , 2) + pow(fabs(location.m_Y - dbLocation.m_Y), 2));
    client->Message(0, "Grid assign: spawn2 id = %d updated - fuzzy match: deviation %f", spawn2id, difference);
}

/******************
* ModifyGrid - Either adds an empty grid, or removes a grid and all its waypoints, for a particular zone.
*	remove:		TRUE if we are deleting the specified grid, FALSE if we are adding it
*	id:		The ID# of the grid to add or delete
*	type,type2:	The type and type2 values for the grid being created (ignored if grid is being deleted)
*	zoneid:		The ID number of the zone the grid is being created/deleted in
*/
void ZoneDatabase::ModifyGrid(Client *client, bool remove, uint32 id, uint8 type, uint8 type2, uint16 zoneid) {

	if (!remove)
	{
        std::string query = StringFormat("INSERT INTO grid(id, zoneid, type, type2) "
                                            "VALUES (%i, %i, %i, %i)", id, zoneid, type, type2);
        auto results = QueryDatabase(query);
        if (!results.Success()) {
            LogFile->write(EQEmuLog::Error, "Error creating grid entry '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
            return;
        }

        if(client)
			client->LogSQL(query.c_str());

		return;
	}

    std::string query = StringFormat("DELETE FROM grid where id=%i", id);
    auto results = QueryDatabase(query);
    if (!results.Success())
		LogFile->write(EQEmuLog::Error, "Error deleting grid '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
	else if(client)
        client->LogSQL(query.c_str());

    query = StringFormat("DELETE FROM grid_entries WHERE zoneid = %i AND gridid = %i", zoneid, id);
    results = QueryDatabase(query);
    if(!results.Success())
        LogFile->write(EQEmuLog::Error, "Error deleting grid entries '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
    else if(client)
        client->LogSQL(query.c_str());

}

/**************************************
* AddWP - Adds a new waypoint to a specific grid for a specific zone.
*/
void ZoneDatabase::AddWP(Client *client, uint32 gridid, uint32 wpnum, const xyz_heading& position, uint32 pause, uint16 zoneid)
{
	std::string query = StringFormat("INSERT INTO grid_entries (gridid, zoneid, `number`, x, y, z, pause, heading) "
                                    "VALUES (%i, %i, %i, %f, %f, %f, %i, %f)",
                                    gridid, zoneid, wpnum, position.m_X, position.m_Y, position.m_Z, pause, position.m_Heading);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		LogFile->write(EQEmuLog::Error, "Error adding waypoint '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
		return;
	}

    if(client)
        client->LogSQL(query.c_str());
}


/**********
* ModifyWP() has been obsoleted. The #wp command either uses AddWP() or DeleteWaypoint()
***********/

/******************
* DeleteWaypoint - Removes a specific waypoint from the grid
*	grid_id:	The ID number of the grid whose wp is being deleted
*	wp_num:		The number of the waypoint being deleted
*	zoneid:		The ID number of the zone that contains the waypoint being deleted
*/
void ZoneDatabase::DeleteWaypoint(Client *client, uint32 grid_num, uint32 wp_num, uint16 zoneid)
{
	std::string query = StringFormat("DELETE FROM grid_entries WHERE "
                                    "gridid = %i AND zoneid = %i AND `number` = %i",
                                    grid_num, zoneid, wp_num);
    auto results = QueryDatabase(query);
	if(!results.Success()) {
        LogFile->write(EQEmuLog::Error, "Error deleting waypoint '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
		return;
	}

	if(client)
        client->LogSQL(query.c_str());
}


/******************
* AddWPForSpawn - Used by the #wpadd command - for a given spawn, this will add a new waypoint to whatever grid that spawn is assigned to.
* If there is currently no grid assigned to the spawn, a new grid will be created using the next available Grid ID number for the zone
* the spawn is in.
* Returns 0 if the function didn't have to create a new grid. If the function had to create a new grid for the spawn, then the ID of
* the created grid is returned.
*/
uint32 ZoneDatabase::AddWPForSpawn(Client *client, uint32 spawn2id, const xyz_heading& position, uint32 pause, int type1, int type2, uint16 zoneid) {

	uint32 grid_num;	 // The grid number the spawn is assigned to (if spawn has no grid, will be the grid number we end up creating)
	uint32 next_wp_num;	 // The waypoint number we should be assigning to the new waypoint
	bool createdNewGrid; // Did we create a new grid in this function?

	// See what grid number our spawn is assigned
	std::string query = StringFormat("SELECT pathgrid FROM spawn2 WHERE id = %i", spawn2id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		// Query error
		LogFile->write(EQEmuLog::Error, "Error setting pathgrid '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
		return 0;
	}

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	grid_num = atoi(row[0]);

	if (grid_num == 0)
	{ // Our spawn doesn't have a grid assigned to it -- we need to create a new grid and assign it to the spawn
		createdNewGrid = true;
		grid_num = GetFreeGrid(zoneid);
		if(grid_num == 0)	// There are no grids for the current zone -- create Grid #1
			grid_num = 1;

		query = StringFormat("INSERT INTO grid SET id = '%i', zoneid = %i, type ='%i', type2 = '%i'",
							grid_num, zoneid, type1, type2);
		results = QueryDatabase(query);
		if(!results.Success())
			LogFile->write(EQEmuLog::Error, "Error adding grid '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
		else if(client)
			client->LogSQL(query.c_str());

		query = StringFormat("UPDATE spawn2 SET pathgrid = '%i' WHERE id = '%i'", grid_num, spawn2id);
		results = QueryDatabase(query);
		if(!results.Success())
			LogFile->write(EQEmuLog::Error, "Error updating spawn2 pathing '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
		else if(client)
			client->LogSQL(query.c_str());
	}
	else	// NPC had a grid assigned to it
		createdNewGrid = false;

	// Find out what the next waypoint is for this grid
	query = StringFormat("SELECT max(`number`) FROM grid_entries WHERE zoneid = '%i' AND gridid = '%i'", zoneid, grid_num);

	results = QueryDatabase(query);
	if(!results.Success()) { // Query error
		LogFile->write(EQEmuLog::Error, "Error getting next waypoint id '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
		return 0;
	}

	row = results.begin();
	if(row[0] != 0)
		next_wp_num = atoi(row[0]) + 1;
	else	// No waypoints in this grid yet
		next_wp_num = 1;

	query = StringFormat("INSERT INTO grid_entries(gridid, zoneid, `number`, x, y, z, pause, heading) "
						"VALUES (%i, %i, %i, %f, %f, %f, %i, %f)",
						grid_num, zoneid, next_wp_num, position.m_X, position.m_Y, position.m_Z, pause, position.m_Heading);
	results = QueryDatabase(query);
	if(!results.Success())
		LogFile->write(EQEmuLog::Error, "Error adding grid entry '%s': '%s'", query.c_str(), results.ErrorMessage().c_str());
	else if(client)
		client->LogSQL(query.c_str());

	return createdNewGrid? grid_num: 0;
}

uint32 ZoneDatabase::GetFreeGrid(uint16 zoneid) {

	std::string query = StringFormat("SELECT max(id) FROM grid WHERE zoneid = %i", zoneid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        LogFile->write(EQEmuLog::Error, "Error in GetFreeGrid query '%s': %s", query.c_str(), results.ErrorMessage().c_str());
        return 0;
	}

	if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();
    uint32 freeGridID = 1;
	freeGridID = atoi(row[0]) + 1;

    return freeGridID;
}

int ZoneDatabase::GetHighestWaypoint(uint32 zoneid, uint32 gridid) {

	std::string query = StringFormat("SELECT COALESCE(MAX(number), 0) FROM grid_entries "
                                    "WHERE zoneid = %i AND gridid = %i", zoneid, gridid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        LogFile->write(EQEmuLog::Error, "Error in GetHighestWaypoint query '%s': %s", query.c_str(), results.ErrorMessage().c_str());
        return 0;
	}

	if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();
    return atoi(row[0]);
}

void NPC::SaveGuardSpotCharm()
{
    m_GuardPointSaved = m_GuardPoint;
}

void NPC::RestoreGuardSpotCharm()
{
    m_GuardPoint = m_GuardPointSaved;
}
