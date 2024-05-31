/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#include "skills.h"

#include <string.h>

float Skill::GetPushForce(uint16 skill_id)
{
	// This is the force/magnitude of the push from an attack of this skill type
	// You can find these numbers in the clients skill struct
	switch (skill_id) {
		case OneHandBlunt:
		case OneHandSlashing:
		case HandToHand:
		case Throwing:
			return 0.1f;
		case TwoHandBlunt:
		case TwoHandSlashing:
		case EagleStrike:
		case Kick:
		case TigerClaw:
		case TwoHandPiercing:
			return 0.2f;
		case Archery:
			return 0.15f;
		case Backstab:
		case Bash:
			return 0.3f;
		case DragonPunch:
		case RoundKick:
			return 0.25f;
		case FlyingKick:
			return 0.4f;
		case OneHandPiercing:
		case Frenzy:
			return 0.05f;
		case Intimidation:
			return 2.5f;
		default:
			return 0.0f;
	}
}

std::string Skill::GetName(uint16 skill_id)
{
	return IsValid(skill_id) ? skill_names[skill_id] : "UNKNOWN SKILL";
}

bool Skill::IsTradeskill(uint16 skill_id)
{
	switch (skill_id) {
		case Fishing:
		case MakePoison:
		case Tinkering:
		case Research:
		case Alchemy:
		case Baking:
		case Tailoring:
		case Blacksmithing:
		case Fletching:
		case Brewing:
		case Pottery:
		case JewelryMaking:
			return true;
		default:
			return false;
	}
}

bool Skill::IsSpecialized(uint16 skill_id)
{
	// this could be a simple if, but if this is more portable if any IDs change (probably won't)
	// or any other specialized are added (also unlikely)
	switch (skill_id) {
		case SpecializeAbjuration:
		case SpecializeAlteration:
		case SpecializeConjuration:
		case SpecializeDivination:
		case SpecializeEvocation:
			return true;
		default:
			return false;
	}
}

bool Skill::IsBardInstrument(uint16 skill_id)
{
	switch (skill_id) {
		case BrassInstruments:
		case Singing:
		case StringedInstruments:
		case WindInstruments:
		case PercussionInstruments:
			return true;
		default:
			return false;
	}
}

const std::map<EQ::skills::SkillType, std::string>& EQ::skills::GetSkillTypeMap()
{
	switch (skill_id) {
		case Abjuration:
		case Alteration:
		case Conjuration:
		case Divination:
		case Evocation:
			return true;
		default:
			return false;
	}
}

bool Skill::IsMonk(uint16 skill_id)
{
	return (
		skill_id == Skill::DragonPunch ||
		skill_id == Skill::EagleStrike ||
		skill_id == Skill::FlyingKick ||
		skill_id == Skill::RoundKick ||
		skill_id == Skill::TigerClaw
	);
}

int Skill::GetBaseDamage(uint16 skill_id)
{
	switch (skill_id) {
		case Bash:
			return 2;
		case DragonPunch:
			return 12;
		case EagleStrike:
			return 7;
		case FlyingKick:
			return 25;
		case Kick:
			return 3;
		case RoundKick:
			return 5;
		case TigerClaw:
			return 4;
		case Frenzy:
			return 10;
		default:
			return 0;
	}
}

bool Skill::IsValid(uint16 skill_id)
{
	return skill_names.find(skill_id) != skill_names.end();
}
