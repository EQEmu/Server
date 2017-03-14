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

#endif // BOTS

#endif // BOT_STRUCTS
