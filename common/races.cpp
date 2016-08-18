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

#include "../common/races.h"

const char* GetRaceIDName(uint16 race_id)
{
	switch (race_id) {
	case HUMAN:
		return "Human";
	case BARBARIAN:
		return "Barbarian";
	case ERUDITE:
		return "Erudite";
	case WOOD_ELF:
		return "Wood Elf";
	case HIGH_ELF:
		return "High Elf";
	case DARK_ELF:
		return "Dark Elf";
	case HALF_ELF:
		return "Half Elf";
	case DWARF:
		return "Dwarf";
	case TROLL:
		return "Troll";
	case OGRE:
		return "Ogre";
	case HALFLING:
		return "Halfling";
	case GNOME:
		return "Gnome";
	case IKSAR:
		return "Iksar";
	case WEREWOLF:
		return "Werewolf";
	case SKELETON:
		return "Skeleton";
	case ELEMENTAL:
		return "Elemental";
	case EYE_OF_ZOMM:
		return "Eye of Zomm";
	case WOLF_ELEMENTAL:
		return "Wolf Elemental";
	case IKSAR_SKELETON:
		return "Iksar Skeleton";
	case VAHSHIR:
		return "Vah Shir";
	case FROGLOK:
	case FROGLOK2:
		return "Froglok";
	case DRAKKIN:
		return "Drakkin";
	default:
		return "Unknown";
	}
}

const char* GetPlayerRaceName(uint32 player_race_value)
{
	return GetRaceIDName(GetRaceIDFromPlayerRaceValue(player_race_value));
}

uint32 GetPlayerRaceValue(uint16 race_id)
{
	switch (race_id) {
	case HUMAN:
	case BARBARIAN:
	case ERUDITE:
	case WOOD_ELF:
	case HIGH_ELF:
	case DARK_ELF:
	case HALF_ELF:
	case DWARF:
	case TROLL:
	case OGRE:
	case HALFLING:
	case GNOME:
		return race_id;
	case IKSAR:
		return PLAYER_RACE_IKSAR;
	case VAHSHIR:
		return PLAYER_RACE_VAHSHIR;
	case FROGLOK:
	case FROGLOK2:
		return PLAYER_RACE_FROGLOK;
	case DRAKKIN:
		return PLAYER_RACE_DRAKKIN;
	default:
		return PLAYER_RACE_UNKNOWN; // watch
	}
}

uint32 GetPlayerRaceBit(uint16 race_id)
{
	switch (race_id) {
	case HUMAN:
		return PLAYER_RACE_HUMAN_BIT;
	case BARBARIAN:
		return PLAYER_RACE_BARBARIAN_BIT;
	case ERUDITE:
		return PLAYER_RACE_ERUDITE_BIT;
	case WOOD_ELF:
		return PLAYER_RACE_WOOD_ELF_BIT;
	case HIGH_ELF:
		return PLAYER_RACE_HIGH_ELF_BIT;
	case DARK_ELF:
		return PLAYER_RACE_DARK_ELF_BIT;
	case HALF_ELF:
		return PLAYER_RACE_HALF_ELF_BIT;
	case DWARF:
		return PLAYER_RACE_DWARF_BIT;
	case TROLL:
		return PLAYER_RACE_TROLL_BIT;
	case OGRE:
		return PLAYER_RACE_OGRE_BIT;
	case HALFLING:
		return PLAYER_RACE_HALFLING_BIT;
	case GNOME:
		return PLAYER_RACE_GNOME_BIT;
	case IKSAR:
		return PLAYER_RACE_IKSAR_BIT;
	case VAHSHIR:
		return PLAYER_RACE_VAHSHIR_BIT;
	case FROGLOK:
		return PLAYER_RACE_FROGLOK_BIT;
	case DRAKKIN:
		return PLAYER_RACE_DRAKKIN_BIT;
	default:
		return PLAYER_RACE_UNKNOWN_BIT;
	}
}

uint16 GetRaceIDFromPlayerRaceValue(uint32 player_race_value)
{
	switch (player_race_value) {
	case PLAYER_RACE_HUMAN:
	case PLAYER_RACE_BARBARIAN:
	case PLAYER_RACE_ERUDITE:
	case PLAYER_RACE_WOOD_ELF:
	case PLAYER_RACE_HIGH_ELF:
	case PLAYER_RACE_DARK_ELF:
	case PLAYER_RACE_HALF_ELF:
	case PLAYER_RACE_DWARF:
	case PLAYER_RACE_TROLL:
	case PLAYER_RACE_OGRE:
	case PLAYER_RACE_HALFLING:
	case PLAYER_RACE_GNOME:
		return player_race_value;
	case PLAYER_RACE_IKSAR:
		return IKSAR;
	case PLAYER_RACE_VAHSHIR:
		return VAHSHIR;
	case PLAYER_RACE_FROGLOK:
		return FROGLOK;
	case PLAYER_RACE_DRAKKIN:
		return DRAKKIN;
	default:
		return PLAYER_RACE_UNKNOWN; // watch
	}
}

uint16 GetRaceIDFromPlayerRaceBit(uint32 player_race_bit)
{
	switch (player_race_bit) {
	case PLAYER_RACE_HUMAN_BIT:
		return HUMAN;
	case PLAYER_RACE_BARBARIAN_BIT:
		return BARBARIAN;
	case PLAYER_RACE_ERUDITE_BIT:
		return ERUDITE;
	case PLAYER_RACE_WOOD_ELF_BIT:
		return WOOD_ELF;
	case PLAYER_RACE_HIGH_ELF_BIT:
		return HIGH_ELF;
	case PLAYER_RACE_DARK_ELF_BIT:
		return DARK_ELF;
	case PLAYER_RACE_HALF_ELF_BIT:
		return HALF_ELF;
	case PLAYER_RACE_DWARF_BIT:
		return DWARF;
	case PLAYER_RACE_TROLL_BIT:
		return TROLL;
	case PLAYER_RACE_OGRE_BIT:
		return OGRE;
	case PLAYER_RACE_HALFLING_BIT:
		return HALFLING;
	case PLAYER_RACE_GNOME_BIT:
		return GNOME;
	case PLAYER_RACE_IKSAR_BIT:
		return IKSAR;
	case PLAYER_RACE_VAHSHIR_BIT:
		return VAHSHIR;
	case PLAYER_RACE_FROGLOK_BIT:
		return FROGLOK;
	case PLAYER_RACE_DRAKKIN_BIT:
		return DRAKKIN;
	default:
		return PLAYER_RACE_UNKNOWN; // watch
	}
}


// PlayerAppearance prep
#define HUMAN_MALE ((HUMAN << 8) | MALE)
#define HUMAN_FEMALE ((HUMAN << 8) | FEMALE)
#define BARBARIAN_MALE ((BARBARIAN << 8) | MALE)
#define BARBARIAN_FEMALE ((BARBARIAN << 8) | FEMALE)
#define ERUDITE_MALE ((ERUDITE << 8) | MALE)
#define ERUDITE_FEMALE ((ERUDITE << 8) | FEMALE)
#define WOOD_ELF_MALE ((WOOD_ELF << 8) | MALE)
#define WOOD_ELF_FEMALE ((WOOD_ELF << 8) | FEMALE)
#define HIGH_ELF_MALE ((HIGH_ELF << 8) | MALE)
#define HIGH_ELF_FEMALE ((HIGH_ELF << 8) | FEMALE)
#define DARK_ELF_MALE ((DARK_ELF << 8) | MALE)
#define DARK_ELF_FEMALE ((DARK_ELF << 8) | FEMALE)
#define HALF_ELF_MALE ((HALF_ELF << 8) | MALE)
#define HALF_ELF_FEMALE ((HALF_ELF << 8) | FEMALE)
#define DWARF_MALE ((DWARF << 8) | MALE)
#define DWARF_FEMALE ((DWARF << 8) | FEMALE)
#define TROLL_MALE ((TROLL << 8) | MALE)
#define TROLL_FEMALE ((TROLL << 8) | FEMALE)
#define OGRE_MALE ((OGRE << 8) | MALE)
#define OGRE_FEMALE ((OGRE << 8) | FEMALE)
#define HALFLING_MALE ((HALFLING << 8) | MALE)
#define HALFLING_FEMALE ((HALFLING << 8) | FEMALE)
#define GNOME_MALE ((GNOME << 8) | MALE)
#define GNOME_FEMALE ((GNOME << 8) | FEMALE)
#define IKSAR_MALE ((IKSAR << 8) | MALE)
#define IKSAR_FEMALE ((IKSAR << 8) | FEMALE)
#define VAHSHIR_MALE ((VAHSHIR << 8) | MALE)
#define VAHSHIR_FEMALE ((VAHSHIR << 8) | FEMALE)
#define FROGLOK_MALE ((FROGLOK << 8) | MALE)
#define FROGLOK_FEMALE ((FROGLOK << 8) | FEMALE)
#define DRAKKIN_MALE ((DRAKKIN << 8) | MALE)
#define DRAKKIN_FEMALE ((DRAKKIN << 8) | FEMALE)

#define BINDRG(r, g) (((int)r << 8) | g)


bool PlayerAppearance::IsValidBeard(uint16 race_id, uint8 gender_id, uint8 beard_value, bool use_luclin)
{
	if (beard_value == 0xFF)
		return true;

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
		case DWARF_FEMALE:
			if (beard_value <= 1)
				return true;
			break;
		case HIGH_ELF_MALE:
		case DARK_ELF_MALE:
		case HALF_ELF_MALE:
		case DRAKKIN_FEMALE:
			if (beard_value <= 3)
				return true;
			break;
		case HUMAN_MALE:
		case BARBARIAN_MALE:
		case ERUDITE_MALE:
		case DWARF_MALE:
		case HALFLING_MALE:
		case GNOME_MALE:
			if (beard_value <= 5)
				return true;
			break;
		case DRAKKIN_MALE:
			if (beard_value <= 11)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	else {
		switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_FEMALE:
			if (beard_value <= 3)
				return true;
			break;
		case DRAKKIN_MALE:
			if (beard_value <= 11)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
}

bool PlayerAppearance::IsValidBeardColor(uint16 race_id, uint8 gender_id, uint8 beard_color_value, bool use_luclin)
{
	if (beard_color_value == 0xFF)
	return true;
	
	switch (BINDRG(race_id, gender_id)) {
	case GNOME_MALE:
		if (beard_color_value <= 24)
			return true;
		break;
	case HUMAN_MALE:
	case BARBARIAN_MALE:
	case ERUDITE_MALE:
	case HALF_ELF_MALE:
	case DWARF_MALE:
	case DWARF_FEMALE:
	case HALFLING_MALE:
		if (beard_color_value <= 19)
			return true;
		break;
	case DARK_ELF_MALE:
		if (beard_color_value >= 13 &&  beard_color_value <= 18)
			return true;
		break;
	case HIGH_ELF_MALE:
		if (beard_color_value <= 14)
			return true;
		break;
	case FROGLOK_MALE:
	case FROGLOK_FEMALE:
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (beard_color_value <= 3)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidDetail(uint16 race_id, uint8 gender_id, uint32 detail_value, bool use_luclin)
{
	if (detail_value == 0xFFFFFFFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (detail_value <= 7)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidEyeColor(uint16 race_id, uint8 gender_id, uint8 eye_color_value, bool use_luclin)
{
	return true; // need valid criteria

	switch (BINDRG(race_id, gender_id)) {
	case HUMAN_MALE:
	case HUMAN_FEMALE:
	case BARBARIAN_MALE:
	case BARBARIAN_FEMALE:
	case ERUDITE_MALE:
	case ERUDITE_FEMALE:
	case WOOD_ELF_MALE:
	case WOOD_ELF_FEMALE:
	case HIGH_ELF_MALE:
	case HIGH_ELF_FEMALE:
	case DARK_ELF_MALE:
	case DARK_ELF_FEMALE:
	case HALF_ELF_MALE:
	case HALF_ELF_FEMALE:
	case DWARF_MALE:
	case DWARF_FEMALE:
	case OGRE_MALE:
	case OGRE_FEMALE:
	case HALFLING_MALE:
	case HALFLING_FEMALE:
	case GNOME_MALE:
	case GNOME_FEMALE:
	case IKSAR_MALE:
	case IKSAR_FEMALE:
	case VAHSHIR_MALE:
	case VAHSHIR_FEMALE:
		if (eye_color_value <= 9)
			return true;
		break;	
	case TROLL_MALE:
	case TROLL_FEMALE:
		if (eye_color_value <= 10)
			return true;
		break;
	case FROGLOK_MALE:
	case FROGLOK_FEMALE:
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (eye_color_value <= 11)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidFace(uint16 race_id, uint8 gender_id, uint8 face_value, bool use_luclin)
{
	if (face_value == 0xFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (face_value <= 6)
			return true;
		break;
	case HUMAN_MALE:
	case HUMAN_FEMALE:
	case BARBARIAN_MALE:
	case BARBARIAN_FEMALE:
	case ERUDITE_MALE:
	case ERUDITE_FEMALE:
	case WOOD_ELF_MALE:
	case WOOD_ELF_FEMALE:
	case HIGH_ELF_MALE:
	case HIGH_ELF_FEMALE:
	case DARK_ELF_MALE:
	case DARK_ELF_FEMALE:
	case HALF_ELF_MALE:
	case HALF_ELF_FEMALE:
	case DWARF_MALE:
	case DWARF_FEMALE:
	case TROLL_MALE:
	case TROLL_FEMALE:
	case OGRE_MALE:
	case OGRE_FEMALE:
	case HALFLING_MALE:
	case HALFLING_FEMALE:
	case GNOME_MALE:
	case GNOME_FEMALE:
	case IKSAR_MALE:
	case IKSAR_FEMALE:
	case VAHSHIR_MALE:
	case VAHSHIR_FEMALE:
		if (face_value <= 7)
			return true;
		break;
	case FROGLOK_MALE:
	case FROGLOK_FEMALE:
		if (face_value <= 9)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidHair(uint16 race_id, uint8 gender_id, uint8 hair_value, bool use_luclin)
{
	if (hair_value == 0xFF)
		return true;

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case TROLL_FEMALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
		case GNOME_MALE:
		case GNOME_FEMALE:
			if (hair_value <= 3)
				return true;
			break;
		case ERUDITE_MALE:
			if (hair_value <= 5)
				return true;
			break;
		case DRAKKIN_FEMALE:
			if (hair_value <= 7)
				return true;
			break;
		case ERUDITE_FEMALE:
		case DRAKKIN_MALE:
			if (hair_value <= 8)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	else {
		switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_FEMALE:
			if (hair_value <= 7)
				return true;
			break;
		case DRAKKIN_MALE:
			if (hair_value <= 8)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
}

bool PlayerAppearance::IsValidHairColor(uint16 race_id, uint8 gender_id, uint8 hair_color_value, bool use_luclin)
{
	if (hair_color_value == 0xFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case GNOME_MALE:
	case GNOME_FEMALE:
		if (hair_color_value <= 24)
			return true;
		break;
	case TROLL_FEMALE:
	case OGRE_FEMALE:
		if (hair_color_value <= 23)
			return true;
		break;
	case HUMAN_MALE:
	case HUMAN_FEMALE:
	case BARBARIAN_MALE:
	case BARBARIAN_FEMALE:
	case WOOD_ELF_MALE:
	case WOOD_ELF_FEMALE:
	case HALF_ELF_MALE:
	case HALF_ELF_FEMALE:
	case DWARF_MALE:
	case DWARF_FEMALE:
	case HALFLING_MALE:
	case HALFLING_FEMALE:
		if (hair_color_value <= 19)
			return true;
		break;
	case DARK_ELF_MALE:
	case DARK_ELF_FEMALE:
		if (hair_color_value >= 13 && hair_color_value <= 18)
			return true;
		break;
	case HIGH_ELF_MALE:
	case HIGH_ELF_FEMALE:
		if (hair_color_value <= 14)
			return true;
		break;
	case FROGLOK_MALE:
	case FROGLOK_FEMALE:
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (hair_color_value <= 3)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidHead(uint16 race_id, uint8 gender_id, uint8 head_value, bool use_luclin)
{
	if (head_value == 0xFF)
		return true;

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case TROLL_MALE:
		case TROLL_FEMALE:
		case OGRE_MALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
		case GNOME_MALE:
		case GNOME_FEMALE:
		case IKSAR_MALE:
		case IKSAR_FEMALE:
		case VAHSHIR_MALE:
		case VAHSHIR_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (head_value <= 3)
				return true;
			break;
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
			if (head_value <= 4)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	else {
		switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case TROLL_MALE:
		case TROLL_FEMALE:
		case OGRE_MALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
		case IKSAR_MALE:
		case IKSAR_FEMALE:
		case VAHSHIR_MALE:
		case VAHSHIR_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (head_value <= 3)
				return true;
			break;
		case GNOME_MALE:
		case GNOME_FEMALE:
			if (head_value <= 4)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
}

bool PlayerAppearance::IsValidHeritage(uint16 race_id, uint8 gender_id, uint32 heritage_value, bool use_luclin)
{
	if (heritage_value == 0xFFFFFFFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (heritage_value <= 7) // > 5 seems to jumble other features..else, some heritages have 'specialized' features
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidTattoo(uint16 race_id, uint8 gender_id, uint32 tattoo_value, bool use_luclin)
{
	if (tattoo_value == 0xFFFFFFFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (tattoo_value <= 7)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidTexture(uint16 race_id, uint8 gender_id, uint8 texture_value, bool use_luclin)
{
	if (texture_value == 0xFF)
		return true;
	
	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case IKSAR_MALE:
		case IKSAR_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if ((texture_value >= 10 && texture_value <= 16) || texture_value <= 4)
				return true;
			break;
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case GNOME_MALE:
		case GNOME_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
			if ((texture_value >= 10 && texture_value <= 16) || texture_value <= 3)
				return true;
			break;
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case TROLL_MALE:
		case TROLL_FEMALE:
		case OGRE_MALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
		case VAHSHIR_MALE:
		case VAHSHIR_FEMALE:
			if (texture_value <= 3)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	else {
		switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if ((texture_value >= 10 && texture_value <= 16) || texture_value <= 4)
				return true;
			break;
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case GNOME_MALE:
		case GNOME_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
			if ((texture_value >= 10 && texture_value <= 16) || texture_value <= 3)
				return true;
			break;
		case VAHSHIR_MALE:
		case VAHSHIR_FEMALE:
			if (texture_value == 50 || texture_value <= 3)
				return true;
			break;
		case IKSAR_MALE:
		case IKSAR_FEMALE:
			if (texture_value == 10 || texture_value <= 4)
				return true;
			break;
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case TROLL_MALE:
		case TROLL_FEMALE:
		case OGRE_MALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
			if (texture_value <= 3)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
}

bool PlayerAppearance::IsValidWoad(uint16 race_id, uint8 gender_id, uint8 woad_value, bool use_luclin)
{
	if (woad_value == 0xFF)
		return true;

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
			if (woad_value <= 8)
				return true;
			break;
		default:
			break;
		}
	}
	return false;
}
