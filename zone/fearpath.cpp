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
#include "zone.h"

#ifdef _WINDOWS
#define snprintf	_snprintf
#endif

extern Zone* zone;

#define FEAR_PATHING_DEBUG

//this is called whenever we are damaged to process possible fleeing
void Mob::CheckFlee()
{

	// if mob is dead why would you run?
	if (GetHP() == 0) {
		return;
	}

	// if were already fleeing, don't need to check more...
	if (flee_mode && currently_fleeing) {
		return;
	}

	//dont bother if we are immune to fleeing
	if (GetSpecialAbility(IMMUNE_FLEEING) || spellbonuses.ImmuneToFlee) {
		LogFlee("Mob [{}] is immune to fleeing via special ability or spell bonus", GetCleanName());
		return;
	}

	// Check if Flee Timer is cleared
	if (!flee_timer.Check()) {
		return;
	}

	int hp_ratio   = GetIntHPRatio();
	int flee_ratio = GetSpecialAbility(FLEE_PERCENT); // if a special flee_percent exists
	Mob *hate_top  = GetHateTop();

	LogFlee("Mob [{}] hp_ratio [{}] flee_ratio [{}]", GetCleanName(), hp_ratio, flee_ratio);

	// Sanity Check for race conditions
	if (hate_top == nullptr) {
		return;
	}

	// If no special flee_percent check for Gray or Other con rates
	if (GetLevelCon(hate_top->GetLevel(), GetLevel()) == CON_GRAY && flee_ratio == 0 && RuleB(Combat, FleeGray) &&
		GetLevel() <= RuleI(Combat, FleeGrayMaxLevel)) {
		flee_ratio = RuleI(Combat, FleeGrayHPRatio);
		LogFlee("Mob [{}] using combat flee gray hp_ratio [{}] flee_ratio [{}]", GetCleanName(), hp_ratio, flee_ratio);
	}
	else if (flee_ratio == 0) {
		flee_ratio = RuleI(Combat, FleeHPRatio);
		LogFlee("Mob [{}] using combat flee hp_ratio [{}] flee_ratio [{}]", GetCleanName(), hp_ratio, flee_ratio);
	}

	bool mob_has_low_enough_health_to_flee = hp_ratio >= flee_ratio;
	if (mob_has_low_enough_health_to_flee) {
		LogFlee(
			"Mob [{}] does not have low enough health to flee | hp_ratio [{}] flee_ratio [{}]",
			GetCleanName(),
			hp_ratio,
			flee_ratio
		);
		return;
	}

	// Sanity Check this should never happen...
	if (!hate_top) {
		currently_fleeing = true;
		StartFleeing();
		return;
	}

	int other_ratio = hate_top->GetIntHPRatio();
	// If the Client is nearing death the NPC will not flee and instead try to kill the client.
	if (other_ratio < 20) {
		return;
	}

	// Flee Chance checking based on con.
	uint32 con = GetLevelCon(hate_top->GetLevel(), GetLevel());
	int    flee_chance;
	switch (con) {
		//these values are not 100% researched
		case CON_GRAY:
			flee_chance = 100;
			break;
		case CON_GREEN:
			flee_chance = 90;
			break;
		case CON_LIGHTBLUE:
			flee_chance = 90;
			break;
		case CON_BLUE:
			flee_chance = 80;
			break;
		default:
			flee_chance = 70;
			break;
	}

	LogFlee(
		"Post con-switch | Mob [{}] con [{}] hp_ratio [{}] flee_ratio [{}] flee_chance [{}]",
		GetCleanName(),
		con,
		hp_ratio,
		flee_ratio,
		flee_chance
	);

	// If we got here we are allowed to roll on flee chance if there is not other hated NPC's in the area.
	// ALWAYS_FLEE, skip roll
	// if FleeIfNotAlone is true, we skip alone check
	// roll chance
	if (GetSpecialAbility(ALWAYS_FLEE) ||
		((RuleB(Combat, FleeIfNotAlone) || entity_list.GetHatedCount(hate_top, this, true) == 0) &&
		 zone->random.Roll(flee_chance))) {

		LogFlee(
			"Passed all checks to flee | Mob [{}] con [{}] hp_ratio [{}] flee_ratio [{}] flee_chance [{}]",
			GetCleanName(),
			con,
			hp_ratio,
			flee_ratio,
			flee_chance
		);

		currently_fleeing = true;
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

	int hpratio = GetIntHPRatio();
	int fleeratio = GetSpecialAbility(FLEE_PERCENT); // if a special flee_percent exists
	Mob *hate_top = GetHateTop();

	// If no special flee_percent check for Gray or Other con rates
	if(hate_top != nullptr && GetLevelCon(hate_top->GetLevel(), GetLevel()) == CON_GRAY && fleeratio == 0 && RuleB(Combat, FleeGray)) {
		fleeratio = RuleI(Combat, FleeGrayHPRatio);
	} else if(fleeratio == 0) {
		fleeratio = RuleI(Combat, FleeHPRatio );
	}

	// Mob is still too low. Keep Running
	if(hpratio < fleeratio) {
		return;
	}

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
	if (RuleB(Pathing, Fear) && zone->pathing) {
		auto Node = zone->pathing->GetRandomLocation(glm::vec3(GetX(), GetY(), GetZ()));
		if (Node.x != 0.0f || Node.y != 0.0f || Node.z != 0.0f) {
			m_FearWalkTarget  = Node;
			currently_fleeing = true;

			return;
		}

		LogPathing("No path found to selected node during CalculateNewFearpoint.");
	}
}

