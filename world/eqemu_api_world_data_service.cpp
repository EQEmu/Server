/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "clientlist.h"
#include "cliententry.h"
#include "eqemu_api_world_data_service.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "../common/database_schema.h"

extern ZSList     zoneserver_list;
extern ClientList client_list;

void callGetZoneList(Json::Value &response)
{
	for (auto &zone : zoneserver_list.getZoneServerList()) {
		Json::Value row;

		if (!zone->IsConnected()) {
			continue;
		}

		row["booting_up"]           = zone->IsBootingUp();
		row["client_address"]       = zone->GetCAddress();
		row["client_local_address"] = zone->GetCLocalAddress();
		row["client_port"]          = zone->GetCPort();
		row["compile_time"]         = zone->GetCompileTime();
		row["id"]                   = zone->GetID();
		row["instance_id"]          = zone->GetInstanceID();
		row["ip"]                   = zone->GetIP();
		row["is_static_zone"]       = zone->IsStaticZone();
		row["launch_name"]          = zone->GetLaunchName();
		row["launched_name"]        = zone->GetLaunchedName();
		row["number_players"]       = zone->NumPlayers();
		row["port"]                 = zone->GetPort();
		row["previous_zone_id"]     = zone->GetPrevZoneID();
		row["uuid"]                 = zone->GetUUID();
		row["zone_id"]              = zone->GetZoneID();
		row["zone_long_name"]       = zone->GetZoneLongName();
		row["zone_name"]            = zone->GetZoneName();
		row["zone_os_pid"]          = zone->GetZoneOSProcessID();

		response.append(row);
	}
}

void callGetDatabaseSchema(Json::Value &response)
{
	Json::Value              player_tables_json;
	std::vector<std::string> player_tables  = DatabaseSchema::GetPlayerTables();
	for (const auto          &table : player_tables) {
		player_tables_json.append(table);
	}

	Json::Value              content_tables_json;
	std::vector<std::string> content_tables = DatabaseSchema::GetContentTables();
	for (const auto          &table : content_tables) {
		content_tables_json.append(table);
	}

	Json::Value              server_tables_json;
	std::vector<std::string> server_tables  = DatabaseSchema::GetServerTables();
	for (const auto          &table : server_tables) {
		server_tables_json.append(table);
	}

	Json::Value              login_tables_json;
	std::vector<std::string> login_tables   = DatabaseSchema::GetLoginTables();
	for (const auto          &table : login_tables) {
		login_tables_json.append(table);
	}

	Json::Value              state_tables_json;
	std::vector<std::string> state_tables   = DatabaseSchema::GetStateTables();
	for (const auto          &table : state_tables) {
		state_tables_json.append(table);
	}

	Json::Value              version_tables_json;
	std::vector<std::string> version_tables = DatabaseSchema::GetVersionTables();
	for (const auto          &table : version_tables) {
		version_tables_json.append(table);
	}

	Json::Value                        character_table_columns_json;
	std::map<std::string, std::string> character_table = DatabaseSchema::GetCharacterTables();
	for (const auto                    &ctc : character_table) {
		character_table_columns_json[ctc.first] = ctc.second;
	}

	Json::Value schema;

	schema["character_table_columns"] = character_table_columns_json;
	schema["content_tables"]          = content_tables_json;
	schema["login_tables"]            = login_tables_json;
	schema["player_tables"]           = player_tables_json;
	schema["server_tables"]           = server_tables_json;
	schema["state_tables"]            = state_tables_json;
	schema["version_tables"]          = version_tables_json;

	response.append(schema);
}

void callGetClientList(Json::Value &response)
{
	client_list.GetClientList(response);
}

void EQEmuApiWorldDataService::get(Json::Value &response, const std::vector<std::string> &args)
{
	std::string method = args[0];

	if (method == "get_zone_list") {
		callGetZoneList(response);
	}
	if (method == "get_database_schema") {
		callGetDatabaseSchema(response);
	}
	if (method == "get_client_list") {
		callGetClientList(response);
	}
}
