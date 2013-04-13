#define DONT_SHARED_OPCODES
/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include "../common/features.h"
#include <iostream>
using namespace std;
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
	#ifdef _CRTDBG_MAP_ALLOC
		#undef new
	#endif
#include <fstream>
	#ifdef _CRTDBG_MAP_ALLOC
		#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#endif
using namespace std;
#ifdef _WINDOWS
#include <conio.h>
#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
#define strncasecmp	_strnicmp
#define strcasecmp  _stricmp
#endif

volatile bool RunLoops = true;
extern volatile bool ZoneLoaded;

#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/EQStream.h"
#include "../common/EQStreamFactory.h"
#include "../common/eq_packet_structs.h"
#include "../common/Mutex.h"
#include "../common/version.h"
#include "../common/EQEMuError.h"
#include "ZoneConfig.h"
#include "../common/packet_dump_file.h"
#include "../common/opcodemgr.h"
#include "../common/guilds.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"
#include "../common/rulesys.h"
#include "../common/MiscFunctions.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/ipc_mutex.h"
#include "../common/memory_mapped_file.h"
#include "../common/eqemu_exception.h"

#include "masterentity.h"
#include "worldserver.h"
#include "net.h"
#include "../common/spdat.h"
#include "zone.h"
#include "command.h"
#include "parser.h"
#include "embparser.h"
#include "perlparser.h"
#include "client_logs.h"
#include "questmgr.h"
#include "titles.h"
#include "guild_mgr.h"
#include "tasks.h"
#include "QuestParserCollection.h"

TimeoutManager timeout_manager;
NetConnection net;
EntityList entity_list;
WorldServer worldserver;
uint32 numclients = 0;
char errorname[32];
uint16 adverrornum = 0;
extern Zone* zone;
EQStreamFactory eqsf(ZoneStream);
npcDecayTimes_Struct npcCorpseDecayTimes[100];
TitleManager title_manager;
DBAsyncFinishedQueue MTdbafq;
DBAsync *dbasync = NULL;
TaskManager *taskmanager = 0;
QuestParserCollection *parse = 0;

const SPDat_Spell_Struct* spells; 
void LoadSpells(EQEmu::MemoryMappedFile **mmf);
int32 SPDAT_RECORDS = -1;

#ifdef _WINDOWS
#include <process.h>
#else
#include <pthread.h>
#include "../common/unix.h"
#endif

void Shutdown();
extern void MapOpcodes();

int main(int argc, char** argv) {
    RegisterExecutablePlatform(ExePlatformZone);
    set_exception_handler();

	const char *zone_name;
	
	if(argc == 3) {
		worldserver.SetLauncherName(argv[2]);
		worldserver.SetLaunchedName(argv[1]);
		if(strncmp(argv[1], "dynamic_", 8) == 0) {
			//dynamic zone with a launcher name correlation
			zone_name = ".";
		} else {
			zone_name = argv[1];
			worldserver.SetLaunchedName(zone_name);
		}
	} else if (argc == 2) {
		worldserver.SetLauncherName("NONE");
		worldserver.SetLaunchedName(argv[1]);
		if(strncmp(argv[1], "dynamic_", 8) == 0) {
			//dynamic zone with a launcher name correlation
			zone_name = ".";
		} else {
			zone_name = argv[1];
			worldserver.SetLaunchedName(zone_name);
		}
	} else {
		zone_name = ".";
		worldserver.SetLaunchedName(".");
		worldserver.SetLauncherName("NONE");
	}

	_log(ZONE__INIT, "Loading server configuration..");
	if (!ZoneConfig::LoadConfig()) {
		_log(ZONE__INIT_ERR, "Loading server configuration failed.");
		return(1);
	}
	const ZoneConfig *Config=ZoneConfig::get();

	if(!load_log_settings(Config->LogSettingsFile.c_str()))
		_log(ZONE__INIT, "Warning: Unable to read %s", Config->LogSettingsFile.c_str());
	else
		_log(ZONE__INIT, "Log settings loaded from %s", Config->LogSettingsFile.c_str());
	
	worldserver.SetPassword(Config->SharedKey.c_str());

	_log(ZONE__INIT, "Connecting to MySQL...");
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		_log(ZONE__INIT_ERR, "Cannot continue without a database connection.");
		return(1);
	}
	dbasync = new DBAsync(&database);
	dbasync->AddFQ(&MTdbafq);
	guild_mgr.SetDatabase(&database);

	GuildBanks = NULL;

#ifdef _EQDEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	_log(ZONE__INIT, "CURRENT_VERSION: %s", CURRENT_VERSION);
	
	/*
	 * Setup nice signal handlers
	 */
	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(ZONE__INIT_ERR, "Could not set signal handler");
		return 0;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(ZONE__INIT_ERR, "Could not set signal handler");
		return 0;
	}
	#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)	{
		_log(ZONE__INIT_ERR, "Could not set signal handler");
		return 0;
	}
	#endif

	const char *log_ini_file = "./log.ini";
	if(!load_log_settings(log_ini_file))
		_log(ZONE__INIT, "Warning: Unable to read %s", log_ini_file);
	else
		_log(ZONE__INIT, "Log settings loaded from %s", log_ini_file);
	
	_log(ZONE__INIT, "Mapping Incoming Opcodes");
	MapOpcodes();
	_log(ZONE__INIT, "Loading Variables");
	database.LoadVariables();
	_log(ZONE__INIT, "Loading zone names");
	database.LoadZoneNames();
	_log(ZONE__INIT, "Loading items");
	if (!database.LoadItems()) {
		_log(ZONE__INIT_ERR, "Loading items FAILED!");
		_log(ZONE__INIT, "Failed.  But ignoring error and going on...");
	}

	_log(ZONE__INIT, "Loading npc faction lists");
	if (!database.LoadNPCFactionLists()) {
		_log(ZONE__INIT_ERR, "Loading npcs faction lists FAILED!");
		CheckEQEMuErrorAndPause();
		return 0;
	}
	_log(ZONE__INIT, "Loading loot tables");
	if (!database.LoadLoot()) {
		_log(ZONE__INIT_ERR, "Loading loot FAILED!");
		CheckEQEMuErrorAndPause();
		return 0;
    }
	_log(ZONE__INIT, "Loading skill caps");
	if (!database.LoadSkillCaps()) {
		_log(ZONE__INIT_ERR, "Loading skill caps FAILED!");
		CheckEQEMuErrorAndPause();
		return 0;
	}
	
    _log(ZONE__INIT, "Loading spells");
    EQEmu::MemoryMappedFile *mmf = NULL;
	LoadSpells(&mmf);

	_log(ZONE__INIT, "Loading guilds");
	guild_mgr.LoadGuilds();
	_log(ZONE__INIT, "Loading factions");
	database.LoadFactionData();
	_log(ZONE__INIT, "Loading titles");
	title_manager.LoadTitles();
	_log(ZONE__INIT, "Loading AA effects");
	database.LoadAAEffects();
	_log(ZONE__INIT, "Loading tributes");
	database.LoadTributes();
	_log(ZONE__INIT, "Loading corpse timers");
	database.GetDecayTimes(npcCorpseDecayTimes);
	_log(ZONE__INIT, "Loading commands");
	int retval=command_init();
	if(retval<0)
		_log(ZONE__INIT_ERR, "Command loading FAILED");
	else
		_log(ZONE__INIT, "%d commands loaded", retval);

	//rules:
	{
		char tmp[64];
		if (database.GetVariable("RuleSet", tmp, sizeof(tmp)-1)) {
			_log(ZONE__INIT, "Loading rule set '%s'", tmp);
			if(!RuleManager::Instance()->LoadRules(&database, tmp)) {
				_log(ZONE__INIT_ERR, "Failed to load ruleset '%s', falling back to defaults.", tmp);
			}
		} else {
			if(!RuleManager::Instance()->LoadRules(&database, "default")) {
				_log(ZONE__INIT, "No rule set configured, using default rules");
			} else {
				_log(ZONE__INIT, "Loaded default rule set 'default'", tmp);
			}
		}
	}

	if(RuleB(TaskSystem, EnableTaskSystem)) {
		_log(ZONE__INIT, "Loading Tasks");
		taskmanager = new TaskManager;
		taskmanager->LoadTasks();
	}

    parse = new QuestParserCollection();
    PerlXSParser *pxs = new PerlXSParser();
    Parser *ps = new Parser();
    parse->RegisterQuestInterface(pxs, "pl");
    //parse->RegisterQuestInterface(ps, "qst");


	//now we have our parser, load the quests
	_log(ZONE__INIT, "Loading quests");
	parse->ReloadQuests();
	

#ifdef CLIENT_LOGS
	LogFile->SetAllCallbacks(ClientLogs::EQEmuIO_buf);
	LogFile->SetAllCallbacks(ClientLogs::EQEmuIO_fmt);
	LogFile->SetAllCallbacks(ClientLogs::EQEmuIO_pva);
#endif
	if (!worldserver.Connect()) {
		_log(ZONE__INIT_ERR, "worldserver.Connect() FAILED!");
	}
	
	Timer InterserverTimer(INTERSERVER_TIMER); // does MySQL pings and auto-reconnect
#ifdef EQPROFILE
#ifdef PROFILE_DUMP_TIME
	Timer profile_dump_timer(PROFILE_DUMP_TIME*1000);
	profile_dump_timer.Start();
#endif
#endif
	if (!strlen(zone_name) || !strcmp(zone_name,".")) {
		_log(ZONE__INIT, "Entering sleep mode");
	} else if (!Zone::Bootup(database.GetZoneID(zone_name), 0, true)) { //todo: go above and fix this to allow cmd line instance
		_log(ZONE__INIT_ERR, "Zone bootup FAILED!");
		zone = 0;
	}
	
	//register all the patches we have avaliable with the stream identifier.
	EQStreamIdentifier stream_identifier;
	RegisterAllPatches(stream_identifier);
	
#ifndef WIN32
	_log(COMMON__THREADS, "Main thread running with thread id %d", pthread_self());
#endif
	
	Timer quest_timers(100);
	UpdateWindowTitle();
	bool worldwasconnected = worldserver.Connected();
	EQStream* eqss;
	EQStreamInterface *eqsi;
	Timer temp_timer(10);
	temp_timer.Start();
	while(RunLoops) {
		{	//profiler block to omit the sleep from times
		_ZP(net_main);
		
		//Advance the timer to our current point in time
		Timer::SetCurrentTime();
		
		//process stuff from world
		worldserver.Process();

		if (!eqsf.IsOpen() && Config->ZonePort!=0) {
			_log(ZONE__INIT, "Starting EQ Network server on port %d",Config->ZonePort);
			if (!eqsf.Open(Config->ZonePort)) {
				_log(ZONE__INIT_ERR, "Failed to open port %d",Config->ZonePort);
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
			_log(WORLD__CLIENT, "New connection from %s:%d", inet_ntoa(in),ntohs(eqss->GetRemotePort()));
			stream_identifier.AddStream(eqss);	//takes the stream
		}
		
		//give the stream identifier a chance to do its work....
		stream_identifier.Process();
		
		//check the stream identifier for any now-identified streams
		while((eqsi = stream_identifier.PopIdentified())) {
			//now that we know what patch they are running, start up their client object
			struct in_addr	in;
			in.s_addr = eqsi->GetRemoteIP();
			_log(WORLD__CLIENT, "New client from %s:%d", inet_ntoa(in), ntohs(eqsi->GetRemotePort()));
			Client* client = new Client(eqsi);
			entity_list.AddClient(client);
		}
		
		
		//check for timeouts in other threads
		timeout_manager.CheckTimeouts();
		
		if (worldserver.Connected()) {
			worldwasconnected = true;
		}
		else {
			if (worldwasconnected && ZoneLoaded)
				entity_list.ChannelMessageFromWorld(0, 0, 6, 0, 0, "WARNING: World server connection lost");
			worldwasconnected = false;
		}

		if (ZoneLoaded && temp_timer.Check()) {
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

				if (zone) {
					if(!zone->Process()) {
						Zone::Shutdown();
					}
				}

				if(quest_timers.Check())
					quest_manager.Process();

			}
		}
		DBAsyncWork* dbaw = 0;
		while ((dbaw = MTdbafq.Pop())) {
			DispatchFinishedDBAsync(dbaw);
		}
		if (InterserverTimer.Check()) {
			InterserverTimer.Start();
			database.ping();
			AsyncLoadVariables(dbasync, &database);
			entity_list.UpdateWho();
			if (worldserver.TryReconnect() && (!worldserver.Connected()))
				worldserver.AsyncConnect();
		}

#if defined(_EQDEBUG) && defined(DEBUG_PC)
		QueryPerformanceCounter(&tmp3);
		mainloop_time += tmp3.QuadPart - tmp2.QuadPart;
		if (!--tmp0) {
			tmp0 = 200;
			printf("Elapsed Tics  : %9.0f (%1.4f sec)\n", (double)mainloop_time, ((double)mainloop_time/tmp.QuadPart));
			printf("NPCAI Tics    : %9.0f (%1.2f%%)\n", (double)npcai_time, ((double)npcai_time/mainloop_time)*100);
			printf("FindSpell Tics: %9.0f (%1.2f%%)\n", (double)findspell_time, ((double)findspell_time/mainloop_time)*100);
			printf("AtkAllowd Tics: %9.0f (%1.2f%%)\n", (double)IsAttackAllowed_time, ((double)IsAttackAllowed_time/mainloop_time)*100);
			printf("ClientPro Tics: %9.0f (%1.2f%%)\n", (double)clientprocess_time, ((double)clientprocess_time/mainloop_time)*100);
			printf("ClientAtk Tics: %9.0f (%1.2f%%)\n", (double)clientattack_time, ((double)clientattack_time/mainloop_time)*100);
			mainloop_time = 0;
			npcai_time = 0;
			findspell_time = 0;
			IsAttackAllowed_time = 0;
			clientprocess_time = 0;
			clientattack_time = 0;
		}
#endif
#ifdef EQPROFILE
#ifdef PROFILE_DUMP_TIME
		if(profile_dump_timer.Check()) {
			DumpZoneProfile();
		}
#endif
#endif
		}	//end extra profiler block
		Sleep(ZoneTimerResolution);
	}

	entity_list.Clear();

	safe_delete(parse);
	safe_delete(pxs);
	safe_delete(ps);
	safe_delete(mmf);
	
	if (zone != 0)
		Zone::Shutdown(true);
	//Fix for Linux world server problem.
	eqsf.Close();
	worldserver.Disconnect();
	dbasync->CommitWrites();
	dbasync->StopThread();
	safe_delete(taskmanager);
	command_deinit();
	
	CheckEQEMuErrorAndPause();
	_log(ZONE__INIT, "Proper zone shutdown complete.");
	return 0;
}

void CatchSignal(int sig_num) {
#ifdef _WINDOWS
	_log(ZONE__INIT, "Recieved signal: %i", sig_num);
#endif
	RunLoops = false;
}

void Shutdown()
{
	Zone::Shutdown(true);
	RunLoops = false;
	worldserver.Disconnect();
	//	safe_delete(worldserver);
	_log(ZONE__INIT, "Shutting down...");
}

uint32 NetConnection::GetIP()
{
	char     name[255+1];
	size_t   len = 0;
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

void NetConnection::SaveInfo(char* address, uint32 port, char* waddress, char* filename) {

	ZoneAddress = new char[strlen(address)+1];
	strcpy(ZoneAddress, address);
	ZonePort = port;
	WorldAddress = new char[strlen(waddress)+1];
	strcpy(WorldAddress, waddress);
	strn0cpy(ZoneFileName, filename, sizeof(ZoneFileName));
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
	ZonePort = 0;
	ZoneAddress = 0;
	WorldAddress = 0;
	group_timer.Disable();
	raid_timer.Disable();
	corpse_timer.Disable();
	door_timer.Disable();
	object_timer.Disable();
	trap_timer.Disable();
}

NetConnection::~NetConnection() {
	if (ZoneAddress != 0)
		safe_delete_array(ZoneAddress);
	if (WorldAddress != 0)
		safe_delete_array(WorldAddress);
}

void LoadSpells(EQEmu::MemoryMappedFile **mmf) {
    int records = database.GetMaxSpellID() + 1;

    try {
        EQEmu::IPCMutex mutex("spells");
        mutex.Lock();
        *mmf = new EQEmu::MemoryMappedFile("shared/spells");
        uint32 size = (*mmf)->Size();
        if(size != (records * sizeof(SPDat_Spell_Struct))) {
            EQ_EXCEPT("Zone", "Unable to load spells: (*mmf)->Size() != records * sizeof(SPDat_Spell_Struct)");
        }

        spells = reinterpret_cast<SPDat_Spell_Struct*>((*mmf)->Get());
        mutex.Unlock();
    } catch(std::exception &ex) {
        LogFile->write(EQEMuLog::Error, "Error loading spells: %s", ex.what());
        return;
    }

    SPDAT_RECORDS = records;
}


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
				snprintf(tmp, sizeof(tmp), "%i: %s, %i clients", ZoneConfig::get()->ZonePort, zone->GetShortName(), numclients);
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

