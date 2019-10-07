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

#include "../common/global_define.h"
#include "../common/types.h"
#include "../common/opcodemgr.h"
#include "../common/event/event_loop.h"
#include "../common/timer.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/eqemu_logsys.h"
#include "../common/http/httplib.h"
#include "login_server.h"
#include "loginserver_webserver.h"
#include "loginserver_command_handler.h"
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sstream>

LoginServer server;
EQEmuLogSys LogSys;
bool        run_server = true;

void CatchSignal(int sig_num)
{
}

int main(int argc, char **argv)
{
	RegisterExecutablePlatform(ExePlatformLogin);
	set_exception_handler();

	LogInfo("Logging System Init");

	if (argc == 1) {
		LogSys.LoadLogSettingsDefaults();
	}

	server.config = EQ::JsonConfigFile::Load("login.json");
	LogInfo("Config System Init");

	/**
	 * options: logging
	 */
	server.options.Trace(server.config.GetVariableBool("logging", "trace", false));
	server.options.WorldTrace(server.config.GetVariableBool("logging", "world_trace", false));
	server.options.DumpInPackets(server.config.GetVariableBool("logging", "dump_packets_in", false));
	server.options.DumpOutPackets(server.config.GetVariableBool("logging", "dump_packets_out", false));

	/**
	 * options: worldservers
	 */
	server.options.RejectDuplicateServers(
		server.config.GetVariableBool(
			"worldservers",
			"reject_duplicate_servers",
			false
		));
	server.options.AllowUnregistered(server.config.GetVariableBool("worldservers", "unregistered_allowed", true));

	/**
	 * options: account
	 */
	server.options.AutoCreateAccounts(server.config.GetVariableBool("account", "auto_create_accounts", true));
	server.options.AutoLinkAccounts(server.config.GetVariableBool("account", "auto_link_accounts", false));

#ifdef LSPX
	server.options.EQEmuLoginServerAddress(
		server.config.GetVariableString(
			"general",
			"eqemu_loginserver_address",
			"login.eqemulator.net:5999"
		)
	);
#endif

	server.options.DefaultLoginServerName(
		server.config.GetVariableString(
			"general",
			"default_loginserver_name",
			"local"
		)
	);

#ifdef ENABLE_SECURITY
	server.options.EncryptionMode(server.config.GetVariableInt("security", "mode", 13));
#else
	server.options.EncryptionMode(server.config.GetVariableInt("security", "mode", 6));
#endif

	server.options.AllowTokenLogin(server.config.GetVariableBool("security", "allow_token_login", false));
	server.options.AllowPasswordLogin(server.config.GetVariableBool("security", "allow_password_login", true));
	server.options.UpdateInsecurePasswords(
		server.config.GetVariableBool(
			"security",
			"update_insecure_passwords",
			true
		)
	);

	/**
	 * mysql connect
	 */
	LogInfo("MySQL Database Init");

	server.db = new Database(
		server.config.GetVariableString("database", "user", "root"),
		server.config.GetVariableString("database", "password", ""),
		server.config.GetVariableString("database", "host", "localhost"),
		server.config.GetVariableString("database", "port", "3306"),
		server.config.GetVariableString("database", "db", "peq")
	);

	if (argc == 1) {
		server.db->LoadLogSettings(LogSys.log_settings);
		LogSys.StartFileLogs();
	}

	/**
	 * make sure our database got created okay, otherwise cleanup and exit
	 */
	if (!server.db) {
		LogError("Database Initialization Failure");
		LogInfo("Log System Shutdown");
		return 1;
	}

	/**
	 * create server manager
	 */
	LogInfo("Server Manager Init");
	server.server_manager = new ServerManager();
	if (!server.server_manager) {
		LogError("Server Manager Failed to Start");
		LogInfo("Database System Shutdown");
		delete server.db;
		return 1;
	}

	/**
	 * create client manager
	 */
	LogInfo("Client Manager Init");
	server.client_manager           = new ClientManager();
	if (!server.client_manager) {
		LogError("Client Manager Failed to Start");
		LogInfo("Server Manager Shutdown");
		delete server.server_manager;

		LogInfo("Database System Shutdown");
		delete server.db;
		return 1;
	}

#ifdef WIN32
#ifdef UNICODE
		SetConsoleTitle(L"EQEmu Login Server");
#else
		SetConsoleTitle("EQEmu Login Server");
#endif
#endif

	LogInfo("Server Started");
	if (LogSys.log_settings[Logs::Loginserver].log_to_console == 1) {
		LogInfo("Loginserver logging set to level [1] for more debugging, enable detail [3]");
	}

	/**
	 * Web API
	 */
	httplib::Server api;
	int             web_api_port    = server.config.GetVariableInt("web_api", "port", 6000);
	bool            web_api_enabled = server.config.GetVariableBool("web_api", "enabled", true);
	if (web_api_enabled) {
		api.bind("0.0.0.0", web_api_port);
		LogInfo("Webserver API now listening on port [{0}]", web_api_port);
		LoginserverWebserver::RegisterRoutes(api);
	}

	if (argc > 1) {
		LogSys.LoadLogSettingsDefaults();
		LogSys.log_settings[Logs::Debug].log_to_console      = static_cast<uint8>(Logs::General);
		LogSys.log_settings[Logs::Debug].is_category_enabled = 1;

		LoginserverCommandHandler::CommandHandler(argc, argv);
	}

	LogInfo("[Config] [Logging] IsTraceOn [{0}]", server.options.IsTraceOn());
	LogInfo("[Config] [Logging] IsWorldTraceOn [{0}]", server.options.IsWorldTraceOn());
	LogInfo("[Config] [Logging] IsDumpInPacketsOn [{0}]", server.options.IsDumpInPacketsOn());
	LogInfo("[Config] [Logging] IsDumpOutPacketsOn [{0}]", server.options.IsDumpOutPacketsOn());
	LogInfo("[Config] [Account] CanAutoCreateAccounts [{0}]", server.options.CanAutoCreateAccounts());
#ifdef LSPX
	LogInfo("[Config] [Account] CanAutoLinkAccounts [{0}]", server.options.CanAutoLinkAccounts());
#endif
	LogInfo("[Config] [WorldServer] IsRejectingDuplicateServers [{0}]", server.options.IsRejectingDuplicateServers());
	LogInfo("[Config] [WorldServer] IsUnregisteredAllowed [{0}]", server.options.IsUnregisteredAllowed());
	LogInfo("[Config] [Security] GetEncryptionMode [{0}]", server.options.GetEncryptionMode());
	LogInfo("[Config] [Security] IsTokenLoginAllowed [{0}]", server.options.IsTokenLoginAllowed());
	LogInfo("[Config] [Security] IsPasswordLoginAllowed [{0}]", server.options.IsPasswordLoginAllowed());
	LogInfo("[Config] [Security] IsUpdatingInsecurePasswords [{0}]", server.options.IsUpdatingInsecurePasswords());

	while (run_server) {
		Timer::SetCurrentTime();
		server.client_manager->Process();
		EQ::EventLoop::Get().Process();

		if (web_api_enabled) {
			api.poll();
		}

		Sleep(5);
	}

	LogInfo("Server Shutdown");

	LogInfo("Client Manager Shutdown");
	delete server.client_manager;

	LogInfo("Server Manager Shutdown");
	delete server.server_manager;

	LogInfo("Database System Shutdown");
	delete server.db;

	return 0;
}
