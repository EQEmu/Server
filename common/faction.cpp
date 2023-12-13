/*	 EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include "faction.h"
#include "races.h"
#include "rulesys.h"

const char *FactionValueToString(FACTION_VALUE faction_value)
{
	switch (faction_value) {
		case FACTION_ALLY:
			return "Ally";
		case FACTION_WARMLY:
			return "Warmly";
		case FACTION_KINDLY:
			return "Kindly";
		case FACTION_AMIABLY:
			return "Amiably";
		case FACTION_INDIFFERENTLY:
			return "Indifferently";
		case FACTION_APPREHENSIVELY:
			return "Apprehensively";
		case FACTION_DUBIOUSLY:
			return "Dubiously";
		case FACTION_THREATENINGLY:
			return "Threateningly";
		case FACTION_SCOWLS:
			return "Scowls";
		default:
			break;
	}
	return "Unknown";
}


//o--------------------------------------------------------------
//| Name: CalculateFaction; Dec. 16, 2001
//o--------------------------------------------------------------
//| Notes: Returns the faction message value.
//|		Modify these values to taste.
//o--------------------------------------------------------------
FACTION_VALUE CalculateFaction(FactionMods* fm, int32 tmpCharacter_value)
{
	int32 character_value = tmpCharacter_value;
	if (fm) {
		character_value += fm->base + fm->class_mod + fm->race_mod + fm->deity_mod;
	}
	if (character_value >= RuleI(Faction, AllyFactionMinimum)) {
		return FACTION_ALLY;
	}
	if (character_value >= RuleI(Faction, WarmlyFactionMinimum)) {
		return FACTION_WARMLY;
	}
	if (character_value >= RuleI(Faction, KindlyFactionMinimum)) {
		return FACTION_KINDLY;
	}
	if (character_value >= RuleI(Faction, AmiablyFactionMinimum)) {
		return FACTION_AMIABLY;
	}
	if (character_value >= RuleI(Faction, IndifferentlyFactionMinimum)) {
		return FACTION_INDIFFERENTLY;
	}
	if (character_value >= RuleI(Faction, ApprehensivelyFactionMinimum)) {
		return FACTION_APPREHENSIVELY;
	}
	if (character_value >= RuleI(Faction, DubiouslyFactionMinimum)) {
		return FACTION_DUBIOUSLY;
	}
	if (character_value >= RuleI(Faction, ThreateninglyFactionMinimum)) {
		return FACTION_THREATENINGLY;
	}
	return FACTION_SCOWLS;
}

// this function should check if some races have more than one race define
bool IsOfEqualRace(int r1, int r2)
{
	if (r1 == r2) {
		return true;
	}
	// TODO: add more values
	switch (r1) {
		case Races::DarkElf:
			if (r2 == Races::NeriakCitizen) {
				return true;
			}
			break;
		case Races::Barbarian:
			if (r2 == Races::HalasCitizen) {
				return true;
			}
	}
	return false;
}

// trolls endure ogres, dark elves, ...
bool IsOfIndiffRace(int r1, int r2)
{
	if (r1 == r2) {
		return true;
	}
	// TODO: add more values
	switch (r1) {
		case Races::DarkElf:
		case Races::Ogre:
		case Races::Troll:
			if (r2 == Races::Ogre || r2 == Races::Troll || r2 == Races::DarkElf) {
				return true;
			}
			break;
		case Races::Human:
		case Races::Barbarian:
		case Races::HalfElf:
		case Races::Gnome:
		case Races::Halfling:
		case Races::WoodElf:
			if (r2 == Races::Human ||
			    r2 == Races::Barbarian ||
			    r2 == Races::Erudite ||
			    r2 == Races::HalfElf ||
			    r2 == Races::Gnome ||
			    r2 == Races::Halfling ||
			    r2 == Races::Dwarf ||
			    r2 == Races::HighElf ||
			    r2 == Races::WoodElf) {
				return true;
			}
			break;
		case Races::Erudite:
			if (r2 == Races::Human || r2 == Races::HalfElf) {
				return true;
			}
			break;
		case Races::Dwarf:
			if (r2 == Races::Halfling || r2 == Races::Gnome) {
				return true;
			}
			break;
		case Races::HighElf:
			if (r2 == Races::WoodElf) {
				return true;
			}
			break;
		case Races::VahShir:
			return true;
		case Races::Iksar:
			return false;
	}
	return false;
}

