/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#include "../common/debug.h"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include "../common/debug.h"
#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQPacket.h"
#include "client.h"
#include "worlddb.h"
#include "../common/seperator.h"
#include "../common/version.h"
#include "../common/eqtime.h"
#include "../common/timeoutmgr.h"
#include "../common/EQEMuError.h"
#include "../common/opcodemgr.h"
#include "../common/guilds.h"
#include "../common/EQStreamIdent.h"
//#include "../common/patches/Client62.h"
#include "../common/rulesys.h"
#include "../common/platform.h"
#include "../common/crash.h"
#ifdef _WINDOWS
	#include <process.h>
	#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
	#include <conio.h>
#else
	#include <pthread.h>
	#include "../common/unix.h"
	#include <sys/types.h>
	#include <sys/ipc.h>
	#include <sys/sem.h>
	#include <sys/shm.h>
	#ifndef FREEBSD
		union semun {
			int val;
			struct semid_ds *buf;
			ushort *array;
			struct seminfo *__buf;
			void *__pad;
		};
	#endif

#endif

/*
Zone only right now.
#ifdef EQPROFILE
#ifdef COMMON_PROFILE
CommonProfiler _cp;
#endif
#endif*/

#include "zoneserver.h"
#include "console.h"
#include "LoginServer.h"
#include "LoginServerList.h"
#include "EQWHTTPHandler.h"
#include "../common/dbasync.h"
#include "../common/EmuTCPServer.h"
#include "WorldConfig.h"
#include "../common/patches/patches.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "clientlist.h"
#include "LauncherList.h"
#include "wguild_mgr.h"
#include "lfplist.h"
#include "AdventureManager.h"
#include "ucs.h"
#include "queryserv.h"

TimeoutManager timeout_manager;
EQStreamFactory eqsf(WorldStream,9000);
EmuTCPServer tcps;
ClientList client_list;
GroupLFPList LFPGroupList;
ZSList zoneserver_list;
LoginServerList loginserverlist;
EQWHTTPServer http_server;
UCSConnection UCSLink;
QueryServConnection QSLink;
LauncherList launcher_list;
AdventureManager adventure_manager;
DBAsync *dbasync = nullptr;
volatile bool RunLoops = true;
uint32 numclients = 0;
uint32 numzones = 0;
bool holdzones = false;


extern ConsoleList console_list;

void CatchSignal(int sig_num);

int main(int argc, char** argv) {
	RegisterExecutablePlatform(ExePlatformWorld);
	set_exception_handler();

	// Load server configuration
	_log(WORLD__INIT, "Loading server configuration..");
	if (!WorldConfig::LoadConfig()) {
		_log(WORLD__INIT_ERR, "Loading server configuration failed.");
		return(1);
	}
	const WorldConfig *Config=WorldConfig::get();

	if(!load_log_settings(Config->LogSettingsFile.c_str()))
		_log(WORLD__INIT, "Warning: Unable to read %s", Config->LogSettingsFile.c_str());
	else
		_log(WORLD__INIT, "Log settings loaded from %s", Config->LogSettingsFile.c_str());


	_log(WORLD__INIT, "CURRENT_VERSION: %s", CURRENT_VERSION);

	#ifdef _DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(WORLD__INIT_ERR, "Could not set signal handler");
		return 0;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(WORLD__INIT_ERR, "Could not set signal handler");
		return 0;
	}
	#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)	{
		_log(WORLD__INIT_ERR, "Could not set signal handler");
		return 0;
	}
	#endif

	// add login server config to list
	if (Config->LoginCount == 0) {
		if (Config->LoginHost.length()) {
			loginserverlist.Add(Config->LoginHost.c_str(), Config->LoginPort, Config->LoginAccount.c_str(), Config->LoginPassword.c_str());
			_log(WORLD__INIT, "Added loginserver %s:%i", Config->LoginHost.c_str(), Config->LoginPort);
		}
	} else {
		LinkedList<LoginConfig*> loginlist=Config->loginlist;
		LinkedListIterator<LoginConfig*> iterator(loginlist);
		iterator.Reset();
		while(iterator.MoreElements()) {
			loginserverlist.Add(iterator.GetData()->LoginHost.c_str(), iterator.GetData()->LoginPort, iterator.GetData()->LoginAccount.c_str(), iterator.GetData()->LoginPassword.c_str());
			_log(WORLD__INIT, "Added loginserver %s:%i", iterator.GetData()->LoginHost.c_str(), iterator.GetData()->LoginPort);
			iterator.Advance();
		}
	}

	_log(WORLD__INIT, "Connecting to MySQL...");
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		_log(WORLD__INIT_ERR, "Cannot continue without a database connection.");
		return(1);
	}
	dbasync = new DBAsync(&database);
	guild_mgr.SetDatabase(&database);

	if (argc >= 2) {
		char tmp[2];
		if (strcasecmp(argv[1], "help") == 0 || strcasecmp(argv[1], "?") == 0 || strcasecmp(argv[1], "/?") == 0 || strcasecmp(argv[1], "-?") == 0 || strcasecmp(argv[1], "-h") == 0 || strcasecmp(argv[1], "-help") == 0) {
			std::cout << "Worldserver command line commands:" << std::endl;
			std::cout << "adduser username password flag    - adds a user account" << std::endl;
			std::cout << "flag username flag    - sets GM flag on the account" << std::endl;
			std::cout << "startzone zoneshortname    - sets the starting zone" << std::endl;
			std::cout << "-holdzones    - reboots lost zones" << std::endl;
			return 0;
		}
		else if (strcasecmp(argv[1], "-holdzones") == 0) {
			std::cout << "Reboot Zones mode ON" << std::endl;
			holdzones = true;
		}
		else if (database.GetVariable("disablecommandline", tmp, 2)) {
			if (strlen(tmp) == 1) {
				if (tmp[0] == '1') {
					std::cout << "Command line disabled in database... exiting" << std::endl;
					return 0;
				}
			}
		}
		else if (strcasecmp(argv[1], "adduser") == 0) {
			if (argc == 5) {
				if (Seperator::IsNumber(argv[4])) {
					if (atoi(argv[4]) >= 0 && atoi(argv[4]) <= 255) {
						if (database.CreateAccount(argv[2], argv[3], atoi(argv[4])) == 0)
							std::cout << "database.CreateAccount failed." << std::endl;
						else
							std::cout << "Account created: Username='" << argv[2] << "', Password='" << argv[3] << "', status=" << argv[4] << std::endl;
						return 0;
					}
				}
			}
			std::cout << "Usage: world adduser username password flag" << std::endl;
			std::cout << "flag = 0, 1 or 2" << std::endl;
			return 0;
		}
		else if (strcasecmp(argv[1], "flag") == 0) {
			if (argc == 4) {
				if (Seperator::IsNumber(argv[3])) {

					if (atoi(argv[3]) >= 0 && atoi(argv[3]) <= 255) {
						if (database.SetAccountStatus(argv[2], atoi(argv[3])))
							std::cout << "Account flagged: Username='" << argv[2] << "', status=" << argv[3] << std::endl;
						else
							std::cout << "database.SetAccountStatus failed." << std::endl;
						return 0;
					}
				}
			}
			std::cout << "Usage: world flag username flag" << std::endl;
			std::cout << "flag = 0-200" << std::endl;
			return 0;
		}
		else if (strcasecmp(argv[1], "startzone") == 0) {
			if (argc == 3) {
				if (strlen(argv[2]) < 3) {
					std::cout << "Error: zone name too short" << std::endl;
				}
				else if (strlen(argv[2]) > 15) {
					std::cout << "Error: zone name too long" << std::endl;
				}
				else {
					if (database.SetVariable("startzone", argv[2]))
						std::cout << "Starting zone changed: '" << argv[2] << "'" << std::endl;
					else
						std::cout << "database.SetVariable failed." << std::endl;
				}
				return 0;
			}
			std::cout << "Usage: world startzone zoneshortname" << std::endl;
			return 0;
		}
		else {
			std::cout << "Error, unknown command line option" << std::endl;
			return 0;
		}
	}

	if(Config->WorldHTTPEnabled) {
		_log(WORLD__INIT, "Starting HTTP world service...");
		http_server.Start(Config->WorldHTTPPort, Config->WorldHTTPMimeFile.c_str());
	} else {
		_log(WORLD__INIT, "HTTP world service disabled.");
	}

	_log(WORLD__INIT, "Loading variables..");
	database.LoadVariables();
	_log(WORLD__INIT, "Loading zones..");
	database.LoadZoneNames();
	_log(WORLD__INIT, "Clearing groups..");
	database.ClearGroup();
	_log(WORLD__INIT, "Clearing raids..");
	database.ClearRaid();
	database.ClearRaidDetails();
	_log(WORLD__INIT, "Loading items..");
	if (!database.LoadItems()) {
		_log(WORLD__INIT_ERR, "Error: Could not load item data. But ignoring");
	}
	_log(WORLD__INIT, "Loading guilds..");
	guild_mgr.LoadGuilds();
	//rules:
	{
		char tmp[64];
		if (database.GetVariable("RuleSet", tmp, sizeof(tmp)-1)) {
			_log(WORLD__INIT, "Loading rule set '%s'", tmp);
			if(!RuleManager::Instance()->LoadRules(&database, tmp)) {
				_log(WORLD__INIT_ERR, "Failed to load ruleset '%s', falling back to defaults.", tmp);
			}
		} else {
			if(!RuleManager::Instance()->LoadRules(&database, "default")) {
				_log(WORLD__INIT, "No rule set configured, using default rules");
			} else {
				_log(WORLD__INIT, "Loaded default rule set 'default'", tmp);
			}
		}
	}
	if(RuleB(World, ClearTempMerchantlist)){
		_log(WORLD__INIT, "Clearing temporary merchant lists..");
		database.ClearMerchantTemp();
	}
	_log(WORLD__INIT, "Loading EQ time of day..");
	if (!zoneserver_list.worldclock.loadFile(Config->EQTimeFile.c_str()))
		_log(WORLD__INIT_ERR, "Unable to load %s", Config->EQTimeFile.c_str());
	_log(WORLD__INIT, "Loading launcher list..");
	launcher_list.LoadList();

	char tmp[20];
	tmp[0] = '\0';
	database.GetVariable("holdzones",tmp, 20);
	if ((strcasecmp(tmp, "1") == 0)) {
		holdzones = true;
	}
	_log(WORLD__INIT, "Reboot zone modes %s",holdzones ? "ON" : "OFF");

	_log(WORLD__INIT, "Deleted %i stale player corpses from database", database.DeleteStalePlayerCorpses());
	if (RuleB(World, DeleteStaleCorpeBackups) == true) {
	_log(WORLD__INIT, "Deleted %i stale player backups from database", database.DeleteStalePlayerBackups());
	}

	_log(WORLD__INIT, "Loading adventures...");
	if(!adventure_manager.LoadAdventureTemplates())
	{
		_log(WORLD__INIT_ERR, "Unable to load adventure templates.");
	}

	if(!adventure_manager.LoadAdventureEntries())
	{
		_log(WORLD__INIT_ERR, "Unable to load adventure templates.");
	}

	adventure_manager.Load();
	adventure_manager.LoadLeaderboardInfo();

	_log(WORLD__INIT, "Purging expired instances");
	database.PurgeExpiredInstances();
	Timer PurgeInstanceTimer(450000);
	PurgeInstanceTimer.Start(450000);

	_log(WORLD__INIT, "Loading char create info...");
	database.LoadCharacterCreateAllocations();
	database.LoadCharacterCreateCombos();

	char errbuf[TCPConnection_ErrorBufferSize];
	if (tcps.Open(Config->WorldTCPPort, errbuf)) {
		_log(WORLD__INIT,"Zone (TCP) listener started.");
	} else {
		_log(WORLD__INIT_ERR,"Failed to start zone (TCP) listener on port %d:",Config->WorldTCPPort);
		_log(WORLD__INIT_ERR,"        %s",errbuf);
		return 1;
	}
	if (eqsf.Open()) {
		_log(WORLD__INIT,"Client (UDP) listener started.");
	} else {
		_log(WORLD__INIT_ERR,"Failed to start client (UDP) listener (port 9000)");
		return 1;
	}

	//register all the patches we have avaliable with the stream identifier.
	EQStreamIdentifier stream_identifier;
	RegisterAllPatches(stream_identifier);
	zoneserver_list.shutdowntimer = new Timer(60000);
	zoneserver_list.shutdowntimer->Disable();
	zoneserver_list.reminder = new Timer(20000);
	zoneserver_list.reminder->Disable();
	Timer InterserverTimer(INTERSERVER_TIMER); // does MySQL pings and auto-reconnect
	InterserverTimer.Trigger();
	uint8 ReconnectCounter = 100;
	EQStream* eqs;
	EmuTCPConnection* tcpc;
	EQStreamInterface *eqsi;

	while(RunLoops) {
		Timer::SetCurrentTime();

		//check the factory for any new incoming streams.
		while ((eqs = eqsf.Pop())) {
			//pull the stream out of the factory and give it to the stream identifier
			//which will figure out what patch they are running, and set up the dynamic
			//structures and opcodes for that patch.
			struct in_addr	in;
			in.s_addr = eqs->GetRemoteIP();
			_log(WORLD__CLIENT, "New connection from %s:%d", inet_ntoa(in),ntohs(eqs->GetRemotePort()));
			stream_identifier.AddStream(eqs);	//takes the stream
		}

		//give the stream identifier a chance to do its work....
		stream_identifier.Process();

		//check the stream identifier for any now-identified streams
		while((eqsi = stream_identifier.PopIdentified())) {
			//now that we know what patch they are running, start up their client object
			struct in_addr	in;
			in.s_addr = eqsi->GetRemoteIP();
			if (RuleB(World, UseBannedIPsTable)){ //Lieka: Check to see if we have the responsibility for blocking IPs.
				_log(WORLD__CLIENT, "Checking inbound connection %s against BannedIPs table", inet_ntoa(in));
				if (!database.CheckBannedIPs(inet_ntoa(in))){ //Lieka: Check inbound IP against banned IP table.
					_log(WORLD__CLIENT, "Connection %s PASSED banned IPs check. Processing connection.", inet_ntoa(in));
					Client* client = new Client(eqsi);
					// @merth: client->zoneattempt=0;
					client_list.Add(client);
				} else {
					_log(WORLD__CLIENT, "Connection from %s FAILED banned IPs check. Closing connection.", inet_ntoa(in));
					eqsi->Close(); //Lieka: If the inbound IP is on the banned table, close the EQStream.
				}
			}
			if (!RuleB(World, UseBannedIPsTable)){
					_log(WORLD__CLIENT, "New connection from %s:%d, processing connection", inet_ntoa(in), ntohs(eqsi->GetRemotePort()));
					Client* client = new Client(eqsi);
					// @merth: client->zoneattempt=0;
					client_list.Add(client);
			}
		}

		client_list.Process();

		while ((tcpc = tcps.NewQueuePop())) {
			struct in_addr in;
			in.s_addr = tcpc->GetrIP();
			_log(WORLD__ZONE, "New TCP connection from %s:%d", inet_ntoa(in),tcpc->GetrPort());
			console_list.Add(new Console(tcpc));
		}

		if(PurgeInstanceTimer.Check())
		{
			database.PurgeExpiredInstances();
		}

		//check for timeouts in other threads
		timeout_manager.CheckTimeouts();

		loginserverlist.Process();

		console_list.Process();

		zoneserver_list.Process();

		launcher_list.Process();

		UCSLink.Process();

		QSLink.Process();

		LFPGroupList.Process();

		adventure_manager.Process();

		if (InterserverTimer.Check()) {
			InterserverTimer.Start();
			database.ping();
			AsyncLoadVariables(dbasync, &database);
			ReconnectCounter++;
			if (ReconnectCounter >= 12) { // only create thread to reconnect every 10 minutes. previously we were creating a new thread every 10 seconds
				ReconnectCounter = 0;
				if (loginserverlist.AllConnected() == false) {
#ifdef _WINDOWS
					_beginthread(AutoInitLoginServer, 0, nullptr);
#else
					pthread_t thread;
					pthread_create(&thread, nullptr, &AutoInitLoginServer, nullptr);
#endif
				}
			}
		}
		if (numclients == 0) {
			Sleep(50);
			continue;
		}
		Sleep(20);
	}
	_log(WORLD__SHUTDOWN,"World main loop completed.");
	_log(WORLD__SHUTDOWN,"Shutting down console connections (if any).");
	console_list.KillAll();
	_log(WORLD__SHUTDOWN,"Shutting down zone connections (if any).");
	zoneserver_list.KillAll();
	_log(WORLD__SHUTDOWN,"Zone (TCP) listener stopped.");
	tcps.Close();
	_log(WORLD__SHUTDOWN,"Client (UDP) listener stopped.");
	eqsf.Close();
	_log(WORLD__SHUTDOWN,"Signaling HTTP service to stop...");
	http_server.Stop();

	CheckEQEMuErrorAndPause();
	return 0;
}

void CatchSignal(int sig_num) {
	_log(WORLD__SHUTDOWN,"Caught signal %d",sig_num);
	if(zoneserver_list.worldclock.saveFile(WorldConfig::get()->EQTimeFile.c_str())==false)
		_log(WORLD__SHUTDOWN,"Failed to save time file.");
	RunLoops = false;
}

void UpdateWindowTitle(char* iNewTitle) {
#ifdef _WINDOWS
	char tmp[500];
	if (iNewTitle) {
		snprintf(tmp, sizeof(tmp), "World: %s", iNewTitle);
	}
	else {
		snprintf(tmp, sizeof(tmp), "World");
	}
	SetConsoleTitle(tmp);
#endif
}

