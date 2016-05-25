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

#include "types.h"
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

struct EQEmu::SkillProfile // prototype - not implemented
{
	union {
		struct {
			uint32 _1HBlunt;
			uint32 _1HSlashing;
			uint32 _2HBlunt;
			uint32 _2HSlashing;
			uint32 Abjuration;
			uint32 Alteration;
			uint32 ApplyPoison;
			uint32 Archery;
			uint32 Backstab;
			uint32 BindWound;
			uint32 Bash;
			uint32 Block;
			uint32 BrassInstruments;
			uint32 Channeling;
			uint32 Conjuration;
			uint32 Defense;
			uint32 Disarm;
			uint32 DisarmTraps;
			uint32 Divination;
			uint32 Dodge;
			uint32 DoubleAttack;
			union {
				uint32 DragonPunch;
				uint32 TailRake;
			};
			uint32 DualWield;
			uint32 EagleStrike;
			uint32 Evocation;
			uint32 FeignDeath;
			uint32 FlyingKick;
			uint32 Forage;
			uint32 HandtoHand;
			uint32 Hide;
			uint32 Kick;
			uint32 Meditate;
			uint32 Mend;
			uint32 Offense;
			uint32 Parry;
			uint32 PickLock;
			uint32 _1HPiercing;
			uint32 Riposte;
			uint32 RoundKick;
			uint32 SafeFall;
			uint32 SenseHeading;
			uint32 Singing;
			uint32 Sneak;
			uint32 SpecializeAbjure;
			uint32 SpecializeAlteration;
			uint32 SpecializeConjuration;
			uint32 SpecializeDivination;
			uint32 SpecializeEvocation;
			uint32 PickPockets;
			uint32 StringedInstruments;
			uint32 Swimming;
			uint32 Throwing;
			uint32 TigerClaw;
			uint32 Tracking;
			uint32 WindInstruments;
			uint32 Fishing;
			uint32 MakePoison;
			uint32 Tinkering;
			uint32 Research;
			uint32 Alchemy;
			uint32 Baking;
			uint32 Tailoring;
			uint32 SenseTraps;
			uint32 Blacksmithing;
			uint32 Fletching;
			uint32 Brewing;
			uint32 AlcoholTolerance;
			uint32 Begging;
			uint32 JewelryMaking;
			uint32 Pottery;
			uint32 PercussionInstruments;
			uint32 Intimidation;
			uint32 Berserking;
			uint32 Taunt;
			uint32 Frenzy;
			uint32 RemoveTraps;
			uint32 TripleAttack;
			uint32 _2HPiercing;
			uint32 unused1;
			uint32 unused2;
			uint32 unused3;
			uint32 unused4;
			uint32 unused5;
			uint32 unused6;
			uint32 unused7;
			uint32 unused8;
			uint32 unused9;
			uint32 unused10;
			uint32 unused11;
			uint32 unused12;
			uint32 unused13;
			uint32 unused14;
			uint32 unused15;
			uint32 unused16;
			uint32 unused17;
			uint32 unused18;
			uint32 unused19;
			uint32 unused20;
			uint32 unused21;
			uint32 unused22;
		};
		uint32 skill[PACKET_SKILL_ARRAY_SIZE];
	};

	SkillProfile();

	uint32* GetSkills() { return reinterpret_cast<uint32*>(&skill); }

	skills::SkillType GetLastUseableSkill() { return EQEmu::skills::Skill2HPiercing; }

	size_t GetSkillsArraySize() { return PACKET_SKILL_ARRAY_SIZE; }
	uint32 GetSkill(int skill_id);
	
	uint32 operator[](int skill_id) { return GetSkill(skill_id); }

	// const
	uint32* GetSkills() const { return const_cast<SkillProfile*>(this)->GetSkills(); }

	skills::SkillType GetLastUseableSkill() const { return const_cast<SkillProfile*>(this)->GetLastUseableSkill(); }

	size_t GetSkillsArraySize() const { return const_cast<SkillProfile*>(this)->GetSkillsArraySize(); }
	uint32 GetSkill(int skill_id) const { return const_cast<SkillProfile*>(this)->GetSkill(skill_id); }

	uint32 operator[](int skill_id) const { return const_cast<SkillProfile*>(this)->GetSkill(skill_id); }
};

EQEmu::SkillProfile::SkillProfile()
{
	memset(&skill, 0, (sizeof(uint32) * PACKET_SKILL_ARRAY_SIZE));
}

uint32 EQEmu::SkillProfile::GetSkill(int skill_id)
{
	if (skill_id < 0 || skill_id >= PACKET_SKILL_ARRAY_SIZE)
		return 0;

	return skill[skill_id];
}
