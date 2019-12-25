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

#include "world_server_command_handler.h"
#include "../common/eqemu_logsys.h"
#include "../common/json/json.h"
#include "../common/version.h"
#include "worlddb.h"
#include "../common/database_schema.h"

namespace WorldserverCommandHandler {

	/**
	 * @param argc
	 * @param argv
	 */
	void CommandHandler(int argc, char **argv)
	{
		if (argc == 1) { return; }

		argh::parser cmd;
		cmd.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
		EQEmuCommand::DisplayDebug(cmd);

		/**
		 * Declare command mapping
		 */
		auto function_map = EQEmuCommand::function_map;

		/**
		 * Register commands
		 */
		function_map["world:version"]               = &WorldserverCommandHandler::Version;
		function_map["database:version"]            = &WorldserverCommandHandler::DatabaseVersion;
		function_map["database:set-account-status"] = &WorldserverCommandHandler::DatabaseSetAccountStatus;
		function_map["database:schema"]             = &WorldserverCommandHandler::DatabaseGetSchema;

		EQEmuCommand::HandleMenu(function_map, cmd, argc, argv);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void DatabaseVersion(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Shows database version";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		Json::Value database_version;

		database_version["database_version"]      = CURRENT_BINARY_DATABASE_VERSION;
		database_version["bots_database_version"] = CURRENT_BINARY_BOTS_DATABASE_VERSION;

		std::stringstream payload;
		payload << database_version;

		std::cout << payload.str() << std::endl;
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void Version(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Shows server version";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		Json::Value database_version;

		database_version["bots_database_version"] = CURRENT_BINARY_BOTS_DATABASE_VERSION;
		database_version["compile_date"]          = COMPILE_DATE;
		database_version["compile_time"]          = COMPILE_TIME;
		database_version["database_version"]      = CURRENT_BINARY_DATABASE_VERSION;
		database_version["server_version"]        = CURRENT_VERSION;

		std::stringstream payload;
		payload << database_version;

		std::cout << payload.str() << std::endl;
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void DatabaseSetAccountStatus(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Sets account status by account name";

		std::vector<std::string> arguments = {
			"{name}",
			"{status}"
		};

		std::vector<std::string> options = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		database.SetAccountStatus(
			cmd(2).str(),
			std::stoi(cmd(3).str())
		);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void DatabaseGetSchema(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Displays server database schema";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

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

		Json::Value schema;

		schema["content_tables"] = content_tables_json;
		schema["login_tables"]   = login_tables_json;
		schema["player_tables"]  = player_tables_json;
		schema["server_tables"]  = server_tables_json;
		schema["state_tables"]   = state_tables_json;
		schema["version_tables"] = version_tables_json;

		std::stringstream payload;
		payload << schema;

		std::cout << payload.str() << std::endl;
	}

}