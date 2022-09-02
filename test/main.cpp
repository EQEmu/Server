#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../common/eqemu_logsys.h"
#include "../common/global_define.h"
#include "../common/shareddb.h"
#include "../common/eqemu_config.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/content/world_content_service.h"
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
#include "../common/string_util.h"
#include "../common/crash.h"
#include "../common/memory_mapped_file.h"
#include "../common/spdat.h"
#include "../common/eqemu_logsys.h"

#include "../zone/api_service.h"
#include "../zone/zone_config.h"
#include "../zone/masterentity.h"
#include "../zone/worldserver.h"
#include "../zone/zone.h"
#include "../zone/queryserv.h"
#include "../zone/command.h"
#ifdef BOTS
#include "../zone/bot_command.h"
#endif
#include "../zone/zonedb.h"
#include "../zone/zone_store.h"
#include "../zone/titles.h"
#include "../zone/guild_mgr.h"
#include "../zone/task_manager.h"
#include "../zone/quest_parser_collection.h"
#include "../zone/embparser.h"
#include "../zone/lua_parser.h"
#include "../zone/questmgr.h"
#include "../zone/npc_scale_manager.h"

#include "../common/net/eqstream.h"
#include "../common/content/world_content_service.h"

#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <chrono>

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
#include "zone_store.h"
#include "zone_event_scheduler.h"

#endif

volatile bool RunLoops = true;
#ifdef __FreeBSD__
#include <pthread_np.h>
#endif

extern volatile bool is_zone_loaded;

EntityList entity_list;
WorldServer worldserver;
ZoneStore zone_store;
uint32 numclients = 0;
char errorname[32];
extern Zone* zone;
npcDecayTimes_Struct npcCorpseDecayTimes[100];
TitleManager title_manager;
QueryServ *QServ          = 0;
TaskManager *task_manager = 0;
NpcScaleManager *npc_scale_manager;
QuestParserCollection *parse = 0;
EQEmuLogSys          LogSys;
ZoneEventScheduler event_scheduler;
WorldContentService  content_service;
const SPDat_Spell_Struct* spells;
int32 SPDAT_RECORDS = -1;
const ZoneConfig *Config;
double frame_time = 0.0;

using namespace std;

TEST_CASE("client simple attack npc", "[client][npc]") {
	streambuf* originalBuf = cout.rdbuf();
	ostringstream buf;

	RegisterExecutablePlatform(ExePlatformZone);
	LogSys.LoadLogSettingsDefaults();

	QServ = new QueryServ;
	parse = new QuestParserCollection();

	LogSys.log_settings[Logs::Combat].is_category_enabled = 1;
	LogSys.log_settings[Logs::Combat].log_to_console = 3;
	LogSys.log_settings[Logs::Attack].is_category_enabled = 1;
	LogSys.log_settings[Logs::Attack].log_to_console = 3;
	RuleManager::Instance()->SetRule("Logging:PrintFileFunctionAndLine", "1");
	//EQEmuConfig::LoadConfig();
	/*try {
		REQUIRE_NOTHROW(EQEmuConfig::LoadConfig());
	}
	catch (Json::Exception e) {
		LogError("Unable to load configuration file: {}", e.what());
		FAIL();
	}
	catch (std::exception e) {
		LogError("Unable to load configuration file generic fail: {}", e.what());
		FAIL();
	}*/
	//LogInfo("CURRENT_VERSION: {}", CURRENT_VERSION);
	zone = new Zone(-1, 0, "test");

	NPCType* npc_type = new NPCType();
	NPC* npc_source = new NPC(npc_type, nullptr, glm::vec4(0, 0, 0, 0), GravityBehavior::Water);
	npc_source->SetName("npc_source");
	npc_source->ModifyNPCStat("max_hp", "1000");
	npc_source->ModifyNPCStat("accuracy", "100");
	npc_source->SetHP(1000);

	Client* client_target = new Client(nullptr);
	client_target->SetName("client_target");
	SECTION("Attack client_target") {
		LogInfo("Testing Attack");
		//cout.rdbuf(buf.rdbuf());

		REQUIRE(npc_source->Attack(client_target));
		//cout.rdbuf(originalBuf);
		//REQUIRE(buf.str().find("Final damage against") != std::string::npos);
		//buf.str("");
	}

	DamageHitInfo my_hit = DamageHitInfo();
	SECTION("DoAttack no damage client_target") {
		LogInfo("Testing DoAttack no damage");
		my_hit.tohit = 100;
		my_hit.damage_done = 0;
		npc_source->DoAttack(client_target, my_hit);
		REQUIRE(client_target->GetHP() == 1000);
		return;
	}


	SECTION("ImmuneToMelee client_target") {
		LogInfo("Testing ImmuneToMelee");
		my_hit.tohit = 100;
		my_hit.damage_done = 5;
		npc_source->SetSpecialAbility(IMMUNE_MELEE, 1);
		npc_source->Attack(client_target);
		npc_source->SetSpecialAbility(IMMUNE_MELEE, 0);
		return;
	}

	SECTION("DoAttack client_target 5 base damage") {
		LogInfo("Testing DoAttack 5 base damage");
		my_hit.base_damage = 5;
		my_hit.damage_done = 5;
		my_hit.min_damage = 1;
		my_hit.tohit = 100;
		npc_source->DoAttack(client_target, my_hit);
		return;
	}

	SECTION("CommonOutgoingHitSuccess client_target 5 base damage") {
		LogInfo("Testing CommonOutgoingHitSuccess 5 base damage");

		my_hit.base_damage = 5;
		my_hit.min_damage = 1;
		my_hit.damage_done = 5;
		my_hit.tohit = 100;
		npc_source->CommonOutgoingHitSuccess(client_target, my_hit);
		return;
	}

	SECTION("Damage client_target 5 base damage") {
		LogInfo("Testing Damage(self) 5 base damage");
		my_hit.damage_done = 5;
		npc_source->Damage(client_target, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill, true);
	}


	LogInfo("{} dealt {} damage to {}", npc_source->GetName(), 1000 - client_target->GetHP(), client_target->GetName());
	cout.rdbuf(originalBuf);
}

TEST_CASE("npc simple attack", "[npc]") {

	streambuf* originalBuf = cout.rdbuf();
	ostringstream buf;

	RegisterExecutablePlatform(ExePlatformZone);
	LogSys.LoadLogSettingsDefaults();
	
	QServ = new QueryServ;
	parse = new QuestParserCollection();
	
	LogSys.log_settings[Logs::Combat].is_category_enabled = 1;
	LogSys.log_settings[Logs::Combat].log_to_console = 3;
	LogSys.log_settings[Logs::Attack].is_category_enabled = 1;
	LogSys.log_settings[Logs::Attack].log_to_console = 3;
	RuleManager::Instance()->SetRule("Logging:PrintFileFunctionAndLine", "1");
	//EQEmuConfig::LoadConfig();
	/*try {
		REQUIRE_NOTHROW(EQEmuConfig::LoadConfig());
	}
	catch (Json::Exception e) {
		LogError("Unable to load configuration file: {}", e.what());
		FAIL();
	}
	catch (std::exception e) {
		LogError("Unable to load configuration file generic fail: {}", e.what());
		FAIL();
	}*/
	//LogInfo("CURRENT_VERSION: {}", CURRENT_VERSION);
	zone = new Zone(-1, 0, "test");

	NPCType* npc_type = new NPCType();
	NPC* npc_source = new NPC(npc_type, nullptr, glm::vec4(0, 0, 0, 0), GravityBehavior::Water);
	npc_source->SetName("npc_source");
	npc_source->ModifyNPCStat("max_hp", "1000");
	npc_source->ModifyNPCStat("accuracy", "100");
	npc_source->SetHP(1000);

	NPC* npc_target= new NPC(npc_type, nullptr, glm::vec4(0, 0, 0, 0), GravityBehavior::Water);
	npc_target->SetName("npc_target");
	npc_target->ModifyNPCStat("max_hp", "1000");
	npc_target->ModifyNPCStat("accuracy", "100");
	npc_target->SetHP(1000);

	SECTION("Attack npc_target") {
		LogInfo("Testing Attack");
		//cout.rdbuf(buf.rdbuf());
		
		REQUIRE(npc_source->Attack(npc_target));
		//cout.rdbuf(originalBuf);
		//REQUIRE(buf.str().find("Final damage against") != std::string::npos);
		//buf.str("");
	}

	DamageHitInfo my_hit = DamageHitInfo();
	SECTION("DoAttack no damage npc_target") {
		LogInfo("Testing DoAttack no damage");
		my_hit.tohit = 100;
		my_hit.damage_done = 0;
		npc_source->DoAttack(npc_target, my_hit);
		REQUIRE(npc_target->GetHP() == 1000);
		return;
	}


	SECTION("ImmuneToMelee npc_target") {
		LogInfo("Testing ImmuneToMelee");
		my_hit.tohit = 100;
		my_hit.damage_done = 5;
		npc_source->SetSpecialAbility(IMMUNE_MELEE, 1);
		npc_source->Attack(npc_target);
		npc_source->SetSpecialAbility(IMMUNE_MELEE, 0);
		return;
	}

	SECTION("DoAttack npc_target 5 base damage") {
		LogInfo("Testing DoAttack 5 base damage");
		my_hit.base_damage = 5;
		my_hit.damage_done = 5;
		my_hit.min_damage = 1;
		my_hit.tohit = 100;
		npc_source->DoAttack(npc_target, my_hit);
		return;
	}

	SECTION("CommonOutgoingHitSuccess npc_target 5 base damage") {
		LogInfo("Testing CommonOutgoingHitSuccess 5 base damage");
		
		my_hit.base_damage = 5;
		my_hit.min_damage = 1;
		my_hit.damage_done = 5;
		my_hit.tohit = 100;
		npc_source->CommonOutgoingHitSuccess(npc_target, my_hit);
		return;
	}

	SECTION("Damage npc_target 5 base damage") {
		LogInfo("Testing Damage(self) 5 base damage");
		my_hit.damage_done = 5;
		npc_source->Damage(npc_target, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill, true);
	}
	
	

	LogInfo("{} dealt {} damage to {}", npc_source->GetName(), 1000-npc_target->GetHP(), npc_target->GetName());
	cout.rdbuf(originalBuf);
}

TEST_CASE("npc death by attack", "[npc]") {
	RegisterExecutablePlatform(ExePlatformZone);
	LogSys.LoadLogSettingsDefaults();

	QServ = new QueryServ;
	parse = new QuestParserCollection();
	//EQEmuConfig::LoadConfig();
	/*try {
		REQUIRE_NOTHROW(EQEmuConfig::LoadConfig());
	}
	catch (Json::Exception e) {
		LogError("Unable to load configuration file: {}", e.what());
		FAIL();
	}
	catch (std::exception e) {
		LogError("Unable to load configuration file generic fail: {}", e.what());
		FAIL();
	}*/
	zone = new Zone(-1, 0, "test");

	NPCType* npc_type = new NPCType();
	NPC* npc = new NPC(npc_type, nullptr, glm::vec4(0, 0, 0, 0), GravityBehavior::Water);
	npc->ModifyNPCStat("max_hp", "1");
	npc->SetHP(1);
	npc->Attack(npc);
}

void Shutdown()
{
	Zone::Shutdown(true);
	LogInfo("Shutting down...");
	LogSys.CloseFileLogs();
	EQ::EventLoop::Get().Shutdown();
}

void CatchSignal(int sig_num) {
#ifdef _WINDOWS
	LogInfo("Recieved signal: [{}]", sig_num);
#endif
	Shutdown();
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
