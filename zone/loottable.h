/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

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
/*
alter table npc_types add column loottable_id int(11) unsigned not null;

create table loottable (id int(11) unsigned auto_increment primary key, name varchar(255) not null unique, 
mincash int(11) unsigned not null, maxcash int(11) unsigned not null, avgcoin smallint(4) unsigned not null default 0);

create table loottable_entries (loottable_id int(11) unsigned not null, lootdrop_id int(11) unsigned not null, 
multiplier tinyint(2) unsigned default 1 not null, probability tinyint(2) unsigned default 100 not null, 
primary key (loottable_id, lootdrop_id));

create table lootdrop (id int(11) unsigned auto_increment primary key, name varchar(255) not null unique);

create table lootdrop_entries (lootdrop_id int(11) unsigned not null, item_id int(11) not null, 
item_charges tinyint(2) default 1 not null, equip_item tinyint(2) unsigned not null, 
chance tinyint(2) unsigned default 1 not null, primary key (lootdrop_id, item_id));

ALTER TABLE  `loottable_entries` ADD  `probability` FLOAT NOT NULL DEFAULT  '100';
*/


#ifndef LOOTTABLE_H
#define LOOTTABLE_H
#include "zonedump.h"
#include "../common/linked_list.h"

#include <list>
using namespace std;

#pragma pack(1)
struct LootTableEntries_Struct {
	uint32	lootdrop_id;
	uint8	droplimit;
	uint8	mindrop;
	uint8	multiplier;
    float   probability;
};

struct LootTable_Struct {
	uint32	mincash;
	uint32	maxcash;
	uint32	avgcoin;
	uint32	NumEntries;
	LootTableEntries_Struct Entries[0];
};

struct LootDropEntries_Struct {
	uint32	item_id;
	int8	item_charges;
	uint8	equip_item;
	float	chance;
	uint8	minlevel;
	uint8	maxlevel;
	uint8	multiplier;
};

struct LootDrop_Struct {
	uint32	NumEntries;
	LootDropEntries_Struct Entries[0];
};
#pragma pack()

typedef list<ServerLootItem_Struct*> ItemList;

#endif
