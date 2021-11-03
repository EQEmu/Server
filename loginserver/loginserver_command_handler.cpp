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
		function_map["login-user:check-credentials"]          = &LoginserverCommandHandler::CheckLoginserverUserCredentials;
		function_map["login-user:check-external-credentials"] = &LoginserverCommandHandler::CheckExternalLoginserverUserCredentials;
		function_map["login-user:create"]                     = &LoginserverCommandHandler::CreateLocalLoginserverAccount;
		function_map["login-user:update-credentials"]         = &LoginserverCommandHandler::UpdateLoginserverUserCredentials;
		function_map["web-api-token:create"]                  = &LoginserverCommandHandler::CreateLoginserverApiToken;
		function_map["web-api-token:list"]                    = &LoginserverCommandHandler::ListLoginserverApiTokens;
		function_map["world-admin:create"]                    = &LoginserverCommandHandler::CreateLoginserverWorldAdminAccount;
		function_map["world-admin:update"]                    = &LoginserverCommandHandler::UpdateLoginserverWorldAdminAccountPassword;
		function_map["health:check-login"]                    = &LoginserverCommandHandler::HealthCheckLogin;

		EQEmuCommand::HandleMenu(function_map, cmd, argc, argv);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void CreateLoginserverApiToken(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Creates Loginserver API Token";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		std::vector<std::string> arguments = {};
		std::vector<std::string> options   = {
			"--read",
			"--write"
		};

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

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
	 * @param description
	 */
	void ListLoginserverApiTokens(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Lists Loginserver API Tokens";

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		server.token_manager = new LoginserverWebserver::TokenManager;
		server.token_manager->LoadApiTokens();

		for (auto &it : server.token_manager->loaded_api_tokens) {
			LogInfo(
				"token [{0}] can_write [{1}] can_read [{2}]",
				it.second.token,
				it.second.can_write,
				it.second.can_read
			);
		}
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void CreateLocalLoginserverAccount(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Creates Local Loginserver Account";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {
			"--email=*"
		};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		AccountManagement::CreateLoginServerAccount(
			cmd(2).str(),
			cmd(3).str(),
			cmd("--email").str()
		);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void CreateLoginserverWorldAdminAccount(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Creates Loginserver World Administrator Account";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}",
			"{email}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		AccountManagement::CreateLoginserverWorldAdminAccount(
			cmd(2).str(),
			cmd(3).str(),
			cmd(4).str()
		);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void CheckLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Check user login credentials";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		auto res = AccountManagement::CheckLoginserverUserCredentials(
			cmd(2).str(),
			cmd(3).str()
		);

		LogInfo("Credentials were {0}", res != 0 ? "accepted" : "not accepted");
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void UpdateLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Change user login credentials";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		AccountManagement::UpdateLoginserverUserCredentials(
			cmd(2).str(),
			cmd(3).str()
		);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void CheckExternalLoginserverUserCredentials(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Check user external login credentials";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		auto res = AccountManagement::CheckExternalLoginserverUserCredentials(
			cmd(2).str(),
			cmd(3).str()
		);

		LogInfo("Credentials were {0}", res ? "accepted" : "not accepted");
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void UpdateLoginserverWorldAdminAccountPassword(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Update world admin account password";

		std::vector<std::string> arguments = {
			"{username}",
			"{password}"
		};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

		AccountManagement::UpdateLoginserverWorldAdminAccountPasswordByName(
			cmd(2).str(),
			cmd(3).str()
		);
	}

	/**
	 * @param argc
	 * @param argv
	 * @param cmd
	 * @param description
	 */
	void HealthCheckLogin(int argc, char **argv, argh::parser &cmd, std::string &description)
	{
		description = "Checks login health using a test user";

		std::vector<std::string> arguments = {};
		std::vector<std::string> options   = {};

		if (cmd[{"-h", "--help"}]) {
			return;
		}

		LogInfo("[CLI] [HealthCheck] Response code [{}]", AccountManagement::HealthCheckUserLogin());
	}
}
