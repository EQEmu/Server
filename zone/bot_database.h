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


#ifndef BOT_DATABASE_H
#define BOT_DATABASE_H

#ifdef BOTS

#include "../common/dbcore.h"
#include "../common/eq_packet_structs.h"

#include <list>
#include <map>
#include <vector>

class BotDatabase : public DBcore
{

public:
	BotDatabase();
	BotDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port);
	virtual ~BotDatabase();

	bool Connect(const char* host, const char* user, const char* passwd, const char* database, uint32 port);

	bool GetCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &bot_command_settings);

	// Bot command functions
	bool GetInspectMessage(uint32 bot_id, InspectMessage_Struct* message);
	bool SetInspectMessage(uint32 bot_id, const InspectMessage_Struct* message);
	bool SetAllInspectMessages(uint32 owner_id, const InspectMessage_Struct* message);

	bool SetAllArmorColorBySlot(uint32 owner_id, int16 slot_id, uint32 rgb_value);
	bool SetAllArmorColors(uint32 owner_id, uint32 rgb_value);

	bool SetHelmAppearance(uint32 owner_id, uint32 bot_id, bool show_flag = true);
	bool SetAllHelmAppearances(uint32 owner_id, bool show_flag = true);

	bool ToggleHelmAppearance(uint32 owner_id, uint32 bot_id);
	bool ToggleAllHelmAppearances(uint32 owner_id);

	bool SetFollowDistance(uint32 owner_id, uint32 bot_id, uint32 follow_distance);
	bool SetAllFollowDistances(uint32 owner_id, uint32 follow_distance);

	uint32 Clone(uint32 owner_id, uint32 bot_id, const char* clone_name);
	bool CloneInventory(uint32 owner_id, uint32 bot_id, uint32 clone_id);

	// Bot-group functions
	bool DoesBotGroupExist(std::string& group_name);

	uint32 GetGroupIDByGroupName(std::string& group_name, std::string& error_message);
	uint32 GetLeaderIDByGroupName(std::string& group_name, std::string& error_message);
	std::string GetGroupNameByGroupID(uint32 group_id, std::string& error_message);
	std::string GetGroupNameByLeaderID(uint32 leader_id, std::string& error_message);
	uint32 GetGroupIDByLeaderID(uint32 leader_id, std::string& error_message);
	uint32 GetLeaderIDByGroupID(uint32 group_id, std::string& error_message);

	uint32 GetGroupIDByMemberID(uint32 member_id, std::string& error_message);

	bool CreateBotGroup(std::string& group_name, uint32 leader_id, std::string& error_message);
	bool DeleteBotGroup(uint32 leader_id, std::string& error_message);
	bool AddMemberToBotGroup(uint32 leader_id, uint32 member_id, std::string& error_message);
	bool RemoveMemberFromBotGroup(uint32 member_id, std::string& error_message);

	uint32 GetGroupIDForLoadGroup(uint32 owner_id, std::string& group_name, std::string& error_message);
	std::map<uint32, std::list<uint32>> LoadGroup(std::string& group_name, std::string& error_message);
	
	std::list<std::pair<std::string, std::string>> GetGroupsListByOwnerID(uint32 owner_id, std::string& error_message);
};

extern BotDatabase botdb;

#endif

#endif // BOTS
