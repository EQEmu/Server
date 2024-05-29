/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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
#ifndef CLASSES_CH
#define CLASSES_CH

#include "../common/types.h"
#include "../common/rulesys.h"
#include <string>
#include <map>
#include <fmt/format.h>

namespace Class {
	constexpr uint8 None                      = 0;
	constexpr uint8 Warrior                   = 1;
	constexpr uint8 Cleric                    = 2;
	constexpr uint8 Paladin                   = 3;
	constexpr uint8 Ranger                    = 4;
	constexpr uint8 ShadowKnight              = 5;
	constexpr uint8 Druid                     = 6;
	constexpr uint8 Monk                      = 7;
	constexpr uint8 Bard                      = 8;
	constexpr uint8 Rogue                     = 9;
	constexpr uint8 Shaman                    = 10;
	constexpr uint8 Necromancer               = 11;
	constexpr uint8 Wizard                    = 12;
	constexpr uint8 Magician                  = 13;
	constexpr uint8 Enchanter                 = 14;
	constexpr uint8 Beastlord                 = 15;
	constexpr uint8 Berserker                 = 16;
	constexpr uint8 WarriorGM                 = 20;
	constexpr uint8 ClericGM                  = 21;
	constexpr uint8 PaladinGM                 = 22;
	constexpr uint8 RangerGM                  = 23;
	constexpr uint8 ShadowKnightGM            = 24;
	constexpr uint8 DruidGM                   = 25;
	constexpr uint8 MonkGM                    = 26;
	constexpr uint8 BardGM                    = 27;
	constexpr uint8 RogueGM                   = 28;
	constexpr uint8 ShamanGM                  = 29;
	constexpr uint8 NecromancerGM             = 30;
	constexpr uint8 WizardGM                  = 31;
	constexpr uint8 MagicianGM                = 32;
	constexpr uint8 EnchanterGM               = 33;
	constexpr uint8 BeastlordGM               = 34;
	constexpr uint8 BerserkerGM               = 35;
	constexpr uint8 Banker                    = 40;
	constexpr uint8 Merchant                  = 41;
	constexpr uint8 DiscordMerchant           = 59;
	constexpr uint8 AdventureRecruiter        = 60;
	constexpr uint8 AdventureMerchant         = 61;
	constexpr uint8 LDoNTreasure              = 62;
	constexpr uint8 TributeMaster             = 63;
	constexpr uint8 GuildTributeMaster        = 64;
	constexpr uint8 GuildBanker               = 66;
	constexpr uint8 NorrathsKeepersMerchant   = 67;
	constexpr uint8 DarkReignMerchant         = 68;
	constexpr uint8 FellowshipMaster          = 69;
	constexpr uint8 AlternateCurrencyMerchant = 70;
	constexpr uint8 MercenaryLiaison          = 71;
};

namespace PlayerClassBitmask {
	constexpr uint16 Unknown      = 0;
	constexpr uint16 Warrior      = 1;
	constexpr uint16 Cleric       = 2;
	constexpr uint16 Paladin      = 4;
	constexpr uint16 Ranger       = 8;
	constexpr uint16 ShadowKnight = 16;
	constexpr uint16 Druid        = 32;
	constexpr uint16 Monk         = 64;
	constexpr uint16 Bard         = 128;
	constexpr uint16 Rogue        = 256;
	constexpr uint16 Shaman       = 512;
	constexpr uint16 Necromancer  = 1024;
	constexpr uint16 Wizard       = 2048;
	constexpr uint16 Magician     = 4096;
	constexpr uint16 Enchanter    = 8192;
	constexpr uint16 Beastlord    = 16384;
	constexpr uint16 Berserker    = 32768;
	constexpr uint16 All          = 65535;
};

static std::map<uint8, uint16> player_class_bitmasks = {
	{ Class::Warrior,      PlayerClassBitmask::Warrior },
	{ Class::Cleric,       PlayerClassBitmask::Cleric },
	{ Class::Paladin,      PlayerClassBitmask::Paladin },
	{ Class::Ranger,       PlayerClassBitmask::Ranger },
	{ Class::ShadowKnight, PlayerClassBitmask::ShadowKnight },
	{ Class::Druid,        PlayerClassBitmask::Druid },
	{ Class::Monk,         PlayerClassBitmask::Monk },
	{ Class::Bard,         PlayerClassBitmask::Bard },
	{ Class::Rogue,        PlayerClassBitmask::Rogue },
	{ Class::Shaman,       PlayerClassBitmask::Shaman },
	{ Class::Necromancer,  PlayerClassBitmask::Necromancer },
	{ Class::Wizard,       PlayerClassBitmask::Wizard },
	{ Class::Magician,     PlayerClassBitmask::Magician },
	{ Class::Enchanter,    PlayerClassBitmask::Enchanter },
	{ Class::Beastlord,    PlayerClassBitmask::Beastlord },
	{ Class::Berserker,    PlayerClassBitmask::Berserker },
};

static std::string shadow_knight_class_name = (
	RuleB(World, UseOldShadowKnightClassExport) ?
	"Shadowknight" :
	"Shadow Knight"
);

static std::map<uint8, std::string> player_class_abbreviations = {
	{ Class::Warrior,      "WAR" },
	{ Class::Cleric,       "CLR" },
	{ Class::Paladin,      "PAL" },
	{ Class::Ranger,       "RNG" },
	{ Class::ShadowKnight, "SHD" },
	{ Class::Druid,        "DRU" },
	{ Class::Monk,         "MNK" },
	{ Class::Bard,         "BRD" },
	{ Class::Rogue,        "ROG" },
	{ Class::Shaman,       "SHM" },
	{ Class::Necromancer,  "NEC" },
	{ Class::Wizard,       "WIZ" },
	{ Class::Magician,     "MAG" },
	{ Class::Enchanter,    "ENC" },
	{ Class::Beastlord,    "BST" },
	{ Class::Berserker,    "BER" },
};

static std::map<uint8, std::vector<std::string>> player_class_level_names = {
	{ Class::Warrior,      { "Champion",     "Myrmidon",   "Warlord",     "Overlord",       "Vanquisher",     "Imperator" }},
	{ Class::Cleric,       { "Vicar",        "Templar",    "High Priest", "Archon",         "Prelate",        "Exemplar" }},
	{ Class::Paladin,      { "Cavalier",     "Knight",     "Crusader",    "Lord Protector", "Lord",           "Holy Defender" }},
	{ Class::Ranger,       { "Pathfinder",   "Outrider",   "Warder",      "Forest Stalker", "Plainswalker",   "Huntmaster" }},
	{ Class::ShadowKnight, { "Reaver",       "Revenant",   "Grave Lord",  "Dread Lord",     "Scourge Knight", "Bloodreaver" }},
	{ Class::Druid,        { "Wanderer",     "Preserver",  "Hierophant",  "Storm Warden",   "Natureguard",    "Storm Caller" }},
	{ Class::Monk,         { "Disciple",     "Master",     "Grandmaster", "Transcendent",   "Stone Fist",     "Ashenhand" }},
	{ Class::Bard,         { "Minstrel",     "Troubadour", "Virtuoso",    "Maestro",        "Performer",      "Lyricist" }},
	{ Class::Rogue,        { "Rake",         "Blackguard", "Assassin",    "Deceiver",       "Nemesis",        "Shadowblade" }},
	{ Class::Shaman,       { "Mystic",       "Luminary",   "Oracle",      "Prophet",        "Soothsayer",     "Spiritwatcher" }},
	{ Class::Necromancer,  { "Heretic",      "Defiler",    "Warlock",     "Arch Lich",      "Wraith",         "Deathcaller" }},
	{ Class::Wizard,       { "Channeler",    "Evoker",     "Sorcerer",    "Arcanist",       "Grand Arcanist", "Pyromancer" }},
	{ Class::Magician,     { "Elementalist", "Conjurer",   "Arch Mage",   "Arch Convoker",  "Arch Magus",     "Grand Summoner" }},
	{ Class::Enchanter,    { "Illusionist",  "Beguiler",   "Phantasmist", "Coercer",        "Bedazzler",      "Entrancer" }},
	{ Class::Beastlord,    { "Primalist",    "Animist",    "Savage Lord", "Feral Lord",     "Wildblood",      "Frostblood" }},
	{ Class::Berserker,    { "Brawler",      "Vehement",   "Rager",       "Fury",           "Ravager",        "Juggernaut" }},
};

static std::map<uint8, std::string> class_map = {
	{ Class::Warrior,                   "Warrior" },
	{ Class::Cleric,                    "Cleric" },
	{ Class::Paladin,                   "Paladin" },
	{ Class::Ranger,                    "Ranger" },
	{ Class::ShadowKnight,              shadow_knight_class_name },
	{ Class::Druid,                     "Druid" },
	{ Class::Monk,                      "Monk" },
	{ Class::Bard,                      "Bard" },
	{ Class::Rogue,                     "Rogue" },
	{ Class::Shaman,                    "Shaman" },
	{ Class::Necromancer,               "Necromancer" },
	{ Class::Wizard,                    "Wizard" },
	{ Class::Magician,                  "Magician" },
	{ Class::Enchanter,                 "Enchanter" },
	{ Class::Beastlord,                 "Beastlord" },
	{ Class::Berserker,                 "Berserker" },
	{ Class::Banker,                    "Banker" },
	{ Class::WarriorGM,                 "Warrior Guildmaster" },
	{ Class::ClericGM,                  "Cleric Guildmaster" },
	{ Class::PaladinGM,                 "Paladin Guildmaster" },
	{ Class::RangerGM,                  "Ranger Guildmaster" },
	{ Class::ShadowKnightGM,            fmt::format("{} Guildmaster", shadow_knight_class_name) },
	{ Class::DruidGM,                   "Druid Guildmaster" },
	{ Class::MonkGM,                    "Monk Guildmaster" },
	{ Class::BardGM,                    "Bard Guildmaster" },
	{ Class::BardGM,                    "Bard Guildmaster" },
	{ Class::ShamanGM,                  "Shaman Guildmaster" },
	{ Class::NecromancerGM,             "Necromancer Guildmaster" },
	{ Class::WizardGM,                  "Wizard Guildmaster" },
	{ Class::MagicianGM,                "Magician Guildmaster" },
	{ Class::EnchanterGM,               "Enchanter Guildmaster" },
	{ Class::BeastlordGM,               "Beastlord Guildmaster" },
	{ Class::BerserkerGM,               "Berserker Guildmaster" },
	{ Class::Merchant,                  "Merchant" },
	{ Class::DiscordMerchant,           "Discord Merchant" },
	{ Class::AdventureRecruiter,        "Adventure Recruiter" },
	{ Class::AdventureMerchant,         "Adventure Merchant" },
	{ Class::LDoNTreasure,              "LDoN Treasure" },
	{ Class::TributeMaster,             "Tribute Master" },
	{ Class::GuildTributeMaster,        "Guild Tribute Master" },
	{ Class::GuildBanker,               "Guild Banker" },
	{ Class::NorrathsKeepersMerchant,   "Radiant Crystal Merchant" },
	{ Class::DarkReignMerchant,         "Ebon Crystal Merchant" },
	{ Class::FellowshipMaster,          "Fellowship Master" },
	{ Class::AlternateCurrencyMerchant, "Alternate Currency Merchant" },
	{ Class::MercenaryLiaison,          "Mercenary Liaison" }
};

namespace ArmorType {
	constexpr uint8 Unknown = 0;
	constexpr uint8 Cloth   = 1;
	constexpr uint8 Leather = 2;
	constexpr uint8 Chain   = 3;
	constexpr uint8 Plate   = 4;
};

namespace EQ {
	namespace classes {
		const std::string& GetClassName(uint8 class_id, uint8 level = 0);
		const std::string& GetClassLevelName(uint8 class_id, uint8 level = 0);
		const std::string GetPlayerClassAbbreviation(uint8 class_id);

		uint8 GetClassArmorType(uint8 class_id);
		uint8 GetPlayerClassValue(uint8 class_id);
		uint16 GetPlayerClassBit(uint8 class_id);

		bool IsCasterClass(uint8 class_id);
		bool IsChainClass(uint8 class_id);
		bool IsClothClass(uint8 class_id);
		bool IsFighterClass(uint8 class_id);
		bool IsHeroicINTCasterClass(uint8 class_id);
		bool IsHeroicWISCasterClass(uint8 class_id);
		bool IsHybridClass(uint8 class_id);
		bool IsINTCasterClass(uint8 class_id);
		bool IsLeatherClass(uint8 class_id);
		bool IsNonSpellFighterClass(uint8 class_id);
		bool IsPlateClass(uint8 class_id);
		bool IsPlayerClass(uint8 class_id);
		bool IsSpellFighterClass(uint8 class_id);
		bool IsValidClass(uint8 class_id);
		bool IsWISCasterClass(uint8 class_id);
	}
}
#endif
