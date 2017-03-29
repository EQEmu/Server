/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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

#define DONT_SHARED_OPCODES
#define PLATFORM_ZONE 1

#include "../common/global_define.h"
#include "../common/features.h"
#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/eq_stream.h"
#include "../common/eq_stream_factory.h"
#include "../common/eq_packet_structs.h"
#include "../common/mutex.h"
#include "../common/version.h"
#include "../common/packet_dump_file.h"
#include "../common/opcodemgr.h"
#include "../common/guilds.h"
#include "../common/eq_stream_ident.h"
#include "../common/patches/patches.h"
#include "../common/rulesys.h"
#include "../common/misc_functions.h"
#include "../common/string_util.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/ipc_mutex.h"
#include "../common/memory_mapped_file.h"
#include "../common/eqemu_exception.h"
#include "../common/spdat.h"
#include "../common/eqemu_logsys.h"


#include "zone_config.h"
#include "masterentity.h"
#include "worldserver.h"
#include "net.h"
#include "zone.h"
#include "queryserv.h"
#include "command.h"
#ifdef BOTS
#include "bot_command.h"
#include "bot_database.h"
#endif
#include "zone_config.h"
#include "titles.h"
#include "guild_mgr.h"
#include "tasks.h"
#include "quest_parser_collection.h"
#include "embparser.h"
#include "lua_parser.h"
#include "questmgr.h"

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <ctime>

#ifdef _CRTDBG_MAP_ALLOC
	#undef new
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifdef _WINDOWS
	#include <conio.h>
	#include <process.h>
#else
	#include <pthread.h>
	#include "../common/unix.h"
#endif

volatile bool RunLoops = true;
extern volatile bool is_zone_loaded;

TimeoutManager timeout_manager;
NetConnection net;
EntityList entity_list;
WorldServer worldserver;
uint32 numclients = 0;
char errorname[32];
extern Zone* zone;
EQStreamFactory eqsf(ZoneStream);
npcDecayTimes_Struct npcCorpseDecayTimes[100];
TitleManager title_manager;
QueryServ *QServ = 0;
TaskManager *taskmanager = 0;
QuestParserCollection *parse = 0;
EQEmuLogSys Log;
const SPDat_Spell_Struct* spells;
int32 SPDAT_RECORDS = -1;
const ZoneConfig *Config;

void Shutdown();
extern void MapOpcodes();

int main(int argc, char** argv) {
	RegisterExecutablePlatform(ExePlatformZone); 
	Log.LoadLogSettingsDefaults();

	set_exception_handler(); 

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

	Log.Out(Logs::General, Logs::Zone_Server, "Loading server configuration..");
	if(!ZoneConfig::LoadConfig()) {
		Log.Out(Logs::General, Logs::Error, "Loading server configuration failed.");
		return 1;
	}
	Config = ZoneConfig::get();

	const char *zone_name;
	uint32 instance_id = 0;
	std::string z_name;
	if(argc == 4) {
		instance_id = atoi(argv[3]);
		worldserver.SetLauncherName(argv[2]);
		auto zone_port = SplitString(argv[1], ':');

		if(!zone_port.empty()) {
			z_name = zone_port[0];
		}

		if(zone_port.size() > 1) {
			std::string p_name = zone_port[1];
			Config->SetZonePort(atoi(p_name.c_str()));
		}

		worldserver.SetLaunchedName(z_name.c_str());
		if(strncmp(z_name.c_str(), "dynamic_", 8) == 0) {
			zone_name = ".";
		}
		else {
			zone_name = z_name.c_str();
		}
	} else if(argc == 3) {
		worldserver.SetLauncherName(argv[2]);
		auto zone_port = SplitString(argv[1], ':');

		if(!zone_port.empty()) {
			z_name = zone_port[0];
		}

		if(zone_port.size() > 1) {
			std::string p_name = zone_port[1];
			Config->SetZonePort(atoi(p_name.c_str()));
		}

		worldserver.SetLaunchedName(z_name.c_str());
		if(strncmp(z_name.c_str(), "dynamic_", 8) == 0) {
			zone_name = ".";
		} else {
			zone_name = z_name.c_str();
		}
	} else if (argc == 2) {
		worldserver.SetLauncherName("NONE");
		auto zone_port = SplitString(argv[1], ':');

		if(!zone_port.empty()) {
			z_name = zone_port[0];
		}

		if(zone_port.size() > 1) {
			std::string p_name = zone_port[1];
			Config->SetZonePort(atoi(p_name.c_str()));
		}

		worldserver.SetLaunchedName(z_name.c_str());
		if(strncmp(z_name.c_str(), "dynamic_", 8) == 0) {
			zone_name = ".";
		}
		else {
			zone_name = z_name.c_str();
		}
	} else {
		zone_name = ".";
		worldserver.SetLaunchedName(".");
		worldserver.SetLauncherName("NONE");
	}

	worldserver.SetPassword(Config->SharedKey.c_str());
	
	Log.Out(Logs::General, Logs::Zone_Server, "Connecting to MySQL...");
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		Log.Out(Logs::General, Logs::Error, "Cannot continue without a database connection.");
		return 1;
	}

#ifdef BOTS
	if (!botdb.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		Log.Out(Logs::General, Logs::Error, "Cannot continue without a bots database connection.");
		return 1;
	}
#endif

	/* Register Log System and Settings */
	Log.OnLogHookCallBackZone(&Zone::GMSayHookCallBackProcess);
	database.LoadLogSettings(Log.log_settings); 
	Log.StartFileLogs();

	/* Guilds */
	guild_mgr.SetDatabase(&database);
	GuildBanks = nullptr;

#ifdef _EQDEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Log.Out(Logs::General, Logs::Zone_Server, "CURRENT_VERSION: %s", CURRENT_VERSION);

	/*
	* Setup nice signal handlers
	*/
	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		Log.Out(Logs::General, Logs::Error, "Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		Log.Out(Logs::General, Logs::Error, "Could not set signal handler");
		return 1;
	}
	#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)	{
		Log.Out(Logs::General, Logs::Error, "Could not set signal handler");
		return 1;
	}
	#endif

	Log.Out(Logs::General, Logs::Zone_Server, "Mapping Incoming Opcodes");
	MapOpcodes();

	Log.Out(Logs::General, Logs::Zone_Server, "Loading Variables");
	database.LoadVariables();

	std::string hotfix_name;
	if(database.GetVariable("hotfix_name", hotfix_name)) {
		if(!hotfix_name.empty()) {
			Log.Out(Logs::General, Logs::Zone_Server, "Current hotfix in use: '%s'", hotfix_name.c_str());
		}
	}

	Log.Out(Logs::General, Logs::Zone_Server, "Loading zone names");
	database.LoadZoneNames();

	Log.Out(Logs::General, Logs::Zone_Server, "Loading items");
	if(!database.LoadItems(hotfix_name)) {
		Log.Out(Logs::General, Logs::Error, "Loading items FAILED!");
		Log.Out(Logs::General, Logs::Error, "Failed. But ignoring error and going on...");
	}

	Log.Out(Logs::General, Logs::Zone_Server, "Loading npc faction lists");
	if(!database.LoadNPCFactionLists(hotfix_name)) {
		Log.Out(Logs::General, Logs::Error, "Loading npcs faction lists FAILED!");
		return 1;
	}
	Log.Out(Logs::General, Logs::Zone_Server, "Loading loot tables");
	if(!database.LoadLoot(hotfix_name)) {
		Log.Out(Logs::General, Logs::Error, "Loading loot FAILED!");
		return 1;
	}
	Log.Out(Logs::General, Logs::Zone_Server, "Loading skill caps");
	if(!database.LoadSkillCaps(std::string(hotfix_name))) {
		Log.Out(Logs::General, Logs::Error, "Loading skill caps FAILED!");
		return 1;
	}

	Log.Out(Logs::General, Logs::Zone_Server, "Loading spells");
	if(!database.LoadSpells(hotfix_name, &SPDAT_RECORDS, &spells)) {
		Log.Out(Logs::General, Logs::Error, "Loading spells FAILED!");
		return 1;
	}

	Log.Out(Logs::General, Logs::Zone_Server, "Loading base data");
	if(!database.LoadBaseData(hotfix_name)) {
		Log.Out(Logs::General, Logs::Error, "Loading base data FAILED!");
		return 1;
	}

	Log.Out(Logs::General, Logs::Zone_Server, "Loading guilds");
	guild_mgr.LoadGuilds();
	
	Log.Out(Logs::General, Logs::Zone_Server, "Loading factions");
	database.LoadFactionData();
	
	Log.Out(Logs::General, Logs::Zone_Server, "Loading titles");
	title_manager.LoadTitles();
	
	Log.Out(Logs::General, Logs::Zone_Server, "Loading tributes");
	database.LoadTributes();
	
	Log.Out(Logs::General, Logs::Zone_Server, "Loading corpse timers");
	database.GetDecayTimes(npcCorpseDecayTimes);
	
	Log.Out(Logs::General, Logs::Zone_Server, "Loading commands");
	int retval=command_init();
	if(retval<0)
		Log.Out(Logs::General, Logs::Error, "Command loading FAILED");
	else
		Log.Out(Logs::General, Logs::Zone_Server, "%d commands loaded", retval);

	//rules:
	{
		std::string tmp;
		if (database.GetVariable("RuleSet", tmp)) {
			Log.Out(Logs::General, Logs::Zone_Server, "Loading rule set '%s'", tmp.c_str());
			if(!RuleManager::Instance()->LoadRules(&database, tmp.c_str())) {
				Log.Out(Logs::General, Logs::Error, "Failed to load ruleset '%s', falling back to defaults.", tmp.c_str());
			}
		} else {
			if(!RuleManager::Instance()->LoadRules(&database, "default")) {
				Log.Out(Logs::General, Logs::Zone_Server, "No rule set configured, using default rules");
			} else {
				Log.Out(Logs::General, Logs::Zone_Server, "Loaded default rule set 'default'", tmp.c_str());
			}
		}
	}

#ifdef BOTS
	Log.Out(Logs::General, Logs::Zone_Server, "Loading bot commands");
	int botretval = bot_command_init();
	if (botretval<0)
		Log.Out(Logs::General, Logs::Error, "Bot command loading FAILED");
	else
		Log.Out(Logs::General, Logs::Zone_Server, "%d bot commands loaded", botretval);

	Log.Out(Logs::General, Logs::Zone_Server, "Loading bot spell casting chances");
	if (!botdb.LoadBotSpellCastingChances())
		Log.Out(Logs::General, Logs::Error, "Bot spell casting chances loading FAILED");
#endif

	if(RuleB(TaskSystem, EnableTaskSystem)) {
		Log.Out(Logs::General, Logs::Tasks, "[INIT] Loading Tasks");
		taskmanager = new TaskManager;
		taskmanager->LoadTasks();
	}

	parse = new QuestParserCollection();
#ifdef LUA_EQEMU
	auto lua_parser = new LuaParser();
	parse->RegisterQuestInterface(lua_parser, "lua");
#endif

#ifdef EMBPERL
	auto perl_parser = new PerlembParser();
	parse->RegisterQuestInterface(perl_parser, "pl");

	/* Load Perl Event Export Settings */
	parse->LoadPerlEventExportSettings(parse->perl_event_export_settings);

#endif

	//now we have our parser, load the quests
	Log.Out(Logs::General, Logs::Zone_Server, "Loading quests");
	parse->ReloadQuests();

	if (!worldserver.Connect()) {
		Log.Out(Logs::General, Logs::Error, "Worldserver Connection Failed :: worldserver.Connect()");
	}

	Timer InterserverTimer(INTERSERVER_TIMER); // does MySQL pings and auto-reconnect
#ifdef EQPROFILE
#ifdef PROFILE_DUMP_TIME
	Timer profile_dump_timer(PROFILE_DUMP_TIME*1000);
	profile_dump_timer.Start();
#endif
#endif
	if (!strlen(zone_name) || !strcmp(zone_name,".")) {
		Log.Out(Logs::General, Logs::Zone_Server, "Entering sleep mode");
	} else if (!Zone::Bootup(database.GetZoneID(zone_name), instance_id, true)) {
		Log.Out(Logs::General, Logs::Error, "Zone Bootup failed :: Zone::Bootup");
		zone = 0;
	}

	//register all the patches we have avaliable with the stream identifier.
	EQStreamIdentifier stream_identifier;
	RegisterAllPatches(stream_identifier);

#ifndef WIN32
	Log.Out(Logs::Detail, Logs::None,  "Main thread running with thread id %d", pthread_self());
#endif

	Timer quest_timers(100);
	UpdateWindowTitle();
	bool worldwasconnected = worldserver.Connected();
	std::shared_ptr<EQStream> eqss;
	EQStreamInterface *eqsi;
	uint8 IDLEZONEUPDATE = 200;
	uint8 ZONEUPDATE = 10;
	Timer zoneupdate_timer(ZONEUPDATE);
	zoneupdate_timer.Start();
	while(RunLoops) {
		{	//profiler block to omit the sleep from times

		//Advance the timer to our current point in time
		Timer::SetCurrentTime();

		worldserver.Process();

		if (!eqsf.IsOpen() && Config->ZonePort != 0) {
			Log.Out(Logs::General, Logs::Zone_Server, "Starting EQ Network server on port %d", Config->ZonePort);
			if (!eqsf.Open(Config->ZonePort)) {
				Log.Out(Logs::General, Logs::Error, "Failed to open port %d", Config->ZonePort);
				ZoneConfig::SetZonePort(0);
				worldserver.Disconnect();
				worldwasconnected = false;
			}
		}

		//check the factory for any new incoming streams.
		while ((eqss = eqsf.Pop())) {
			//pull the stream out of the factory and give it to the stream identifier
			//which will figure out what patch they are running, and set up the dynamic
			//structures and opcodes for that patch.
			struct in_addr	in;
			in.s_addr = eqss->GetRemoteIP();
			Log.Out(Logs::Detail, Logs::World_Server, "New connection from %s:%d", inet_ntoa(in), ntohs(eqss->GetRemotePort()));
			stream_identifier.AddStream(eqss);	//takes the stream
		}

		//give the stream identifier a chance to do its work....
		stream_identifier.Process();

		//check the stream identifier for any now-identified streams
		while((eqsi = stream_identifier.PopIdentified())) {
			//now that we know what patch they are running, start up their client object
			struct in_addr	in;
			in.s_addr = eqsi->GetRemoteIP();
			Log.Out(Logs::Detail, Logs::World_Server, "New client from %s:%d", inet_ntoa(in), ntohs(eqsi->GetRemotePort()));
			auto client = new Client(eqsi);
			entity_list.AddClient(client);
		}

		if ( numclients < 1 && zoneupdate_timer.GetDuration() != IDLEZONEUPDATE )
			zoneupdate_timer.SetTimer(IDLEZONEUPDATE);
		else if ( numclients > 0 && zoneupdate_timer.GetDuration() == IDLEZONEUPDATE )
		{
			zoneupdate_timer.SetTimer(ZONEUPDATE);
			zoneupdate_timer.Trigger();
		}

		//check for timeouts in other threads
		timeout_manager.CheckTimeouts();

		if (worldserver.Connected()) {
			worldwasconnected = true;
		}
		else {
			if (worldwasconnected && is_zone_loaded)
				entity_list.ChannelMessageFromWorld(0, 0, 6, 0, 0, "WARNING: World server connection lost");
			worldwasconnected = false;
		}

		if (is_zone_loaded && zoneupdate_timer.Check()) {
			{
				if(net.group_timer.Enabled() && net.group_timer.Check())
					entity_list.GroupProcess();

				if(net.door_timer.Enabled() && net.door_timer.Check())
					entity_list.DoorProcess();

				if(net.object_timer.Enabled() && net.object_timer.Check())
					entity_list.ObjectProcess();

				if(net.corpse_timer.Enabled() && net.corpse_timer.Check())
					entity_list.CorpseProcess();

				if(net.trap_timer.Enabled() && net.trap_timer.Check())
					entity_list.TrapProcess();

				if(net.raid_timer.Enabled() && net.raid_timer.Check())
					entity_list.RaidProcess();

				entity_list.Process(); 
				entity_list.MobProcess(); 
				entity_list.BeaconProcess();
				entity_list.EncounterProcess();

				if (zone) {
					if(!zone->Process()) {
						Zone::Shutdown();
					}
				}

				if(quest_timers.Check())
					quest_manager.Process();

			}
		}
		if (InterserverTimer.Check()) {
			InterserverTimer.Start();
			database.ping();
			// AsyncLoadVariables(dbasync, &database);
			entity_list.UpdateWho();
			if (worldserver.TryReconnect() && (!worldserver.Connected()))
				worldserver.AsyncConnect();
		}

#ifdef EQPROFILE
#ifdef PROFILE_DUMP_TIME
		if(profile_dump_timer.Check()) {
			DumpZoneProfile();
		}
#endif
#endif
		}	//end extra profiler block 
		if (is_zone_loaded && numclients > 0) {
			Sleep(ZoneTimerResolution);
		}
		else {
			Sleep(1000);
		}
		
	}

	entity_list.Clear();
	entity_list.RemoveAllEncounters(); // gotta do it manually or rewrite lots of shit :P

	parse->ClearInterfaces();

#ifdef EMBPERL
 	safe_delete(perl_parser);
#endif

#ifdef LUA_EQEMU
	safe_delete(lua_parser);
#endif

	safe_delete(Config);

	if (zone != 0)
		Zone::Shutdown(true);
	//Fix for Linux world server problem.
	eqsf.Close();
	worldserver.Disconnect();
	safe_delete(taskmanager);
	command_deinit();
#ifdef BOTS
	bot_command_deinit();
#endif
	safe_delete(parse);
	Log.Out(Logs::General, Logs::Zone_Server, "Proper zone shutdown complete.");
	Log.CloseFileLogs();
	return 0;
}

void CatchSignal(int sig_num) {
#ifdef _WINDOWS
	Log.Out(Logs::General, Logs::Zone_Server, "Recieved signal: %i", sig_num);
#endif
	RunLoops = false;
}

void Shutdown()
{
	Zone::Shutdown(true);
	RunLoops = false;
	worldserver.Disconnect(); 
	Log.Out(Logs::General, Logs::Zone_Server, "Shutting down...");
	Log.CloseFileLogs();
}

uint32 NetConnection::GetIP()
{
	char name[255+1];
	size_t len = 0;
	hostent* host = 0;

	if (gethostname(name, len) < 0 || len <= 0)
	{
		return 0;
	}

	host = (hostent*)gethostbyname(name);
	if (host == 0)
	{
		return 0;
	}

	return inet_addr(host->h_addr);
}

uint32 NetConnection::GetIP(char* name)
{
	hostent* host = 0;

	host = (hostent*)gethostbyname(name);
	if (host == 0)
	{
		return 0;
	}

	return inet_addr(host->h_addr);

}

NetConnection::NetConnection()
:
	object_timer(5000),
	door_timer(5000),
	corpse_timer(2000),
	group_timer(1000),
	raid_timer(1000),
	trap_timer(1000)
{
	group_timer.Disable();
	raid_timer.Disable();
	corpse_timer.Disable();
	door_timer.Disable();
	object_timer.Disable();
	trap_timer.Disable();
}

NetConnection::~NetConnection() {
}

/* Update Window Title with relevant information */
void UpdateWindowTitle(char* iNewTitle) {
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
