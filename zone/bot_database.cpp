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

#ifdef BOTS

#include "../common/global_define.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/eqemu_logsys.h"

#include "bot_database.h"
#include "bot.h"

BotDatabase botdb;


BotDatabase::BotDatabase()
{

}

BotDatabase::BotDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
{
	Connect(host, user, passwd, database, port);
}

BotDatabase::~BotDatabase()
{
	
}

bool BotDatabase::Connect(const char* host, const char* user, const char* passwd, const char* database, uint32 port) {
	uint32 errnum = 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(host, user, passwd, database, port, &errnum, errbuf)) {
		Log.Out(Logs::General, Logs::Error, "Failed to connect to bot database: Error: %s", errbuf);
		return false;
	}
	else {
		Log.Out(Logs::General, Logs::Status, "Using bot database '%s' at %s:%d", database, host, port);
		return true;
	}
}

bool BotDatabase::GetCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &bot_command_settings)
{
	bot_command_settings.clear();

	std::string query = "SELECT `bot_command`, `access`, `aliases` FROM `bot_command_settings`";
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		bot_command_settings[row[0]].first = atoi(row[1]);
		if (row[2][0] == 0)
			continue;

		auto aliases = SplitString(row[2], '|');
		for (auto iter : aliases) {
			if (!iter.empty())
				bot_command_settings[row[0]].second.push_back(iter);
		}
	}

	return true;
}


// Bot command functions
bool BotDatabase::GetInspectMessage(uint32 bot_id, InspectMessage_Struct* message)
{
	std::string query = StringFormat("SELECT `inspect_message` FROM `bot_inspect_messages` WHERE `bot_id` = %i LIMIT 1", bot_id);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	auto row = results.begin();
	memset(message, '\0', sizeof(InspectMessage_Struct));
	for (auto row = results.begin(); row != results.end(); ++row) {
		memcpy(message, row[0], sizeof(InspectMessage_Struct));
	}

	return true;
}

bool BotDatabase::SetInspectMessage(uint32 bot_id, const InspectMessage_Struct* message)
{
	std::string query = StringFormat("REPLACE INTO `bot_inspect_messages` (bot_id, inspect_message) VALUES (%u, '%s')", bot_id, EscapeString(message->text).c_str());
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::SetAllInspectMessages(uint32 owner_id, const InspectMessage_Struct* message)
{
	std::string query = StringFormat(
		"UPDATE `bot_inspect_messages`"
		" SET `inspect_message` = '%s'"
		" WHERE `bot_id`"
		" IN (SELECT `bot_id` FROM `bot_data` WHERE `owner_id` = '%u')",
		EscapeString(message->text).c_str(), owner_id
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::SetAllArmorColorBySlot(uint32 owner_id, int16 slot_id, uint32 rgb_value)
{
	if (!owner_id)
		return false;

	std::string query = StringFormat(
		"UPDATE `bot_inventories` bi"
		" INNER JOIN `bot_data` bd"
		" ON bd.`owner_id` = '%u'"
		" SET bi.`inst_color` = '%u'"
		" WHERE bi.`bot_id` = bd.`bot_id`"
		" AND bi.`slot_id` IN (%u, %u, %u, %u, %u, %u, %u, %u)"
		" AND bi.`slot_id` = '%i'",
		owner_id,
		rgb_value,
		MainHead, MainChest, MainArms, MainWrist1, MainWrist2, MainHands, MainLegs, MainFeet,
		slot_id
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::SetAllArmorColors(uint32 owner_id, uint32 rgb_value)
{
	if (!owner_id)
		return false;

	std::string query = StringFormat(
		"UPDATE `bot_inventories` bi"
		" INNER JOIN `bot_data` bd"
		" ON bd.`owner_id` = '%u'"
		" SET bi.`inst_color` = '%u'"
		" WHERE bi.`bot_id` = bd.`bot_id`"
		" AND bi.`slot_id` IN (%u, %u, %u, %u, %u, %u, %u, %u)",
		owner_id,
		rgb_value,
		MainHead, MainChest, MainArms, MainWrist1, MainWrist2, MainHands, MainLegs, MainFeet
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::SetHelmAppearance(uint32 owner_id, uint32 bot_id, bool show_flag)
{
	if (!owner_id || !bot_id)
		return false;
	
	std::string query = StringFormat(
		"UPDATE `bot_data`"
		" SET `show_helm` = '%u'"
		" WHERE `owner_id` = '%u'"
		" AND `bot_id` = '%u'",
		(show_flag ? 1 : 0),
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::SetAllHelmAppearances(uint32 owner_id, bool show_flag)
{
	if (!owner_id)
		return false;

	std::string query = StringFormat(
		"UPDATE `bot_data`"
		" SET `show_helm` = '%u'"
		" WHERE `owner_id` = '%u'",
		(show_flag ? 1 : 0),
		owner_id
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::ToggleHelmAppearance(uint32 owner_id, uint32 bot_id)
{
	if (!owner_id || !bot_id)
		return false;

	std::string query = StringFormat(
		"UPDATE `bot_data`"
		" SET `show_helm` = (`show_helm` XOR '1')"
		" WHERE `owner_id` = '%u'"
		" AND `bot_id` = '%u'",
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::ToggleAllHelmAppearances(uint32 owner_id)
{
	if (!owner_id)
		return false;

	std::string query = StringFormat(
		"UPDATE `bot_data`"
		" SET `show_helm` = (`show_helm` XOR '1')"
		" WHERE `owner_id` = '%u'",
		owner_id
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::SetFollowDistance(uint32 owner_id, uint32 bot_id, uint32 follow_distance)
{
	if (!owner_id || !bot_id || !follow_distance)
		return false;

	std::string query = StringFormat(
		"UPDATE `bot_data`"
		" SET `follow_distance` = '%u'"
		" WHERE `owner_id` = '%u'"
		" AND `bot_id` = '%u'",
		follow_distance,
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool BotDatabase::SetAllFollowDistances(uint32 owner_id, uint32 follow_distance)
{
	if (!owner_id || !follow_distance)
		return false;

	std::string query = StringFormat(
		"UPDATE `bot_data`"
		" SET `follow_distance` = '%u'"
		" WHERE `owner_id` = '%u'",
		follow_distance,
		owner_id
	);
	auto results = QueryDatabase(query);

	return results.Success();
}

uint32 BotDatabase::Clone(uint32 owner_id, uint32 bot_id, const char* clone_name)
{
	if (!owner_id || !bot_id || !clone_name)
		return 0;
	
	std::string data_query = StringFormat(
		"INSERT INTO `bot_data`"
		" ("
		"`owner_id`,"
		" `spells_id`,"
		" `name`,"
		" `last_name`,"
		" `title`,"
		" `suffix`,"
		" `zone_id`,"
		" `gender`,"
		" `race`,"
		" `class`,"
		" `level`,"
		" `deity`,"
		" `creation_day`,"
		" `last_spawn`,"
		" `time_spawned`,"
		" `size`,"
		" `face`,"
		" `hair_color`,"
		" `hair_style`,"
		" `beard`,"
		" `beard_color`,"
		" `eye_color_1`,"
		" `eye_color_2`,"
		" `drakkin_heritage`,"
		" `drakkin_tattoo`,"
		" `drakkin_details`,"
		" `ac`,"
		" `atk`,"
		" `hp`,"
		" `mana`,"
		" `str`,"
		" `sta`,"
		" `cha`,"
		" `dex`,"
		" `int`,"
		" `agi`,"
		" `wis`,"
		" `fire`,"
		" `cold`,"
		" `magic`,"
		" `poison`,"
		" `disease`,"
		" `corruption`,"
		" `show_helm`,"
		" `follow_distance`"
		")"
		" SELECT"
		" bd.`owner_id`,"
		" bd.`spells_id`,"
		" '%s',"
		" '',"
		" bd.`title`,"
		" bd.`suffix`,"
		" bd.`zone_id`,"
		" bd.`gender`,"
		" bd.`race`,"
		" bd.`class`,"
		" bd.`level`,"
		" bd.`deity`,"
		" UNIX_TIMESTAMP(),"
		" UNIX_TIMESTAMP(),"
		" '0',"
		" bd.`size`,"
		" bd.`face`,"
		" bd.`hair_color`,"
		" bd.`hair_style`,"
		" bd.`beard`,"
		" bd.`beard_color`,"
		" bd.`eye_color_1`,"
		" bd.`eye_color_2`,"
		" bd.`drakkin_heritage`,"
		" bd.`drakkin_tattoo`,"
		" bd.`drakkin_details`,"
		" bd.`ac`,"
		" bd.`atk`,"
		" bd.`hp`,"
		" bd.`mana`,"
		" bd.`str`,"
		" bd.`sta`,"
		" bd.`cha`,"
		" bd.`dex`,"
		" bd.`int`,"
		" bd.`agi`,"
		" bd.`wis`,"
		" bd.`fire`,"
		" bd.`cold`,"
		" bd.`magic`,"
		" bd.`poison`,"
		" bd.`disease`,"
		" bd.`corruption`,"
		" bd.`show_helm`,"
		" bd.`follow_distance`"
		" FROM `bot_data` bd"
		" WHERE"
		" bd.`owner_id` = '%u'"
		" AND"
		" bd.`bot_id` = '%u'",
		clone_name,
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(data_query);
	if (!results.Success())
		return 0;

	return results.LastInsertedID();
}

bool BotDatabase::CloneInventory(uint32 owner_id, uint32 bot_id, uint32 clone_id)
{
	if (!owner_id || !bot_id || !clone_id)
		return false;

	std::string inv_query = StringFormat(
		"INSERT INTO `bot_inventories`"
		" ("
		"bot_id,"
		" `slot_id`,"
		" `item_id`,"
		" `inst_charges`,"
		" `inst_color`,"
		" `inst_no_drop`,"
		" `inst_custom_data`,"
		" `ornament_icon`,"
		" `ornament_id_file`,"
		" `ornament_hero_model`,"
		" `augment_1`,"
		" `augment_2`,"
		" `augment_3`,"
		" `augment_4`,"
		" `augment_5`,"
		" `augment_6`"
		")"
		" SELECT"
		" '%u' bot_id,"
		" bi.`slot_id`,"
		" bi.`item_id`,"
		" bi.`inst_charges`,"
		" bi.`inst_color`,"
		" bi.`inst_no_drop`,"
		" bi.`inst_custom_data`,"
		" bi.`ornament_icon`,"
		" bi.`ornament_id_file`,"
		" bi.`ornament_hero_model`,"
		" bi.`augment_1`,"
		" bi.`augment_2`,"
		" bi.`augment_3`,"
		" bi.`augment_4`,"
		" bi.`augment_5`,"
		" bi.`augment_6`"
		" FROM `bot_inventories` bi"
		" WHERE"
		" bi.`bot_id` = '%u'"
		" AND"
		" '%u' = (SELECT `owner_id` FROM `bot_data` WHERE `bot_id` = '%u')",
		clone_id,
		bot_id,
		owner_id,
		bot_id
	);
	auto results = QueryDatabase(inv_query);
	
	return results.Success();
}


// Bot-group functions
bool BotDatabase::DoesBotGroupExist(std::string& group_name)
{
	if (group_name.empty())
		return false;

	std::string query = StringFormat("SELECT `group_name` FROM `vw_bot_groups` WHERE `group_name` LIKE '%s' LIMIT 1", group_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() == 0)
		return false;

	auto row = results.begin();
	if (!group_name.compare(row[0]))
		return true;
	
	return false;
}

uint32 BotDatabase::GetGroupIDByGroupName(std::string& group_name, std::string& error_message)
{
	if (group_name.empty())
		return 0;

	std::string query = StringFormat("SELECT `groups_index` FROM `bot_groups` WHERE `group_name` = '%s'", group_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		error_message = results.ErrorMessage();
		return 0;
	}
	auto row = results.begin();

	return atoi(row[0]);
}

uint32 BotDatabase::GetLeaderIDByGroupName(std::string& group_name, std::string& error_message)
{
	if (group_name.empty())
		return 0;

	std::string query = StringFormat("SELECT `group_leader_id` FROM `bot_groups` WHERE `group_name` = '%s'", group_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		error_message = results.ErrorMessage();
		return 0;
	}
	auto row = results.begin();

	return atoi(row[0]);
}

std::string BotDatabase::GetGroupNameByGroupID(uint32 group_id, std::string& error_message)
{
	if (!group_id)
		return std::string();

	std::string query = StringFormat("SELECT `group_name` FROM `bot_groups` WHERE `groups_index` = '%u'", group_id);
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		error_message = results.ErrorMessage();
		return std::string();
	}
	auto row = results.begin();

	return std::string(row[0]);
}

std::string BotDatabase::GetGroupNameByLeaderID(uint32 leader_id, std::string& error_message)
{
	if (!leader_id)
		return std::string();

	std::string query = StringFormat("SELECT `group_name` FROM `bot_groups` WHERE `group_leader_id` = '%u'", leader_id);
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		error_message = results.ErrorMessage();
		return std::string();
	}
	auto row = results.begin();

	return std::string(row[0]);
}

uint32 BotDatabase::GetGroupIDByLeaderID(uint32 leader_id, std::string& error_message)
{
	if (!leader_id)
		return 0;

	std::string query = StringFormat("SELECT `groups_index` FROM `bot_groups` WHERE `group_leader_id` = '%u'", leader_id);
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		error_message = results.ErrorMessage();
		return 0;
	}
	auto row = results.begin();

	return atoi(row[0]);
}

uint32 BotDatabase::GetLeaderIDByGroupID(uint32 group_id, std::string& error_message)
{
	if (!group_id)
		return 0;

	std::string query = StringFormat("SELECT `group_leader_id` FROM `bot_groups` WHERE `groups_index` = '%u'", group_id);
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		error_message = results.ErrorMessage();
		return 0;
	}
	auto row = results.begin();

	return atoi(row[0]);
}

uint32 BotDatabase::GetGroupIDByMemberID(uint32 member_id, std::string& error_message)
{
	if (!member_id)
		return 0;

	std::string query = StringFormat("SELECT `groups_index` FROM `bot_group_members` WHERE `bot_id` = '%u'", member_id);
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		error_message = results.ErrorMessage();
		return 0;
	}
	auto row = results.begin();

	return atoi(row[0]);
}

bool BotDatabase::CreateBotGroup(std::string& group_name, uint32 leader_id, std::string& error_message)
{
	if (group_name.empty() || !leader_id)
		return false;

	if (DoesBotGroupExist(group_name))
		return false;

	std::string query = StringFormat("INSERT INTO `bot_groups` (`group_leader_id`, `group_name`) VALUES ('%u', '%s')", leader_id, group_name.c_str());
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		error_message = results.ErrorMessage();
		return false;
	}

	auto group_id = results.LastInsertedID();
	if (!group_id)
		return false;

	query = StringFormat("INSERT INTO `bot_group_members` (`groups_index`, `bot_id`) VALUES ('%u', '%u')", group_id, leader_id);
	results = QueryDatabase(query);
	if (!results.Success()) {
		error_message = results.ErrorMessage();
		return false;
	}

	return true;
}

bool BotDatabase::DeleteBotGroup(uint32 leader_id, std::string& error_message)
{
	if (!leader_id)
		return false;

	uint32 group_id = GetGroupIDByLeaderID(leader_id, error_message);
	if (!group_id || !error_message.empty())
		return false;

	std::string query = StringFormat("DELETE FROM `bot_group_members` WHERE `groups_index` = '%u'", group_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		error_message = results.ErrorMessage();
		return false;
	}

	query = StringFormat("DELETE FROM `bot_groups` WHERE `groups_index` = '%u'", group_id);
	results = QueryDatabase(query);
	if (!results.Success()) {
		error_message = results.ErrorMessage();
		return false;
	}

	return true;
}

bool BotDatabase::AddMemberToBotGroup(uint32 leader_id, uint32 member_id, std::string& error_message)
{
	if (!leader_id || !member_id)
		return false;

	uint32 group_id = GetGroupIDByLeaderID(leader_id, error_message);
	if (!group_id || !error_message.empty())
		return false;

	std::string query = StringFormat("INSERT INTO `bot_group_members` (`groups_index`, `bot_id`) VALUES ('%u', '%u')", group_id, member_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		error_message = results.ErrorMessage();
		return false;
	}

	return true;
}

bool BotDatabase::RemoveMemberFromBotGroup(uint32 member_id, std::string& error_message)
{
	if (!member_id)
		return false;

	if (GetGroupIDByLeaderID(member_id, error_message))
		return DeleteBotGroup(member_id, error_message);

	if (!error_message.empty())
		return false;

	std::string query = StringFormat("DELETE FROM `bot_group_members` WHERE `bot_id` = '%u'", member_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		error_message = results.ErrorMessage();
		return false;
	}

	return true;
}

uint32 BotDatabase::GetGroupIDForLoadGroup(uint32 owner_id, std::string& group_name, std::string& error_message)
{
	if (!owner_id || group_name.empty())
		return 0;

	std::string query = StringFormat("SELECT `groups_index`, `group_name` FROM `vw_bot_groups` WHERE `owner_id` = '%u'", owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		error_message = results.ErrorMessage();
		return 0;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (!group_name.compare(row[1]))
			return atoi(row[0]);
	}

	return 0;
}

std::map<uint32, std::list<uint32>> BotDatabase::LoadGroup(std::string& group_name, std::string& error_message)
{
	std::map<uint32, std::list<uint32>> group_list;
	if (group_name.empty())
		return group_list;

	uint32 group_id = GetGroupIDByGroupName(group_name, error_message);
	if (!group_id || !error_message.empty())
		return group_list;

	std::string query = StringFormat("SELECT `bot_id` FROM `bot_group_members` WHERE `groups_index` = '%u'", group_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		error_message = results.ErrorMessage();
		return group_list;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
		group_list[group_id].push_back(atoi(row[0]));
	
	return group_list;
}

std::list<std::pair<std::string, std::string>> BotDatabase::GetGroupsListByOwnerID(uint32 owner_id, std::string& error_message)
{
	std::list<std::pair<std::string, std::string>> groups_list;
	if (!owner_id)
		return groups_list;

	std::string query = StringFormat("SELECT `group_name`, `group_leader_name` FROM `vw_bot_groups` WHERE `owner_id` = '%u'", owner_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		error_message = results.ErrorMessage();
		return groups_list;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
		groups_list.push_back(std::pair<std::string, std::string>(row[0], row[1]));
	
	return groups_list;
}

#endif // BOTS
