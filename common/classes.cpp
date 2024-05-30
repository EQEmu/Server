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
#include "data_verification.h"

const std::string& EQ::classes::GetClassName(uint8 class_id, uint8 level)
{
	if (level >= 51 && IsPlayerClass(class_id)) {
		return GetClassLevelName(class_id, level);
	}

	const auto& e = class_map.find(class_id);
	return e != class_map.end() ? e->second : "UNKNOWN CLASS";
}

const std::string& EQ::classes::GetClassLevelName(uint8 class_id, uint8 level)
{
	if (level < 51 || !IsPlayerClass(class_id)) {
		return GetClassName(class_id);
	}

	const std::map<uint8, uint8>& levels = {
		{ 75, 5 },
		{ 70, 4 },
		{ 65, 3 },
		{ 60, 2 },
		{ 55, 1 }
	};

	for (const auto& e : levels) {
		if (level >= e.first) {
			return player_class_level_names.at(class_id).at(e.second);
		}
	}

	return std::string();
}

uint8 EQ::classes::GetPlayerClassValue(uint8 class_id)
{
	return IsPlayerClass(class_id) ? class_id : 0;
}

uint16 EQ::classes::GetPlayerClassBit(uint8 class_id)
{
	return IsPlayerClass(class_id) ? player_class_bitmasks[class_id] : 0;
}

bool EQ::classes::IsFighterClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Warrior:
		case Class::Paladin:
		case Class::Ranger:
		case Class::ShadowKnight:
		case Class::Monk:
		case Class::Bard:
		case Class::Rogue:
		case Class::Beastlord:
		case Class::Berserker:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsSpellFighterClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Paladin:
		case Class::Ranger:
		case Class::ShadowKnight:
		case Class::Beastlord:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsNonSpellFighterClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Warrior:
		case Class::Monk:
		case Class::Bard:
		case Class::Rogue:
		case Class::Berserker:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsHybridClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Paladin:
		case Class::Ranger:
		case Class::ShadowKnight:
		case Class::Bard:
		case Class::Beastlord:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsCasterClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsINTCasterClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsHeroicINTCasterClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
		case Class::ShadowKnight:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsWISCasterClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsHeroicWISCasterClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
		case Class::Paladin:
		case Class::Beastlord:
		case Class::Ranger:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsPlateClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Warrior:
		case Class::Cleric:
		case Class::Paladin:
		case Class::ShadowKnight:
		case Class::Bard:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsChainClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Ranger:
		case Class::Rogue:
		case Class::Shaman:
		case Class::Berserker:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsLeatherClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Druid:
		case Class::Monk:
		case Class::Beastlord:
			return true;
		default:
			return false;
	}
}

bool EQ::classes::IsClothClass(uint8 class_id)
{
	switch (class_id) {
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
			return true;
		default:
			return false;
	}
}

uint8 EQ::classes::GetClassArmorType(uint8 class_id)
{
	uint8 armor_type = ArmorType::Unknown;
	if (!IsPlayerClass(class_id)) {
		return armor_type;
	}

	if (IsChainClass(class_id)) {
		armor_type = ArmorType::Chain;
	} else if (IsClothClass(class_id)) {
		armor_type = ArmorType::Cloth;
	} else if (IsLeatherClass(class_id)) {
		armor_type = ArmorType::Leather;
	} else if (IsPlateClass(class_id)) {
		armor_type = ArmorType::Plate;
	}

	return armor_type;
}

const std::string EQ::classes::GetPlayerClassAbbreviation(uint8 class_id)
{
	return (
		!IsPlayerClass(class_id) ?
		"UNK" :
		player_class_abbreviations[class_id]
	);
}

bool EQ::classes::IsPlayerClass(uint8 class_id) {
	return EQ::ValueWithin(class_id, Class::Warrior, Class::Berserker);
}

bool EQ::classes::IsValidClass(uint8 class_id)
{
	return class_map.find(class_id) != class_map.end();
}
