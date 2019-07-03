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
#include "login_server.h"
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

int main()
{
	RegisterExecutablePlatform(ExePlatformLogin);
	set_exception_handler();

	LogSys.LoadLogSettingsDefaults();

	LogSys.log_settings[Logs::Error].log_to_console      = Logs::General;
	LogSys.log_settings[Logs::Error].is_category_enabled = 1;

	Log(Logs::General, Logs::Login_Server, "Logging System Init.");

	server.config = EQ::JsonConfigFile::Load("login.json");
	Log(Logs::General, Logs::Login_Server, "Config System Init.");

	server.options.Trace(server.config.GetVariableBool("general", "trace", false));
	server.options.WorldTrace(server.config.GetVariableBool("general", "world_trace", false));
	server.options.DumpInPackets(server.config.GetVariableBool("general", "dump_packets_in", false));
	server.options.DumpOutPackets(server.config.GetVariableBool("general", "dump_packets_out", false));
	server.options.LocalNetwork(server.config.GetVariableString("general", "local_network", "192.168.1."));
	server.options.RejectDuplicateServers(server.config.GetVariableBool("general", "reject_duplicate_servers", false));
	server.options.AutoCreateAccounts(server.config.GetVariableBool("general", "auto_create_accounts", true));
	server.options.AutoLinkAccounts(server.config.GetVariableBool("general", "auto_link_accounts", true));

	server.options.EQEmuLoginServerAddress(
		server.config.GetVariableString(
			"general",
			"eqemu_loginserver_address",
			"login.eqemulator.net:5999"
		)
	);

	server.options.DefaultLoginServerName(
		server.config.GetVariableString(
			"general",
			"default_loginserver_name",
			"peq"
		)
	);


#ifdef ENABLE_SECURITY
	server.options.EncryptionMode(server.config.GetVariableInt("security", "mode", 13));
#else
	server.options.EncryptionMode(server.config.GetVariableInt("security", "mode", 6));
#endif

	server.options.AllowUnregistered(server.config.GetVariableBool("security", "unregistered_allowed", true));
	server.options.AllowTokenLogin(server.config.GetVariableBool("security", "allow_token_login", false));
	server.options.AllowPasswordLogin(server.config.GetVariableBool("security", "allow_password_login", true));
	server.options.UpdateInsecurePasswords(
		server.config.GetVariableBool(
			"security",
			"update_insecure_passwords",
			true
		)
	);
	server.options.AccountTable(server.config.GetVariableString("schema", "account_table", "tblLoginServerAccounts"));
	server.options.WorldRegistrationTable(
		server.config.GetVariableString(
			"schema",
			"world_registration_table",
			"tblWorldServerRegistration"
		)
	);
	server.options.WorldAdminRegistrationTable(
		server.config.GetVariableString(
			"schema",
			"world_admin_registration_table",
			"tblServerAdminRegistration"
		)
	);
	server.options.WorldServerTypeTable(
		server.config.GetVariableString(
			"schema",
			"world_server_type_table",
			"tblServerListType"
		)
	);

	/**
	 * mysql connect
	 */
	Log(Logs::General, Logs::Login_Server, "MySQL Database Init.");

	server.db = (Database *) new DatabaseMySQL(
		server.config.GetVariableString("database", "user", "root"),
		server.config.GetVariableString("database", "password", ""),
		server.config.GetVariableString("database", "host", "localhost"),
		server.config.GetVariableString("database", "port", "3306"),
		server.config.GetVariableString("database", "db", "peq")
	);


	/**
	 * make sure our database got created okay, otherwise cleanup and exit
	 */
	if (!server.db) {
		Log(Logs::General, Logs::Error, "Database Initialization Failure.");
		Log(Logs::General, Logs::Login_Server, "Log System Shutdown.");
		return 1;
	}

	/**
	 * create server manager
	 */
	Log(Logs::General, Logs::Login_Server, "Server Manager Initialize.");
	server.server_manager = new ServerManager();
	if (!server.server_manager) {
		Log(Logs::General, Logs::Error, "Server Manager Failed to Start.");
		Log(Logs::General, Logs::Login_Server, "Database System Shutdown.");
		delete server.db;
		return 1;
	}

	/**
	 * create client manager
	 */
	Log(Logs::General, Logs::Login_Server, "Client Manager Initialize.");
	server.client_manager = new ClientManager();
	if (!server.client_manager) {
		Log(Logs::General, Logs::Error, "Client Manager Failed to Start.");
		Log(Logs::General, Logs::Login_Server, "Server Manager Shutdown.");
		delete server.server_manager;

		Log(Logs::General, Logs::Login_Server, "Database System Shutdown.");
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

	Log(Logs::General, Logs::Login_Server, "Server Started.");
	while (run_server) {
		Timer::SetCurrentTime();
		server.client_manager->Process();
		EQ::EventLoop::Get().Process();
		Sleep(5);
	}

	Log(Logs::General, Logs::Login_Server, "Server Shutdown.");
	Log(Logs::General, Logs::Login_Server, "Client Manager Shutdown.");
	delete server.client_manager;
	Log(Logs::General, Logs::Login_Server, "Server Manager Shutdown.");
	delete server.server_manager;

	Log(Logs::General, Logs::Login_Server, "Database System Shutdown.");
	delete server.db;
	return 0;
}
