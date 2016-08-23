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


bool EQEmu::skills::IsTradeskill(SkillType skill)
{
	switch (skill) {
	case SkillFishing:
	case SkillMakePoison:
	case SkillTinkering:
	case SkillResearch:
	case SkillAlchemy:
	case SkillBaking:
	case SkillTailoring:
	case SkillBlacksmithing:
	case SkillFletching:
	case SkillBrewing:
	case SkillPottery:
	case SkillJewelryMaking:
		return true;
	default:
		return false;
	}
}

bool EQEmu::skills::IsSpecializedSkill(SkillType skill)
{
	// this could be a simple if, but if this is more portable if any IDs change (probably won't)
	// or any other specialized are added (also unlikely)
	switch (skill) {
	case SkillSpecializeAbjure:
	case SkillSpecializeAlteration:
	case SkillSpecializeConjuration:
	case SkillSpecializeDivination:
	case SkillSpecializeEvocation:
		return true;
	default:
		return false;
	}
}

float EQEmu::skills::GetSkillMeleePushForce(SkillType skill)
{
	// This is the force/magnitude of the push from an attack of this skill type
	// You can find these numbers in the clients skill struct
	switch (skill) {
	case Skill1HBlunt:
	case Skill1HSlashing:
	case SkillHandtoHand:
	case SkillThrowing:
		return 0.1f;
	case Skill2HBlunt:
	case Skill2HSlashing:
	case SkillEagleStrike:
	case SkillKick:
	case SkillTigerClaw:
	case Skill2HPiercing:
		return 0.2f;
	case SkillArchery:
		return 0.15f;
	case SkillBackstab:
	case SkillBash:
		return 0.3f;
	case SkillDragonPunch:
	case SkillRoundKick:
		return 0.25f;
	case SkillFlyingKick:
		return 0.4f;
	case Skill1HPiercing:
	case SkillFrenzy:
		return 0.05f;
	case SkillIntimidation:
		return 2.5f;
	default:
		return 0.0f;
	}
}

bool EQEmu::skills::IsBardInstrumentSkill(SkillType skill)
{
	switch (skill) {
	case SkillBrassInstruments:
	case SkillSinging:
	case SkillStringedInstruments:
	case SkillWindInstruments:
	case SkillPercussionInstruments:
		return true;
	default:
		return false;
	}
}

bool EQEmu::skills::IsCastingSkill(SkillType skill)
{
	switch (skill) {
	case SkillAbjuration:
	case SkillAlteration:
	case SkillConjuration:
	case SkillDivination:
	case SkillEvocation:
		return true;
	default:
		return false;
	}
}

const std::map<EQEmu::skills::SkillType, std::string>& EQEmu::skills::GetSkillTypeMap()
{
	/* VS2013 code
	static const std::map<SkillUseTypes, std::string> skill_use_types_map = {
		{ Skill1HBlunt, "1H Blunt" },
		{ Skill1HSlashing, "1H Slashing" },
		{ Skill2HBlunt, "2H Blunt" },
		{ Skill2HSlashing, "2H Slashing" },
		{ SkillAbjuration, "Abjuration" },
		{ SkillAlteration, "Alteration" },
		{ SkillApplyPoison, "Apply Poison" },
		{ SkillArchery, "Archery" },
		{ SkillBackstab, "Backstab" },
		{ SkillBindWound, "Bind Wound" },
		{ SkillBash, "Bash" },
		{ SkillBlock, "Block" },
		{ SkillBrassInstruments, "Brass Instruments" },
		{ SkillChanneling, "Channeling" },
		{ SkillConjuration, "Conjuration" },
		{ SkillDefense, "Defense" },
		{ SkillDisarm, "Disarm" },
		{ SkillDisarmTraps, "Disarm Traps" },
		{ SkillDivination, "Divination" },
		{ SkillDodge, "Dodge" },
		{ SkillDoubleAttack, "Double Attack" },
		{ SkillDragonPunch, "Dragon Punch" },
		{ SkillDualWield, "Dual Wield" },
		{ SkillEagleStrike, "Eagle Strike" },
		{ SkillEvocation, "Evocation" },
		{ SkillFeignDeath, "Feign Death" },
		{ SkillFlyingKick, "Flying Kick" },
		{ SkillForage, "Forage" },
		{ SkillHandtoHand, "Hand to Hand" },
		{ SkillHide, "Hide" },
		{ SkillKick, "Kick" },
		{ SkillMeditate, "Meditate" },
		{ SkillMend, "Mend" },
		{ SkillOffense, "Offense" },
		{ SkillParry, "Parry" },
		{ SkillPickLock, "Pick Lock" },
		{ Skill1HPiercing, "1H Piercing" },
		{ SkillRiposte, "Riposte" },
		{ SkillRoundKick, "Round Kick" },
		{ SkillSafeFall, "Safe Fall" },
		{ SkillSenseHeading, "Sense Heading" },
		{ SkillSinging, "Singing" },
		{ SkillSneak, "Sneak" },
		{ SkillSpecializeAbjure, "Specialize Abjuration" },
		{ SkillSpecializeAlteration, "Specialize Alteration" },
		{ SkillSpecializeConjuration, "Specialize Conjuration" },
		{ SkillSpecializeDivination, "Specialize Divination" },
		{ SkillSpecializeEvocation, "Specialize Evocation" },
		{ SkillPickPockets, "Pick Pockets" },
		{ SkillStringedInstruments, "Stringed Instruments" },
		{ SkillSwimming, "Swimming" },
		{ SkillThrowing, "Throwing" },
		{ SkillTigerClaw, "Tiger Claw" },
		{ SkillTracking, "Tracking" },
		{ SkillWindInstruments, "Wind Instruments" },
		{ SkillFishing, "Fishing" },
		{ SkillMakePoison, "Make Poison" },
		{ SkillTinkering, "Tinkering" },
		{ SkillResearch, "Research" },
		{ SkillAlchemy, "Alchemy" },
		{ SkillBaking, "Baking" },
		{ SkillTailoring, "Tailoring" },
		{ SkillSenseTraps, "Sense Traps" },
		{ SkillBlacksmithing, "Blacksmithing" },
		{ SkillFletching, "Fletching" },
		{ SkillBrewing, "Brewing" },
		{ SkillAlcoholTolerance, "Alcohol Tolerance" },
		{ SkillBegging, "Begging" },
		{ SkillJewelryMaking, "Jewelry Making" },
		{ SkillPottery, "Pottery" },
		{ SkillPercussionInstruments, "Percussion Instruments" },
		{ SkillIntimidation, "Intimidation" },
		{ SkillBerserking, "Berserking" },
		{ SkillTaunt, "Taunt" },
		{ SkillFrenzy, "Frenzy" },
		{ SkillRemoveTraps, "Remove Traps" },
		{ SkillTripleAttack, "Triple Attack" },
		{ Skill2HPiercing, "2H Piercing" }
	};
	*/

	/* VS2012 code - begin */

	static const char* skill_use_names[SkillCount] = {
		"1H Blunt",
		"1H Slashing",
		"2H Blunt",
		"2H Slashing",
		"Abjuration",
		"Alteration",
		"Apply Poison",
		"Archery",
		"Backstab",
		"Bind Wound",
		"Bash",
		"Block",
		"Brass Instruments",
		"Channeling",
		"Conjuration",
		"Defense",
		"Disarm",
		"Disarm Traps",
		"Divination",
		"Dodge",
		"Double Attack",
		"Dragon Punch",
		"Dual Wield",
		"Eagle Strike",
		"Evocation",
		"Feign Death",
		"Flying Kick",
		"Forage",
		"Hand to Hand",
		"Hide",
		"Kick",
		"Meditate",
		"Mend",
		"Offense",
		"Parry",
		"Pick Lock",
		"1H Piercing",
		"Riposte",
		"Round Kick",
		"Safe Fall",
		"Sense Heading",
		"Singing",
		"Sneak",
		"Specialize Abjuration",
		"Specialize Alteration",
		"Specialize Conjuration",
		"Specialize Divination",
		"Specialize Evocation",
		"Pick Pockets",
		"Stringed Instruments",
		"Swimming",
		"Throwing",
		"Tiger Claw",
		"Tracking",
		"Wind Instruments",
		"Fishing",
		"Make Poison",
		"Tinkering",
		"Research",
		"Alchemy",
		"Baking",
		"Tailoring",
		"Sense Traps",
		"Blacksmithing",
		"Fletching",
		"Brewing",
		"Alcohol Tolerance",
		"Begging",
		"Jewelry Making",
		"Pottery",
		"Percussion Instruments",
		"Intimidation",
		"Berserking",
		"Taunt",
		"Frenzy",
		"Remove Traps",
		"Triple Attack",
		"2H Piercing"
	};

	static std::map<SkillType, std::string> skill_type_map;

	skill_type_map.clear();

	for (int i = Skill1HBlunt; i < SkillCount; ++i)
		skill_type_map[(SkillType)i] = skill_use_names[i];

	/* VS2012 code - end */

	return skill_type_map;
}

EQEmu::SkillProfile::SkillProfile()
{
	memset(&Skill, 0, (sizeof(uint32) * PACKET_SKILL_ARRAY_SIZE));
}

uint32 EQEmu::SkillProfile::GetSkill(int skill_id)
{
	if (skill_id < 0 || skill_id >= PACKET_SKILL_ARRAY_SIZE)
		return 0;

	return Skill[skill_id];
}
