/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemu.org)

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

#ifndef _EQEMU_LOOTTABLE_H
#define _EQEMU_LOOTTABLE_H

#include "types.h"

#pragma pack(1)
struct LootTableEntries_Struct {
	uint32	lootdrop_id;
	uint8	droplimit;
	uint8	mindrop;
	uint8	multiplier;
	float	probability;
};

struct LootTable_Struct {
	uint32	mincash;
	uint32	maxcash;
	uint32	avgcoin;
	uint32	NumEntries;
	LootTableEntries_Struct Entries[0];
};

struct LootDropEntries_Struct {
	uint32 item_id;
	int8   item_charges;
	uint8  equip_item;
	float  chance;
	uint16 trivial_min_level;
	uint16 trivial_max_level;
	uint16 npc_min_level;
	uint16 npc_max_level;
	uint8  multiplier;
};

struct LootDrop_Struct {
	uint32	NumEntries;
	LootDropEntries_Struct Entries[0];
};
#pragma pack()

#endif
