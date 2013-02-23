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
#include "features.h"
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

bool spells_loaded = false;
volatile bool RunLoops = true;
extern volatile bool ZoneLoaded;
#ifdef SHAREMEM
       #include "../common/EMuShareMem.h"
       extern LoadEMuShareMemDLL EMuShareMemDLL;
    #ifndef WIN32
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
#endif




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

#include "masterentity.h"
#include "worldserver.h"
#include "net.h"
#include "spdat.h"
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

TimeoutManager          timeout_manager;
NetConnection		net;
EntityList			entity_list;
WorldServer			worldserver;
uint32				numclients = 0;
#ifdef CATCH_CRASH
uint8			error = 0;
#endif
char errorname[32];
uint16 adverrornum = 0;
extern Zone* zone;
EQStreamFactory eqsf(ZoneStream);
npcDecayTimes_Struct npcCorpseDecayTimes[100];
TitleManager title_manager;
DBAsyncFinishedQueue MTdbafq;
DBAsync *dbasync = NULL;
RuleManager *rules = new RuleManager();
TaskManager *taskmanager = 0;
QuestParserCollection *parse = 0;

bool zoneprocess;

#if defined(NEW_LoadSPDat) || defined(DB_LoadSPDat)
	// For NewLoadSPDat function
	const SPDat_Spell_Struct* spells; 
	SPDat_Spell_Struct* spells_delete; 
	int32 GetMaxSpellID();


	void LoadSPDat();
	bool FileLoadSPDat(SPDat_Spell_Struct* sp, int32 iMaxSpellID);
	int32 SPDAT_RECORDS = -1;
#else
	#define SPDat_Location	"spdat.eff"
	SPDat_Spell_Struct spells[SPDAT_RECORDS];
	void LoadSPDat(SPDat_Spell_Struct** SpellsPointer = 0);


#endif

#ifdef _WINDOWS
#include <process.h>
#else
#include <pthread.h>
#include "../common/unix.h"
#endif

void Shutdown();
extern void MapOpcodes();

//bool ZoneBootup(uint32 iZoneID, bool iStaticZone = false);
//char *strsep(char **stringp, const char *delim);

#ifdef ADDONCMD
#include "addoncmd.h"
extern  AddonCmd addonCmd;
#endif

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
//	_crtBreakAlloc = 2025;
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
	database.LoadItemStatus();
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
	LoadSPDat();

	// New Load function.  keeping it commented till I figure out why its not working correctly in linux. Trump.
	// NewLoadSPDat();
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
			if(!rules->LoadRules(&database, tmp)) {
				_log(ZONE__INIT_ERR, "Failed to load ruleset '%s', falling back to defaults.", tmp);
			}
		} else {
			if(!rules->LoadRules(&database, "default")) {
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
	

#ifdef ADDONCMD	
	_log(ZONE__INIT, "Looding addon commands from dll");
	if ( !addonCmd.openLib() ) {
		_log(ZONE__INIT_ERR, "Loading addons failed =(");
	}
#endif
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
#ifdef CATCH_CRASH
		try{
#endif
			worldserver.Process();
#ifdef CATCH_CRASH
		}
		catch(...){
			error = 1;
			worldserver.Disconnect();
			worldwasconnected = false;
		}
#endif
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
				uint8 error2 = 4;
#ifdef CATCH_CRASH
				try{
#endif
					if(net.group_timer.Enabled() && net.group_timer.Check())
						entity_list.GroupProcess();
					error2 = 99;
					if(net.door_timer.Enabled() && net.door_timer.Check())
						entity_list.DoorProcess();
					error2 = 98;
					if(net.object_timer.Enabled() && net.object_timer.Check())
						entity_list.ObjectProcess();
					error2 = 97;
					if(net.corpse_timer.Enabled() && net.corpse_timer.Check())
						entity_list.CorpseProcess();
					if(net.trap_timer.Enabled() && net.trap_timer.Check())
						entity_list.TrapProcess();
					if(net.raid_timer.Enabled() && net.raid_timer.Check())
						entity_list.RaidProcess();
					error2 = 98;
					error2 = 96;
					entity_list.Process();
					error2 = 95;
#ifdef CATCH_CRASH
					try{
					entity_list.MobProcess();
					}
					catch(...){
						printf("Catching Mob Crash...\n");
					}
#else
					entity_list.MobProcess();
#endif
					error2 = 94;
					entity_list.BeaconProcess();
#ifdef CATCH_CRASH
				}
				catch(...){
					error=error2;
				}
				try{
#endif
					if (zone) {
						zoneprocess= zone->Process();
						if (!zoneprocess) {
							Zone::Shutdown();
						}
					}
#ifdef CATCH_CRASH
				}
				catch(...){
					error = 2;
				}
				try{
#endif
					if(quest_timers.Check())
						quest_manager.Process();
#ifdef CATCH_CRASH
				}
				catch(...){
					error = 77777;
				}
#endif
			}
		}
		DBAsyncWork* dbaw = 0;
		while ((dbaw = MTdbafq.Pop())) {
			DispatchFinishedDBAsync(dbaw);
		}
		if (InterserverTimer.Check()
#ifdef CATCH_CRASH
			&& !error
#endif
			) {
#ifdef CATCH_CRASH
			try{
#endif
				InterserverTimer.Start();
				database.ping();
				AsyncLoadVariables(dbasync, &database);
//				NPC::GetAILevel(true);
				entity_list.UpdateWho();
				if (worldserver.TryReconnect() && (!worldserver.Connected()))
					worldserver.AsyncConnect();
#ifdef CATCH_CRASH
			}
			catch(...)
			{
				error = 16;
				RunLoops = false;
			}
#endif
		}
#ifdef CATCH_CRASH
		if (error){
			RunLoops = false;
		}
#endif
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

    safe_delete(parse);
    safe_delete(pxs);
    safe_delete(ps);
	
#ifdef CATCH_CRASH
	if (error)
		FilePrint("eqemudebug.log",true,true,"Zone %i crashed. Errorcode: %i/%i. Current zone loaded:%s. Current clients:%i. Caused by: %s",Config->ZonePort, error,adverrornum, zone->GetShortName(), numclients,errorname);
	try{
		entity_list.Message(0, 15, "ZONEWIDE_MESSAGE: This zone caused a fatal error and will shut down now. Your character will be restored to the last saved status. We are sorry for any inconvenience!");
	}
	catch(...){}
	if (error){
#ifdef _WINDOWS		
		ExitProcess(error);
#else	
		entity_list.Clear();
		safe_delete(zone);
#endif
	}
#endif

	entity_list.Clear();
	if (zone != 0
#ifdef CATCH_CRASH
		& !error
#endif
		)
		Zone::Shutdown(true);
	//Fix for Linux world server problem.
	eqsf.Close();
	worldserver.Disconnect();
	dbasync->CommitWrites();
	dbasync->StopThread();
#if defined(NEW_LoadSPDat) || defined(DB_LoadSPDat)
	safe_delete(spells_delete);
#endif
	safe_delete(taskmanager);
	command_deinit();
	
	CheckEQEMuErrorAndPause();
	_log(ZONE__INIT, "Proper zone shutdown complete.");
	return 0;
}

void CatchSignal(int sig_num) {
#ifdef _WINDOWS
	_log(ZONE__INIT, "Recieved signal: %i", sig_num);
#else
	_log(ZONE__INIT, "Recieved signal: %i in thread %d", sig_num, pthread_self());
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
bool chrcmpI(const char* a, const char* b) {
#if EQDEBUG >= 11
    _log(EQEMuLog::Debug, "crhcmpl() a:%i b:%i", (int*) a, (int*) b);
#endif
	if(((int)* a)==((int)* b))
		return false;
	else
		return true;
}

#if defined(NEW_LoadSPDat) || defined(DB_LoadSPDat)
int32 GetMaxSpellID() {
#ifdef NEW_LoadSPDat
	int tempid=0, oldid=-1;
	char spell_line_start[2048];
	char* spell_line = spell_line_start;
	char token[64]="";
	char seps[] = "^";
	const char *spells_file=ZoneConfig::get()->SpellsFile.c_str();
	//ifstream in(spells_file);
	
	/*struct stat s;
	if(stat(spells_file, &s) != 0) {
		_log(SPELLS__LOAD_ERR, "File '%s' not found (stat failed), spell loading FAILED!", spells_file);
		return(-1);
	}
	*/

	FILE *sf = fopen(spells_file, "r");
	
	if(sf == NULL) {
		_log(SPELLS__LOAD_ERR, "File '%s' not found, spell loading FAILED!", spells_file);
		return -1;
	}
	
	fgets(spell_line, sizeof(spell_line_start), sf);
	while(!feof(sf)) {
		strcpy(token,strtok(spell_line, seps));
		if(token!=NULL);
		{
			tempid = atoi(token);
			if(tempid>oldid)
				oldid = tempid;
			else
				break;
		}
		fgets(spell_line, sizeof(spell_line_start), sf);
	}
	
	fclose(sf);
	
	/*ifstream in(spells_file);
	
	if(!in) {
		_log(SPELLS__LOAD_ERR, "File '%s' not found, spell loading FAILED!", spells_file);
		return -1;
	}
	
	in.getline(spell_line, sizeof(spell_line_start));
	while(strlen(spell_line)>1)
	{
		strcpy(token,strtok(spell_line, seps));
		if(token!=NULL);
		{
			tempid = atoi(token);
			if(tempid>oldid)
				oldid = tempid;
			else
				break;
		}
		in.getline(spell_line, sizeof(spell_line_start));
	}*/
	
		
	return oldid;

#else	// defined(DB_LoadSPDat)
	//load from DB
	
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	int32 ret = 0;
	if (database.RunQuery(query, MakeAnyLenString(&query, 
		"SELECT MAX(id) FROM spells_new"), 
		errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		ret = atoi(row[0]);
		mysql_free_result(result);
	} else {
		_log(SPELLS__LOAD_ERR, "Error in GetMaxSpellID query '%s' %s", query, errbuf);
		safe_delete_array(query);
		ret = -1;
	}
	return ret;
#endif
}
#ifdef SHAREMEM
extern "C" bool extFileLoadSPDat(void* sp, int32 iMaxSpellID) { return FileLoadSPDat((SPDat_Spell_Struct*) sp, iMaxSpellID); }
#endif

void LoadSPDat() {
	if (SPDAT_RECORDS != -1) {
		_log(SPELLS__LOAD, "LoadSPDat() SPDAT_RECORDS:%i != -1, spells already loaded?", SPDAT_RECORDS);
	}
	int32 MaxSpellID = GetMaxSpellID();
	if (MaxSpellID == -1) {
#ifdef NEW_LoadSPDat
		_log(SPELLS__LOAD, "LoadSPDat() MaxSpellID == -1, %s missing?", ZoneConfig::get()->SpellsFile.c_str());
#else	// defined(DB_LoadSPDat)
		_log(SPELLS__LOAD, "LoadSPDat() MaxSpellID == -1, error in GetMaxSpellID()?");
#endif
		return;
	}
#ifdef SHAREMEM
	if (!EMuShareMemDLL.Load())
		return;
	SPDAT_RECORDS = MaxSpellID+1;
	if (EMuShareMemDLL.Spells.DLLLoadSPDat((const CALLBACK_FileLoadSPDat)&extFileLoadSPDat, (const void**) &spells, &SPDAT_RECORDS, sizeof(SPDat_Spell_Struct))) {
		spells_loaded = true;
	}
	else {
		SPDAT_RECORDS = 0;
		_log(SPELLS__LOAD_ERR, "LoadSPDat() EMuShareMemDLL.Spells.DLLLoadSPDat() returned false");
		return;
	}
#else
	spells_delete = new SPDat_Spell_Struct[MaxSpellID+1];
	if (FileLoadSPDat(spells_delete, MaxSpellID)) {
		spells = spells_delete;
		SPDAT_RECORDS = MaxSpellID+1;
		spells_loaded = true;
	}
	else {
		safe_delete(spells_delete);
		_log(SPELLS__LOAD_ERR, "LoadSPDat() FileLoadSPDat() returned false");
		return;
	}
#endif
}

bool FileLoadSPDat(SPDat_Spell_Struct* sp, int32 iMaxSpellID) {
#ifdef NEW_LoadSPDat
	int tempid=0;
	uint16 counter=0;
	char spell_line[2048];
	const char *spells_file=ZoneConfig::get()->SpellsFile.c_str();
	_log(SPELLS__LOAD,"FileLoadSPDat() Loading spells from %s", spells_file);
	
	FILE *sf = fopen(spells_file, "r");
	
	if(sf == NULL) {
		_log(SPELLS__LOAD_ERR, "File '%s' not found, spell loading FAILED!", spells_file);
		return false;
	}
/*	ifstream in(spells_file);
	if(!in) {
		_log(SPELLS__LOAD_ERR, "File '%s' not found, spell loading FAILED!", spells_file);
		return false;
	}
	*/
	if (iMaxSpellID < 0) {
		_log(SPELLS__LOAD_ERR,"FileLoadSPDat() Loading spells FAILED! iMaxSpellID:%i < 0", iMaxSpellID);
		return false;
	}
/*

This is hanging on freebsd for me, not sure why...

//#if EQDEBUG >= 1
	else {
		_log(SPELLS__LOAD,"FileLoadSPDat() Highest spell ID:%i", iMaxSpellID);
	}
//#endif
*/
/*	in.close();
	in.open(spells_file);
	if(!in) {
		_log(SPELLS__LOAD_ERR, "File '%s' not found, spell loading FAILED!", spells_file);
		return false;
	}
	while(!in.eof()) {
		in.getline(spell_line, sizeof(spell_line));
		Seperator sep(spell_line, '^', 200, 100, false, 0, 0, false);
		
		if(spell_line[0]=='\0')
			break;
		
		tempid = atoi(sep.arg[0]);
		if (tempid > iMaxSpellID) {
			_log(SPELLS__LOAD_ERR, "FATAL FileLoadSPDat() tempid:%i >= iMaxSpellID:%i", tempid, iMaxSpellID);
			return false;
		}
		*/
		
	while(!feof(sf)) {
		if(fgets(spell_line, sizeof(spell_line), sf) == NULL)
			break;
		if(spell_line[0]=='\0')
			continue;
		
		Seperator sep(spell_line, '^', 220, 100, false, 0, 0, false);
		
		
		tempid = atoi(sep.arg[0]);
		if (tempid > iMaxSpellID) {
			_log(SPELLS__LOAD_ERR, "FATAL FileLoadSPDat() tempid:%i >= iMaxSpellID:%i", tempid, iMaxSpellID);
			return false;
		}
		
		counter++;
		strcpy(sp[tempid].name, sep.arg[1]);
		strcpy(sp[tempid].player_1, sep.arg[2]);
		strcpy(sp[tempid].teleport_zone, sep.arg[3]);
		strcpy(sp[tempid].you_cast,  sep.arg[4]);
		strcpy(sp[tempid].other_casts, sep.arg[5]);
		strcpy(sp[tempid].cast_on_you, sep.arg[6]);
		strcpy(sp[tempid].cast_on_other, sep.arg[7]);
		strcpy(sp[tempid].spell_fades, sep.arg[8]);

		sp[tempid].range=atof(sep.arg[9]);
		sp[tempid].aoerange=atof(sep.arg[10]);
		sp[tempid].pushback=atof(sep.arg[11]);
		sp[tempid].pushup=atof(sep.arg[12]);
		sp[tempid].cast_time=atoi(sep.arg[13]);
		sp[tempid].recovery_time=atoi(sep.arg[14]);
		sp[tempid].recast_time=atoi(sep.arg[15]);
		sp[tempid].buffdurationformula=atoi(sep.arg[16]);
		sp[tempid].buffduration=atoi(sep.arg[17]);
		sp[tempid].AEDuration=atoi(sep.arg[18]);
		sp[tempid].mana=atoi(sep.arg[19]);
		
		int y=0;
		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].base[y]=atoi(sep.arg[20+y]);
		for(y=0; y < EFFECT_COUNT; y++)
			sp[tempid].base2[y]=atoi(sep.arg[32+y]);
		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].max[y]=atoi(sep.arg[44+y]);
		
		sp[tempid].icon=atoi(sep.arg[56]);
		sp[tempid].memicon=atoi(sep.arg[57]);
		
		for(y=0; y< 4;y++)
			sp[tempid].components[y]=atoi(sep.arg[58+y]);
		
		for(y=0; y< 4;y++)
			sp[tempid].component_counts[y]=atoi(sep.arg[62+y]);
		
		for(y=0; y< 4;y++)
			sp[tempid].NoexpendReagent[y]=atoi(sep.arg[66+y]);
		
		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].formula[y]=atoi(sep.arg[70+y]);
		
		sp[tempid].LightType=atoi(sep.arg[82]);
		sp[tempid].goodEffect=atoi(sep.arg[83]);
		sp[tempid].Activated=atoi(sep.arg[84]);
		sp[tempid].resisttype=atoi(sep.arg[85]);
		
		for(y=0; y< 12;y++)
			sp[tempid].effectid[y]=atoi(sep.arg[86+y]);
		
		sp[tempid].targettype = (SpellTargetType) atoi(sep.arg[98]);
		sp[tempid].basediff=atoi(sep.arg[99]);
		int tmp_skill = atoi(sep.arg[100]);;
		if(tmp_skill < 0 || tmp_skill > HIGHEST_SKILL)
			sp[tempid].skill = BEGGING;	/* not much better we can do. */
		else
			sp[tempid].skill = (SkillType) tmp_skill;
		sp[tempid].zonetype=atoi(sep.arg[101]);
		sp[tempid].EnvironmentType=atoi(sep.arg[102]);
		sp[tempid].TimeOfDay=atoi(sep.arg[103]);
		
		for(y=0; y < PLAYER_CLASS_COUNT;y++)
			sp[tempid].classes[y]=atoi(sep.arg[104+y]);
		
		sp[tempid].CastingAnim=atoi(sep.arg[120]);
		sp[tempid].TargetAnim=atoi(sep.arg[121]);
		sp[tempid].TravelType=atoi(sep.arg[122]);
		sp[tempid].SpellAffectIndex=atoi(sep.arg[123]);
		sp[tempid].disallow_sit = atoi(sep.arg[124]);
		sp[tempid].spacing125=atoi(sep.arg[125]);

		for (y = 0; y < 16; y++)
			sp[tempid].deities[y]=atoi(sep.arg[126+y]);

		for (y = 0; y < 2; y++)
			sp[tempid].spacing142[y]=atoi(sep.arg[142+y]);

		sp[tempid].new_icon=atoi(sep.arg[144]);
		sp[tempid].spellanim=atoi(sep.arg[145]);
		sp[tempid].uninterruptable=atoi(sep.arg[146]);
		sp[tempid].ResistDiff=atoi(sep.arg[147]);
		sp[tempid].dot_stacking_exempt=atoi(sep.arg[148]);
		sp[tempid].deletable=atoi(sep.arg[149]);
		sp[tempid].RecourseLink = atoi(sep.arg[150]);

		for(y = 0; y < 3;y++)
			sp[tempid].spacing151[y]=atoi(sep.arg[151+y]);

		sp[tempid].short_buff_box = atoi(sep.arg[154]);
		sp[tempid].descnum = atoi(sep.arg[155]);
		sp[tempid].typedescnum = atoi(sep.arg[156]);
		sp[tempid].effectdescnum = atoi(sep.arg[157]);
		
		for(y = 0; y < 4;y++)
			sp[tempid].spacing158[y]=atoi(sep.arg[158+y]);

		sp[tempid].bonushate=atoi(sep.arg[162]);

		for(y = 0; y < 3;y++)
			sp[tempid].spacing163[y]=atoi(sep.arg[163+y]);

		sp[tempid].EndurCost=atoi(sep.arg[166]);
		sp[tempid].EndurTimerIndex=atoi(sep.arg[167]);
        sp[tempid].IsDisciplineBuff=atoi(sep.arg[168]);

		for(y = 0; y < 4;y++)
			sp[tempid].spacing169[y]=atoi(sep.arg[169+y]);

		sp[tempid].HateAdded=atoi(sep.arg[173]);
		sp[tempid].EndurUpkeep=atoi(sep.arg[174]);

		sp[tempid].spacing175=atoi(sep.arg[175]);
		sp[tempid].numhits = atoi(sep.arg[176]);

		sp[tempid].pvpresistbase=atoi(sep.arg[177]);
		sp[tempid].pvpresistcalc=atoi(sep.arg[178]);
		sp[tempid].pvpresistcap=atoi(sep.arg[179]);
		sp[tempid].spell_category=atoi(sep.arg[180]);

		for(y = 0; y < 4;y++)
			sp[tempid].spacing181[y]=atoi(sep.arg[181+y]);

		sp[tempid].can_mgb=atoi(sep.arg[185]);
		sp[tempid].dispel_flag = atoi(sep.arg[186]);
		sp[tempid].MinResist = atoi(sep.arg[189]);
		sp[tempid].MaxResist = atoi(sep.arg[190]);
		sp[tempid].viral_targets = atoi(sep.arg[191]);
		sp[tempid].viral_timer = atoi(sep.arg[192]);
		sp[tempid].NimbusEffect = atoi(sep.arg[193]);
		sp[tempid].directional_start = (float)atoi(sep.arg[194]);
		sp[tempid].directional_end = (float)atoi(sep.arg[195]);
		sp[tempid].spellgroup=atoi(row[207]);
		sp[tempid].field209=atoi(row[209]);
        sp[tempid].CastRestriction = atoi(sep.arg[211]);
		sp[tempid].AllowRest = atoi(sep.arg[212]);
	
		// May crash zone
		/*
		sp[tempid].nodispell=atoi(row[186]);
		sp[tempid].npc_category=atoi(row[187]);
		sp[tempid].npc_usefulness=atoi(row[188]);

		for (y = 0; y < 18; y++)
			sp[tempid].spacing189[y]=atoi(row[189+y]);

		sp[tempid].spellgroup=atoi(row[207]);

		for (y = 0; y < 18; y++)
			sp[tempid].spacing208[y]=atoi(row[208+y]);
		*/
		sp[tempid].DamageShieldType = 0;

	} 
	_log(SPELLS__LOAD, "FileLoadSPDat() spells loaded: %i", counter);
	//in.close();
	fclose(sf);
	// Now fill in the DamageShieldType from the damageshieldtypes table, if it exists.
	//
	database.DBLoadDamageShieldTypes(sp, iMaxSpellID);

	return true;

#else	// defined(DB_LoadSPDat)
	//load from db

	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

	_log(SPELLS__LOAD,"FileLoadSPDat() Loading spells from database");

	if (iMaxSpellID < 0) {
		_log(SPELLS__LOAD_ERR,"FileLoadSPDat() Loading spells FAILED! iMaxSpellID:%i < 0", iMaxSpellID);
		return false;
	} else 
		_log(SPELLS__LOAD,"FileLoadSPDat() Highest spell ID:%i", iMaxSpellID);
	
	if (database.RunQuery(query, MakeAnyLenString(&query, 
		"SELECT * FROM spells_new ORDER BY id ASC"), 
		errbuf, &result)) {
		safe_delete_array(query);

		int tempid = 0;
		uint16 counter = 0;

		while (row = mysql_fetch_row(result)) {

			tempid = atoi(row[0]);
			if (tempid > iMaxSpellID) {	// Is this really needed?
				_log(SPELLS__LOAD_ERR, "FATAL FileLoadSPDat() tempid:%i >= iMaxSpellID:%i", tempid, iMaxSpellID);
				return false;
			}

			counter++;
			// String fields
			strn0cpy(sp[tempid].name, row[1], sizeof(sp[tempid].name));
			strn0cpy(sp[tempid].player_1, row[2], sizeof(sp[tempid].player_1));
			strn0cpy(sp[tempid].teleport_zone, row[3], sizeof(sp[tempid].teleport_zone));
			strn0cpy(sp[tempid].you_cast,  row[4], sizeof(sp[tempid].you_cast));
			strn0cpy(sp[tempid].other_casts, row[5], sizeof(sp[tempid].other_casts));
			strn0cpy(sp[tempid].cast_on_you, row[6], sizeof(sp[tempid].cast_on_you));
			strn0cpy(sp[tempid].cast_on_other, row[7], sizeof(sp[tempid].cast_on_other));
			strn0cpy(sp[tempid].spell_fades, row[8], sizeof(sp[tempid].spell_fades));

			// Numeric fields (everything else)
			sp[tempid].range=atof(row[9]);
			sp[tempid].aoerange=atof(row[10]);
			sp[tempid].pushback=atof(row[11]);
			sp[tempid].pushup=atof(row[12]);
			sp[tempid].cast_time=atoi(row[13]);
			sp[tempid].recovery_time=atoi(row[14]);
			sp[tempid].recast_time=atoi(row[15]);
			sp[tempid].buffdurationformula=atoi(row[16]);
			sp[tempid].buffduration=atoi(row[17]);
			sp[tempid].AEDuration=atoi(row[18]);
			sp[tempid].mana=atoi(row[19]);
			
			int y=0;
			for(y=0; y< EFFECT_COUNT;y++)
				sp[tempid].base[y]=atoi(row[20+y]);	// effect_base_value
			for(y=0; y < EFFECT_COUNT; y++)
				sp[tempid].base2[y]=atoi(row[32+y]);	// effect_limit_value
			for(y=0; y< EFFECT_COUNT;y++)
				sp[tempid].max[y]=atoi(row[44+y]);
			
			sp[tempid].icon=atoi(row[56]);
			sp[tempid].memicon=atoi(row[57]);
			
			for(y=0; y< 4;y++)
				sp[tempid].components[y]=atoi(row[58+y]);
			
			for(y=0; y< 4;y++)
				sp[tempid].component_counts[y]=atoi(row[62+y]);
			
			for(y=0; y< 4;y++)
				sp[tempid].NoexpendReagent[y]=atoi(row[66+y]);
			
			for(y=0; y< EFFECT_COUNT;y++)
				sp[tempid].formula[y]=atoi(row[70+y]);
			
			sp[tempid].LightType=atoi(row[82]);
			sp[tempid].goodEffect=atoi(row[83]);
			sp[tempid].Activated=atoi(row[84]);
			sp[tempid].resisttype=atoi(row[85]);
			
			for(y=0; y< EFFECT_COUNT;y++)
				sp[tempid].effectid[y]=atoi(row[86+y]);
			
			sp[tempid].targettype = (SpellTargetType) atoi(row[98]);
			sp[tempid].basediff=atoi(row[99]);
			int tmp_skill = atoi(row[100]);;
			if(tmp_skill < 0 || tmp_skill > HIGHEST_SKILL)
				sp[tempid].skill = BEGGING;	/* not much better we can do. */
			else
				sp[tempid].skill = (SkillType) tmp_skill;
			sp[tempid].zonetype=atoi(row[101]);
			sp[tempid].EnvironmentType=atoi(row[102]);
			sp[tempid].TimeOfDay=atoi(row[103]);
			
			for(y=0; y < PLAYER_CLASS_COUNT;y++)
				sp[tempid].classes[y]=atoi(row[104+y]);
			
			sp[tempid].CastingAnim=atoi(row[120]);
			sp[tempid].TargetAnim=atoi(row[121]);
			sp[tempid].TravelType=atoi(row[122]);
			sp[tempid].SpellAffectIndex=atoi(row[123]);
			sp[tempid].disallow_sit=atoi(row[124]);
			sp[tempid].spacing125=atoi(row[125]);

			for (y = 0; y < 16; y++)
				sp[tempid].deities[y]=atoi(row[126+y]);

			for (y = 0; y < 2; y++)
				sp[tempid].spacing142[y]=atoi(row[142+y]);

			sp[tempid].new_icon=atoi(row[144]);
			sp[tempid].spellanim=atoi(row[145]);
			sp[tempid].uninterruptable=atoi(row[146]);
			sp[tempid].ResistDiff=atoi(row[147]);
			sp[tempid].dot_stacking_exempt=atoi(row[148]);
			sp[tempid].deletable=atoi(row[149]);
			sp[tempid].RecourseLink = atoi(row[150]);

			for(y = 0; y < 3;y++)
				sp[tempid].spacing151[y]=atoi(row[151+y]);

			sp[tempid].short_buff_box = atoi(row[154]);
			sp[tempid].descnum = atoi(row[155]);
			sp[tempid].typedescnum = atoi(row[156]);
			sp[tempid].effectdescnum = atoi(row[157]);
			
			for(y = 0; y < 4;y++)
				sp[tempid].spacing158[y]=atoi(row[158+y]);

			sp[tempid].bonushate=atoi(row[162]);

			for(y = 0; y < 3;y++)
				sp[tempid].spacing163[y]=atoi(row[163+y]);

			sp[tempid].EndurCost=atoi(row[166]);
			sp[tempid].EndurTimerIndex=atoi(row[167]);
            sp[tempid].IsDisciplineBuff=atoi(row[168]);

			for(y = 0; y < 4; y++)
				sp[tempid].spacing169[y]=atoi(row[169+y]);

			sp[tempid].HateAdded=atoi(row[173]);
			sp[tempid].EndurUpkeep=atoi(row[174]);

			sp[tempid].spacing175=atoi(row[175]);
			sp[tempid].numhits = atoi(row[176]);

			sp[tempid].pvpresistbase=atoi(row[177]);
			sp[tempid].pvpresistcalc=atoi(row[178]);
			sp[tempid].pvpresistcap=atoi(row[179]);
			sp[tempid].spell_category=atoi(row[180]);

			for(y = 0; y < 4;y++)
				sp[tempid].spacing181[y]=atoi(row[181+y]);

			sp[tempid].can_mgb=atoi(row[185]);
			sp[tempid].dispel_flag = atoi(row[186]);
			sp[tempid].MinResist = atoi(row[189]);
			sp[tempid].MaxResist = atoi(row[190]);
			sp[tempid].viral_targets = atoi(row[191]);
			sp[tempid].viral_timer = atoi(row[192]);
			sp[tempid].NimbusEffect = atoi(row[193]);
			sp[tempid].directional_start = (float)atoi(row[194]);
			sp[tempid].directional_end = (float)atoi(row[195]);
			sp[tempid].spellgroup=atoi(row[207]);
			sp[tempid].field209=atoi(row[209]);
            sp[tempid].CastRestriction = atoi(row[211]);
			sp[tempid].AllowRest = atoi(row[212]);

			// May crash zone
/*
			sp[tempid].nodispell=atoi(row[186]);
			sp[tempid].npc_category=atoi(row[187]);
			sp[tempid].npc_usefulness=atoi(row[188]);

			for (y = 0; y < 18; y++)
				sp[tempid].spacing189[y]=atoi(row[189+y]);

			sp[tempid].spellgroup=atoi(row[207]);

			for (y = 0; y < 18; y++)
				sp[tempid].spacing208[y]=atoi(row[208+y]);
*/
			sp[tempid].DamageShieldType = 0;

		}
		mysql_free_result(result);
		_log(SPELLS__LOAD, "FileLoadSPDat() spells loaded: %i", counter);
		// Now fill in the DamageShieldType from the damageshieldtypes table, if it exists.
		//
		database.DBLoadDamageShieldTypes(sp, iMaxSpellID);
		
		return true;
	} else {
		_log(SPELLS__LOAD_ERR, "Error in FileLoadSPDat query '%s' %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
#endif

}

#endif	//from just above GetMaxSpellID(): #if defined(NEW_LoadSPDat) || defined(DB_LoadSPDat)

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

/*bool ZoneBootup(uint32 iZoneID, bool iStaticZone) {
	const char* zonename = database.GetZoneName(iStaticZone);
	if (iZoneID == 0 || zonename == 0)
		return false;
	if (zone != 0 || ZoneLoaded) {
		cerr << "Error: Zone::Bootup call when zone already booted!" << endl;
		worldserver.SetZone(0);
		return false;
	}
	numclients = 0;
	zone = new Zone(iZoneID, zonename, net.GetZoneAddress(), net.GetZonePort());
	if (!zone->Init(iStaticZone)) {
		safe_delete(zone);
		cerr << "Zone->Init failed" << endl;
		worldserver.SetZone(0);
		return false;
	}
	if (!eqns.Open(net.GetZonePort())) {
		safe_delete(zone);
		cerr << "NetConnection::Init failed" << endl;
		worldserver.SetZone(0);
		return false;
	}
	if (!zone->LoadZoneCFG(zone->GetShortName(), true)) // try loading the zone name...
		zone->LoadZoneCFG(zone->GetFileName()); // if that fails, try the file name, then load defaults
	
	//petition_list.ClearPetitions();
	//petition_list.ReadDatabase();
	ZoneLoaded = true;
	worldserver.SetZone(iZoneID);
	zone->GetTimeSync();
	cout << "-----------" << endl << "Zone server '" << zonename << "' listening on port:" << net.GetZonePort() << endl << "-----------" << endl;
	//entity_list.WriteEntityIDs();
	UpdateWindowTitle();
	return true;
}*/

// Original source found at http://www.castaglia.org/proftpd/doc/devel-guide/src/lib/strsep.c.html
char *strsep(char **stringp, const char *delim)
{
  char *res;

  if(!stringp || !*stringp || !**stringp)
    return (char*)0;

  res = *stringp;
  while(**stringp && !strchr(delim,**stringp))
    (*stringp)++;
  
  if(**stringp) {
    **stringp = '\0';
    (*stringp)++;
  }

  return res;
}
