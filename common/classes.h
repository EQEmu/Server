/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#define Array_Class_UNKNOWN 0
#define WARRIOR			1
#define CLERIC			2
#define PALADIN			3
#define RANGER			4
#define SHADOWKNIGHT	5
#define DRUID			6
#define MONK			7
#define BARD			8
#define ROGUE			9
#define SHAMAN			10
#define NECROMANCER		11
#define WIZARD			12
#define MAGICIAN		13
#define ENCHANTER		14
#define BEASTLORD		15
#define BERSERKER		16
#define PLAYER_CLASS_COUNT	16 // used for array defines, must be the count of playable classes
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
#define LDON_TREASURE 62 //objects you can use /open on first seen in LDONs
#define CORPSE_CLASS 62	//only seen on Danvi's Corpse in Akheva so far..
#define TRIBUTE_MASTER	63
#define GUILD_TRIBUTE_MASTER	64	//not sure
#define NORRATHS_KEEPERS_MERCHANT 67
#define DARK_REIGN_MERCHANT 68
#define FELLOWSHIP_MASTER 69
#define ALT_CURRENCY_MERCHANT 70
#define MERCERNARY_MASTER 71
#define warrior_1 1
#define monk_1 64
#define paladin_1 4
#define shadow_1 16
#define bard_1 128
#define cleric_1 2
#define necromancer_1 1024
#define ranger_1 8
#define druid_1 32
#define mage_1 4096
#define wizard_1 2048
#define enchanter_1 8192
#define rogue_1 256
#define shaman_1 512
#define beastlord_1 16384
#define berserker_1 32768
#define call_1 65536

const char* GetEQClassName(uint8 class_, uint8 level = 0);
uint32 GetArrayEQClass(uint8 eqclass);
uint8 GetEQArrayEQClass(uint8 eqclass);
#endif

