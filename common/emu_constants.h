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

#ifndef COMMON_EMU_CONSTANTS_H
#define COMMON_EMU_CONSTANTS_H

#include "eq_limits.h"
#include "emu_versions.h"
#include "bodytypes.h"

#include <string.h>

namespace AccountStatus {
	constexpr uint8 Player          = 0;
	constexpr uint8 Steward         = 10;
	constexpr uint8 ApprenticeGuide = 20;
	constexpr uint8 Guide           = 50;
	constexpr uint8 QuestTroupe     = 80;
	constexpr uint8 SeniorGuide     = 81;
	constexpr uint8 GMTester        = 85;
	constexpr uint8 EQSupport       = 90;
	constexpr uint8 GMStaff         = 95;
	constexpr uint8 GMAdmin         = 100;
	constexpr uint8 GMLeadAdmin     = 150;
	constexpr uint8 QuestMaster     = 160;
	constexpr uint8 GMAreas         = 170;
	constexpr uint8 GMCoder         = 180;
	constexpr uint8 GMMgmt          = 200;
	constexpr uint8 GMImpossible    = 250;
	constexpr uint8 Max             = 255;

	std::string GetName(uint8 account_status);
}

static std::map<uint8, std::string> account_status_names = {
	{ AccountStatus::Player,          "Player" },
	{ AccountStatus::Steward,         "Steward" },
	{ AccountStatus::ApprenticeGuide, "Apprentice Guide" },
	{ AccountStatus::Guide,           "Guide" },
	{ AccountStatus::QuestTroupe,     "Quest Troupe" },
	{ AccountStatus::SeniorGuide,     "Senior Guide" },
	{ AccountStatus::GMTester,        "GM Tester" },
	{ AccountStatus::EQSupport,       "EQ Support" },
	{ AccountStatus::GMStaff,         "GM Staff" },
	{ AccountStatus::GMAdmin,         "GM Admin" },
	{ AccountStatus::GMLeadAdmin,     "GM Lead Admin" },
	{ AccountStatus::QuestMaster,     "Quest Master" },
	{ AccountStatus::GMAreas,         "GM Areas" },
	{ AccountStatus::GMCoder,         "GM Coder" },
	{ AccountStatus::GMMgmt,          "GM Mgmt" },
	{ AccountStatus::GMImpossible,    "GM Impossible" },
	{ AccountStatus::Max,             "GM Max" }
};

namespace ComparisonType {
	constexpr uint8 Equal          = 0;
	constexpr uint8 NotEqual       = 1;
	constexpr uint8 GreaterOrEqual = 2;
	constexpr uint8 LesserOrEqual  = 3;
	constexpr uint8 Greater        = 4;
	constexpr uint8 Lesser         = 5;
	constexpr uint8 Any            = 6;
	constexpr uint8 NotAny         = 7;
	constexpr uint8 Between        = 8;
	constexpr uint8 NotBetween     = 9;

	std::string GetName(uint8 type);
	bool IsValid(uint8 type);
}

static std::map<uint8, std::string> comparison_types = {
	{ ComparisonType::Equal,          "Equal" },
	{ ComparisonType::NotEqual,       "Not Equal" },
	{ ComparisonType::GreaterOrEqual, "Greater or Equal" },
	{ ComparisonType::LesserOrEqual,  "Lesser or Equal" },
	{ ComparisonType::Greater,        "Greater" },
	{ ComparisonType::Lesser,         "Lesser" },
	{ ComparisonType::Any,            "Any" },
	{ ComparisonType::NotAny,         "Not Any" },
	{ ComparisonType::Between,        "Between" },
	{ ComparisonType::NotBetween,     "Not Between" },
};

// local definitions are the result of using hybrid-client or server-only values and methods
namespace EQ
{
	using RoF2::IINVALID;
	using RoF2::INULL;

	namespace invtype {
		using namespace RoF2::invtype::enum_;

		using RoF2::invtype::POSSESSIONS_SIZE;
		using RoF2::invtype::BANK_SIZE;
		using RoF2::invtype::SHARED_BANK_SIZE;
		using RoF2::invtype::TRADE_SIZE;
		using RoF2::invtype::WORLD_SIZE;
		using RoF2::invtype::LIMBO_SIZE;
		using RoF2::invtype::TRIBUTE_SIZE;
		using RoF2::invtype::TROPHY_TRIBUTE_SIZE;
		using RoF2::invtype::GUILD_TRIBUTE_SIZE;
		using RoF2::invtype::MERCHANT_SIZE;
		using RoF2::invtype::DELETED_SIZE;
		using RoF2::invtype::CORPSE_SIZE;
		using RoF2::invtype::BAZAAR_SIZE;
		using RoF2::invtype::INSPECT_SIZE;
		using RoF2::invtype::REAL_ESTATE_SIZE;
		using RoF2::invtype::VIEW_MOD_PC_SIZE;
		using RoF2::invtype::VIEW_MOD_BANK_SIZE;
		using RoF2::invtype::VIEW_MOD_SHARED_BANK_SIZE;
		using RoF2::invtype::VIEW_MOD_LIMBO_SIZE;
		using RoF2::invtype::ALT_STORAGE_SIZE;
		using RoF2::invtype::ARCHIVED_SIZE;
		using RoF2::invtype::MAIL_SIZE;
		using RoF2::invtype::GUILD_TROPHY_TRIBUTE_SIZE;
		using RoF2::invtype::KRONO_SIZE;
		using RoF2::invtype::OTHER_SIZE;

		using Titanium::invtype::TRADE_NPC_SIZE;

		using RoF2::invtype::TYPE_INVALID;
		using RoF2::invtype::TYPE_BEGIN;
		using RoF2::invtype::TYPE_END;
		using RoF2::invtype::TYPE_COUNT;

		int16 GetInvTypeSize(int16 inv_type);
		using RoF2::invtype::GetInvTypeName;

	} // namespace invtype

	namespace DevTools {
		const int32 GM_ACCOUNT_STATUS_LEVEL = 150;
	}

	namespace popupresponse {
		const int32 SERVER_INTERNAL_USE_BASE = 2000000000;
		const int32 MOB_INFO_DISMISS         = 2000000001;
	}

	namespace invslot {
		using namespace RoF2::invslot::enum_;

		using RoF2::invslot::SLOT_INVALID;
		using RoF2::invslot::SLOT_BEGIN;

		using Titanium::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;

		const int16 SLOT_AUGMENT_GENERIC_RETURN = 1001; // clients don't appear to use this method... (internal inventory return value)

		using RoF2::invslot::POSSESSIONS_BEGIN;
		using RoF2::invslot::POSSESSIONS_END;
		using RoF2::invslot::POSSESSIONS_COUNT;

		using RoF2::invslot::EQUIPMENT_BEGIN;
		using RoF2::invslot::EQUIPMENT_END;
		using RoF2::invslot::EQUIPMENT_COUNT;

		using RoF2::invslot::GENERAL_BEGIN;
		using RoF2::invslot::GENERAL_END;
		using RoF2::invslot::GENERAL_COUNT;

		using RoF2::invslot::BONUS_BEGIN;
		using RoF2::invslot::BONUS_STAT_END;
		using RoF2::invslot::BONUS_SKILL_END;

		using Titanium::invslot::BANK_BEGIN;
		using SoF::invslot::BANK_END;

		using Titanium::invslot::SHARED_BANK_BEGIN;
		using Titanium::invslot::SHARED_BANK_END;

		using Titanium::invslot::TRADE_BEGIN;
		using Titanium::invslot::TRADE_END;

		using Titanium::invslot::TRADE_NPC_END;

		using Titanium::invslot::WORLD_BEGIN;
		using Titanium::invslot::WORLD_END;

		using Titanium::invslot::TRIBUTE_BEGIN;
		using Titanium::invslot::TRIBUTE_END;

		using Titanium::invslot::GUILD_TRIBUTE_BEGIN;
		using Titanium::invslot::GUILD_TRIBUTE_END;

		const int16 CORPSE_BEGIN = invslot::slotGeneral1;
		const int16 CORPSE_END = CORPSE_BEGIN + invslot::slotCursor;

		using RoF2::invslot::EQUIPMENT_BITMASK;
		using RoF2::invslot::GENERAL_BITMASK;
		using RoF2::invslot::CURSOR_BITMASK;
		using RoF2::invslot::POSSESSIONS_BITMASK;
		using RoF2::invslot::CORPSE_BITMASK;

		using RoF2::invslot::GetInvPossessionsSlotName;
		using RoF2::invslot::GetInvSlotName;

	} // namespace invslot

	namespace invbag {
		using Titanium::invbag::SLOT_INVALID;
		using Titanium::invbag::SLOT_BEGIN;
		using Titanium::invbag::SLOT_END;
		using Titanium::invbag::SLOT_COUNT;

		using Titanium::invbag::GENERAL_BAGS_BEGIN;
		const int16 GENERAL_BAGS_COUNT = invslot::GENERAL_COUNT * SLOT_COUNT;
		const int16 GENERAL_BAGS_END = (GENERAL_BAGS_BEGIN + GENERAL_BAGS_COUNT) - 1;

		const int16 GENERAL_BAGS_8_COUNT = 8 * SLOT_COUNT;
		const int16 GENERAL_BAGS_8_END = (GENERAL_BAGS_BEGIN + GENERAL_BAGS_8_COUNT) - 1;

		const int16 CURSOR_BAG_BEGIN = 351;
		const int16 CURSOR_BAG_COUNT = SLOT_COUNT;
		const int16 CURSOR_BAG_END = (CURSOR_BAG_BEGIN + CURSOR_BAG_COUNT) - 1;

		using Titanium::invbag::BANK_BAGS_BEGIN;
		const int16 BANK_BAGS_COUNT = (invtype::BANK_SIZE * SLOT_COUNT);
		const int16 BANK_BAGS_END = (BANK_BAGS_BEGIN + BANK_BAGS_COUNT) - 1;

		const int16 BANK_BAGS_16_COUNT = 16 * SLOT_COUNT;
		const int16 BANK_BAGS_16_END = (BANK_BAGS_BEGIN + BANK_BAGS_16_COUNT) - 1;

		using Titanium::invbag::SHARED_BANK_BAGS_BEGIN;
		const int16 SHARED_BANK_BAGS_COUNT = invtype::SHARED_BANK_SIZE * SLOT_COUNT;
		const int16 SHARED_BANK_BAGS_END = (SHARED_BANK_BAGS_BEGIN + SHARED_BANK_BAGS_COUNT) - 1;

		using Titanium::invbag::TRADE_BAGS_BEGIN;
		const int16 TRADE_BAGS_COUNT = invtype::TRADE_SIZE * SLOT_COUNT;
		const int16 TRADE_BAGS_END = (TRADE_BAGS_BEGIN + TRADE_BAGS_COUNT) - 1;

		using Titanium::invbag::GetInvBagIndexName;

	} // namespace invbag

	namespace invaug {
		using RoF2::invaug::SOCKET_INVALID;
		using RoF2::invaug::SOCKET_BEGIN;
		using RoF2::invaug::SOCKET_END;
		using RoF2::invaug::SOCKET_COUNT;

		using RoF2::invaug::GetInvAugIndexName;

	} // namespace invaug

	namespace constants {
		const EQ::versions::ClientVersion CHARACTER_CREATION_CLIENT = EQ::versions::ClientVersion::Titanium;

		using RoF2::constants::EXPANSION;
		using RoF2::constants::EXPANSION_BIT;
		using RoF2::constants::EXPANSIONS_MASK;

		using RoF2::constants::CHARACTER_CREATION_LIMIT;

		const size_t SAY_LINK_OPENER_SIZE = 1;
		using RoF2::constants::SAY_LINK_BODY_SIZE;
		const size_t SAY_LINK_TEXT_SIZE = 256; // this may be varied until it breaks something (tested:374) - the others are constant
		const size_t SAY_LINK_CLOSER_SIZE = 1;
		const size_t SAY_LINK_MAXIMUM_SIZE = (SAY_LINK_OPENER_SIZE + SAY_LINK_BODY_SIZE + SAY_LINK_TEXT_SIZE + SAY_LINK_CLOSER_SIZE);

		enum StanceType : int {
			stanceUnknown = 0,
			stancePassive,
			stanceBalanced,
			stanceEfficient,
			stanceReactive,
			stanceAggressive,
			stanceAssist,
			stanceBurn,
			stanceEfficient2,
			stanceBurnAE
		};

		enum BotSpellIDs : int {
			Warrior = 3001,
			Cleric,
			Paladin,
			Ranger,
			Shadowknight,
			Druid,
			Monk,
			Bard,
			Rogue,
			Shaman,
			Necromancer,
			Wizard,
			Magician,
			Enchanter,
			Beastlord,
			Berserker
		};

		enum GravityBehavior : int8 {
			Ground,
			Flying,
			Levitating,
			Water,
			Floating,
			LevitateWhileRunning
		};

		enum EnvironmentalDamage : uint8 {
			Lava = 250,
			Drowning,
			Falling,
			Trap
		};

		enum StuckBehavior : uint8 {
			RunToTarget,
			WarpToTarget,
			TakeNoAction,
			EvadeCombat
		};

		enum SpawnAnimations : uint8 {
			Standing,
			Sitting,
			Crouching,
			Laying,
			Looting
		};

		enum WeatherTypes : uint8 {
			None,
			Raining,
			Snowing
		};

		enum EmoteEventTypes : uint8 {
			LeaveCombat,
			EnterCombat,
			OnDeath,
			AfterDeath,
			Hailed,
			KilledPC,
			KilledNPC,
			OnSpawn,
			OnDespawn
		};

		enum EmoteTypes : uint8 {
			Say,
			Emote,
			Shout,
			Proximity
		};

		const char *GetStanceName(StanceType stance_type);
		int ConvertStanceTypeToIndex(StanceType stance_type);

		extern const std::map<uint8, std::string>& GetLanguageMap();
		std::string GetLanguageName(uint8 language_id);

		extern const std::map<uint32, std::string>& GetLDoNThemeMap();
		std::string GetLDoNThemeName(uint32 theme_id);

		extern const std::map<int8, std::string>& GetFlyModeMap();
		std::string GetFlyModeName(int8 flymode_id);

		extern const std::map<uint8, std::string>& GetConsiderLevelMap();
		std::string GetConsiderLevelName(uint8 consider_level);

		extern const std::map<uint8, std::string>& GetEnvironmentalDamageMap();
		std::string GetEnvironmentalDamageName(uint8 damage_type);

		extern const std::map<uint8, std::string>& GetStuckBehaviorMap();
		std::string GetStuckBehaviorName(uint8 behavior_id);

		extern const std::map<uint8, std::string>& GetSpawnAnimationMap();
		std::string GetSpawnAnimationName(uint8 animation_id);

		extern const std::map<uint8, std::string>& GetWeatherTypeMap();
		std::string GetWeatherTypeName(uint8 weather_type);

		extern const std::map<uint8, std::string>& GetEmoteEventTypeMap();
		std::string GetEmoteEventTypeName(uint8 emote_event_type);

		extern const std::map<uint8, std::string>& GetEmoteTypeMap();
		std::string GetEmoteTypeName(uint8 emote_type);

		extern const std::map<uint32, std::string>& GetAppearanceTypeMap();
		std::string GetAppearanceTypeName(uint32 animation_type);

		extern const std::map<uint32, std::string>& GetConsiderColorMap();
		std::string GetConsiderColorName(uint32 consider_color);

		const int STANCE_TYPE_FIRST = stancePassive;
		const int STANCE_TYPE_LAST = stanceBurnAE;
		const int STANCE_TYPE_COUNT = stanceBurnAE;

	} /*constants*/

	namespace profile {
		using RoF2::profile::BANDOLIERS_SIZE;
		using RoF2::profile::BANDOLIER_ITEM_COUNT;

		using RoF2::profile::POTION_BELT_SIZE;

		using RoF2::profile::SKILL_ARRAY_SIZE;

	} // namespace profile

	namespace behavior {
		using RoF2::behavior::CoinHasWeight;

	} // namespace behavior

	namespace spells {
		enum class CastingSlot : uint32 { // hybrid declaration
			Gem1 = 0,
			Gem2 = 1,
			Gem3 = 2,
			Gem4 = 3,
			Gem5 = 4,
			Gem6 = 5,
			Gem7 = 6,
			Gem8 = 7,
			Gem9 = 8,
			Gem10 = 9,
			Gem11 = 10,
			Gem12 = 11,
			MaxGems = 12,
			Ability = 20, // HT/LoH for Tit
			PotionBelt = 21, // Tit uses a different slot for PB
			Item = 22,
			Discipline = 23,
			AltAbility = 0xFF
		};

		using RoF2::spells::SPELL_ID_MAX;
		using RoF2::spells::SPELLBOOK_SIZE;
		using UF::spells::SPELL_GEM_COUNT; // RoF+ clients define more than UF client..but, they are not valid beyond UF

		using RoF2::spells::LONG_BUFFS;
		using RoF2::spells::SHORT_BUFFS;
		using RoF2::spells::DISC_BUFFS;
		using RoF2::spells::TOTAL_BUFFS;
		using RoF2::spells::NPC_BUFFS;
		using RoF2::spells::PET_BUFFS;
		using RoF2::spells::MERC_BUFFS;

	} // namespace spells

	enum WaypointStatus : int {
		RoamBoxPauseInProgress = -3,
		QuestControlNoGrid = -2,
		QuestControlGrid = -1
	};

	namespace consent {
		enum eConsentType : uint8 {
			Normal = 0,
			Group,
			Raid,
			Guild
		};
	}; // namespace consent
} /*EQEmu*/

enum ServerLockType : int {
	List,
	Lock,
	Unlock
};

enum Invisibility : uint8 {
	Visible,
	Invisible,
	Special = 255
};

enum AugmentActions : int {
	Insert,
	Remove,
	Swap,
	Destroy
};

enum ConsiderLevel : uint8 {
	Ally = 1,
	Warmly,
	Kindly,
	Amiably,
	Indifferently,
	Apprehensively,
	Dubiously,
	Threateningly,
	Scowls
};

namespace ConsiderColor {
	constexpr uint32 Green         = 2;
	constexpr uint32 DarkBlue      = 4;
	constexpr uint32 Gray          = 6;
	constexpr uint32 White         = 10;
	constexpr uint32 Red           = 13;
	constexpr uint32 Yellow        = 15;
	constexpr uint32 LightBlue     = 18;
	constexpr uint32 WhiteTitanium = 20;
};

enum TargetDescriptionType : uint8 {
	LCSelf,
	UCSelf,
	LCYou,
	UCYou,
	LCYour,
	UCYour
};

enum ReloadWorld : uint8 {
	NoRepop = 0,
	Repop,
	ForceRepop
};

enum class EntityFilterType {
	All,
	Bots,
	Clients,
	NPCs
};

enum class ApplySpellType {
	Solo,
	Group,
	Raid
};

namespace SpecialAbility {
	constexpr int Summon                     = 1;
	constexpr int Enrage                     = 2;
	constexpr int Rampage                    = 3;
	constexpr int AreaRampage                = 4;
	constexpr int Flurry                     = 5;
	constexpr int TripleAttack               = 6;
	constexpr int QuadrupleAttack            = 7;
	constexpr int DualWield                  = 8;
	constexpr int BaneAttack                 = 9;
	constexpr int MagicalAttack              = 10;
	constexpr int RangedAttack               = 11;
	constexpr int SlowImmunity               = 12;
	constexpr int MesmerizeImmunity          = 13;
	constexpr int CharmImmunity              = 14;
	constexpr int StunImmunity               = 15;
	constexpr int SnareImmunity              = 16;
	constexpr int FearImmunity               = 17;
	constexpr int DispellImmunity            = 18;
	constexpr int MeleeImmunity              = 19;
	constexpr int MagicImmunity              = 20;
	constexpr int FleeingImmunity            = 21;
	constexpr int MeleeImmunityExceptBane    = 22;
	constexpr int MeleeImmunityExceptMagical = 23;
	constexpr int AggroImmunity              = 24;
	constexpr int BeingAggroImmunity         = 25;
	constexpr int CastingFromRangeImmunity   = 26;
	constexpr int FeignDeathImmunity         = 27;
	constexpr int TauntImmunity              = 28;
	constexpr int TunnelVision               = 29;
	constexpr int NoBuffHealFriends          = 30;
	constexpr int PacifyImmunity             = 31;
	constexpr int Leash                      = 32;
	constexpr int Tether                     = 33;
	constexpr int DestructibleObject         = 34;
	constexpr int HarmFromClientImmunity     = 35;
	constexpr int AlwaysFlee                 = 36;
	constexpr int FleePercent                = 37;
	constexpr int AllowBeneficial            = 38;
	constexpr int DisableMelee               = 39;
	constexpr int NPCChaseDistance           = 40;
	constexpr int AllowedToTank              = 41;
	constexpr int IgnoreRootAggroRules       = 42;
	constexpr int CastingResistDifficulty    = 43;
	constexpr int CounterAvoidDamage         = 44;
	constexpr int ProximityAggro             = 45;
	constexpr int RangedAttackImmunity       = 46;
	constexpr int ClientDamageImmunity       = 47;
	constexpr int NPCDamageImmunity          = 48;
	constexpr int ClientAggroImmunity        = 49;
	constexpr int NPCAggroImmunity           = 50;
	constexpr int ModifyAvoidDamage          = 51;
	constexpr int MemoryFadeImmunity         = 52;
	constexpr int OpenImmunity               = 53;
	constexpr int AssassinateImmunity        = 54;
	constexpr int HeadshotImmunity           = 55;
	constexpr int BotAggroImmunity           = 56;
	constexpr int BotDamageImmunity          = 57;
	constexpr int Max                        = 58;

	constexpr int MaxParameters = 9;

	std::string GetName(int ability_id);
	bool IsValid(int ability_id);
}

static std::map<int, std::string> special_ability_names = {
	{ SpecialAbility::Summon,                     "Summon" },
	{ SpecialAbility::Enrage,                     "Enrage" },
	{ SpecialAbility::Rampage,                    "Rampage" },
	{ SpecialAbility::AreaRampage,                "Area Rampage" },
	{ SpecialAbility::Flurry,                     "Flurry" },
	{ SpecialAbility::TripleAttack,               "Triple Attack" },
	{ SpecialAbility::QuadrupleAttack,            "Quadruple Attack" },
	{ SpecialAbility::DualWield,                  "Dual Wield" },
	{ SpecialAbility::BaneAttack,                 "Bane Attack" },
	{ SpecialAbility::MagicalAttack,              "Magical Attack" },
	{ SpecialAbility::RangedAttack,               "Ranged Attack" },
	{ SpecialAbility::SlowImmunity,               "Immune to Slow" },
	{ SpecialAbility::MesmerizeImmunity,          "Immune to Mesmerize" },
	{ SpecialAbility::CharmImmunity,              "Immune to Charm" },
	{ SpecialAbility::StunImmunity,               "Immune to Stun" },
	{ SpecialAbility::SnareImmunity,              "Immune to Snare" },
	{ SpecialAbility::FearImmunity,               "Immune to Fear" },
	{ SpecialAbility::DispellImmunity,            "Immune to Dispell" },
	{ SpecialAbility::MeleeImmunity,              "Immune to Melee" },
	{ SpecialAbility::MagicImmunity,              "Immune to Magic" },
	{ SpecialAbility::FleeingImmunity,            "Immune to Fleeing" },
	{ SpecialAbility::MeleeImmunityExceptBane,    "Immune to Melee except Bane" },
	{ SpecialAbility::MeleeImmunityExceptMagical, "Immune to Non-Magical Melee" },
	{ SpecialAbility::AggroImmunity,              "Immune to Aggro" },
	{ SpecialAbility::BeingAggroImmunity,         "Immune to Being Aggro" },
	{ SpecialAbility::CastingFromRangeImmunity,   "Immune to Ranged Spells" },
	{ SpecialAbility::FeignDeathImmunity,         "Immune to Feign Death" },
	{ SpecialAbility::TauntImmunity,              "Immune to Taunt" },
	{ SpecialAbility::TunnelVision,               "Tunnel Vision" },
	{ SpecialAbility::NoBuffHealFriends,          "Does Not Heal or Buff Allies" },
	{ SpecialAbility::PacifyImmunity,             "Immune to Pacify" },
	{ SpecialAbility::Leash,                      "Leashed" },
	{ SpecialAbility::Tether,                     "Tethered" },
	{ SpecialAbility::DestructibleObject,         "Destructible Object" },
	{ SpecialAbility::HarmFromClientImmunity,     "Immune to Harm from Client" },
	{ SpecialAbility::AlwaysFlee,                 "Always Flees" },
	{ SpecialAbility::FleePercent,                "Flee Percentage" },
	{ SpecialAbility::AllowBeneficial,            "Allows Beneficial Spells" },
	{ SpecialAbility::DisableMelee,               "Melee is Disabled" },
	{ SpecialAbility::NPCChaseDistance,           "Chase Distance" },
	{ SpecialAbility::AllowedToTank,              "Allowed to Tank" },
	{ SpecialAbility::IgnoreRootAggroRules,       "Ignores Root Aggro" },
	{ SpecialAbility::CastingResistDifficulty,    "Casting Resist Difficulty" },
	{ SpecialAbility::CounterAvoidDamage,         "Counter Damage Avoidance" },
	{ SpecialAbility::ProximityAggro,             "Proximity Aggro" },
	{ SpecialAbility::RangedAttackImmunity,       "Immune to Ranged Attacks" },
	{ SpecialAbility::ClientDamageImmunity,       "Immune to Client Damage" },
	{ SpecialAbility::NPCDamageImmunity,          "Immune to NPC Damage" },
	{ SpecialAbility::ClientAggroImmunity,        "Immune to Client Aggro" },
	{ SpecialAbility::NPCAggroImmunity,           "Immune to NPC Aggro" },
	{ SpecialAbility::ModifyAvoidDamage,          "Modify Damage Avoidance" },
	{ SpecialAbility::MemoryFadeImmunity,         "Immune to Memory Fades" },
	{ SpecialAbility::OpenImmunity,               "Immune to Open" },
	{ SpecialAbility::AssassinateImmunity,        "Immune to Assassinate" },
	{ SpecialAbility::HeadshotImmunity,           "Immune to Headshot" },
	{ SpecialAbility::BotAggroImmunity,           "Immune to Bot Aggro" },
	{ SpecialAbility::BotDamageImmunity,          "Immune to Bot Damage" },
};

namespace HeroicBonusBucket
{
	const std::string WisMaxMana 			= "HWIS-MaxMana";
	const std::string WisManaRegen			= "HWIS-ManaRegen";
	const std::string WisHealAmt			= "HWIS-HealAmt";
	const std::string IntMaxMana			= "HINT-MaxMana";
	const std::string IntManaRegen			= "HINT-ManaRegen";
	const std::string IntSpellDmg			= "HINT-SpellDmg";
	const std::string StrMeleeDamage		= "HSTR-MeleeDamage";
	const std::string StrShieldAC			= "HSTR-ShieldAC";
	const std::string StrMaxEndurance		= "HSTR-MaxEndurance";
	const std::string StrEnduranceRegen		= "HSTR-EnduranceRegen";
	const std::string StaMaxHP			= "HSTA-MaxHP";
	const std::string StaHPRegen			= "HSTA-HPRegen";
	const std::string StaMaxEndurance		= "HSTA-MaxEndurance";
	const std::string StaEnduranceRegen		= "HSTA-EnduranceRegen";
	const std::string AgiAvoidance			= "HAGI-Avoidance";
	const std::string AgiMaxEndurance		= "HAGI-MaxEndurance";
	const std::string AgiEnduranceRegen		= "HAGI-EnduranceRegen";
	const std::string DexRangedDamage		= "HDEX-RangedDamage";
	const std::string DexMaxEndurance		= "HDEX-MaxEndurance";
	const std::string DexEnduranceRegen		= "HDEX-EnduranceRegen";
}

namespace Bug {
	namespace Category {
		constexpr uint32 Other         = 0;
		constexpr uint32 Video         = 1;
		constexpr uint32 Audio         = 2;
		constexpr uint32 Pathing       = 3;
		constexpr uint32 Quest         = 4;
		constexpr uint32 Tradeskills   = 5;
		constexpr uint32 SpellStacking = 6;
		constexpr uint32 DoorsPortals  = 7;
		constexpr uint32 Items         = 8;
		constexpr uint32 NPC           = 9;
		constexpr uint32 Dialogs       = 10;
		constexpr uint32 LoNTCG        = 11;
		constexpr uint32 Mercenaries   = 12;
	}

	namespace InformationFlag {
		constexpr uint32 None           = 0;
		constexpr uint32 Repeatable     = 1;
		constexpr uint32 Crash          = 2;
		constexpr uint32 TargetInfo     = 4;
		constexpr uint32 CharacterFlags = 8;
		constexpr uint32 Unknown        = 4294967280;
	}

	uint32 GetID(const std::string& category_name);
	std::string GetName(uint32 category_id);
	bool IsValid(uint32 category_id);
}

static std::map<uint32, std::string> bug_category_names = {
	{ Bug::Category::Other,         "Other" },
	{ Bug::Category::Video,         "Video" },
	{ Bug::Category::Audio,         "Audio" },
	{ Bug::Category::Pathing,       "Pathing" },
	{ Bug::Category::Quest,         "Quest" },
	{ Bug::Category::Tradeskills,   "Tradeskills" },
	{ Bug::Category::SpellStacking, "Spell Stacking" },
	{ Bug::Category::DoorsPortals,  "Doors and Portals" },
	{ Bug::Category::Items,         "Items" },
	{ Bug::Category::NPC,           "NPC" },
	{ Bug::Category::Dialogs,       "Dialogs" },
	{ Bug::Category::LoNTCG,        "LoN - TCG" },
	{ Bug::Category::Mercenaries,   "Mercenaries" }
};

#endif /*COMMON_EMU_CONSTANTS_H*/
