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
#include "../common/discord/discord.h"
#include "../common/json/json.h"
#include "../common/version.h"
#include "worlddb.h"
#include "../common/database_schema.h"
#include "../common/database/database_dump_service.h"
#include "../common/content/world_content_service.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "../common/rulesys.h"
#include "../common/repositories/instance_list_repository.h"
#include "../common/repositories/zone_repository.h"
#include "../zone/queryserv.h"

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
		function_map["character:copy-character"]    = &WorldserverCommandHandler::CopyCharacter;
		function_map["database:version"]            = &WorldserverCommandHandler::DatabaseVersion;
		function_map["database:set-account-status"] = &WorldserverCommandHandler::DatabaseSetAccountStatus;
		function_map["database:schema"]             = &WorldserverCommandHandler::DatabaseGetSchema;
		function_map["database:dump"]               = &WorldserverCommandHandler::DatabaseDump;
		function_map["test:test"]                   = &WorldserverCommandHandler::TestCommand;
		function_map["test:expansion"]              = &WorldserverCommandHandler::ExpansionTestCommand;
		function_map["test:repository"]             = &WorldserverCommandHandler::TestRepository;
		function_map["test:repository2"]            = &WorldserverCommandHandler::TestRepository2;

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
		std::vector<std::string> player_tables = DatabaseSchema::GetPlayerTables();
		for (const auto          &table: player_tables) {
			player_tables_json.append(table);
		}

		Json::Value              content_tables_json;
		std::vector<std::string> content_tables = DatabaseSchema::GetContentTables();
		for (const auto          &table: content_tables) {
			content_tables_json.append(table);
		}

		Json::Value              server_tables_json;
		std::vector<std::string> server_tables = DatabaseSchema::GetServerTables();
		for (const auto          &table: server_tables) {
			server_tables_json.append(table);
		}

		Json::Value              login_tables_json;
		std::vector<std::string> login_tables = DatabaseSchema::GetLoginTables();
		for (const auto          &table: login_tables) {
			login_tables_json.append(table);
		}

		Json::Value              state_tables_json;
		std::vector<std::string> state_tables = DatabaseSchema::GetStateTables();
		for (const auto          &table: state_tables) {
			state_tables_json.append(table);
		}

		Json::Value              version_tables_json;
		std::vector<std::string> version_tables = DatabaseSchema::GetVersionTables();
		for (const auto          &table: version_tables) {
			version_tables_json.append(table);
		}
		
		Json::Value              bot_tables_json;
		std::vector<std::string> bot_tables = DatabaseSchema::GetBotTables();
		for (const auto          &table: bot_tables) {
			bot_tables_json.append(table);
		}

		Json::Value schema;

		schema["content_tables"] = content_tables_json;
		schema["login_tables"]   = login_tables_json;
		schema["player_tables"]  = player_tables_json;
		schema["server_tables"]  = server_tables_json;
		schema["state_tables"]   = state_tables_json;
		schema["version_tables"] = version_tables_json;
		schema["bot_tables"]     = bot_tables_json;

		std::stringstream payload;
		payload << schema;

		std::cout << payload.str() << std::endl;
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void DatabaseDump(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Dumps server database tables";

		std::vector<std::string> arguments = {};
		std::vector<std::string> options   = {
			"--all",
			"--content-tables",
			"--login-tables",
			"--player-tables",
			"--bot-tables",
			"--state-tables",
			"--system-tables",
			"--query-serv-tables",
			"--table-structure-only",
			"--table-lock",
			"--dump-path=",
			"--dump-output-to-console",
			"--drop-table-syntax-only",
			"--compress"
		};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		auto database_dump_service = new DatabaseDumpService();
		bool dump_all              = cmd[{"-a", "--all"}];

		if (!cmd("--dump-path").str().empty()) {
			database_dump_service->SetDumpPath(cmd("--dump-path").str());
		}

		/**
		 * Set Option
		 */
		database_dump_service->SetDumpContentTables(cmd[{"--content-tables"}] || dump_all);
		database_dump_service->SetDumpLoginServerTables(cmd[{"--login-tables"}] || dump_all);
		database_dump_service->SetDumpPlayerTables(cmd[{"--player-tables"}] || dump_all);
		database_dump_service->SetDumpBotTables(cmd[{"--bot-tables"}] || dump_all);
		database_dump_service->SetDumpStateTables(cmd[{"--state-tables"}] || dump_all);
		database_dump_service->SetDumpSystemTables(cmd[{"--system-tables"}] || dump_all);
		database_dump_service->SetDumpQueryServerTables(cmd[{"--query-serv-tables"}] || dump_all);
		database_dump_service->SetDumpAllTables(dump_all);

		database_dump_service->SetDumpWithNoData(cmd[{"--table-structure-only"}]);
		database_dump_service->SetDumpTableLock(cmd[{"--table-lock"}]);
		database_dump_service->SetDumpWithCompression(cmd[{"--compress"}]);
		database_dump_service->SetDumpOutputToConsole(cmd[{"--dump-output-to-console"}]);
		database_dump_service->SetDumpDropTableSyntaxOnly(cmd[{"--drop-table-syntax-only"}]);

		/**
		 * Dump
		 */
		database_dump_service->Dump();
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Test command";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void ExpansionTestCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Expansion test command";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		if (!RuleManager::Instance()->LoadRules(&database, "default", false)) {
			LogInfo("No rule set configured, using default rules");
		}

		content_service.SetCurrentExpansion(RuleI(Expansion, CurrentExpansion));

		std::vector<ContentFlagsRepository::ContentFlags> flags = {};
		auto                                              f     = ContentFlagsRepository::NewEntity();
		f.enabled = 1;

		std::vector<std::string> flag_names = {
			"hateplane_enabled",
			"patch_nerf_7077",
		};

		for (auto &name: flag_names) {
			f.flag_name = name;
			flags.push_back(f);
		}

		content_service.SetContentFlags(flags);

		LogInfo(
			"Current expansion is [{}] ({}) is Velious Enabled [{}] Criteria [{}]",
			content_service.GetCurrentExpansion(),
			content_service.GetCurrentExpansionName(),
			content_service.IsTheScarsOfVeliousEnabled() ? "true" : "false",
			ContentFilterCriteria::apply()
		);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void TestRepository(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Test command";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		/**
		 * Insert one
		 */
		auto instance_list_entry = InstanceListRepository::NewEntity();

		instance_list_entry.zone          = 999;
		instance_list_entry.version       = 1;
		instance_list_entry.is_global     = 1;
		instance_list_entry.start_time    = 0;
		instance_list_entry.duration      = 0;
		instance_list_entry.never_expires = 1;

		auto instance_list_inserted = InstanceListRepository::InsertOne(database, instance_list_entry);

		LogInfo("Inserted ID is [{}] zone [{}]", instance_list_inserted.id, instance_list_inserted.zone);

		/**
		 * Find one
		 */
		auto found_instance_list = InstanceListRepository::FindOne(database, instance_list_inserted.id);

		LogInfo("Found ID is [{}] zone [{}]", found_instance_list.id, found_instance_list.zone);

		/**
		 * Update one
		 */
		LogInfo("Updating instance id [{}] zone [{}]", found_instance_list.id, found_instance_list.zone);

		int update_instance_list_count = InstanceListRepository::UpdateOne(database, found_instance_list);

		found_instance_list.zone = 777;

		LogInfo(
			"Updated instance id [{}] zone [{}] affected [{}]",
			found_instance_list.id,
			found_instance_list.zone,
			update_instance_list_count
		);


		/**
		 * Delete one
		 */
		int deleted = InstanceListRepository::DeleteOne(database, found_instance_list.id);

		LogInfo("Deleting one instance [{}] deleted count [{}]", found_instance_list.id, deleted);

		/**
		 * Insert many
		 */
		std::vector<InstanceListRepository::InstanceList> instance_lists;

		auto instance_list_entry_bulk = InstanceListRepository::NewEntity();

		instance_list_entry_bulk.zone          = 999;
		instance_list_entry_bulk.version       = 1;
		instance_list_entry_bulk.is_global     = 1;
		instance_list_entry_bulk.start_time    = 0;
		instance_list_entry_bulk.duration      = 0;
		instance_list_entry_bulk.never_expires = 1;

		for (int i = 0; i < 10; i++) {
			instance_lists.push_back(instance_list_entry_bulk);
		}

		/**
		 * Fetch all
		 */
		int inserted_count = InstanceListRepository::InsertMany(database, instance_lists);

		LogInfo("Bulk insertion test, inserted [{}]", inserted_count);

		for (auto &entry: InstanceListRepository::GetWhere(database, fmt::format("zone = {}", 999))) {
			LogInfo("Iterating through entry id [{}] zone [{}]", entry.id, entry.zone);
		}

		/**
		 * Delete where
		 */
		int deleted_count = InstanceListRepository::DeleteWhere(database, fmt::format("zone = {}", 999));

		LogInfo("Bulk deletion test, deleted [{}]", deleted_count);

	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void TestRepository2(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Test command";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		auto zones = ZoneRepository::GetWhere(content_db, "short_name = 'anguish'");

		for (auto &zone: zones) {
			LogInfo(
				"Zone [{}] long_name [{}] id [{}]",
				zone.short_name,
				zone.long_name,
				zone.id
			);
		}
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void CopyCharacter(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Copies a character into a destination account";

		std::vector<std::string> arguments = {
			"source_character_name",
			"destination_character_name",
			"destination_account_name"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		std::string source_character_name      = cmd(2).str();
		std::string destination_character_name = cmd(3).str();
		std::string destination_account_name   = cmd(4).str();

		LogInfo(
			"Attempting to copy character [{}] to [{}] via account [{}]",
			source_character_name,
			destination_character_name,
			destination_account_name
		);

		database.CopyCharacter(
			source_character_name,
			destination_character_name,
			destination_account_name
		);
	}

}
