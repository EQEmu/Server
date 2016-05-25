/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/rulesys.h"

#include "map.h"
#include "pathing.h"
#include "zone.h"

#ifdef _WINDOWS
#define snprintf	_snprintf
#endif

extern Zone* zone;

#define FEAR_PATHING_DEBUG

//this is called whenever we are damaged to process possible fleeing
void Mob::CheckFlee() {
	//if were allready fleeing, dont need to check more...
	if(flee_mode && currently_fleeing)
		return;

	//dont bother if we are immune to fleeing
	if(GetSpecialAbility(IMMUNE_FLEEING) || spellbonuses.ImmuneToFlee)
		return;

	if(!flee_timer.Check())
		return;	//only do all this stuff every little while, since
				//its not essential that we start running RIGHT away

	//see if were possibly hurt enough
	float ratio = GetHPRatio();
	float fleeratio = GetSpecialAbility(FLEE_PERCENT);
	fleeratio = fleeratio > 0 ? fleeratio : RuleI(Combat, FleeHPRatio);

	if(ratio >= fleeratio)
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
			run_ratio = fleeratio;
			break;
		case CON_LIGHTBLUE:
			run_ratio = fleeratio * 9 / 10;
			break;
		case CON_BLUE:
			run_ratio = fleeratio * 8 / 10;
			break;
		default:
			run_ratio = fleeratio * 7 / 10;
			break;
	}
	if(ratio < run_ratio)
	{
		if (RuleB(Combat, FleeIfNotAlone) ||
			GetSpecialAbility(ALWAYS_FLEE) ||
			(!RuleB(Combat, FleeIfNotAlone) && (entity_list.GetHatedCount(hate_top, this) == 0)))
			StartFleeing();
	}
}

void Mob::ProcessFlee()
{

	//Stop fleeing if effect is applied after they start to run.
	//When ImmuneToFlee effect fades it will turn fear back on and check if it can still flee.
	if (flee_mode && (GetSpecialAbility(IMMUNE_FLEEING) || spellbonuses.ImmuneToFlee) &&
			!spellbonuses.IsFeared && !spellbonuses.IsBlind) {
		currently_fleeing = false;
		return;
	}

	//see if we are still dying, if so, do nothing
	float fleeratio = GetSpecialAbility(FLEE_PERCENT);
	fleeratio = fleeratio > 0 ? fleeratio : RuleI(Combat, FleeHPRatio);
	if (GetHPRatio() < fleeratio)
		return;

	//we are not dying anymore... see what we do next

	flee_mode = false;

	//see if we are legitimately feared or blind now
	if (!spellbonuses.IsFeared && !spellbonuses.IsBlind) {
		//not feared or blind... were done...
		currently_fleeing = false;
		return;
	}
}

void Mob::CalculateNewFearpoint()
{
	if(RuleB(Pathing, Fear) && zone->pathing)
	{
		int Node = zone->pathing->GetRandomPathNode();

		glm::vec3 Loc = zone->pathing->GetPathNodeCoordinates(Node);

		++Loc.z;

		glm::vec3 CurrentPosition(GetX(), GetY(), GetZ());

		std::deque<int> Route = zone->pathing->FindRoute(CurrentPosition, Loc);

		if(!Route.empty())
		{
            m_FearWalkTarget = glm::vec3(Loc.x, Loc.y, Loc.z);
			currently_fleeing = true;

			Log.Out(Logs::Detail, Logs::None, "Feared to node %i (%8.3f, %8.3f, %8.3f)", Node, Loc.x, Loc.y, Loc.z);
			return;
		}

		Log.Out(Logs::Detail, Logs::None, "No path found to selected node. Falling through to old fear point selection.");
	}

	int loop = 0;
	float ranx, rany, ranz;
	currently_fleeing = false;
	while (loop < 100) //Max 100 tries
	{
		int ran = 250 - (loop*2);
		loop++;
		ranx = GetX()+zone->random.Int(0, ran-1)-zone->random.Int(0, ran-1);
		rany = GetY()+zone->random.Int(0, ran-1)-zone->random.Int(0, ran-1);
		ranz = FindGroundZ(ranx,rany);
		if (ranz == -999999)
			continue;
		float fdist = ranz - GetZ();
		if (fdist >= -12 && fdist <= 12 && CheckCoordLosNoZLeaps(GetX(),GetY(),GetZ(),ranx,rany,ranz))
		{
			currently_fleeing = true;
			break;
		}
	}
	if (currently_fleeing)
        m_FearWalkTarget = glm::vec3(ranx, rany, ranz);
	else //Break fear
		BuffFadeByEffect(SE_Fear);
}

