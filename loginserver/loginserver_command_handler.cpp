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

#include <iostream>
#include <random>
#include "loginserver_command_handler.h"
#include "../common/util/uuid.h"
#include "login_server.h"
#include "loginserver_webserver.h"
#include "account_management.h"

extern LoginServer server;

namespace LoginserverCommandHandler {

	/**
	 * @param cmd
	 */
	void DisplayDebug(argh::parser &cmd)
	{
		if (cmd[{"-d", "--debug"}]) {
			std::cout << "Positional args:\n";
			for (auto &pos_arg : cmd)
				std::cout << '\t' << pos_arg << std::endl;

			std::cout << "Positional args:\n";
			for (auto &pos_arg : cmd.pos_args())
				std::cout << '\t' << pos_arg << std::endl;

			std::cout << "\nFlags:\n";
			for (auto &flag : cmd.flags())
				std::cout << '\t' << flag << std::endl;

			std::cout << "\nParameters:\n";
			for (auto &param : cmd.params())
				std::cout << '\t' << param.first << " : " << param.second << std::endl;
		}
	}

	/**
	 * @param argc
	 * @param argv
	 */
	void CommandHandler(int argc, char **argv)
	{
		if (argc == 1) { return; }

		argh::parser cmd;
		cmd.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
		LoginserverCommandHandler::DisplayDebug(cmd);

		/**
		 * Declare command mapping
		 */
		std::map<std::string, void (*)(int argc, char **argv, argh::parser &cmd)> function_map;

		function_map["create-loginserver-api-token"]           = &LoginserverCommandHandler::CreateLoginserverApiToken;
		function_map["list-loginserver-api-tokens"]            = &LoginserverCommandHandler::ListLoginserverApiTokens;
		function_map["create-loginserver-account"]             = &LoginserverCommandHandler::CreateLocalLoginserverAccount;
		function_map["create-loginserver-world-admin-account"] = &LoginserverCommandHandler::CreateLoginserverWorldAdminAccount;

		std::map<std::string, void (*)(
			int argc,
			char **argv,
			argh::parser &cmd
		)>::const_iterator it = function_map.begin();

		std::map<std::string, void (*)(
			int argc,
			char **argv,
			argh::parser &cmd
		)>::const_iterator end = function_map.end();

		bool ran_command = false;
		while (it != end) {
			if (it->first == argv[1]) {
				std::cout << std::endl;
				std::cout << "###########################################################" << std::endl;
				std::cout << "# Executing CLI Command" << std::endl;
				std::cout << "###########################################################" << std::endl;
				std::cout << std::endl;

				(it->second)(argc, argv, cmd);
				ran_command = true;
			}
			++it;
		}

		if (cmd[{"-h", "--help"}] || !ran_command) {
			std::cout << std::endl;
			std::cout << "###########################################################" << std::endl;
			std::cout << "# Loginserver CLI Menu" << std::endl;
			std::cout << "###########################################################" << std::endl;
			std::cout << std::endl;
			std::cout << "# API" << std::endl;
			std::cout << "> create-loginserver-api-token --write --read" << std::endl;
			std::cout << "> list-loginserver-api-tokens" << std::endl;
			std::cout << std::endl;
			std::cout << "# User Accounts" << std::endl;
			std::cout << "> create-loginserver-account --username=* --password=*" << std::endl;
			std::cout << std::endl;
			std::cout << "# World Accounts" << std::endl;
			std::cout << "> create-loginserver-world-admin-account --username=* --password=* --email=*" << std::endl;
			std::cout << std::endl;
			std::cout << std::endl;
		}

		exit(1);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 */
	void CreateLoginserverApiToken(int argc, char **argv, argh::parser &cmd)
	{
		bool can_read  = cmd[{"-r", "--read"}];
		bool can_write = cmd[{"-w", "--write"}];

		if (!can_read || !can_write) {
			LogInfo("[{0}] --read or --write must be set or both!", __func__);
			exit(1);
		}

		std::string token = server.db->CreateLoginserverApiToken(can_write, can_read);
		if (!token.empty()) {
			LogInfo("[{0}] Created Loginserver API token [{1}]", __func__, token);
		}
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 */
	void ListLoginserverApiTokens(int argc, char **argv, argh::parser &cmd)
	{
		for (auto it = server.token_manager->loaded_api_tokens.begin();
			it != server.token_manager->loaded_api_tokens.end();
			++it) {
			LogInfo(
				"token [{0}] can_write [{1}] can_read [{2}]",
				it->second.token,
				it->second.can_write,
				it->second.can_read
			);
		}
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 */
	void CreateLocalLoginserverAccount(int argc, char **argv, argh::parser &cmd)
	{
		if (cmd("--username").str().empty() || cmd("--password").str().empty()) {
			LogInfo("Command Example: create-loginserver-account --username=user --password=password");
			exit(1);
		}

		AccountManagement::CreateLocalLoginServerAccount(cmd("--username").str(), cmd("--password").str());
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 */
	void CreateLoginserverWorldAdminAccount(int argc, char **argv, argh::parser &cmd)
	{
		if (
			cmd("--username").str().empty() ||
			cmd("--password").str().empty() ||
			cmd("--email").str().empty()) {

			LogInfo("Command Example: create-loginserver-world-admin-account --username=* --password=* --email=*");
			exit(1);
		}

		AccountManagement::CreateLoginserverWorldAdminAccount(
			cmd("--username").str(),
			cmd("--password").str(),
			cmd("--email").str()
		);
	}

}
