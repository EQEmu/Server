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

#define WARRIOR 1
#define CLERIC 2
#define PALADIN 3
#define RANGER 4
#define SHADOWKNIGHT 5
#define DRUID 6
#define MONK 7
#define BARD 8
#define ROGUE 9
#define SHAMAN 10
#define NECROMANCER 11
#define WIZARD 12
#define MAGICIAN 13
#define ENCHANTER 14
#define BEASTLORD 15
#define BERSERKER 16
#define WARRIORGM 20
#define CLERICGM 21
#define PALADINGM 22
#define RANGERGM 23
#define SHADOWKNIGHTGM 24
#define DRUIDGM 25
#define MONKGM 26
#define BARDGM 27
#define ROGUEGM 28
#define SHAMANGM 29
#define NECROMANCERGM 30
#define WIZARDGM 31
#define MAGICIANGM 32
#define ENCHANTERGM 33
#define BEASTLORDGM 34
#define BERSERKERGM 35
#define BANKER 40
#define MERCHANT 41
#define DISCORD_MERCHANT 59
#define ADVENTURERECRUITER 60
#define ADVENTUREMERCHANT 61
#define LDON_TREASURE 62	// objects you can use /open on first seen in LDONs
#define CORPSE_CLASS 62	// only seen on Danvi's Corpse in Akheva so far..
#define TRIBUTE_MASTER 63
#define GUILD_TRIBUTE_MASTER 64	// not sure
#define NORRATHS_KEEPERS_MERCHANT 67
#define DARK_REIGN_MERCHANT 68
#define FELLOWSHIP_MASTER 69
#define ALT_CURRENCY_MERCHANT 70
#define MERCERNARY_MASTER 71


// player class values
#define PLAYER_CLASS_UNKNOWN 0
#define PLAYER_CLASS_WARRIOR 1
#define PLAYER_CLASS_CLERIC 2
#define PLAYER_CLASS_PALADIN 3
#define PLAYER_CLASS_RANGER 4
#define PLAYER_CLASS_SHADOWKNIGHT 5
#define PLAYER_CLASS_DRUID 6
#define PLAYER_CLASS_MONK 7
#define PLAYER_CLASS_BARD 8
#define PLAYER_CLASS_ROGUE 9
#define PLAYER_CLASS_SHAMAN 10
#define PLAYER_CLASS_NECROMANCER 11
#define PLAYER_CLASS_WIZARD 12
#define PLAYER_CLASS_MAGICIAN 13
#define PLAYER_CLASS_ENCHANTER 14
#define PLAYER_CLASS_BEASTLORD 15
#define PLAYER_CLASS_BERSERKER 16

#define PLAYER_CLASS_COUNT 16


// player class bits
#define PLAYER_CLASS_UNKNOWN_BIT 0
#define PLAYER_CLASS_WARRIOR_BIT 1
#define PLAYER_CLASS_CLERIC_BIT 2
#define PLAYER_CLASS_PALADIN_BIT 4
#define PLAYER_CLASS_RANGER_BIT 8
#define PLAYER_CLASS_SHADOWKNIGHT_BIT 16
#define PLAYER_CLASS_DRUID_BIT 32
#define PLAYER_CLASS_MONK_BIT 64
#define PLAYER_CLASS_BARD_BIT 128
#define PLAYER_CLASS_ROGUE_BIT 256
#define PLAYER_CLASS_SHAMAN_BIT 512
#define PLAYER_CLASS_NECROMANCER_BIT 1024
#define PLAYER_CLASS_WIZARD_BIT 2048
#define PLAYER_CLASS_MAGICIAN_BIT 4096
#define PLAYER_CLASS_ENCHANTER_BIT 8192
#define PLAYER_CLASS_BEASTLORD_BIT 16384
#define PLAYER_CLASS_BERSERKER_BIT 32768

#define PLAYER_CLASS_ALL_MASK 65535	// was 65536


#define ARMOR_TYPE_UNKNOWN 0
#define ARMOR_TYPE_CLOTH 1
#define ARMOR_TYPE_LEATHER 2
#define ARMOR_TYPE_CHAIN 3
#define ARMOR_TYPE_PLATE 4

#define ARMOR_TYPE_FIRST ARMOR_TYPE_UNKNOWN
#define ARMOR_TYPE_LAST ARMOR_TYPE_PLATE
#define ARMOR_TYPE_COUNT 5


const char* GetClassIDName(uint8 class_id, uint8 level = 0);
const char* GetPlayerClassName(uint32 player_class_value, uint8 level = 0);

uint32 GetPlayerClassValue(uint8 class_id);
uint32 GetPlayerClassBit(uint8 class_id);

uint8 GetClassIDFromPlayerClassValue(uint32 player_class_value);
uint8 GetClassIDFromPlayerClassBit(uint32 player_class_bit);

bool IsFighterClass(uint8 class_id);
bool IsSpellFighterClass(uint8 class_id);
bool IsNonSpellFighterClass(uint8 class_id);
bool IsCasterClass(uint8 class_id);
bool IsINTCasterClass(uint8 class_id);
bool IsWISCasterClass(uint8 class_id);

bool IsPlateClass(uint8 class_id);
bool IsChainClass(uint8 class_id);
bool IsLeatherClass(uint8 class_id);
bool IsClothClass(uint8 class_id);
uint8 ClassArmorType(uint8 class_id);

#endif
