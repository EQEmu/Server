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

#ifndef COMMON_SKILLS_H
#define COMMON_SKILLS_H

#include "data_verification.h"
#include "types.h"

#include <string>
#include <map>
#include <vector>

namespace Damage {
	constexpr int64 None         = 0;
	constexpr int64 Block        = -1;
	constexpr int64 Parry        = -2;
	constexpr int64 Riposte      = -3;
	constexpr int64 Dodge        = -4;
	constexpr int64 Invulnerable = -5;
	constexpr int64 Rune         = -6;
}

namespace Skill {
	constexpr int OneHandBlunt          = 0;
	constexpr int OneHandSlashing       = 1;
	constexpr int TwoHandBlunt          = 2;
	constexpr int TwoHandSlashing       = 3;
	constexpr int Abjuration            = 4;
	constexpr int Alteration            = 5;
	constexpr int ApplyPoison           = 6;
	constexpr int Archery               = 7;
	constexpr int Backstab              = 8;
	constexpr int BindWound             = 9;
	constexpr int Bash                  = 10;
	constexpr int Block                 = 11;
	constexpr int BrassInstruments      = 12;
	constexpr int Channeling            = 13;
	constexpr int Conjuration           = 14;
	constexpr int Defense               = 15;
	constexpr int Disarm                = 16;
	constexpr int DisarmTraps           = 17;
	constexpr int Divination            = 18;
	constexpr int Dodge                 = 19;
	constexpr int DoubleAttack          = 20;
	constexpr int DragonPunch           = 21;
	constexpr int TailRake              = 21;
	constexpr int DualWield             = 22;
	constexpr int EagleStrike           = 23;
	constexpr int Evocation             = 24;
	constexpr int FeignDeath            = 25;
	constexpr int FlyingKick            = 26;
	constexpr int Forage                = 27;
	constexpr int HandToHand            = 28;
	constexpr int Hide                  = 29;
	constexpr int Kick                  = 30;
	constexpr int Meditate              = 31;
	constexpr int Mend                  = 32;
	constexpr int Offense               = 33;
	constexpr int Parry                 = 34;
	constexpr int PickLock              = 35;
	constexpr int OneHandPiercing       = 36;
	constexpr int Riposte               = 37;
	constexpr int RoundKick             = 38;
	constexpr int SafeFall              = 39;
	constexpr int SenseHeading          = 40;
	constexpr int Singing               = 41;
	constexpr int Sneak                 = 42;
	constexpr int SpecializeAbjuration  = 43;
	constexpr int SpecializeAlteration  = 44;
	constexpr int SpecializeConjuration = 45;
	constexpr int SpecializeDivination  = 46;
	constexpr int SpecializeEvocation   = 47;
	constexpr int PickPockets           = 48;
	constexpr int StringedInstruments   = 49;
	constexpr int Swimming              = 50;
	constexpr int Throwing              = 51;
	constexpr int TigerClaw             = 52;
	constexpr int Tracking              = 53;
	constexpr int WindInstruments       = 54;
	constexpr int Fishing               = 55;
	constexpr int MakePoison            = 56;
	constexpr int Tinkering             = 57;
	constexpr int Research              = 58;
	constexpr int Alchemy               = 59;
	constexpr int Baking                = 60;
	constexpr int Tailoring             = 61;
	constexpr int SenseTraps            = 62;
	constexpr int Blacksmithing         = 63;
	constexpr int Fletching             = 64;
	constexpr int Brewing               = 65;
	constexpr int AlcoholTolerance      = 66;
	constexpr int Begging               = 67;
	constexpr int JewelryMaking         = 68;
	constexpr int Pottery               = 69;
	constexpr int PercussionInstruments = 70;
	constexpr int Intimidation          = 71;
	constexpr int Berserking            = 72;
	constexpr int Taunt                 = 73;
	constexpr int Frenzy                = 74;
	constexpr int RemoveTraps           = 75;
	constexpr int TripleAttack          = 76;
	constexpr int TwoHandPiercing       = 77;

	constexpr int Max = TwoHandPiercing;
	constexpr int All = -1;

	namespace Reuse {
		constexpr int Backstab     = 9;
		constexpr int Bash         = 5;
		constexpr int DisarmTraps  = 9;
		constexpr int EagleStrike  = 5;
		constexpr int FeignDeath   = 9;
		constexpr int Fishing      = 11;
		constexpr int FlyingKick   = 7;
		constexpr int Foraging     = 50;
		constexpr int Frenzy       = 10;
		constexpr int HarmTouch    = 4300;
		constexpr int Hide         = 8;
		constexpr int InstillDoubt = 9;
		constexpr int Kick         = 5;
		constexpr int LayOnHands   = 4300;
		constexpr int Mend         = 360;
		constexpr int RoundKick    = 9;
		constexpr int SenseTraps   = 9;
		constexpr int Sneak        = 7;
		constexpr int TailRake     = 6;
		constexpr int Taunt        = 5;
		constexpr int TigerClaw    = 6;
	};

	namespace BankType {
		constexpr uint16 Skills    = 0;
		constexpr uint16 Languages = 1;
	}

	constexpr int ArraySize = 100;

		constexpr int format_as(SkillType skill) { return static_cast<int>(skill); }

		bool IsTradeskill(SkillType skill);
		bool IsSpecializedSkill(SkillType skill);
		float GetSkillMeleePushForce(SkillType skill);
		bool IsBardInstrumentSkill(SkillType skill);
		bool IsCastingSkill(SkillType skill);
		int32 GetBaseDamage(SkillType skill);

static std::vector<uint16> extra_damage_skills = {
	Skill::Backstab,
	Skill::Bash,
	Skill::DragonPunch, // Same ID as Tail Rake
	Skill::EagleStrike,
	Skill::FlyingKick,
	Skill::Kick,
	Skill::RoundKick,
	Skill::TigerClaw,
	Skill::Frenzy
};

static std::vector<uint16> monk_skills = {
	Skill::DragonPunch,
	Skill::EagleStrike,
	Skill::FlyingKick,
	Skill::RoundKick,
	Skill::TigerClaw
};

static std::map<uint16, std::string> skill_names = {
	{ Skill::OneHandBlunt,          "1H Blunt" },
	{ Skill::OneHandSlashing,       "1H Slashing" },
	{ Skill::TwoHandBlunt,          "2H Blunt" },
	{ Skill::TwoHandSlashing,       "2H Slashing" },
	{ Skill::Abjuration,            "Abjuration" },
	{ Skill::Alteration,            "Alteration" },
	{ Skill::ApplyPoison,           "Apply Poison" },
	{ Skill::Archery,               "Archery" },
	{ Skill::Backstab,              "Backstab" },
	{ Skill::BindWound,             "Bind Wound" },
	{ Skill::Bash,                  "Bash" },
	{ Skill::Block,                 "Block" },
	{ Skill::BrassInstruments,      "Brass Instruments" },
	{ Skill::Channeling,            "Channeling" },
	{ Skill::Conjuration,           "Conjuration" },
	{ Skill::Defense,               "Defense" },
	{ Skill::Disarm,                "Disarm" },
	{ Skill::DisarmTraps,           "Disarm Traps" },
	{ Skill::Divination,      "Divination" },
	{ Skill::Dodge,           "Dodge" },
	{ Skill::DoubleAttack,    "Double Attack" },
	{ Skill::DragonPunch,     "Dragon Punch" },
	{ Skill::DualWield,       "Dual Wield" },
	{ Skill::EagleStrike,     "Eagle Strike" },
	{ Skill::Evocation,       "Evocation" },
	{ Skill::FeignDeath,      "Feign Death" },
	{ Skill::FlyingKick,      "Flying Kick" },
	{ Skill::Forage,          "Forage" },
	{ Skill::HandToHand,      "Hand to Hand" },
	{ Skill::Hide,            "Hide" },
	{ Skill::Kick,            "Kick" },
	{ Skill::Meditate,        "Meditate" },
	{ Skill::Mend,            "Mend" },
	{ Skill::Offense,         "Offense" },
	{ Skill::Parry,           "Parry" },
	{ Skill::PickLock,        "Pick Lock" },
	{ Skill::OneHandPiercing, "1H Piercing" },
	{ Skill::Riposte,         "Riposte" },
	{ Skill::RoundKick,       "Round Kick" },
	{ Skill::SafeFall,              "Safe Fall" },
	{ Skill::SenseHeading,          "Sense Heading" },
	{ Skill::Singing,               "Singing" },
	{ Skill::Sneak,                 "Sneak" },
	{ Skill::SpecializeAbjuration,  "Specialize Abjuration" },
	{ Skill::SpecializeAlteration,  "Specialize Alteration" },
	{ Skill::SpecializeConjuration, "Specialize Conjuration" },
	{ Skill::SpecializeDivination,  "Specialize Divination" },
	{ Skill::SpecializeEvocation,   "Specialize Evocation" },
	{ Skill::PickPockets,           "Pick Pockets" },
	{ Skill::StringedInstruments,   "Stringed Instruments" },
	{ Skill::Swimming,              "Swimming" },
	{ Skill::Throwing,              "Throwing" },
	{ Skill::TigerClaw,             "Tiger Claw" },
	{ Skill::Tracking,              "Tracking" },
	{ Skill::WindInstruments,       "Wind Instruments" },
	{ Skill::Fishing,               "Fishing" },
	{ Skill::MakePoison,            "Make Poison" },
	{ Skill::Tinkering,             "Tinkering" },
	{ Skill::Research,              "Research" },
	{ Skill::Alchemy,               "Alchemy" },
	{ Skill::Baking,                "Baking" },
	{ Skill::Tailoring,             "Tailoring" },
	{ Skill::SenseTraps,            "Sense Traps" },
	{ Skill::Blacksmithing,         "Blacksmithing" },
	{ Skill::Fletching,             "Fletching" },
	{ Skill::Brewing,               "Brewing" },
	{ Skill::AlcoholTolerance,      "Alcohol Tolerance" },
	{ Skill::Begging,               "Begging" },
	{ Skill::JewelryMaking,         "Jewelry Making" },
	{ Skill::Pottery,               "Pottery" },
	{ Skill::PercussionInstruments, "Percussion Instruments" },
	{ Skill::Intimidation,          "Intimidation" },
	{ Skill::Berserking,            "Berserking" },
	{ Skill::Taunt,                 "Taunt" },
	{ Skill::Frenzy,                "Frenzy" },
	{ Skill::RemoveTraps,           "Remove Traps" },
	{ Skill::TripleAttack,          "Triple Attack" },
	{ Skill::TwoHandPiercing,       "2H Piercing" }
};


#endif /*COMMON_SKILLS_H*/
