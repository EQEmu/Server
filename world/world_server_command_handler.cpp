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
		function_map["test:hello-world"] = &WorldserverCommandHandler::HelloWorld;
		function_map["database:version"] = &WorldserverCommandHandler::DatabaseVersion;

		EQEmuCommand::HandleMenu(function_map, cmd, argc, argv);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void HelloWorld(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Test command";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		std::vector<std::string> arguments = {};
		std::vector<std::string> options   = {
			"--hello",
			"--write"
		};

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		LogInfo("hello world!");

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

}