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
#include <signal.h>
#include <string>
#include <sstream>
#include <fstream>

TimeoutManager timeout_manager;
LoginServer server;
EQEmuLogSys Log;
bool run_server = true;

void CatchSignal(int sig_num)
{
	_eqp_dump_file("loginserver");
}

int main()
{
	RegisterExecutablePlatform(ExePlatformLogin);
	set_exception_handler();
	_eqp

	Log.LoadLogSettingsDefaults();

	//log_settings
	Log.log_settings[Logs::LogCategory::Crash].log_to_file = true;
	Log.log_settings[Logs::LogCategory::Error].log_to_console = true;
	Log.log_settings[Logs::LogCategory::Error].log_to_file = true;
	Log.log_settings[Logs::LogCategory::Debug].log_to_console = true;
	Log.log_settings[Logs::LogCategory::Debug].log_to_file = true;
	Log.log_settings[Logs::LogCategory::Database].log_to_console = true;
	Log.log_settings[Logs::LogCategory::Database].log_to_file = true;
	Log.log_settings[Logs::LogCategory::World_Server].log_to_console = true;
	Log.log_settings[Logs::LogCategory::World_Server].log_to_file = true;
	Log.log_settings[Logs::LogCategory::Netcode].log_to_console = true;
	Log.log_settings[Logs::LogCategory::Netcode].log_to_file = true;
	Log.file_logs_enabled = true;
	Log.StartFileLogs();

	if(signal(SIGINT, CatchSignal) == SIG_ERR)	{
		Log.Out(Logs::Detail, Logs::Error, "Could not set signal handler");
		return 1;
	}
	if(signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		Log.Out(Logs::Detail, Logs::Error, "Could not set signal handler");
		return 1;
	}
	if(signal(SIGBREAK, CatchSignal) == SIG_ERR)	{
		Log.Out(Logs::Detail, Logs::Error, "Could not set signal handler");
		return 1;
	}

	//Create our subsystem and parse the ini file.
	server.config = new Config();
	Log.Out(Logs::General, Logs::Debug, "Config System Init.");
	server.config->Parse("login.ini");

	//Parse unregistered allowed option.
	if(server.config->GetVariable("options", "unregistered_allowed").compare("FALSE") == 0)
	{
		server.options.AllowUnregistered(false);
	}

	//Parse trace option.
	if(server.config->GetVariable("options", "trace").compare("TRUE") == 0)
	{
		server.options.Trace(true);
	}

	//Parse trace option.
	if(server.config->GetVariable("options", "world_trace").compare("TRUE") == 0)
	{
		server.options.WorldTrace(true);
	}

	//Parse packet inc dump option.
	if(server.config->GetVariable("options", "dump_packets_in").compare("TRUE") == 0)
	{
		server.options.DumpInPackets(true);
	}

	//Parse packet out dump option.
	if(server.config->GetVariable("options", "dump_packets_out").compare("TRUE") == 0)
	{
		server.options.DumpOutPackets(true);
	}

	//Parse encryption mode option.
	std::string mode = server.config->GetVariable("security", "mode");
	if(mode.size() > 0)
	{
		server.options.EncryptionMode(atoi(mode.c_str()));
	}

	//Parse local network option.
	std::string ln = server.config->GetVariable("options", "local_network");
	if(ln.size() > 0)
	{
		server.options.LocalNetwork(ln);
	}

	//Parse reject duplicate servers option.
	if(server.config->GetVariable("options", "reject_duplicate_servers").compare("TRUE") == 0)
	{
		server.options.RejectDuplicateServers(true);
	}

	//Parse account table option.
	ln = server.config->GetVariable("schema", "account_table");
	if(ln.size() > 0)
	{
		server.options.AccountTable(ln);
	}

	//Parse world account table option.
	ln = server.config->GetVariable("schema", "world_registration_table");
	if(ln.size() > 0)
	{
		server.options.WorldRegistrationTable(ln);
	}

	//Parse admin world account table option.
	ln = server.config->GetVariable("schema", "world_admin_registration_table");
	if(ln.size() > 0)
	{
		server.options.WorldAdminRegistrationTable(ln);
	}

	//Parse world type table option.
	ln = server.config->GetVariable("schema", "world_server_type_table");
	if(ln.size() > 0)
	{
		server.options.WorldServerTypeTable(ln);
	}

	//Create our DB from options.
	if(server.config->GetVariable("database", "subsystem").compare("MySQL") == 0)
	{
#ifdef EQEMU_MYSQL_ENABLED
		Log.Out(Logs::General, Logs::Debug, "MySQL Database Init.");
		server.db = (Database*)new DatabaseMySQL(
			server.config->GetVariable("database", "user"),
			server.config->GetVariable("database", "password"),
			server.config->GetVariable("database", "host"),
			server.config->GetVariable("database", "port"),
			server.config->GetVariable("database", "db"));
#endif
	}
	else if(server.config->GetVariable("database", "subsystem").compare("PostgreSQL") == 0)
	{
#ifdef EQEMU_POSTGRESQL_ENABLED
		Log.Out(Logs::General, Logs::Debug, "PostgreSQL Database Init.");
		server.db = (Database*)new DatabasePostgreSQL(
			server.config->GetVariable("database", "user"),
			server.config->GetVariable("database", "password"),
			server.config->GetVariable("database", "host"),
			server.config->GetVariable("database", "port"),
			server.config->GetVariable("database", "db"));
#endif
	}

	//Make sure our database got created okay, otherwise cleanup and exit.
	if(!server.db)
	{
		Log.Out(Logs::Detail, Logs::Error, "Database Initialization Failure.");
		Log.Out(Logs::General, Logs::Debug, "Config System Shutdown.");
		delete server.config;
		return 1;
	}

#if WIN32
	//initialize our encryption.
	Log.Out(Logs::General, Logs::Debug, "Encryption Initialize.");
	server.eq_crypto = new Encryption();
	if(server.eq_crypto->LoadCrypto(server.config->GetVariable("security", "plugin")))
	{
		Log.Out(Logs::General, Logs::Debug, "Encryption Loaded Successfully.");
	}
	else
	{
		//We can't run without encryption, cleanup and exit.
		Log.Out(Logs::Detail, Logs::Error, "Encryption Failed to Load.");
		Log.Out(Logs::General, Logs::Debug, "Database System Shutdown.");
		delete server.db;
		Log.Out(Logs::General, Logs::Debug, "Config System Shutdown.");
		delete server.config;
		return 1;
	}
#endif

	//create our server manager.
	Log.Out(Logs::General, Logs::Debug, "Server Manager Initialize.");
	server.SM = new ServerManager();
	if(!server.SM)
	{
		//We can't run without a server manager, cleanup and exit.
		Log.Out(Logs::Detail, Logs::Error, "Server Manager Failed to Start.");
#ifdef WIN32
		Log.Out(Logs::General, Logs::Debug, "Encryption System Shutdown.");
		delete server.eq_crypto;
#endif
		Log.Out(Logs::General, Logs::Debug, "Database System Shutdown.");
		delete server.db;
		Log.Out(Logs::General, Logs::Debug, "Config System Shutdown.");
		delete server.config;
		return 1;
	}

	//create our client manager.
	Log.Out(Logs::General, Logs::Debug, "Client Manager Initialize.");
	server.CM = new ClientManager();
	if(!server.CM)
	{
		//We can't run without a client manager, cleanup and exit.
		Log.Out(Logs::Detail, Logs::Error, "Client Manager Failed to Start.");
		Log.Out(Logs::General, Logs::Debug, "Server Manager Shutdown.");
		delete server.SM;
#ifdef WIN32
		Log.Out(Logs::General, Logs::Debug, "Encryption System Shutdown.");
		delete server.eq_crypto;
#endif
		Log.Out(Logs::General, Logs::Debug, "Database System Shutdown.");
		delete server.db;
		Log.Out(Logs::General, Logs::Debug, "Config System Shutdown.");
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

	Log.Out(Logs::General, Logs::Debug, "Server Started.");
	while(run_server)
	{
		Timer::SetCurrentTime();
		server.CM->Process();
		server.SM->Process();
		Sleep(50);
	}

	Log.Out(Logs::General, Logs::Debug, "Server Shutdown.");
	Log.Out(Logs::General, Logs::Debug, "Client Manager Shutdown.");
	delete server.CM;
	Log.Out(Logs::General, Logs::Debug, "Server Manager Shutdown.");
	delete server.SM;
#ifdef WIN32
	Log.Out(Logs::General, Logs::Debug, "Encryption System Shutdown.");
	delete server.eq_crypto;
#endif
	Log.Out(Logs::General, Logs::Debug, "Database System Shutdown.");
	delete server.db;
	Log.Out(Logs::General, Logs::Debug, "Config System Shutdown.");
	delete server.config;
	return 0;
}

