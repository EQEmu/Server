/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

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
bool run_server = true;

void CatchSignal(int sig_num)
{
}

int main()
{
	RegisterExecutablePlatform(ExePlatformLogin);
	set_exception_handler();
	LogSys.LoadLogSettingsDefaults();

	LogSys.log_settings[Logs::Error].log_to_console = Logs::General;
	LogSys.log_settings[Logs::Error].is_category_enabled = 1;

	Log(Logs::General, Logs::Login_Server, "Logging System Init.");

	/* Parse out login.ini */
	server.config = new Config();
	Log(Logs::General, Logs::Login_Server, "Config System Init.");
	server.config->Parse("login.ini");

	if (server.config->GetVariable("options", "unregistered_allowed").compare("FALSE") == 0)
		server.options.AllowUnregistered(false);

	if (server.config->GetVariable("options", "trace").compare("TRUE") == 0)
		server.options.Trace(true);

	if (server.config->GetVariable("options", "world_trace").compare("TRUE") == 0)
		server.options.WorldTrace(true);

	if (server.config->GetVariable("options", "dump_packets_in").compare("TRUE") == 0)
		server.options.DumpInPackets(true);

	if (server.config->GetVariable("options", "dump_packets_out").compare("TRUE") == 0)
		server.options.DumpOutPackets(true);

	if (server.config->GetVariable("security", "allow_token_login").compare("TRUE") == 0)
		server.options.AllowTokenLogin(true);

	if (server.config->GetVariable("security", "allow_password_login").compare("FALSE") == 0)
		server.options.AllowPasswordLogin(false);

	if (server.config->GetVariable("options", "auto_create_accounts").compare("TRUE") == 0)
		server.options.AutoCreateAccounts(true);

	std::string mode = server.config->GetVariable("security", "mode");
	if (mode.size() > 0)
		server.options.EncryptionMode(atoi(mode.c_str()));

	std::string local_network = server.config->GetVariable("options", "local_network");
	if (local_network.size() > 0)
		server.options.LocalNetwork(local_network);

	if (server.config->GetVariable("options", "reject_duplicate_servers").compare("TRUE") == 0)
		server.options.RejectDuplicateServers(true);

	local_network = server.config->GetVariable("schema", "account_table");
	if (local_network.size() > 0)
		server.options.AccountTable(local_network);

	local_network = server.config->GetVariable("schema", "world_registration_table");
	if (local_network.size() > 0)
		server.options.WorldRegistrationTable(local_network);

	local_network = server.config->GetVariable("schema", "world_admin_registration_table");
	if (local_network.size() > 0)
		server.options.WorldAdminRegistrationTable(local_network);

	local_network = server.config->GetVariable("schema", "world_server_type_table");
	if (local_network.size() > 0)
		server.options.WorldServerTypeTable(local_network);

	/* Create database connection */
	if (server.config->GetVariable("database", "subsystem").compare("MySQL") == 0) {
#ifdef EQEMU_MYSQL_ENABLED
		Log(Logs::General, Logs::Login_Server, "MySQL Database Init.");
		server.db = (Database*)new DatabaseMySQL(
			server.config->GetVariable("database", "user"),
			server.config->GetVariable("database", "password"),
			server.config->GetVariable("database", "host"),
			server.config->GetVariable("database", "port"),
			server.config->GetVariable("database", "db"));
#endif
	}
	else if (server.config->GetVariable("database", "subsystem").compare("PostgreSQL") == 0) {
#ifdef EQEMU_POSTGRESQL_ENABLED
		Log(Logs::General, Logs::Login_Server, "PostgreSQL Database Init.");
		server.db = (Database*)new DatabasePostgreSQL(
			server.config->GetVariable("database", "user"),
			server.config->GetVariable("database", "password"),
			server.config->GetVariable("database", "host"),
			server.config->GetVariable("database", "port"),
			server.config->GetVariable("database", "db"));
#endif
	}

	/* Make sure our database got created okay, otherwise cleanup and exit. */
	if (!server.db) {
		Log(Logs::General, Logs::Error, "Database Initialization Failure.");
		Log(Logs::General, Logs::Login_Server, "Config System Shutdown.");
		delete server.config;
		Log(Logs::General, Logs::Login_Server, "Log System Shutdown.");
		return 1;
	}

	//create our server manager.
	Log(Logs::General, Logs::Login_Server, "Server Manager Initialize.");
	server.server_manager = new ServerManager();
	if (!server.server_manager) {
		//We can't run without a server manager, cleanup and exit.
		Log(Logs::General, Logs::Error, "Server Manager Failed to Start.");

		Log(Logs::General, Logs::Login_Server, "Database System Shutdown.");
		delete server.db;
		Log(Logs::General, Logs::Login_Server, "Config System Shutdown.");
		delete server.config;
		return 1;
	}

	//create our client manager.
	Log(Logs::General, Logs::Login_Server, "Client Manager Initialize.");
	server.client_manager = new ClientManager();
	if (!server.client_manager) {
		//We can't run without a client manager, cleanup and exit.
		Log(Logs::General, Logs::Error, "Client Manager Failed to Start.");
		Log(Logs::General, Logs::Login_Server, "Server Manager Shutdown.");
		delete server.server_manager;

		Log(Logs::General, Logs::Login_Server, "Database System Shutdown.");
		delete server.db;
		Log(Logs::General, Logs::Login_Server, "Config System Shutdown.");
		delete server.config;
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
	Log(Logs::General, Logs::Login_Server, "Config System Shutdown.");
	delete server.config;
	return 0;
}
