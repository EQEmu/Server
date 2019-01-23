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
#include "../common/global_define.h"
#include "../common/classes.h"

const char *GetClassIDName(uint8 class_id, uint8 level)
{
	switch (class_id) {
		case WARRIOR:
			if (level >= 70) {
				return "Vanquisher";
			}
			else if (level >= 65) {
				return "Overlord"; //Baron-Sprite: LEAVE MY CLASSES ALONE.
			}
			else if (level >= 60) {
				return "Warlord";
			}
			else if (level >= 55) {
				return "Myrmidon";
			}
			else if (level >= 51) {
				return "Champion";
			}
			else {
				return "Warrior";
			}
		case CLERIC:
			if (level >= 70) {
				return "Prelate";
			}
			else if (level >= 65) {
				return "Archon";
			}
			else if (level >= 60) {
				return "High Priest";
			}
			else if (level >= 55) {
				return "Templar";
			}
			else if (level >= 51) {
				return "Vicar";
			}
			else {
				return "Cleric";
			}
		case PALADIN:
			if (level >= 70) {
				return "Lord";
			}
			else if (level >= 65) {
				return "Lord Protector";
			}
			else if (level >= 60) {
				return "Crusader";
			}
			else if (level >= 55) {
				return "Knight";
			}
			else if (level >= 51) {
				return "Cavalier";
			}
			else {
				return "Paladin";
			}
		case RANGER:
			if (level >= 70) {
				return "Plainswalker";
			}
			else if (level >= 65) {
				return "Forest Stalker";
			}
			else if (level >= 60) {
				return "Warder";
			}
			else if (level >= 55) {
				return "Outrider";
			}
			else if (level >= 51) {
				return "Pathfinder";
			}
			else {
				return "Ranger";
			}
		case SHADOWKNIGHT:
			if (level >= 70) {
				return "Scourge Knight";
			}
			else if (level >= 65) {
				return "Dread Lord";
			}
			else if (level >= 60) {
				return "Grave Lord";
			}
			else if (level >= 55) {
				return "Revenant";
			}
			else if (level >= 51) {
				return "Reaver";
			}
			else {
				return "Shadowknight";
			}
		case DRUID:
			if (level >= 70) {
				return "Natureguard";
			}
			else if (level >= 65) {
				return "Storm Warden";
			}
			else if (level >= 60) {
				return "Hierophant";
			}
			else if (level >= 55) {
				return "Preserver";
			}
			else if (level >= 51) {
				return "Wanderer";
			}
			else {
				return "Druid";
			}
		case MONK:
			if (level >= 70) {
				return "Stone Fist";
			}
			else if (level >= 65) {
				return "Transcendent";
			}
			else if (level >= 60) {
				return "Grandmaster";
			}
			else if (level >= 55) {
				return "Master";
			}
			else if (level >= 51) {
				return "Disciple";
			}
			else {
				return "Monk";
			}
		case BARD:
			if (level >= 70) {
				return "Performer";
			}
			else if (level >= 65) {
				return "Maestro";
			}
			else if (level >= 60) {
				return "Virtuoso";
			}
			else if (level >= 55) {
				return "Troubadour";
			}
			else if (level >= 51) {
				return "Minstrel";
			}
			else {
				return "Bard";
			}
		case ROGUE:
			if (level >= 70) {
				return "Nemesis";
			}
			else if (level >= 65) {
				return "Deceiver";
			}
			else if (level >= 60) {
				return "Assassin";
			}
			else if (level >= 55) {
				return "Blackguard";
			}
			else if (level >= 51) {
				return "Rake";
			}
			else {
				return "Rogue";
			}
		case SHAMAN:
			if (level >= 70) {
				return "Soothsayer";
			}
			else if (level >= 65) {
				return "Prophet";
			}
			else if (level >= 60) {
				return "Oracle";
			}
			else if (level >= 55) {
				return "Luminary";
			}
			else if (level >= 51) {
				return "Mystic";
			}
			else {
				return "Shaman";
			}
		case NECROMANCER:
			if (level >= 70) {
				return "Wraith";
			}
			else if (level >= 65) {
				return "Arch Lich";
			}
			else if (level >= 60) {
				return "Warlock";
			}
			else if (level >= 55) {
				return "Defiler";
			}
			else if (level >= 51) {
				return "Heretic";
			}
			else {
				return "Necromancer";
			}
		case WIZARD:
			if (level >= 70) {
				return "Grand Arcanist";
			}
			else if (level >= 65) {
				return "Arcanist";
			}
			else if (level >= 60) {
				return "Sorcerer";
			}
			else if (level >= 55) {
				return "Evoker";
			}
			else if (level >= 51) {
				return "Channeler";
			}
			else {
				return "Wizard";
			}
		case MAGICIAN:
			if (level >= 70) {
				return "Arch Magus";
			}
			else if (level >= 65) {
				return "Arch Convoker";
			}
			else if (level >= 60) {
				return "Arch Mage";
			}
			else if (level >= 55) {
				return "Conjurer";
			}
			if (level >= 51) {
				return "Elementalist";
			}
			else {
				return "Magician";
			}
		case ENCHANTER:
			if (level >= 70) {
				return "Bedazzler";
			}
			else if (level >= 65) {
				return "Coercer";
			}
			else if (level >= 60) {
				return "Phantasmist";
			}
			else if (level >= 55) {
				return "Beguiler";
			}
			else if (level >= 51) {
				return "Illusionist";
			}
			else {
				return "Enchanter";
			}
		case BEASTLORD:
			if (level >= 70) {
				return "Wildblood";
			}
			else if (level >= 65) {
				return "Feral Lord";
			}
			else if (level >= 60) {
				return "Savage Lord";
			}
			else if (level >= 55) {
				return "Animist";
			}
			else if (level >= 51) {
				return "Primalist";
			}
			else {
				return "Beastlord";
			}
		case BERSERKER:
			if (level >= 70) {
				return "Ravager";
			}
			else if (level >= 65) {
				return "Fury";
			}
			else if (level >= 60) {
				return "Rager";
			}
			else if (level >= 55) {
				return "Vehement";
			}
			else if (level >= 51) {
				return "Brawler";
			}
			else {
				return "Berserker";
			}
		case BANKER:
			if (level >= 70) {
				return "Master Banker";
			}
			else if (level >= 65) {
				return "Elder Banker";
			}
			else if (level >= 60) {
				return "Oldest Banker";
			}
			else if (level >= 55) {
				return "Older Banker";
			}
			else if (level >= 51) {
				return "Old Banker";
			}
			else {
				return "Banker";
			}
		case WARRIORGM:
			return "Warrior Guildmaster";
		case CLERICGM:
			return "Cleric Guildmaster";
		case PALADINGM:
			return "Paladin Guildmaster";
		case RANGERGM:
			return "Ranger Guildmaster";
		case SHADOWKNIGHTGM:
			return "Shadowknight Guildmaster";
		case DRUIDGM:
			return "Druid Guildmaster";
		case MONKGM:
			return "Monk Guildmaster";
		case BARDGM:
			return "Bard Guildmaster";
		case ROGUEGM:
			return "Rogue Guildmaster";
		case SHAMANGM:
			return "Shaman Guildmaster";
		case NECROMANCERGM:
			return "Necromancer Guildmaster";
		case WIZARDGM:
			return "Wizard Guildmaster";
		case MAGICIANGM:
			return "Magician Guildmaster";
		case ENCHANTERGM:
			return "Enchanter Guildmaster";
		case BEASTLORDGM:
			return "Beastlord Guildmaster";
		case BERSERKERGM:
			return "Berserker Guildmaster";
		case MERCHANT:
			return "Merchant";
		case ADVENTURERECRUITER:
			return "Adventure Recruiter";
		case ADVENTUREMERCHANT:
			return "Adventure Merchant";
		case CORPSE_CLASS:
			return "Corpse Class";
		case TRIBUTE_MASTER:
			return "Tribute Master";
		case GUILD_TRIBUTE_MASTER:
			return "Guild Tribute Master";
		default:
			return "Unknown";
	}
}

const char *GetPlayerClassName(uint32 player_class_value, uint8 level)
{
	return GetClassIDName(GetClassIDFromPlayerClassValue(player_class_value), level);
}

uint32 GetPlayerClassValue(uint8 class_id)
{
	switch (class_id) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case RANGER:
		case SHADOWKNIGHT:
		case DRUID:
		case MONK:
		case BARD:
		case ROGUE:
		case SHAMAN:
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
		case BEASTLORD:
		case BERSERKER:
			return class_id;
		default:
			return PLAYER_CLASS_UNKNOWN; // watch
	}
}

uint32 GetPlayerClassBit(uint8 class_id)
{
	switch (class_id) {
		case WARRIOR:
			return PLAYER_CLASS_WARRIOR_BIT;
		case CLERIC:
			return PLAYER_CLASS_CLERIC_BIT;
		case PALADIN:
			return PLAYER_CLASS_PALADIN_BIT;
		case RANGER:
			return PLAYER_CLASS_RANGER_BIT;
		case SHADOWKNIGHT:
			return PLAYER_CLASS_SHADOWKNIGHT_BIT;
		case DRUID:
			return PLAYER_CLASS_DRUID_BIT;
		case MONK:
			return PLAYER_CLASS_MONK_BIT;
		case BARD:
			return PLAYER_CLASS_BARD_BIT;
		case ROGUE:
			return PLAYER_CLASS_ROGUE_BIT;
		case SHAMAN:
			return PLAYER_CLASS_SHAMAN_BIT;
		case NECROMANCER:
			return PLAYER_CLASS_NECROMANCER_BIT;
		case WIZARD:
			return PLAYER_CLASS_WIZARD_BIT;
		case MAGICIAN:
			return PLAYER_CLASS_MAGICIAN_BIT;
		case ENCHANTER:
			return PLAYER_CLASS_ENCHANTER_BIT;
		case BEASTLORD:
			return PLAYER_CLASS_BEASTLORD_BIT;
		case BERSERKER:
			return PLAYER_CLASS_BERSERKER_BIT;
		default:
			return PLAYER_CLASS_UNKNOWN_BIT;
	}
}

uint8 GetClassIDFromPlayerClassValue(uint32 player_class_value)
{
	switch (player_class_value) {
		case PLAYER_CLASS_WARRIOR:
		case PLAYER_CLASS_CLERIC:
		case PLAYER_CLASS_PALADIN:
		case PLAYER_CLASS_RANGER:
		case PLAYER_CLASS_SHADOWKNIGHT:
		case PLAYER_CLASS_DRUID:
		case PLAYER_CLASS_MONK:
		case PLAYER_CLASS_BARD:
		case PLAYER_CLASS_ROGUE:
		case PLAYER_CLASS_SHAMAN:
		case PLAYER_CLASS_NECROMANCER:
		case PLAYER_CLASS_WIZARD:
		case PLAYER_CLASS_MAGICIAN:
		case PLAYER_CLASS_ENCHANTER:
		case PLAYER_CLASS_BEASTLORD:
		case PLAYER_CLASS_BERSERKER:
			return player_class_value;
		default:
			return PLAYER_CLASS_UNKNOWN; // watch
	}
}

uint8 GetClassIDFromPlayerClassBit(uint32 player_class_bit)
{
	switch (player_class_bit) {
		case PLAYER_CLASS_WARRIOR_BIT:
			return WARRIOR;
		case PLAYER_CLASS_CLERIC_BIT:
			return CLERIC;
		case PLAYER_CLASS_PALADIN_BIT:
			return PALADIN;
		case PLAYER_CLASS_RANGER_BIT:
			return RANGER;
		case PLAYER_CLASS_SHADOWKNIGHT_BIT:
			return SHADOWKNIGHT;
		case PLAYER_CLASS_DRUID_BIT:
			return DRUID;
		case PLAYER_CLASS_MONK_BIT:
			return MONK;
		case PLAYER_CLASS_BARD_BIT:
			return BARD;
		case PLAYER_CLASS_ROGUE_BIT:
			return ROGUE;
		case PLAYER_CLASS_SHAMAN_BIT:
			return SHAMAN;
		case PLAYER_CLASS_NECROMANCER_BIT:
			return NECROMANCER;
		case PLAYER_CLASS_WIZARD_BIT:
			return WIZARD;
		case PLAYER_CLASS_MAGICIAN_BIT:
			return MAGICIAN;
		case PLAYER_CLASS_ENCHANTER_BIT:
			return ENCHANTER;
		case PLAYER_CLASS_BEASTLORD_BIT:
			return BEASTLORD;
		case PLAYER_CLASS_BERSERKER_BIT:
			return BERSERKER;
		default:
			return PLAYER_CLASS_UNKNOWN; // watch
	}
}

bool IsFighterClass(uint8 class_id)
{
	switch (class_id) {
		case WARRIOR:
		case PALADIN:
		case RANGER:
		case SHADOWKNIGHT:
		case MONK:
		case BARD:
		case ROGUE:
		case BEASTLORD:
		case BERSERKER:
			return true;
		default:
			return false;
	}
}

bool IsSpellFighterClass(uint8 class_id)
{
	switch (class_id) {
		case PALADIN:
		case RANGER:
		case SHADOWKNIGHT:
		case BEASTLORD:
			return true;
		default:
			return false;
	}
}

bool IsNonSpellFighterClass(uint8 class_id)
{
	switch (class_id) {
		case WARRIOR:
		case MONK:
		case BARD:
		case ROGUE:
		case BERSERKER:
			return true;
		default:
			return false;
	}
}

bool IsCasterClass(uint8 class_id)
{
	switch (class_id) {
		case CLERIC:
		case DRUID:
		case SHAMAN:
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		default:
			return false;
	}
}

bool IsINTCasterClass(uint8 class_id)
{
	switch (class_id) {
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		default:
			return false;
	}
}

bool IsWISCasterClass(uint8 class_id)
{
	switch (class_id) {
		case CLERIC:
		case DRUID:
		case SHAMAN:
			return true;
		default:
			return false;
	}
}

bool IsPlateClass(uint8 class_id)
{
	switch (class_id) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case SHADOWKNIGHT:
		case BARD:
			return true;
		default:
			return false;
	}
}

bool IsChainClass(uint8 class_id)
{
	switch (class_id) {
		case RANGER:
		case ROGUE:
		case SHAMAN:
		case BERSERKER:
			return true;
		default:
			return false;
	}
}

bool IsLeatherClass(uint8 class_id)
{
	switch (class_id) {
		case DRUID:
		case MONK:
		case BEASTLORD:
			return true;
		default:
			return false;
	}
}

bool IsClothClass(uint8 class_id)
{
	switch (class_id) {
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return true;
		default:
			return false;
	}
}

uint8 ClassArmorType(uint8 class_id)
{
	switch (class_id) {
		case WARRIOR:
		case CLERIC:
		case PALADIN:
		case SHADOWKNIGHT:
		case BARD:
			return ARMOR_TYPE_PLATE;
		case RANGER:
		case ROGUE:
		case SHAMAN:
		case BERSERKER:
			return ARMOR_TYPE_CHAIN;
		case DRUID:
		case MONK:
		case BEASTLORD:
			return ARMOR_TYPE_LEATHER;
		case NECROMANCER:
		case WIZARD:
		case MAGICIAN:
		case ENCHANTER:
			return ARMOR_TYPE_CLOTH;
		default:
			return ARMOR_TYPE_UNKNOWN;
	}
}
