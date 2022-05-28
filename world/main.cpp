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

#define PLATFORM_WORLD 1

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
#include "../common/misc.h"
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
#include "dynamic_zone_manager.h"
#include "expedition_database.h"

#include "../common/net/servertalk_server.h"
#include "../zone/data_bucket.h"
#include "world_server_command_handler.h"
#include "../common/content/world_content_service.h"
#include "../common/repositories/character_task_timers_repository.h"
#include "../common/repositories/merchantlist_temp_repository.h"
#include "world_store.h"
#include "world_event_scheduler.h"
#include "shared_task_manager.h"

WorldStore          world_store;
ClientList          client_list;
GroupLFPList        LFPGroupList;
ZSList              zoneserver_list;
LoginServerList     loginserverlist;
UCSConnection       UCSLink;
QueryServConnection QSLink;
LauncherList        launcher_list;
AdventureManager    adventure_manager;
WorldEventScheduler event_scheduler;
SharedTaskManager   shared_task_manager;
EQ::Random          emu_random;
volatile bool       RunLoops   = true;
uint32              numclients = 0;
uint32              numzones   = 0;
const WorldConfig   *Config;
EQEmuLogSys         LogSys;
WorldContentService content_service;
WebInterfaceList    web_interface;

void CatchSignal(int sig_num);
void CheckForServerScript(bool force_download = false);

inline void UpdateWindowTitle(std::string new_title)
{
#ifdef _WINDOWS
	SetConsoleTitle(new_title.c_str());
#endif
}

void LoadDatabaseConnections()
{
	LogInfo(
		"Connecting to MySQL [{}]@[{}]:[{}]",
		Config->DatabaseUsername.c_str(),
		Config->DatabaseHost.c_str(),
		Config->DatabasePort
	);

	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort
	)) {
		LogError("Cannot continue without a database connection");

		std::exit(1);
	}

	/**
	 * Multi-tenancy: Content database
	 */
	if (!Config->ContentDbHost.empty()) {
		if (!content_db.Connect(
			Config->ContentDbHost.c_str(),
			Config->ContentDbUsername.c_str(),
			Config->ContentDbPassword.c_str(),
			Config->ContentDbName.c_str(),
			Config->ContentDbPort,
			"content"
		)) {
			LogError("Cannot continue without a content database connection");
			std::exit(1);
		}
	}
	else {
		content_db.SetMysql(database.getMySQL());
	}

}

void CheckForXMLConfigUpgrade()
{
	if (!std::ifstream("eqemu_config.json") && std::ifstream("eqemu_config.xml")) {
		CheckForServerScript(true);
		if (system("perl eqemu_server.pl convert_xml")) {}
	}
	else {
		CheckForServerScript();
	}
}

void LoadServerConfig()
{
	LogInfo("Loading server configuration");
	if (!WorldConfig::LoadConfig()) {
		LogError("Loading server configuration failed");
		std::exit(1);
	}
}

void RegisterLoginservers()
{
	if (Config->LoginCount == 0) {
		if (Config->LoginHost.length()) {
			loginserverlist.Add(
				Config->LoginHost.c_str(),
				Config->LoginPort,
				Config->LoginAccount.c_str(),
				Config->LoginPassword.c_str(),
				Config->LoginLegacy
			);
			LogInfo("Added loginserver [{}]:[{}]", Config->LoginHost.c_str(), Config->LoginPort);
		}
	}
	else {
		LinkedList<LoginConfig *>         loginlist = Config->loginlist;
		LinkedListIterator<LoginConfig *> iterator(loginlist);
		iterator.Reset();
		while (iterator.MoreElements()) {
			if (iterator.GetData()->LoginHost.length()) {
				loginserverlist.Add(
					iterator.GetData()->LoginHost.c_str(),
					iterator.GetData()->LoginPort,
					iterator.GetData()->LoginAccount.c_str(),
					iterator.GetData()->LoginPassword.c_str(),
					iterator.GetData()->LoginLegacy
				);

				LogInfo(
					"Added loginserver [{}]:[{}]",
					iterator.GetData()->LoginHost.c_str(),
					iterator.GetData()->LoginPort
				);
			}
			iterator.Advance();
		}
	}
}

static void GMSayHookCallBackProcessWorld(uint16 log_category, std::string message)
{
	// Cut messages down to 4000 max to prevent client crash
	if (!message.empty()) {
		message = message.substr(0, 4000);
	}

	// Replace Occurrences of % or MessageStatus will crash
	find_replace(message, std::string("%"), std::string("."));

	if (message.find('\n') != std::string::npos) {
		auto message_split = SplitString(message, '\n');

		for (size_t iter = 0; iter < message_split.size(); ++iter) {
			zoneserver_list.SendEmoteMessage(
				0,
				0,
				AccountStatus::QuestTroupe,
				LogSys.GetGMSayColorFromCategory(log_category),
				fmt::format(
					" {}{}",
					(iter == 0 ? " ---" : ""),
					message_split[iter]
				).c_str()
			);
		}

		return;
	}

	zoneserver_list.SendEmoteMessage(
		0,
		0,
		AccountStatus::QuestTroupe,
		LogSys.GetGMSayColorFromCategory(log_category),
		"%s",
		message.c_str()
	);
}

/**
 * World process entrypoint
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv)
{
	RegisterExecutablePlatform(ExePlatformWorld);
	LogSys.LoadLogSettingsDefaults();
	set_exception_handler();

	/**
	 * Database version
	 */
	uint32 database_version      = CURRENT_BINARY_DATABASE_VERSION;
	uint32 bots_database_version = CURRENT_BINARY_BOTS_DATABASE_VERSION;
	if (argc >= 2) {
		if (strcasecmp(argv[1], "db_version") == 0) {
			std::cout << "Binary Database Version: " << database_version << " : " << bots_database_version << std::endl;
			return 0;
		}
	}

	/**
	 * Command handler
	 */
	if (argc > 1) {
		LogSys.SilenceConsoleLogging();

		WorldConfig::LoadConfig();
		Config = WorldConfig::get();

		LoadDatabaseConnections();
		LogSys.EnableConsoleLogging();

		WorldserverCommandHandler::CommandHandler(argc, argv);
	}

	CheckForXMLConfigUpgrade();
	LoadServerConfig();

	Config = WorldConfig::get();

	LogInfo("CURRENT_VERSION: [{}]", CURRENT_VERSION);

	if (signal(SIGINT, CatchSignal) == SIG_ERR) {
		LogError("Could not set signal handler");
		return 1;
	}

	if (signal(SIGTERM, CatchSignal) == SIG_ERR) {
		LogError("Could not set signal handler");
		return 1;
	}

#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		LogError("Could not set signal handler");
		return 1;
	}
#endif

	RegisterLoginservers();
	LoadDatabaseConnections();

	guild_mgr.SetDatabase(&database);

	// logging system init
	auto logging = LogSys.SetDatabase(&database)
		->LoadLogDatabaseSettings();

	if (RuleB(Logging, WorldGMSayLogging)) {
		logging->SetGMSayHandler(&GMSayHookCallBackProcessWorld);
	}

	logging->StartFileLogs();

	/**
	 * Parse simple CLI passes
	 */
	bool ignore_db = false;
	if (argc >= 2) {
		if (strcasecmp(argv[1], "ignore_db") == 0) {
			ignore_db = true;
		}
		else {
			std::cerr << "Error, unknown command line option" << std::endl;
			return 1;
		}
	}

	if (!ignore_db) {
		LogInfo("Checking Database Conversions");
		database.CheckDatabaseConversions();
	}
	LogInfo("Loading variables");
	database.LoadVariables();

	std::string hotfix_name;
	if (database.GetVariable("hotfix_name", hotfix_name)) {
		if (!hotfix_name.empty()) {
			LogInfo("Current hotfix in use: [{}]", hotfix_name.c_str());
		}
	}

	LogInfo("Purging expired data buckets");
	database.PurgeAllDeletedDataBuckets();

	LogInfo("Loading zones");

	world_store.LoadZones();

	LogInfo("Clearing groups");
	database.ClearGroup();
	LogInfo("Clearing raids");
	database.ClearRaid();
	database.ClearRaidDetails();
	database.ClearRaidLeader();
	LogInfo("Clearing inventory snapshots");
	database.ClearInvSnapshots();
	LogInfo("Loading items");

	if (!content_db.LoadItems(hotfix_name)) {
		LogError("Error: Could not load item data. But ignoring");
	}

	LogInfo("Loading skill caps");
	if (!content_db.LoadSkillCaps(std::string(hotfix_name))) {
		LogError("Error: Could not load skill cap data. But ignoring");
	}

	LogInfo("Loading guilds");
	guild_mgr.LoadGuilds();

	//rules:
	{
		if (!RuleManager::Instance()->UpdateOrphanedRules(&database)) {
			LogInfo("Failed to process 'Orphaned Rules' update operation.");
		}

		if (!RuleManager::Instance()->UpdateInjectedRules(&database, "default")) {
			LogInfo("Failed to process 'Injected Rules' for ruleset 'default' update operation.");
		}

		std::string tmp;
		if (database.GetVariable("RuleSet", tmp)) {
			LogInfo("Loading rule set [{}]", tmp.c_str());

			if (!RuleManager::Instance()->LoadRules(&database, tmp.c_str(), false)) {
				LogInfo("Failed to load ruleset [{}], falling back to defaults", tmp.c_str());
			}
		}
		else {

			if (!RuleManager::Instance()->LoadRules(&database, "default", false)) {
				LogInfo("No rule set configured, using default rules");
			}
			else {
				LogInfo("Loaded default rule set [default]", tmp.c_str());
			}
		}

		if (!RuleManager::Instance()->RestoreRuleNotes(&database)) {
			LogInfo("Failed to process 'Restore Rule Notes' update operation.");
		}
	}

	EQ::InitializeDynamicLookups();
	LogInfo("Initialized dynamic dictionary entries");

	if (RuleB(World, ClearTempMerchantlist)) {
		LogInfo("Clearing temporary merchant lists");
		database.ClearMerchantTemp();
	}

	LogInfo("Loading EQ time of day");
	TimeOfDay_Struct eqTime;
	time_t           realtime;
	eqTime = database.LoadTime(realtime);
	zoneserver_list.worldclock.SetCurrentEQTimeOfDay(eqTime, realtime);
	Timer EQTimeTimer(600000);
	EQTimeTimer.Start(600000);

	LogInfo("Loading launcher list");
	launcher_list.LoadList();

	LogInfo("Deleted [{}] stale player corpses from database", database.DeleteStalePlayerCorpses());

	LogInfo("Loading adventures");
	if (!adventure_manager.LoadAdventureTemplates()) {
		LogInfo("Unable to load adventure templates");
	}

	if (!adventure_manager.LoadAdventureEntries()) {
		LogInfo("Unable to load adventure templates");
	}

	adventure_manager.LoadLeaderboardInfo();

	LogInfo("Purging expired dynamic zones and members");
	dynamic_zone_manager.PurgeExpiredDynamicZones();

	LogInfo("Purging expired expeditions");
	ExpeditionDatabase::PurgeExpiredExpeditions();
	ExpeditionDatabase::PurgeExpiredCharacterLockouts();

	LogInfo("Purging expired character task timers");
	CharacterTaskTimersRepository::DeleteWhere(database, "expire_time <= NOW()");

	LogInfo("Purging expired instances");
	database.PurgeExpiredInstances();

	Timer PurgeInstanceTimer(450000);
	PurgeInstanceTimer.Start(450000);

	LogInfo("Loading dynamic zones");
	dynamic_zone_manager.CacheAllFromDatabase();

	LogInfo("Loading char create info");
	content_db.LoadCharacterCreateAllocations();
	content_db.LoadCharacterCreateCombos();

	LogInfo("Initializing [EventScheduler]");
	event_scheduler.SetDatabase(&database)->LoadScheduledEvents();

	LogInfo("Initializing [WorldContentService]");
	content_service.SetDatabase(&database)
		->SetExpansionContext()
		->ReloadContentFlags();

	LogInfo("Initializing [SharedTaskManager]");
	shared_task_manager.SetDatabase(&database)
		->SetContentDatabase(&content_db)
		->LoadTaskData()
		->LoadSharedTaskState();

	shared_task_manager.PurgeExpiredSharedTasks();

	std::unique_ptr<EQ::Net::ConsoleServer> console;
	if (Config->TelnetEnabled) {
		LogInfo("Console (TCP) listener started");
		console = std::make_unique<EQ::Net::ConsoleServer>(Config->TelnetIP, Config->TelnetTCPPort);
		RegisterConsoleFunctions(console);
	}

	zoneserver_list.Init();
	std::unique_ptr<EQ::Net::ServertalkServer> server_connection;
	server_connection = std::make_unique<EQ::Net::ServertalkServer>();

	EQ::Net::ServertalkServerOptions server_opts;
	server_opts.port        = Config->WorldTCPPort;
	server_opts.ipv6        = false;
	server_opts.credentials = Config->SharedKey;
	server_connection->Listen(server_opts);
	LogInfo("Server (TCP) listener started");

	server_connection->OnConnectionIdentified(
		"Zone", [&console](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("New Zone Server connection from [{2}] at [{0}:{1}]",
					connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

			numzones++;
			zoneserver_list.Add(new ZoneServer(connection, console.get()));
		}
	);

	server_connection->OnConnectionRemoved(
		"Zone", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("Removed Zone Server connection from [{0}]",
					connection->GetUUID());

			numzones--;
			zoneserver_list.Remove(connection->GetUUID());
		}
	);

	server_connection->OnConnectionIdentified(
		"Launcher", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("New Launcher connection from [{2}] at [{0}:{1}]",
					connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

			launcher_list.Add(connection);
		}
	);

	server_connection->OnConnectionRemoved(
		"Launcher", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("Removed Launcher connection from [{0}]",
					connection->GetUUID());

			launcher_list.Remove(connection);
		}
	);

	server_connection->OnConnectionIdentified(
		"QueryServ", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("New Query Server connection from [{2}] at [{0}:{1}]",
					connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

			QSLink.AddConnection(connection);
		}
	);

	server_connection->OnConnectionRemoved(
		"QueryServ", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("Removed Query Server connection from [{0}]",
					connection->GetUUID());

			QSLink.RemoveConnection(connection);
		}
	);

	server_connection->OnConnectionIdentified(
		"UCS", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("New UCS Server connection from [{2}] at [{0}:{1}]",
					connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

			UCSLink.SetConnection(connection);

			zoneserver_list.UpdateUCSServerAvailable();
		}
	);

	server_connection->OnConnectionRemoved(
		"UCS", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("Connection lost from UCS Server [{0}]", connection->GetUUID());

			auto ucs_connection = UCSLink.GetConnection();

			if (ucs_connection->GetUUID() == connection->GetUUID()) {
				LogInfo("Removing currently active UCS connection");
				UCSLink.SetConnection(nullptr);
				zoneserver_list.UpdateUCSServerAvailable(false);
			}
		}
	);

	server_connection->OnConnectionIdentified(
		"WebInterface", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("New WebInterface Server connection from [{2}] at [{0}:{1}]",
					connection->Handle()->RemoteIP(), connection->Handle()->RemotePort(), connection->GetUUID());

			web_interface.AddConnection(connection);
		}
	);

	server_connection->OnConnectionRemoved(
		"WebInterface", [](std::shared_ptr<EQ::Net::ServertalkServerConnection> connection) {
			LogInfo("Removed WebInterface Server connection from [{0}]",
					connection->GetUUID());

			web_interface.RemoveConnection(connection);
		}
	);

	EQStreamManagerInterfaceOptions opts(9000, false, false);
	opts.daybreak_options.resend_delay_ms     = RuleI(Network, ResendDelayBaseMS);
	opts.daybreak_options.resend_delay_factor = RuleR(Network, ResendDelayFactor);
	opts.daybreak_options.resend_delay_min    = RuleI(Network, ResendDelayMinMS);
	opts.daybreak_options.resend_delay_max    = RuleI(Network, ResendDelayMaxMS);
	opts.daybreak_options.outgoing_data_rate  = RuleR(Network, ClientDataRate);

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
	uint8                              ReconnectCounter = 100;
	std::shared_ptr<EQStreamInterface> eqs;
	EQStreamInterface                  *eqsi;

	eqsm.OnNewConnection(
		[&stream_identifier](std::shared_ptr<EQ::Net::EQStream> stream) {
			stream_identifier.AddStream(stream);
			LogInfo("New connection from IP {}:{}", long2ip(stream->GetRemoteIP()), ntohs(stream->GetRemotePort()));
		}
	);

	while (RunLoops) {
		Timer::SetCurrentTime();
		eqs = nullptr;

		//give the stream identifier a chance to do its work....
		stream_identifier.Process();

		//check the stream identifier for any now-identified streams
		while ((eqsi = stream_identifier.PopIdentified())) {
			//now that we know what patch they are running, start up their client object
			struct in_addr in{};
			in.s_addr = eqsi->GetRemoteIP();
			if (RuleB(World, UseBannedIPsTable)) { //Lieka: Check to see if we have the responsibility for blocking IPs.
				LogInfo("Checking inbound connection [{}] against BannedIPs table", inet_ntoa(in));
				if (!database.CheckBannedIPs(inet_ntoa(in))) { //Lieka: Check inbound IP against banned IP table.
					LogInfo("Connection [{}] PASSED banned IPs check. Processing connection", inet_ntoa(in));
					auto client = new Client(eqsi);
					// @merth: client->zoneattempt=0;
					client_list.Add(client);
				}
				else {
					LogInfo("Connection from [{}] failed banned IPs check. Closing connection", inet_ntoa(in));
					eqsi->Close(); //Lieka: If the inbound IP is on the banned table, close the EQStream.
				}
			}
			if (!RuleB(World, UseBannedIPsTable)) {
				LogInfo(
					"New connection from [{}]:[{}], processing connection",
					inet_ntoa(in),
					ntohs(eqsi->GetRemotePort())
				);
				auto client = new Client(eqsi);
				// @merth: client->zoneattempt=0;
				client_list.Add(client);
			}
		}

		event_scheduler.Process(&zoneserver_list);

		client_list.Process();

		if (PurgeInstanceTimer.Check()) {
			database.PurgeExpiredInstances();
			database.PurgeAllDeletedDataBuckets();
			ExpeditionDatabase::PurgeExpiredCharacterLockouts();
			CharacterTaskTimersRepository::DeleteWhere(database, "expire_time <= NOW()");
		}

		if (EQTimeTimer.Check()) {
			TimeOfDay_Struct tod;
			zoneserver_list.worldclock.GetCurrentEQTimeOfDay(time(0), &tod);
			if (!database.SaveTime(tod.minute, tod.hour, tod.day, tod.month, tod.year))
				LogError("Failed to save eqtime");
			else
				LogDebug("EQTime successfully saved");
		}

		zoneserver_list.Process();
		launcher_list.Process();
		LFPGroupList.Process();
		adventure_manager.Process();
		dynamic_zone_manager.Process();

		if (InterserverTimer.Check()) {
			InterserverTimer.Start();
			database.ping();
			content_db.ping();

			std::string window_title = StringFormat(
				"World: %s Clients: %i",
				Config->LongName.c_str(),
				client_list.GetClientCount()
			);
			UpdateWindowTitle(window_title);
		}

		EQ::EventLoop::Get().Process();
		Sleep(5);
	}
	LogInfo("World main loop completed");
	LogInfo("Shutting down zone connections (if any)");
	zoneserver_list.KillAll();
	LogInfo("Zone (TCP) listener stopped");
	LogInfo("Signaling HTTP service to stop");
	LogSys.CloseFileLogs();

	return 0;
}

void CatchSignal(int sig_num)
{
	LogInfo("Caught signal [{}]", sig_num);
	RunLoops = false;
}

void UpdateWindowTitle(char *iNewTitle)
{
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

void CheckForServerScript(bool force_download)
{
	/* Fetch EQEmu Server script */
	if (!std::ifstream("eqemu_server.pl") || force_download) {

		if (force_download) {
			std::remove("eqemu_server.pl");
		} /* Delete local before fetch */

		std::cout << "Pulling down EQEmu Server Maintenance Script (eqemu_server.pl)..." << std::endl;
#ifdef _WIN32
		if(system("perl -MLWP::UserAgent -e \"require LWP::UserAgent;  my $ua = LWP::UserAgent->new; $ua->timeout(10); $ua->env_proxy; my $response = $ua->get('https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_server.pl'); if ($response->is_success){ open(FILE, '> eqemu_server.pl'); print FILE $response->decoded_content; close(FILE); }\""));
#else
		if (system(
			"wget -N --no-check-certificate --quiet -O eqemu_server.pl https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/eqemu_server.pl"
		)) {}
#endif
	}
}

