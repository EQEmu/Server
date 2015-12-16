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
#include "../common/eq_stream_factory.h"
#include "../common/timer.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/eqemu_logsys.h"
#include "login_server.h"
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sstream>

TimeoutManager timeout_manager;
LoginServer server;
EQEmuLogSys Log;
bool run_server = true;

void CatchSignal(int sig_num)
{
}

int main()
{
	RegisterExecutablePlatform(ExePlatformLogin);
	set_exception_handler();
	Log.LoadLogSettingsDefaults();

	Log.log_settings[Logs::Error].log_to_console = Logs::General;

	Log.Out(Logs::General, Logs::Login_Server, "Logging System Init.");

	/* Parse out login.ini */
	server.config = new Config();
	Log.Out(Logs::General, Logs::Login_Server, "Config System Init.");
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
		Log.Out(Logs::General, Logs::Login_Server, "MySQL Database Init.");
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
		Log.Out(Logs::General, Logs::Login_Server, "PostgreSQL Database Init.");
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
		Log.Out(Logs::General, Logs::Error, "Database Initialization Failure.");
		Log.Out(Logs::General, Logs::Login_Server, "Config System Shutdown.");
		delete server.config;
		Log.Out(Logs::General, Logs::Login_Server, "Log System Shutdown.");
		return 1;
	}

#if WIN32
	//initialize our encryption.
	Log.Out(Logs::General, Logs::Login_Server, "Encryption Initialize.");
	server.eq_crypto = new Encryption();
	if (server.eq_crypto->LoadCrypto(server.config->GetVariable("security", "plugin"))) {
		Log.Out(Logs::General, Logs::Login_Server, "Encryption Loaded Successfully.");
	}
	else {
		//We can't run without encryption, cleanup and exit.
		Log.Out(Logs::General, Logs::Error, "Encryption Failed to Load.");
		Log.Out(Logs::General, Logs::Login_Server, "Database System Shutdown.");
		delete server.db;
		Log.Out(Logs::General, Logs::Login_Server, "Config System Shutdown.");
		delete server.config;
		return 1;
	}
#endif

	//create our server manager.
	Log.Out(Logs::General, Logs::Login_Server, "Server Manager Initialize.");
	server.server_manager = new ServerManager();
	if (!server.server_manager) {
		//We can't run without a server manager, cleanup and exit.
		Log.Out(Logs::General, Logs::Error, "Server Manager Failed to Start.");

#ifdef WIN32
		Log.Out(Logs::General, Logs::Login_Server, "Encryption System Shutdown.");
		delete server.eq_crypto;
#endif

		Log.Out(Logs::General, Logs::Login_Server, "Database System Shutdown.");
		delete server.db;
		Log.Out(Logs::General, Logs::Login_Server, "Config System Shutdown.");
		delete server.config;
		return 1;
	}

	//create our client manager.
	Log.Out(Logs::General, Logs::Login_Server, "Client Manager Initialize.");
	server.client_manager = new ClientManager();
	if (!server.client_manager) {
		//We can't run without a client manager, cleanup and exit.
		Log.Out(Logs::General, Logs::Error, "Client Manager Failed to Start.");
		Log.Out(Logs::General, Logs::Login_Server, "Server Manager Shutdown.");
		delete server.server_manager;

#ifdef WIN32
		Log.Out(Logs::General, Logs::Login_Server, "Encryption System Shutdown.");
		delete server.eq_crypto;
#endif

		Log.Out(Logs::General, Logs::Login_Server, "Database System Shutdown.");
		delete server.db;
		Log.Out(Logs::General, Logs::Login_Server, "Config System Shutdown.");
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

	Log.Out(Logs::General, Logs::Login_Server, "Server Started.");
	while (run_server) {
		Timer::SetCurrentTime();
		server.client_manager->Process();
		server.server_manager->Process();
		Sleep(100);
	}

	Log.Out(Logs::General, Logs::Login_Server, "Server Shutdown.");
	Log.Out(Logs::General, Logs::Login_Server, "Client Manager Shutdown.");
	delete server.client_manager;
	Log.Out(Logs::General, Logs::Login_Server, "Server Manager Shutdown.");
	delete server.server_manager;

#ifdef WIN32
	Log.Out(Logs::General, Logs::Login_Server, "Encryption System Shutdown.");
	delete server.eq_crypto;
#endif

	Log.Out(Logs::General, Logs::Login_Server, "Database System Shutdown.");
	delete server.db;
	Log.Out(Logs::General, Logs::Login_Server, "Config System Shutdown.");
	delete server.config;
	return 0;
}

