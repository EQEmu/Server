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
#include "../common/global_define.h"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "../common/string_util.h"
#include "../common/eqemu_logsys.h"
#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/eq_packet.h"
#include "../common/seperator.h"
#include "../common/version.h"
#include "../common/eqtime.h"
#include "../common/event/event_loop.h"
#include "../common/net/eqstream.h"
#include "../common/opcodemgr.h"
#include "../common/guilds.h"
#include "../common/eq_stream_ident.h"
#include "../common/rulesys.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "client.h"
#include "worlddb.h"
#ifdef _WINDOWS
#include <process.h>
#define snprintf	_snprintf
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
#if not defined (FREEBSD) && not defined (DARWIN)
union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
	struct seminfo *__buf;
	void *__pad;
};
#endif

#endif

#include "../common/patches/patches.h"
#include "../common/random.h"
#include "zoneserver.h"
#include "login_server.h"
#include "login_server_list.h"
#include "world_config.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "clientlist.h"
#include "launcher_list.h"
#include "wguild_mgr.h"
#include "lfplist.h"
#include "adventure_manager.h"
#include "ucs.h"
#include "queryserv.h"
#include "web_interface.h"
#include "console.h"

#include "../common/net/servertalk_server.h"

ClientList client_list;
GroupLFPList LFPGroupList;
ZSList zoneserver_list;
LoginServerList loginserverlist;
UCSConnection UCSLink;
QueryServConnection QSLink;
LauncherList launcher_list;
AdventureManager adventure_manager;
EQEmu::Random emu_random;
volatile bool RunLoops = true;
uint32 numclients = 0;
uint32 numzones = 0;
bool holdzones = false;
const WorldConfig *Config;
EQEmuLogSys LogSys;
WebInterfaceList web_interface;

void CatchSignal(int sig_num);
void CheckForServerScript(bool force_download = false);

inline void UpdateWindowTitle(std::string new_title) {
#ifdef _WINDOWS
	SetConsoleTitle(new_title.c_str());
#endif
}

int main(int argc, char** argv) {
	RegisterExecutablePlatform(ExePlatformWorld);
	LogSys.LoadLogSettingsDefaults();
	set_exception_handler();

	/* If eqemu_config.json does not exist - create it from conversion... */
	if (!std::ifstream("eqemu_config.json")) {
		CheckForServerScript(true);
		/* Run EQEmu Server script (Checks for database updates) */
		system("perl eqemu_server.pl convert_xml");
	}
	else {
		/* Download EQEmu Server Maintenance Script if doesn't exist */
		CheckForServerScript();
	}

	/* Database Version Check */
	uint32 Database_Version = CURRENT_BINARY_DATABASE_VERSION;
	uint32 Bots_Database_Version = CURRENT_BINARY_BOTS_DATABASE_VERSION;
	if (argc >= 2) {
		if (strcasecmp(argv[1], "db_version") == 0) {
			std::cout << "Binary Database Version: " << Database_Version << " : " << Bots_Database_Version << std::endl;
			return 0;
		}
	}

	// Load server configuration
	Log(Logs::General, Logs::World_Server, "Loading server configuration..");
	if (!WorldConfig::LoadConfig()) {
		Log(Logs::General, Logs::World_Server, "Loading server configuration failed.");
		return 1;
	}

	Config = WorldConfig::get();

	Log(Logs::General, Logs::World_Server, "CURRENT_VERSION: %s", CURRENT_VERSION);

	if (signal(SIGINT, CatchSignal) == SIG_ERR) {
		Log(Logs::General, Logs::World_Server, "Could not set signal handler");
		return 1;
	}

	if (signal(SIGTERM, CatchSignal) == SIG_ERR) {
		Log(Logs::General, Logs::World_Server, "Could not set signal handler");
		return 1;
	}

#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		Log(Logs::General, Logs::World_Server, "Could not set signal handler");
		return 1;
	}
#endif

	// add login server config to list
	if (Config->LoginCount == 0) {
		if (Config->LoginHost.length()) {
			loginserverlist.Add(Config->LoginHost.c_str(), Config->LoginPort, Config->LoginAccount.c_str(), Config->LoginPassword.c_str(), Config->LoginLegacy);
			Log(Logs::General, Logs::World_Server, "Added loginserver %s:%i", Config->LoginHost.c_str(), Config->LoginPort);
		}
	}
	else {
		LinkedList<LoginConfig*> loginlist = Config->loginlist;
		LinkedListIterator<LoginConfig*> iterator(loginlist);
		iterator.Reset();
		while (iterator.MoreElements()) {
			loginserverlist.Add(iterator.GetData()->LoginHost.c_str(), iterator.GetData()->LoginPort, iterator.GetData()->LoginAccount.c_str(), iterator.GetData()->LoginPassword.c_str(),
				iterator.GetData()->LoginLegacy);
			Log(Logs::General, Logs::World_Server, "Added loginserver %s:%i", iterator.GetData()->LoginHost.c_str(), iterator.GetData()->LoginPort);
			iterator.Advance();
		}
	}

	Log(Logs::General, Logs::World_Server, "Connecting to MySQL %s@%s:%i...", Config->DatabaseUsername.c_str(), Config->DatabaseHost.c_str(), Config->DatabasePort);
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		Log(Logs::General, Logs::World_Server, "Cannot continue without a database connection.");
		return 1;
	}
	guild_mgr.SetDatabase(&database);

	/* Register Log System and Settings */
	database.LoadLogSettings(LogSys.log_settings);
	LogSys.StartFileLogs();

	bool ignore_db = false;
	if (argc >= 2) {
		std::string tmp;
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
		else if (database.GetVariable("disablecommandline", tmp)) {
			if (tmp.length() == 1) {
				if (tmp[0] == '1') {
					std::cerr << "Command line disabled in database... exiting" << std::endl;
					return 1;
				}
			}
		}
		else if (strcasecmp(argv[1], "adduser") == 0) {
			if (argc == 5) {
				if (Seperator::IsNumber(argv[4])) {
					if (atoi(argv[4]) >= 0 && atoi(argv[4]) <= 255) {
						if (database.CreateAccount(argv[2], argv[3], atoi(argv[4])) == 0) {
							std::cerr << "database.CreateAccount failed." << std::endl;
							return 1;
						}
						else {
							std::cout << "Account created: Username='" << argv[2] << "', Password='" << argv[3] << "', status=" << argv[4] << std::endl;
							return 0;
						}
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
						if (database.SetAccountStatus(argv[2], atoi(argv[3]))) {
							std::cout << "Account flagged: Username='" << argv[2] << "', status=" << argv[3] << std::endl;
							return 0;
						}
						else {
							std::cerr << "database.SetAccountStatus failed." << std::endl;
							return 1;
						}
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
					std::cerr << "Error: zone name too short" << std::endl;
					return 1;
				}
				else if (strlen(argv[2]) > 15) {
					std::cerr << "Error: zone name too long" << std::endl;
					return 1;
				}
				else {
					if (database.SetVariable("startzone", argv[2])) {
						std::cout << "Starting zone changed: '" << argv[2] << "'" << std::endl;
						return 0;
					}
					else {
						std::cerr << "database.SetVariable failed." << std::endl;
						return 1;
					}
				}
			}
			std::cout << "Usage: world startzone zoneshortname" << std::endl;
			return 0;
		}
		else if (strcasecmp(argv[1], "ignore_db") == 0) {
			ignore_db = true;
		}
		else {
			std::cerr << "Error, unknown command line option" << std::endl;
			return 1;
		}
	}

	if (!ignore_db) {
		Log(Logs::General, Logs::World_Server, "Checking Database Conversions..");
		database.CheckDatabaseConversions();
	}
	Log(Logs::General, Logs::World_Server, "Loading variables..");
	database.LoadVariables();

	std::string hotfix_name;
	if (database.GetVariable("hotfix_name", hotfix_name)) {
		if (!hotfix_name.empty()) {
			Log(Logs::General, Logs::Zone_Server, "Current hotfix in use: '%s'", hotfix_name.c_str());
		}
	}

	Log(Logs::General, Logs::World_Server, "Loading zones..");
	database.LoadZoneNames();
	Log(Logs::General, Logs::World_Server, "Clearing groups..");
	database.ClearGroup();
	Log(Logs::General, Logs::World_Server, "Clearing raids..");
	database.ClearRaid();
	database.ClearRaidDetails();
	database.ClearRaidLeader();
	Log(Logs::General, Logs::World_Server, "Clearing inventory snapshots..");
	database.ClearInvSnapshots();
	Log(Logs::General, Logs::World_Server, "Loading items..");
	if (!database.LoadItems(hotfix_name))
		Log(Logs::General, Logs::World_Server, "Error: Could not load item data. But ignoring");
	Log(Logs::General, Logs::World_Server, "Loading skill caps..");
	if (!database.LoadSkillCaps(std::string(hotfix_name)))
		Log(Logs::General, Logs::World_Server, "Error: Could not load skill cap data. But ignoring");
	Log(Logs::General, Logs::World_Server, "Loading guilds..");
	guild_mgr.LoadGuilds();
	//rules:
	{
		std::string tmp;
		if (database.GetVariable("RuleSet", tmp)) {
			Log(Logs::General, Logs::World_Server, "Loading rule set '%s'", tmp.c_str());
			if (!RuleManager::Instance()->LoadRules(&database, tmp.c_str())) {
				Log(Logs::General, Logs::World_Server, "Failed to load ruleset '%s', falling back to defaults.", tmp.c_str());
			}
		}
		else {
			if (!RuleManager::Instance()->LoadRules(&database, "default")) {
				Log(Logs::General, Logs::World_Server, "No rule set configured, using default rules");
			}
			else {
				Log(Logs::General, Logs::World_Server, "Loaded default rule set 'default'", tmp.c_str());
			}
		}
	}

	if (RuleB(World, ClearTempMerchantlist)) {
		Log(Logs::General, Logs::World_Server, "Clearing temporary merchant lists..");
		database.ClearMerchantTemp();
	}

	RuleManager::Instance()->SaveRules(&database);

	Log(Logs::General, Logs::World_Server, "Loading EQ time of day..");
	TimeOfDay_Struct eqTime;
	time_t realtime;
	eqTime = database.LoadTime(realtime);
	zoneserver_list.worldclock.SetCurrentEQTimeOfDay(eqTime, realtime);
	Timer EQTimeTimer(600000);
	EQTimeTimer.Start(600000);

	Log(Logs::General, Logs::World_Server, "Loading launcher list..");
	launcher_list.LoadList();

	std::string tmp;
	database.GetVariable("holdzones", tmp);
	if (tmp.length() == 1 && tmp[0] == '1') {
		holdzones = true;
	}
	Log(Logs::General, Logs::World_Server, "Reboot zone modes %s", holdzones ? "ON" : "OFF");

	Log(Logs::General, Logs::World_Server, "Deleted %i stale player corpses from database", database.DeleteStalePlayerCorpses());

	Log(Logs::General, Logs::World_Server, "Loading adventures...");
	if (!adventure_manager.LoadAdventureTemplates())
	{
		Log(Logs::General, Logs::World_Server, "Unable to load adventure templates.");
	}

	if (!adventure_manager.LoadAdventureEntries())
	{
		Log(Logs::General, Logs::World_Server, "Unable to load adventure templates.");
	}

	adventure_manager.Load();
	adventure_manager.LoadLeaderboardInfo();

	Log(Logs::General, Logs::World_Server, "Purging expired instances");
	database.PurgeExpiredInstances();
	Timer PurgeInstanceTimer(450000);
	PurgeInstanceTimer.Start(450000);

	Log(Logs::General, Logs::World_Server, "Loading char create info...");
	database.LoadCharacterCreateAllocations();
	database.LoadCharacterCreateCombos();

	std::unique_ptr<EQ::Net::ConsoleServer> console;
	if (Config->TelnetEnabled) {
		Log(Logs::General, Logs::World_Server, "Console (TCP) listener started.");
		console.reset(new EQ::Net::ConsoleServer(Config->TelnetIP, Config->TelnetTCPPort));
		RegisterConsoleFunctions(console);
	}

	std::unique_ptr<EQ::Net::ServertalkServer> server_connection;
	server_connection.reset(new EQ::Net::ServertalkServer());

	EQ::Net::ServertalkServerOptions server_opts;
	server_opts.port = Config->WorldTCPPort;
	server_opts.ipv6 = false;
	server_opts.credentials = Config->SharedKey;
	server_connection->Listen(server_opts);
	Log(Logs::General, Logs::World_Server, "Server (TCP) listener started.");

	server_connection->OnConnectionIdentified("Zone", [&console](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "New Zone Server connection from {2} at {0}:{1}",
			connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

		numzones++;
		zoneserver_list.Add(new ZoneServer(connection, console.get()));
	});

	server_connection->OnConnectionRemoved("Zone", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "Removed Zone Server connection from {0}",
			connection->GetUUID());

		numzones--;
		zoneserver_list.Remove(connection->GetUUID());
	});

	server_connection->OnConnectionIdentified("Launcher", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "New Launcher connection from {2} at {0}:{1}",
			connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

		launcher_list.Add(connection);
	});

	server_connection->OnConnectionRemoved("Launcher", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "Removed Launcher connection from {0}",
			connection->GetUUID());

		launcher_list.Remove(connection);
	});

	server_connection->OnConnectionIdentified("QueryServ", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "New Query Server connection from {2} at {0}:{1}",
			connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

		QSLink.AddConnection(connection);
	});

	server_connection->OnConnectionRemoved("QueryServ", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "Removed Query Server connection from {0}",
			connection->GetUUID());

		QSLink.RemoveConnection(connection);
	});

	server_connection->OnConnectionIdentified("UCS", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "New UCS Server connection from {2} at {0}:{1}",
			connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

		UCSLink.SetConnection(connection);

		zoneserver_list.UpdateUCSServerAvailable();
	});

	server_connection->OnConnectionRemoved("UCS", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "Removed Query Server connection from {0}",
			connection->GetUUID());

		UCSLink.SetConnection(nullptr);

		zoneserver_list.UpdateUCSServerAvailable(false);
	});

	server_connection->OnConnectionIdentified("WebInterface", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "New WebInterface Server connection from {2} at {0}:{1}",
			connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

		web_interface.AddConnection(connection);
	});

	server_connection->OnConnectionRemoved("WebInterface", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
		LogF(Logs::General, Logs::World_Server, "Removed WebInterface Server connection from {0}",
			connection->GetUUID());

		web_interface.RemoveConnection(connection);
	});

	EQ::Net::EQStreamManagerOptions opts(9000, false, false);
	EQ::Net::EQStreamManager eqsm(opts);

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
	std::shared_ptr<EQStreamInterface> eqs;
	EQStreamInterface *eqsi;

	eqsm.OnNewConnection([&stream_identifier](std::shared_ptr<EQ::Net::EQStream> stream) {
		stream_identifier.AddStream(stream);
		LogF(Logs::Detail, Logs::World_Server, "New connection from IP {0}:{1}", stream->RemoteEndpoint(), ntohs(stream->GetRemotePort()));
	});

	while (RunLoops) {
		Timer::SetCurrentTime();
		eqs = nullptr;

		//give the stream identifier a chance to do its work....
		stream_identifier.Process();

		//check the stream identifier for any now-identified streams
		while ((eqsi = stream_identifier.PopIdentified())) {
			//now that we know what patch they are running, start up their client object
			struct in_addr	in;
			in.s_addr = eqsi->GetRemoteIP();
			if (RuleB(World, UseBannedIPsTable)) { //Lieka: Check to see if we have the responsibility for blocking IPs.
				Log(Logs::Detail, Logs::World_Server, "Checking inbound connection %s against BannedIPs table", inet_ntoa(in));
				if (!database.CheckBannedIPs(inet_ntoa(in))) { //Lieka: Check inbound IP against banned IP table.
					Log(Logs::Detail, Logs::World_Server, "Connection %s PASSED banned IPs check. Processing connection.", inet_ntoa(in));
					auto client = new Client(eqsi);
					// @merth: client->zoneattempt=0;
					client_list.Add(client);
				}
				else {
					Log(Logs::General, Logs::World_Server, "Connection from %s FAILED banned IPs check. Closing connection.", inet_ntoa(in));
					eqsi->Close(); //Lieka: If the inbound IP is on the banned table, close the EQStream.
				}
			}
			if (!RuleB(World, UseBannedIPsTable)) {
				Log(Logs::Detail, Logs::World_Server, "New connection from %s:%d, processing connection", inet_ntoa(in), ntohs(eqsi->GetRemotePort()));
				auto client = new Client(eqsi);
				// @merth: client->zoneattempt=0;
				client_list.Add(client);
			}
		}

		client_list.Process();

		if (PurgeInstanceTimer.Check())
		{
			database.PurgeExpiredInstances();
		}

		if (EQTimeTimer.Check()) {
			TimeOfDay_Struct tod;
			zoneserver_list.worldclock.GetCurrentEQTimeOfDay(time(0), &tod);
			if (!database.SaveTime(tod.minute, tod.hour, tod.day, tod.month, tod.year))
				Log(Logs::General, Logs::World_Server, "Failed to save eqtime.");
			else
				Log(Logs::Detail, Logs::World_Server, "EQTime successfully saved.");
		}

		zoneserver_list.Process();
		launcher_list.Process();
		LFPGroupList.Process();
		adventure_manager.Process();

		if (InterserverTimer.Check()) {
			InterserverTimer.Start();
			database.ping();

			std::string window_title = StringFormat("World: %s Clients: %i", Config->LongName.c_str(), client_list.GetClientCount());
			UpdateWindowTitle(window_title);
		}

		EQ::EventLoop::Get().Process();
		Sleep(5);
	}
	Log(Logs::General, Logs::World_Server, "World main loop completed.");
	Log(Logs::General, Logs::World_Server, "Shutting down zone connections (if any).");
	zoneserver_list.KillAll();
	Log(Logs::General, Logs::World_Server, "Zone (TCP) listener stopped.");
	Log(Logs::General, Logs::World_Server, "Signaling HTTP service to stop...");
	LogSys.CloseFileLogs();

	return 0;
}

void CatchSignal(int sig_num) {
	Log(Logs::General, Logs::World_Server, "Caught signal %d", sig_num);
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

void CheckForServerScript(bool force_download) {
	/* Fetch EQEmu Server script */
	if (!std::ifstream("eqemu_server.pl") || force_download) {

		if(force_download)
			std::remove("eqemu_server.pl"); /* Delete local before fetch */

		std::cout << "Pulling down EQEmu Server Maintenance Script (eqemu_server.pl)..." << std::endl;
#ifdef _WIN32
		system("perl -MLWP::UserAgent -e \"require LWP::UserAgent;  my $ua = LWP::UserAgent->new; $ua->timeout(10); $ua->env_proxy; my $response = $ua->get('https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_server.pl'); if ($response->is_success){ open(FILE, '> eqemu_server.pl'); print FILE $response->decoded_content; close(FILE); }\"");
#else
		system("wget -N --no-check-certificate --quiet -O eqemu_server.pl https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_server.pl");
#endif
	}
}