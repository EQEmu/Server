/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
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

#define DONT_SHARED_OPCODES
#define PLATFORM_ZONE 1

#include "../common/global_define.h"
#include "../common/timer.h"
#include "../common/eq_packet_structs.h"
#include "../common/mutex.h"
#include "../common/opcodemgr.h"
#include "../common/guilds.h"
#include "../common/eq_stream_ident.h"
#include "../common/patches/patches.h"
#include "../common/rulesys.h"
#include "../common/profanity_manager.h"
#include "../common/strings.h"
#include "../common/crash.h"
#include "../common/memory_mapped_file.h"
#include "../common/spdat.h"
#include "../common/eqemu_logsys.h"
#include "../common/misc.h"

#include "api_service.h"
#include "zone_config.h"
#include "masterentity.h"
#include "worldserver.h"
#include "zone.h"
#include "queryserv.h"
#include "command.h"
#include "bot_command.h"
#include "zonedb.h"
#include "titles.h"
#include "guild_mgr.h"
#include "task_manager.h"
#include "quest_parser_collection.h"
#include "embparser.h"
#include "../common/evolving_items.h"
#include "lua_parser.h"
#include "questmgr.h"
#include "npc_scale_manager.h"

#include "../common/net/eqstream.h"

#include <signal.h>
#include <chrono>

#ifdef _CRTDBG_MAP_ALLOC
#undef new
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifdef _WINDOWS
#else

#include <pthread.h>
#include "../common/unix.h"

#endif

volatile bool RunLoops = true;
#ifdef __FreeBSD__
#include <pthread_np.h>
#endif

extern volatile bool is_zone_loaded;

#include "zone_event_scheduler.h"
#include "../common/file.h"
#include "../common/events/player_event_logs.h"
#include "../common/path_manager.h"
#include "../common/database/database_update.h"
#include "../common/skill_caps.h"
#include "zone_event_scheduler.h"
#include "zone_cli.h"

EntityList  entity_list;
WorldServer worldserver;
ZoneStore   zone_store;
uint32      numclients = 0;
char        errorname[32];
extern Zone *zone;

npcDecayTimes_Struct  npcCorpseDecayTimes[100];
TitleManager          title_manager;
QueryServ             *QServ        = 0;
TaskManager           *task_manager = 0;
NpcScaleManager       *npc_scale_manager;
QuestParserCollection *parse        = 0;
EQEmuLogSys           LogSys;
ZoneEventScheduler    event_scheduler;
WorldContentService   content_service;
PathManager           path;
PlayerEventLogs       player_event_logs;
DatabaseUpdate        database_update;
SkillCaps             skill_caps;
EvolvingItemsManager  evolving_items_manager;

const SPDat_Spell_Struct* spells;
int32 SPDAT_RECORDS = -1;
const ZoneConfig *Config;
double frame_time = 0.0;

void Shutdown();
void UpdateWindowTitle(char *iNewTitle);
void CatchSignal(int sig_num);

extern void MapOpcodes();

bool CheckForCompatibleQuestPlugins();
int main(int argc, char **argv)
{
	RegisterExecutablePlatform(ExePlatformZone);
	LogSys.LoadLogSettingsDefaults();

	set_exception_handler();

	// silence logging if we ran a command
	if (ZoneCLI::RanConsoleCommand(argc, argv) || ZoneCLI::RanTestCommand(argc, argv)) {
		LogSys.SilenceConsoleLogging();
	}

	path.LoadPaths();

#ifdef USE_MAP_MMFS
	if (argc == 3 && strcasecmp(argv[1], "convert_map") == 0) {
		if (!ZoneConfig::LoadConfig())
			return 1;
		Config = ZoneConfig::get();

		std::string mapfile = argv[2];
		std::transform(mapfile.begin(), mapfile.end(), mapfile.begin(), ::tolower);
		std::string filename = Config->MapDir;
		filename += mapfile;

		auto m = new Map();
		auto success = m->Load(filename, true);
		delete m;
		std::cout << mapfile.c_str() << " conversion " << (success ? "succeeded" : "failed") << std::endl;

		return 0;
	}
#endif /*USE_MAP_MMFS*/

	QServ = new QueryServ;

	LogInfo("Loading server configuration");
	if (!ZoneConfig::LoadConfig()) {
		LogError("Loading server configuration failed");
		return 1;
	}
	Config = ZoneConfig::get();

	// static zone booting
	const char  *zone_name;
	uint32      instance_id = 0;
	std::string z_name;
	if (!ZoneCLI::RanSidecarCommand(argc, argv)) {
		if (argc == 4) {
			instance_id = Strings::ToInt(argv[3]);
			worldserver.SetLauncherName(argv[2]);
			auto zone_port = Strings::Split(argv[1], ':');

			if (!zone_port.empty()) {
				z_name = zone_port[0];
			}

			if (zone_port.size() > 1) {
				std::string p_name = zone_port[1];
				Config->SetZonePort(Strings::ToInt(p_name));
			}

			worldserver.SetLaunchedName(z_name.c_str());
			if (strncmp(z_name.c_str(), "dynamic_", 8) == 0) {
				zone_name = ".";
			}
			else {
				zone_name = z_name.c_str();
			}
		}
		else if (argc == 3) {
			worldserver.SetLauncherName(argv[2]);
			auto zone_port = Strings::Split(argv[1], ':');

			if (!zone_port.empty()) {
				z_name = zone_port[0];
			}

			if (zone_port.size() > 1) {
				std::string p_name = zone_port[1];
				Config->SetZonePort(Strings::ToInt(p_name));
			}

			worldserver.SetLaunchedName(z_name.c_str());
			if (strncmp(z_name.c_str(), "dynamic_", 8) == 0) {
				zone_name = ".";
			}
			else {
				zone_name = z_name.c_str();
			}
		}
		else if (argc == 2) {
			worldserver.SetLauncherName("NONE");
			auto zone_port = Strings::Split(argv[1], ':');

			if (!zone_port.empty()) {
				z_name = zone_port[0];
			}

			if (zone_port.size() > 1) {
				std::string p_name = zone_port[1];
				Config->SetZonePort(Strings::ToInt(p_name));
			}

			worldserver.SetLaunchedName(z_name.c_str());
			if (strncmp(z_name.c_str(), "dynamic_", 8) == 0) {
				zone_name = ".";
			}
			else {
				zone_name = z_name.c_str();
			}
		}
		else {
			zone_name = ".";
			worldserver.SetLaunchedName(".");
			worldserver.SetLauncherName("NONE");
		}
	}

	auto mutex = new Mutex;

	LogInfo("Connecting to MySQL");
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort
	)) {
		LogError("Cannot continue without a database connection");
		return 1;
	}

	// Multi-tenancy: Content Database
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
			return 1;
		}
	} else {
		content_db.SetMySQL(database);
		// when database and content_db share the same underlying mysql connection
		// it needs to be protected by a shared mutex otherwise we produce concurrency issues
		// when database actions are occurring in different threads
		database.SetMutex(mutex);
		content_db.SetMutex(mutex);
	}

	//rules:
	{
		std::string tmp;
		if (database.GetVariable("RuleSet", tmp)) {
			LogInfo("Loading rule set [{}]", tmp.c_str());
			if (!RuleManager::Instance()->LoadRules(&database, tmp.c_str(), false)) {
				LogError("Failed to load ruleset [{}], falling back to defaults", tmp.c_str());
			}
		}
		else {
			if (!RuleManager::Instance()->LoadRules(&database, "default", false)) {
				LogInfo("No rule set configured, using default rules");
			}
		}

		EQ::InitializeDynamicLookups();
	}

	// command handler (no sidecar or test commands)
	if (ZoneCLI::RanConsoleCommand(argc, argv) && !(ZoneCLI::RanSidecarCommand(argc, argv) || ZoneCLI::RanTestCommand(argc, argv))) {
		LogSys.EnableConsoleLogging();
		ZoneCLI::CommandHandler(argc, argv);
	}

	LogSys.SetDatabase(&database)
		->SetLogPath(path.GetLogPath())
		->LoadLogDatabaseSettings()
		->SetGMSayHandler(&Zone::GMSayHookCallBackProcess)
		->StartFileLogs();

	if (ZoneCLI::RanTestCommand(argc, argv)) {
		LogSys.SilenceConsoleLogging();
	}

	player_event_logs.SetDatabase(&database)->Init();

	skill_caps.SetContentDatabase(&content_db)->LoadSkillCaps();

	const auto c = EQEmuConfig::get();
	if (c->auto_database_updates) {
		if (database_update.SetDatabase(&database)->HasPendingUpdates()) {
			LogWarning("Database is not up to date [world] needs to be ran to apply updates, shutting down in 5 seconds");
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			LogInfo("Exiting due to pending database updates");
			std::exit(0);
		}
	}

	/* Guilds */
	guild_mgr.SetDatabase(&database)->SetContentDatabase(&content_db);
	GuildBanks = nullptr;

#ifdef _EQDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	LogInfo("CURRENT_VERSION [{}]", CURRENT_VERSION);

	/*
	* Setup nice signal handlers
	*/
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

	MapOpcodes();

	database.LoadVariables();

	std::string hotfix_name;
	if (database.GetVariable("hotfix_name", hotfix_name)) {
		if (!hotfix_name.empty()) {
			LogInfo("Current hotfix in use: [{}]", hotfix_name.c_str());
		}
	}

	zone_store.LoadZones(content_db);

	if (zone_store.GetZones().empty()) {
		LogError("Failed to load zones data, check your schema for possible errors");
		return 1;
	}

	if (!CheckForCompatibleQuestPlugins()) {
		LogError("Incompatible quest plugins detected, please update your plugins to the latest version");
		return 1;
	}

	// load these here for now until spells and items can be truly repointed to "content_db"
	database.SetSharedItemsCount(content_db.GetItemsCount());
	database.SetSharedSpellsCount(content_db.GetSpellsCount());

	if (!database.LoadItems(hotfix_name)) {
		LogError("Loading items failed!");
		LogError("Failed. But ignoring error and going on..");
	}

	if (!database.LoadSpells(hotfix_name, &SPDAT_RECORDS, &spells)) {
		LogError("Loading spells failed!");
		return 1;
	}


	guild_mgr.LoadGuilds();
	content_db.LoadFactionData();
	title_manager.LoadTitles();
	content_db.LoadTributes();

	// Load evolving item data
	evolving_items_manager.SetDatabase(&database);
	evolving_items_manager.SetContentDatabase(&content_db);
	evolving_items_manager.LoadEvolvingItems();

	database.GetDecayTimes(npcCorpseDecayTimes);

	if (!EQ::ProfanityManager::LoadProfanityList(&database)) {
		LogError("Loading profanity list failed!");
	}

	int retval = command_init();
	if (retval < 0) {
		LogError("Command loading failed");
	}
	else {
		LogInfo("Loaded [{}] commands loaded", Strings::Commify(std::to_string(retval)));
	}

	content_service.SetDatabase(&database)
		->SetContentDatabase(&content_db)
		->SetExpansionContext()
		->ReloadContentFlags();

	event_scheduler.SetDatabase(&database)->LoadScheduledEvents();

	EQ::SayLinkEngine::LoadCachedSaylinks();

	if (RuleB(Bots, Enabled) && database.DoesTableExist("bot_command_settings")) {
		LogInfo("Loading bot commands");
		int botretval = bot_command_init();
		if (botretval < 0) {
			LogError("Bot command loading failed");
		}
		else {
			LogInfo("[{}] bot commands loaded", botretval);
		}

		LogInfo("Loading bot spell casting chances");
		if (!database.botdb.LoadBotSpellCastingChances()) {
			LogError("Bot spell casting chances loading failed");
		}
	}

	/**
	 * NPC Scale Manager
	 */
	npc_scale_manager = new NpcScaleManager;
	npc_scale_manager->LoadScaleData();

	if (RuleB(TaskSystem, EnableTaskSystem)) {
		task_manager = new TaskManager;
		task_manager->LoadTasks();
	}

	parse = new QuestParserCollection();
#ifdef LUA_EQEMU
	parse->RegisterQuestInterface(LuaParser::Instance(), "lua");
#endif

#ifdef EMBPERL
	auto perl_parser = new PerlembParser();
	parse->RegisterQuestInterface(perl_parser, "pl");

#ifdef __linux__
	std::string current_version = CURRENT_VERSION;
	// running release binaries
	if (!Strings::Contains(current_version, "-dev")) {
		if (!fs::exists("/opt/eqemu-perl")) {
			LogError("You are running release binaries without having the required eqemu version of perl compiled and installed on this system present at /opt/eqemu-perl");
			LogError("If you are running an old Linux install, you need to install the required perl version from the eqemu-perl");
			LogError("Instructions can be referenced at https://github.com/Akkadius/akk-stack/blob/master/containers/eqemu-server/Dockerfile#L92-L106");
			LogError("Press any key to continue");
			getchar();
			return 0;
		}
	}
#endif

	/* Load Perl Event Export Settings */
	parse->LoadPerlEventExportSettings(parse->perl_event_export_settings);

#endif

	//now we have our parser, load the quests
	LogInfo("Loading quests");
	parse->ReloadQuests();

	QServ->CheckForConnectState();

	worldserver.Connect();
	worldserver.SetScheduler(&event_scheduler);

	// sidecar command handler
	if (ZoneCLI::RanConsoleCommand(argc, argv)
		&& (ZoneCLI::RanSidecarCommand(argc, argv) || ZoneCLI::RanTestCommand(argc, argv))) {
		LogSys.EnableConsoleLogging();
		ZoneCLI::CommandHandler(argc, argv);
	}

	Timer InterserverTimer(INTERSERVER_TIMER); // does MySQL pings and auto-reconnect
#ifdef EQPROFILE
#ifdef PROFILE_DUMP_TIME
	Timer profile_dump_timer(PROFILE_DUMP_TIME * 1000);
	profile_dump_timer.Start();
#endif
#endif
	if (!strlen(zone_name) || !strcmp(zone_name, ".")) {
		LogInfo("Entering sleep mode");
	}
	else if (!Zone::Bootup(ZoneID(zone_name), instance_id, true)) {
		LogError("Zone Bootup failed :: Zone::Bootup");
		zone = nullptr;
	}

	//register all the patches we have avaliable with the stream identifier.
	EQStreamIdentifier stream_identifier;
	RegisterAllPatches(stream_identifier);

#ifdef __linux__
	LogDebug("Main thread running with thread id [{}]", pthread_self());
#elif defined(__FreeBSD__)
	LogDebug("Main thread running with thread id [{}]", pthread_getthreadid_np());
#endif

	bool worldwasconnected       = worldserver.Connected();
	bool eqsf_open               = false;
	bool websocker_server_opened = false;

	Timer quest_timers(100);
	UpdateWindowTitle(nullptr);
	std::shared_ptr<EQStreamInterface>                 eqss;
	EQStreamInterface                                  *eqsi;
	std::unique_ptr<EQ::Net::EQStreamManager>          eqsm;
	std::chrono::time_point<std::chrono::system_clock> frame_prev = std::chrono::system_clock::now();
	std::unique_ptr<EQ::Net::WebsocketServer>          ws_server;

	auto loop_fn = [&](EQ::Timer *t) {
		//Advance the timer to our current point in time
		Timer::SetCurrentTime();

		/**
		 * Calculate frame time
		 */
		std::chrono::time_point<std::chrono::system_clock> frame_now = std::chrono::system_clock::now();
		frame_time = std::chrono::duration_cast<std::chrono::duration<double>>(frame_now - frame_prev).count();
		frame_prev = frame_now;

		/**
		 * Websocket server
		 */
		if (!websocker_server_opened && Config->ZonePort != 0) {
			LogInfo("Websocket Server listener started on address [{}] port [{}]", Config->TelnetIP.c_str(), Config->ZonePort);
			ws_server = std::make_unique<EQ::Net::WebsocketServer>(Config->TelnetIP, Config->ZonePort);
			RegisterApiService(ws_server);
			websocker_server_opened = true;
		}

		/**
		 * EQStreamManager
		 */
		if (!eqsf_open && Config->ZonePort != 0) {
			LogInfo("Starting EQ Network server on port [{}]", Config->ZonePort);

			EQStreamManagerInterfaceOptions opts(Config->ZonePort, false, RuleB(Network, CompressZoneStream));
			opts.daybreak_options.resend_delay_ms     = RuleI(Network, ResendDelayBaseMS);
			opts.daybreak_options.resend_delay_factor = RuleR(Network, ResendDelayFactor);
			opts.daybreak_options.resend_delay_min    = RuleI(Network, ResendDelayMinMS);
			opts.daybreak_options.resend_delay_max    = RuleI(Network, ResendDelayMaxMS);
			opts.daybreak_options.outgoing_data_rate  = RuleR(Network, ClientDataRate);
			eqsm      = std::make_unique<EQ::Net::EQStreamManager>(opts);
			eqsf_open = true;

			eqsm->OnNewConnection(
				[&stream_identifier](std::shared_ptr<EQ::Net::EQStream> stream) {
					stream_identifier.AddStream(stream);
					LogInfo(
						"New connection from address [{}] port [{}]",
						long2ip(stream->GetRemoteIP()),
						ntohs(stream->GetRemotePort())
					);
				}
			);
		}

		//give the stream identifier a chance to do its work....
		stream_identifier.Process();

		//check the stream identifier for any now-identified streams
		while ((eqsi = stream_identifier.PopIdentified())) {
			//now that we know what patch they are running, start up their client object
			struct in_addr in;
			in.s_addr = eqsi->GetRemoteIP();
			LogInfo("New client from [{}]:[{}]", inet_ntoa(in), ntohs(eqsi->GetRemotePort()));
			auto client = new Client(eqsi);
			entity_list.AddClient(client);
		}

		if (worldserver.Connected()) {
			worldwasconnected = true;
		}
		else {
			if (worldwasconnected && is_zone_loaded) {
				entity_list.ChannelMessageFromWorld(
					0,
					0,
					ChatChannel_Broadcast,
					0,
					0,
					100,
					"WARNING: World server connection lost"
				);
				worldwasconnected = false;
			}
		}

		if (is_zone_loaded) {
			{
				entity_list.GroupProcess();
				entity_list.DoorProcess();
				entity_list.ObjectProcess();
				entity_list.CorpseProcess();
				entity_list.TrapProcess();
				entity_list.RaidProcess();
				entity_list.Process();
				entity_list.MobProcess();
				entity_list.BeaconProcess();
				entity_list.EncounterProcess();
				event_scheduler.Process(zone, &content_service);

				if (zone) {
					if (!zone->Process()) {
						Zone::Shutdown();
					}
				}

				if (quest_timers.Check()) {
					quest_manager.Process();
				}
			}
		}

		QServ->CheckForConnectState();

		if (InterserverTimer.Check()) {
			InterserverTimer.Start();
			database.ping();
			content_db.ping();
			entity_list.UpdateWho();
		}
	};

	EQ::Timer process_timer(loop_fn);
	process_timer.Start(32, true);

	EQ::EventLoop::Get().Run();

	entity_list.Clear();
	entity_list.RemoveAllEncounters(); // gotta do it manually or rewrite lots of shit :P

	parse->ClearInterfaces();

#ifdef EMBPERL
	safe_delete(perl_parser);
#endif

	safe_delete(Config);

	if (zone != 0) {
		Zone::Shutdown(true);
	}
	//Fix for Linux world server problem.
	safe_delete(task_manager);
	safe_delete(npc_scale_manager);
	command_deinit();
	bot_command_deinit();
	safe_delete(parse);
	LogInfo("Proper zone shutdown complete.");
	LogSys.CloseFileLogs();

	safe_delete(mutex);
	safe_delete(QServ);

	return 0;
}

void Shutdown()
{
	Zone::Shutdown(true);
	LogInfo("Shutting down...");
	LogSys.CloseFileLogs();
	EQ::EventLoop::Get().Shutdown();
}

void CatchSignal(int sig_num)
{
#ifdef _WINDOWS
	LogInfo("Recieved signal: [{}]", sig_num);
#endif
	Shutdown();
}

/* Update Window Title with relevant information */
void UpdateWindowTitle(char *iNewTitle)
{
#ifdef _WINDOWS
	char tmp[500];
	if (iNewTitle) {
		snprintf(tmp, sizeof(tmp), "%i: %s", ZoneConfig::get()->ZonePort, iNewTitle);
	}
	else {
		if (zone) {
#if defined(GOTFRAGS) || defined(_EQDEBUG)
			snprintf(tmp, sizeof(tmp), "%i: %s, %i clients, %i", ZoneConfig::get()->ZonePort, zone->GetShortName(), numclients, getpid());
#else
			snprintf(tmp, sizeof(tmp), "%s :: clients: %i inst_id: %i inst_ver: %i :: port: %i", zone->GetShortName(), numclients, zone->GetInstanceID(), zone->GetInstanceVersion(), ZoneConfig::get()->ZonePort);
#endif
		}
		else {
#if defined(GOTFRAGS) || defined(_EQDEBUG)
			snprintf(tmp, sizeof(tmp), "%i: sleeping, %i", ZoneConfig::get()->ZonePort, getpid());
#else
			snprintf(tmp, sizeof(tmp), "%i: sleeping", ZoneConfig::get()->ZonePort);
#endif
		}
	}
	SetConsoleTitle(tmp);
#endif
}

bool CheckForCompatibleQuestPlugins()
{
	const std::vector<std::string>& directories = { "lua_modules", "plugins" };

	bool lua_found  = false;
	bool perl_found = false;

	for (const auto& directory : directories) {
		for (const auto& file : fs::directory_iterator(path.GetServerPath() + "/" + directory)) {
			if (file.is_regular_file()) {
				auto f = file.path().string();
				if (File::Exists(f)) {
					auto r = File::GetContents(std::filesystem::path{ f }.string());
					if (Strings::Contains(r.contents, "CheckHandin")) {
						if (Strings::EqualFold(directory, "lua_modules")) {
							lua_found = true;
						} else if (Strings::EqualFold(directory, "plugins")) {
							perl_found = true;
						}

						if (lua_found && perl_found) {
							return true;
						}
					}
				}
			}
		}
	}

	if (!lua_found) {
		LogError("Failed to find CheckHandin in lua_modules");
	}

	if (!perl_found) {
		LogError("Failed to find CheckHandin in plugins");
	}

	return lua_found && perl_found;
}
