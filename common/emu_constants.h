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

		using RoF2::invtype::TRADE_NPC_SIZE;

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

		using RoF2::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;

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

		using RoF2::invslot::BANK_BEGIN;
		using RoF2::invslot::BANK_END;

		using RoF2::invslot::SHARED_BANK_BEGIN;
		using RoF2::invslot::SHARED_BANK_END;

		using RoF2::invslot::TRADE_BEGIN;
		using RoF2::invslot::TRADE_END;

		using RoF2::invslot::TRADE_NPC_END;

		using RoF2::invslot::WORLD_BEGIN;
		using RoF2::invslot::WORLD_END;

		using RoF2::invslot::TRIBUTE_BEGIN;
		using RoF2::invslot::TRIBUTE_END;

		using RoF2::invslot::GUILD_TRIBUTE_BEGIN;
		using RoF2::invslot::GUILD_TRIBUTE_END;

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
		using RoF2::invbag::SLOT_INVALID;
		using RoF2::invbag::SLOT_BEGIN;
		using RoF2::invbag::SLOT_END;
		using RoF2::invbag::SLOT_COUNT;

		using RoF2::invslot::WORLD_END;

		const int16 GENERAL_BAGS_BEGIN = WORLD_END + 1;
		const int16 GENERAL_BAGS_COUNT = invslot::GENERAL_COUNT * SLOT_COUNT;
		const int16 GENERAL_BAGS_END = (GENERAL_BAGS_BEGIN + GENERAL_BAGS_COUNT) - 1;

		const int16 GENERAL_BAGS_8_COUNT = 8 * SLOT_COUNT;
		const int16 GENERAL_BAGS_8_END = (GENERAL_BAGS_BEGIN + GENERAL_BAGS_8_COUNT) - 1;

		const int16 CURSOR_BAG_BEGIN = GENERAL_BAGS_END + 1;
		const int16 CURSOR_BAG_COUNT = SLOT_COUNT;
		const int16 CURSOR_BAG_END = (CURSOR_BAG_BEGIN + CURSOR_BAG_COUNT) - 1;

		const int16 BANK_BAGS_BEGIN = CURSOR_BAG_END + 1;
		const int16 BANK_BAGS_COUNT = (invtype::BANK_SIZE * SLOT_COUNT);
		const int16 BANK_BAGS_END = (BANK_BAGS_BEGIN + BANK_BAGS_COUNT) - 1;

		const int16 BANK_BAGS_16_COUNT = 16 * SLOT_COUNT;
		const int16 BANK_BAGS_16_END = (BANK_BAGS_BEGIN + BANK_BAGS_16_COUNT) - 1;

		const int16 SHARED_BANK_BAGS_BEGIN = BANK_BAGS_END + 1;
		const int16 SHARED_BANK_BAGS_COUNT = invtype::SHARED_BANK_SIZE * SLOT_COUNT;
		const int16 SHARED_BANK_BAGS_END = (SHARED_BANK_BAGS_BEGIN + SHARED_BANK_BAGS_COUNT) - 1;

		const int16 TRADE_BAGS_BEGIN = SHARED_BANK_BAGS_END + 1;
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

		enum ObjectTypes : int {
			SmallBag,
			LargeBag,
			Quiver,
			BeltPouch,
			WristPouch,
			Backpack,
			SmallChest,
			LargeChest,
			Bandolier,
			Medicine,
			Tinkering,
			Lexicon,
			PoisonMaking,
			Quest,
			MixingBowl,
			Baking,
			Tailoring,
			Blacksmithing,
			Fletching,
			Brewing,
			JewelryMaking,
			Pottery,
			Kiln,
			KeyMaker,
			ResearchWIZ,
			ResearchMAG,
			ResearchNEC,
			ResearchENC,
			Unknown,
			ResearchPractice,
			Alchemy,
			HighElfForge,
			DarkElfForge,
			OgreForge,
			DwarfForge,
			GnomeForge,
			BarbarianForge,
			IksarForge,
			HumanForgeOne,
			HumanForgeTwo,
			HalflingTailoringOne,
			HalflingTailoringTwo,
			EruditeTailoring,
			WoodElfTailoring,
			WoodElfFletching,
			IksarPottery,
			Fishing,
			TrollForge,
			WoodElfForge,
			HalflingForge,
			EruditeForge,
			Merchant,
			FroglokForge,
			Augmenter,
			Churn,
			TransformationMold,
			DetransformationMold,
			Unattuner,
			TradeskillBag,
			CollectibleBag,
			NoDeposit
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

		extern const std::map<bodyType, std::string>& GetBodyTypeMap();
		std::string GetBodyTypeName(bodyType bodytype_id);

		extern const std::map<uint8, std::string>& GetAccountStatusMap();
		std::string GetAccountStatusName(uint8 account_status);

		extern const std::map<uint8, std::string>& GetConsiderLevelMap();
		std::string GetConsiderLevelName(uint8 consider_level);

		extern const std::map<uint8, std::string>& GetEnvironmentalDamageMap();
		std::string GetEnvironmentalDamageName(uint8 damage_type);

		extern const std::map<uint8, std::string>& GetStuckBehaviorMap();
		std::string GetStuckBehaviorName(uint8 behavior_id);

		extern const std::map<uint8, std::string>& GetSpawnAnimationMap();
		std::string GetSpawnAnimationName(uint8 animation_id);

		extern const std::map<int, std::string>& GetObjectTypeMap();
		std::string GetObjectTypeName(int object_type);

		extern const std::map<uint8, std::string>& GetWeatherTypeMap();
		std::string GetWeatherTypeName(uint8 weather_type);

		extern const std::map<uint8, std::string>& GetEmoteEventTypeMap();
		std::string GetEmoteEventTypeName(uint8 emote_event_type);

		extern const std::map<uint8, std::string>& GetEmoteTypeMap();
		std::string GetEmoteTypeName(uint8 emote_type);

		extern const std::map<uint32, std::string>& GetAppearanceTypeMap();
		std::string GetAppearanceTypeName(uint32 animation_type);

		extern const std::map<uint32, std::string>& GetSpecialAbilityMap();
		std::string GetSpecialAbilityName(uint32 ability_id);

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

	namespace bug {
		enum CategoryID : uint32 {
			catOther = 0,
			catVideo,
			catAudio,
			catPathing,
			catQuest,
			catTradeskills,
			catSpellStacking,
			catDoorsPortals,
			catItems,
			catNPC,
			catDialogs,
			catLoNTCG,
			catMercenaries
		};

		enum OptionalInfoFlag : uint32 {
			infoNoOptionalInfo = 0x0,
			infoCanDuplicate = 0x1,
			infoCrashBug = 0x2,
			infoTargetInfo = 0x4,
			infoCharacterFlags = 0x8,
			infoUnknownValue = 0xFFFFFFF0
		};

		const char* CategoryIDToCategoryName(CategoryID category_id);
		CategoryID CategoryNameToCategoryID(const char* category_name);

	} // namespace bug

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

enum AccountStatus : uint8 {
	Player = 0,
	Steward = 10,
	ApprenticeGuide = 20,
	Guide = 50,
	QuestTroupe = 80,
	SeniorGuide = 81,
	GMTester = 85,
	EQSupport = 90,
	GMStaff = 95,
	GMAdmin = 100,
	GMLeadAdmin = 150,
	QuestMaster = 160,
	GMAreas = 170,
	GMCoder = 180,
	GMMgmt = 200,
	GMImpossible = 250,
	Max = 255
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

enum BucketComparison : uint8 {
	BucketEqualTo = 0,
	BucketNotEqualTo,
	BucketGreaterThanOrEqualTo,
	BucketLesserThanOrEqualTo,
	BucketGreaterThan,
	BucketLesserThan,
	BucketIsAny,
	BucketIsNotAny,
	BucketIsBetween,
	BucketIsNotBetween
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

enum {
	SPECATK_SUMMON            = 1,
	SPECATK_ENRAGE            = 2,
	SPECATK_RAMPAGE           = 3,
	SPECATK_AREA_RAMPAGE      = 4,
	SPECATK_FLURRY            = 5,
	SPECATK_TRIPLE            = 6,
	SPECATK_QUAD              = 7,
	SPECATK_INNATE_DW         = 8,
	SPECATK_BANE              = 9,
	SPECATK_MAGICAL           = 10,
	SPECATK_RANGED_ATK        = 11,
	UNSLOWABLE                = 12,
	UNMEZABLE                 = 13,
	UNCHARMABLE               = 14,
	UNSTUNABLE                = 15,
	UNSNAREABLE               = 16,
	UNFEARABLE                = 17,
	UNDISPELLABLE             = 18,
	IMMUNE_MELEE              = 19,
	IMMUNE_MAGIC              = 20,
	IMMUNE_FLEEING            = 21,
	IMMUNE_MELEE_EXCEPT_BANE  = 22,
	IMMUNE_MELEE_NONMAGICAL   = 23,
	IMMUNE_AGGRO              = 24,
	IMMUNE_AGGRO_ON           = 25,
	IMMUNE_CASTING_FROM_RANGE = 26,
	IMMUNE_FEIGN_DEATH        = 27,
	IMMUNE_TAUNT              = 28,
	NPC_TUNNELVISION          = 29,
	NPC_NO_BUFFHEAL_FRIENDS   = 30,
	IMMUNE_PACIFY             = 31,
	LEASH                     = 32,
	TETHER                    = 33,
	DESTRUCTIBLE_OBJECT       = 34,
	NO_HARM_FROM_CLIENT       = 35,
	ALWAYS_FLEE               = 36,
	FLEE_PERCENT              = 37,
	ALLOW_BENEFICIAL          = 38,
	DISABLE_MELEE             = 39,
	NPC_CHASE_DISTANCE        = 40,
	ALLOW_TO_TANK             = 41,
	IGNORE_ROOT_AGGRO_RULES   = 42,
	CASTING_RESIST_DIFF       = 43,
	COUNTER_AVOID_DAMAGE      = 44, // Modify by percent NPC's opponents chance to riposte, block, parry or dodge individually, or for all skills
	PROX_AGGRO                = 45,
	IMMUNE_RANGED_ATTACKS     = 46,
	IMMUNE_DAMAGE_CLIENT      = 47,
	IMMUNE_DAMAGE_NPC         = 48,
	IMMUNE_AGGRO_CLIENT       = 49,
	IMMUNE_AGGRO_NPC          = 50,
	MODIFY_AVOID_DAMAGE       = 51, // Modify by percent the NPCs chance to riposte, block, parry or dodge individually, or for all skills
	IMMUNE_FADING_MEMORIES    = 52,
	IMMUNE_OPEN               = 53,
	IMMUNE_ASSASSINATE        = 54,
	IMMUNE_HEADSHOT           = 55,
	IMMUNE_AGGRO_BOT          = 56,
	IMMUNE_DAMAGE_BOT         = 57,
	MAX_SPECIAL_ATTACK        = 58
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

#endif /*COMMON_EMU_CONSTANTS_H*/
