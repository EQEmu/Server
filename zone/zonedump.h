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
/*
Below are the blob structures for zone state dumping to the database
-Quagmire

create table zone_state_dump (zonename varchar(16) not null primary key, spawn2_count int unsigned not null default 0,
npc_count int unsigned not null default 0, npcloot_count int unsigned not null default 0, gmspawntype_count int unsigned not null default 0,
spawn2 mediumblob, npcs mediumblob, npc_loot mediumblob, gmspawntype mediumblob, time timestamp(14));
*/

#ifndef ZONEDUMP_H
#define ZONEDUMP_H
#include "../common/faction.h"
#include "../common/eq_packet_structs.h"
#include "../common/Item.h"

#pragma pack(1)

struct NPCType
{
	char	name[64];
	char	lastname[70];

	int32	cur_hp;
	int32	max_hp;

	float	size;
	float	runspeed;
	uint8	gender;
	uint16	race;
	uint8	class_;
	uint8	bodytype;	// added for targettype support
	uint8	deity;		//not loaded from DB
	uint8	level;
	uint32	npc_id;
	uint8	texture;
	uint8	helmtexture;
	uint32	loottable_id;
	uint32	npc_spells_id;
	int32	npc_faction_id;
	uint32	merchanttype;
	uint32	alt_currency_type;
	uint32	adventure_template;
	uint32	trap_template;
	uint8	light;		//not loaded from DB
	uint16	AC;
	uint32	Mana;	//not loaded from DB
	uint16	ATK;	//not loaded from DB
	uint16	STR;
	uint16	STA;
	uint16	DEX;
	uint16	AGI;
	uint16	INT;
	uint16	WIS;
	uint16	CHA;
	int16	MR;
	int16	FR;
	int16	CR;
	int16	PR;
	int16	DR;
	int16	Corrup;
	uint8	haircolor;
	uint8	beardcolor;
	uint8	eyecolor1;			// the eyecolors always seem to be the same, maybe left and right eye?
	uint8	eyecolor2;
	uint8	hairstyle;
	uint8	luclinface;			//
	uint8	beard;				//
	uint32	drakkin_heritage;
	uint32	drakkin_tattoo;
	uint32	drakkin_details;
	uint32	armor_tint[MAX_MATERIALS];
	uint32	min_dmg;
	uint32	max_dmg;
	int16	attack_count;
	std::string special_abilities;
	uint16	d_meele_texture1;
	uint16	d_meele_texture2;
	uint8	prim_melee_type;
	uint8	sec_melee_type;
	int32	hp_regen;
	int32	mana_regen;
	int32	aggroradius; // added for AI improvement - neotokyo
	uint8	see_invis;			// See Invis flag added
	bool	see_invis_undead;	// See Invis vs. Undead flag added
	bool	see_hide;
	bool	see_improved_hide;
	bool	qglobal;
	bool	npc_aggro;
	uint8	spawn_limit;	//only this many may be in zone at a time (0=no limit)
	uint8	mount_color;	//only used by horse class
	float	attack_speed;	//%+- on attack delay of the mob.
	int		accuracy_rating;	//10 = 1% accuracy
	bool	findable;		//can be found with find command
	bool	trackable;
	float	slow_mitigation;	// Slow mitigation % in decimal form.
	uint8	maxlevel;
	uint32	scalerate;
	bool	private_corpse;
	bool	unique_spawn_by_name;
	bool	underwater;
	uint32	emoteid;
	float	spellscale;
	float	healscale;
};

/*
Below are the blob structures for saving player corpses to the database
-Quagmire

create table player_corpses (id int(11) unsigned not null auto_increment primary key, charid int(11) unsigned not null,
charname varchar(30) not null, zonename varchar(16)not null, x float not null, y float not null, z float not null,
heading float not null, data blob not null, time timestamp(14), index zonename (zonename));
*/

namespace player_lootitem
{
	struct ServerLootItem_Struct {
	uint32	item_id;
	int16	equipSlot;
	uint8	charges;
	uint16	lootslot;
	uint32 aug1;
	uint32 aug2;
	uint32 aug3;
	uint32 aug4;
	uint32 aug5;
	};
}

struct DBPlayerCorpse_Struct {
	uint32	crc;
	bool	locked;
	uint32	itemcount;
	uint32	exp;
	float	size;
	uint8	level;
	uint8	race;
	uint8	gender;
	uint8	class_;
	uint8	deity;
	uint8	texture;
	uint8	helmtexture;
	uint32	copper;
	uint32	silver;
	uint32	gold;
	uint32	plat;
	Color_Struct item_tint[9];
	uint8 haircolor;
	uint8 beardcolor;
	uint8 eyecolor1;
	uint8 eyecolor2;
	uint8 hairstyle;
	uint8 face;
	uint8 beard;
	uint32 drakkin_heritage;
	uint32 drakkin_tattoo;
	uint32 drakkin_details;
	player_lootitem::ServerLootItem_Struct	items[0];
};

namespace classic_db
{
	struct DBPlayerCorpse_Struct {
		uint32	crc;
		bool	locked;
		uint32	itemcount;
		uint32	exp;
		float	size;
		uint8	level;
		uint8	race;
		uint8	gender;
		uint8	class_;
		uint8	deity;
		uint8	texture;
		uint8	helmtexture;
		uint32	copper;
		uint32	silver;
		uint32	gold;
		uint32	plat;
		Color_Struct item_tint[9];
		uint8 haircolor;
		uint8 beardcolor;
		uint8 eyecolor1;
		uint8 eyecolor2;
		uint8 hairstyle;
		uint8 face;
		uint8 beard;
		player_lootitem::ServerLootItem_Struct	items[0];
	};
}

struct Door {
	uint32	db_id;
	uint8	door_id;
	char	zone_name[16];
	char	door_name[32];
	float	pos_x;
	float	pos_y;
	float	pos_z;
	float	heading;
	int		incline;
	uint8	opentype;
	uint32	guild_id;
	uint16	lockpick;
	uint32	keyitem;
	uint8	nokeyring;
	uint8	trigger_door;
	uint8	trigger_type;
	uint32	door_param;
	int		invert_state;
	uint16	size;
	char	dest_zone[16];
	uint32	dest_instance_id;
	float	dest_x;
	float	dest_y;
	float	dest_z;
	float	dest_heading;
	uint8	is_ldon_door;
	uint32	client_version_mask;
};

#pragma pack()

#endif
