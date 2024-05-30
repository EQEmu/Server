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

std::string Class::GetAbbreviation(uint8 class_id)
{
	return IsValidClass(class_id) && IsPlayerClass(class_id) ? player_class_abbreviations[class_id] : "UNK";
}

std::string Class::GetName(uint8 class_id, uint8 level)
{
	if (level >= 51 && IsPlayer(class_id)) {
		return GetLevelName(class_id, level);
	}

	return IsValid(class_id) ? class_names[class_id] : "UNKNOWN CLASS";
}

std::string Class::GetLevelName(uint8 class_id, uint8 level)
{
	if (level < 51 || !IsPlayer(class_id)) {
		return GetName(class_id);
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

uint8 Class::GetPlayerValue(uint8 class_id)
{
	return IsPlayer(class_id) ? class_id : 0;
}

uint16 Class::GetPlayerBit(uint8 class_id)
{
	return IsPlayer(class_id) ? player_class_bitmasks[class_id] : 0;
}

bool Class::IsFighter(uint8 class_id)
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

bool Class::IsSpellFighter(uint8 class_id)
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

bool Class::IsNonSpellFighter(uint8 class_id)
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

bool Class::IsHybrid(uint8 class_id)
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

bool Class::IsCaster(uint8 class_id)
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

bool Class::IsINTCaster(uint8 class_id)
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

bool Class::IsHeroicINTCaster(uint8 class_id)
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

bool Class::IsWISCaster(uint8 class_id)
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

bool Class::IsHeroicWISCaster(uint8 class_id)
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

bool Class::IsPlate(uint8 class_id)
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

bool Class::IsChain(uint8 class_id)
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

bool Class::IsLeather(uint8 class_id)
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

bool Class::IsCloth(uint8 class_id)
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

uint8 Class::GetArmorType(uint8 class_id)
{
	uint8 armor_type = ArmorType::Unknown;
	if (!IsPlayer(class_id)) {
		return armor_type;
	}

	if (IsChain(class_id)) {
		armor_type = ArmorType::Chain;
	} else if (IsCloth(class_id)) {
		armor_type = ArmorType::Cloth;
	} else if (IsLeather(class_id)) {
		armor_type = ArmorType::Leather;
	} else if (IsPlate(class_id)) {
		armor_type = ArmorType::Plate;
	}

	return armor_type;
}

const std::string Class::GetAbbreviation(uint8 class_id)
{
	return IsPlayer(class_id) ? player_class_abbreviations[class_id] : "UNK";
}

bool Class::IsPlayer(uint8 class_id)
{
	return EQ::ValueWithin(class_id, Class::Warrior, Class::Berserker);
}

bool Class::IsValid(uint8 class_id)
{
	return class_names.find(class_id) != class_names.end();
}
