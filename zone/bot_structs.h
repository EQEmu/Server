/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#ifndef BOT_STRUCTS
#define BOT_STRUCTS

#ifdef BOTS

#include "../common/types.h"

#include <sstream>

struct BotsAvailableList {
	uint32 ID;
	char Name[64];
	uint16 Class;
	uint8 Level;
	uint16 Race;
	uint8 Gender;
	char Owner[64];
	uint32 Owner_ID;
};

struct BotGroup {
	uint32 BotGroupID;
	uint32 BotID;
};

struct BotGroupList {
	std::string BotGroupName;
	std::string BotGroupLeaderName;
};

struct SpawnedBotsList {
	char BotName[64];
	char ZoneName[64];
	uint32 BotLeaderCharID;
};

struct BotSpell {
	uint16 SpellId;
	int SpellIndex;
	int16 ManaCost;
};

struct BotSpell_wPriority : public BotSpell {
	uint8 Priority;
};

struct BotCastingRoles {
	bool GroupHealer;
	bool GroupSlower;
	bool GroupNuker;
	bool GroupDoter;
	//bool RaidHealer;
	//bool RaidSlower;
	//bool RaidNuker;
	//bool RaidDoter;
};

struct BotAA {
	uint32 aa_id;
	uint8 req_level;
	uint8 total_levels;
};

struct BotSpellSetting {
	int16  priority;
	uint8  min_level;
	uint8  max_level;
	int8   min_hp;
	int8   max_hp;
	bool   is_enabled;
};

struct BotSpells_Struct {
	uint32		type;			// 0 = never, must be one (and only one) of the defined values
	int16		spellid;			// <= 0 = no spell
	int16		manacost;		// -1 = use spdat, -2 = no cast time
	uint32		time_cancast;	// when we can cast this spell next
	int32		recast_delay;
	int16		priority;
	int16		resist_adjust;
	uint8		minlevel;
	uint8		maxlevel;
	int16		min_hp;			// >0 won't cast if HP is below
	int16		max_hp;			// >0 won't cast if HP is above
	std::string	bucket_name;
	std::string	bucket_value;
	uint8		bucket_comparison;
};

#endif // BOTS

#endif // BOT_STRUCTS
