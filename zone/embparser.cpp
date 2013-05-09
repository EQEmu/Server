/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2013  EQEMu Development Team (http://eqemulator.net)

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


#ifdef EMBPERL

#include "../common/debug.h"
#include "masterentity.h"
#include "embparser.h"
#include "questmgr.h"
#include "command.h"
#include "../common/seperator.h"
#include "../common/MiscFunctions.h"
#include "QGlobals.h"
#include "zone.h"
#include <algorithm>
#include <sstream>

extern Zone* zone;

const char *QuestEventSubroutines[_LargestEventID] = {
	"EVENT_SAY",
	"EVENT_ITEM",
	"EVENT_DEATH",
	"EVENT_SPAWN",
	"EVENT_ATTACK",
	"EVENT_COMBAT",
	"EVENT_AGGRO",
	"EVENT_SLAY",
	"EVENT_NPC_SLAY",
	"EVENT_WAYPOINT_ARRIVE",
	"EVENT_WAYPOINT_DEPART",
	"EVENT_TIMER",
	"EVENT_SIGNAL",
	"EVENT_HP",
	"EVENT_ENTER",
	"EVENT_EXIT",
	"EVENT_ENTERZONE",
	"EVENT_CLICKDOOR",
	"EVENT_LOOT",
	"EVENT_ZONE",
	"EVENT_LEVEL_UP",
	"EVENT_KILLED_MERIT",
	"EVENT_CAST_ON",
	"EVENT_TASKACCEPTED",
	"EVENT_TASK_STAGE_COMPLETE",
	"EVENT_TASK_UPDATE",
	"EVENT_TASK_COMPLETE",
	"EVENT_TASK_FAIL",
	"EVENT_AGGRO_SAY",
	"EVENT_PLAYER_PICKUP",
	"EVENT_POPUPRESPONSE",
	"EVENT_PROXIMITY_SAY",
	"EVENT_CAST",
	"EVENT_SCALE_CALC",
	"EVENT_ITEM_ENTERZONE",
	"EVENT_TARGET_CHANGE",
	"EVENT_HATE_LIST",
	"EVENT_SPELL_EFFECT_CLIENT",
	"EVENT_SPELL_EFFECT_NPC",
	"EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT",
	"EVENT_SPELL_EFFECT_BUFF_TIC_NPC",
	"EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE",
	"EVENT_COMBINE_SUCCESS",
	"EVENT_COMBINE_FAILURE",
	"EVENT_ITEM_CLICK",
	"EVENT_ITEM_CLICK_CAST",
	"EVENT_GROUP_CHANGE",
	"EVENT_FORAGE_SUCCESS",
	"EVENT_FORAGE_FAILURE",
	"EVENT_FISH_START",
	"EVENT_FISH_SUCCESS",
	"EVENT_FISH_FAILURE",
	"EVENT_CLICK_OBJECT",
	"EVENT_DISCOVER_ITEM",
	"EVENT_DISCONNECT",
	"EVENT_CONNECT",
	"EVENT_ITEM_TICK",
	"EVENT_DUEL_WIN",
	"EVENT_DUEL_LOSE"
};

PerlembParser::PerlembParser() : perl(nullptr) {
	global_npc_quest_status_ = questUnloaded;
	player_quest_status_ = questUnloaded;
	global_player_quest_status_ = questUnloaded;
}

PerlembParser::~PerlembParser() {
	safe_delete(perl);
}

void PerlembParser::ReloadQuests() {
	command_clear_perl();

	try {
		if(perl == nullptr) {
			perl = new Embperl;
		} else {
			perl->Reinit();
		}
		MapFunctions();
	}
	catch(std::exception &e) {
		if(perl != nullptr) {
			delete perl;
			perl = nullptr;
		}

		LogFile->write(EQEMuLog::Status, "Error re-initializing perlembed: %s", e.what());
		throw e.what();
	}

	npc_quest_status_.clear();
	global_npc_quest_status_ = questUnloaded;
	player_quest_status_ = questUnloaded;
	global_player_quest_status_ = questUnloaded;
	item_quest_status_.clear();
	spell_quest_status_.clear();
}

void PerlembParser::MapFunctions() {
}

void PerlembParser::EventCommon(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, 
	uint32 extradata, bool global)
{
	if(!perl)
		return;

	if(event >= _LargestEventID)
		return;

	if(perl->InUse()) {
		//PERL_TODO queue event
		return;
	}
}

double PerlembParser::EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	EventCommon(evt, npc->GetNPCTypeID(), data.c_str(), npc, nullptr, init, extra_data, false);
	return 100.0;
}

double PerlembParser::EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	EventCommon(evt, npc->GetNPCTypeID(), data.c_str(), npc, nullptr, init, extra_data, true);
	return 100.0;
}

double PerlembParser::EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	EventCommon(evt, 0, data.c_str(), nullptr, nullptr, client, extra_data, false);
	return 100.0;
}

double PerlembParser::EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	EventCommon(evt, 0, data.c_str(), nullptr, nullptr, client, extra_data, true);
	return 100.0;
}

double PerlembParser::EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) {
	EventCommon(evt, objid, nullptr, nullptr, item, client, extra_data, false);
	return 100.0;
}

double PerlembParser::EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) {
	EventCommon(evt, 0, itoa(spell_id), npc, nullptr, client, extra_data, false);
	return 100.0;
}

bool PerlembParser::HasQuestSub(uint32 npcid, const char *subname) {
	return false;
}

bool PerlembParser::HasGlobalQuestSub(const char *subname) {
	return false;
}

bool PerlembParser::PlayerHasQuestSub(const char *subname) {
	return false;
}

bool PerlembParser::GlobalPlayerHasQuestSub(const char *subname) {
	return false;
}

bool PerlembParser::SpellHasQuestSub(uint32 spell_id, const char *subname) {
	return false;
}

bool PerlembParser::ItemHasQuestSub(ItemInst *itm, const char *subname) {
	return false;
}

void PerlembParser::LoadNPCScript(std::string filename, int npc_id) {
	std::stringstream package_name;
	package_name << "qst_" << npc_id;

	printf("%s = %s\n", package_name.str().c_str(), filename.c_str());
	
}

void PerlembParser::LoadGlobalNPCScript(std::string filename) {
	printf("%s %s\n", "qst_global_npc", filename.c_str());
	if(!perl)
		return;

	if(perl->InUse())
	{
		return;
	}

	if(global_npc_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_npc", filename.c_str());
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "WARNING: error compiling quest file %s: %s", filename.c_str(), err);
		global_npc_quest_status_ = questFailedToLoad;
		return;
	}

	global_npc_quest_status_ = questLoaded;
}

void PerlembParser::LoadPlayerScript(std::string filename) {
	printf("%s %s\n", "qst_player", filename.c_str());
	if(!perl)
		return;

	if(perl->InUse())
	{
		return;
	}

	if(player_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_player", filename.c_str());
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "WARNING: error compiling quest file %s: %s", filename.c_str(), err);
		player_quest_status_ = questFailedToLoad;
		return;
	}

	player_quest_status_ = questLoaded;
}

void PerlembParser::LoadGlobalPlayerScript(std::string filename) {
	printf("%s %s\n", "qst_global_player", filename.c_str());
	if(!perl)
		return;

	if(perl->InUse())
	{
		return;
	}

	if(global_player_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_player", filename.c_str());
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "WARNING: error compiling quest file %s: %s", filename.c_str(), err);
		global_player_quest_status_ = questFailedToLoad;
		return;
	}

	global_player_quest_status_ = questLoaded;
}

void PerlembParser::LoadItemScript(std::string filename, std::string item_script) {
	printf("Load Spell %s = %s\n", item_script.c_str(), filename.c_str());
}

void PerlembParser::LoadSpellScript(std::string filename, uint32 spell_id) {
	printf("Load Item %d = %s\n", spell_id, filename.c_str());
}

void PerlembParser::AddVar(std::string name, std::string val) {
}

std::string PerlembParser::GetVar(std::string name) {
	return std::string();
}

void PerlembParser::ExportHash(const char *pkgprefix, const char *hashname, std::map<std::string, std::string> &vals)
{
	if(!perl)
		return;

	try
	{
		perl->sethash(
			std::string(pkgprefix).append("::").append(hashname).c_str(),
			vals
		);
	} catch(const char *err) {
		LogFile->write(EQEMuLog::Status, "Error exporting hash: %s", err);
	}
}

void PerlembParser::ExportVar(const char * pkgprefix, const char * varname, int value) const
{

	if(!perl)
		return;

	try {
		perl->seti(std::string(pkgprefix).append("::").append(varname).c_str(), value);

	} catch(const char * err) {
		LogFile->write(EQEMuLog::Status, "Error exporting var: %s", err);
	}
}

void PerlembParser::ExportVar(const char * pkgprefix, const char * varname, unsigned int value) const
{

	if(!perl)
		return;

	try {
		perl->seti(std::string(pkgprefix).append("::").append(varname).c_str(), value);

	} catch(const char * err) {
		LogFile->write(EQEMuLog::Status, "Error exporting var: %s", err);
	}
}

void PerlembParser::ExportVar(const char * pkgprefix, const char * varname, float value) const
{

	if(!perl)
		return;

	try {
		perl->setd(std::string(pkgprefix).append("::").append(varname).c_str(), value);
	} catch(const char * err) {
		LogFile->write(EQEMuLog::Status, "Error exporting var: %s", err);
	}
}

void PerlembParser::ExportVarComplex(const char * pkgprefix, const char *varname, const char *value) const
{

	if(!perl)
		return;
	try
	{
		perl->eval(std::string("$").append(pkgprefix).append("::").append(varname).append("=").append(value).append(";").c_str());
	}
	catch(const char * err)
	{
		LogFile->write(EQEMuLog::Status, "Error exporting var: %s", err);
	}
}

void PerlembParser::ExportVar(const char *pkgprefix, const char *varname, const char *value) const
{
	if(!perl)
		return;

	try
	{
		perl->setstr(std::string(pkgprefix).append("::").append(varname).c_str(), value);
	}
	catch(const char * err)
	{
		LogFile->write(EQEMuLog::Status, "Error exporting var: %s", err);
	}
}

void PerlembParser::SendCommands(const char *pkgprefix, const char *event, uint32 npcid, Mob* other, Mob* mob, ItemInst* iteminst) {
}

#endif


//
//void PerlembParser::HandleQueue() {
//	if(eventQueueProcessing)
//		return;
//	eventQueueProcessing = true;
//
//	while(!eventQueue.empty()) {
//		EventRecord e = eventQueue.front();
//		eventQueue.pop();
//
//		EventCommon(e.event, e.objid, e.data.c_str(), e.npcmob, e.iteminst, e.mob, e.extradata, e.global);
//	}
//
//	eventQueueProcessing = false;
//}
//
//void PerlembParser::EventCommon(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, uint32 extradata, bool global)
//{
//	if(!perl)
//		return;
//
//	if(event >= _LargestEventID)
//		return;
//
//	if(perl->InUse()) {
//		//queue the event for later.
//		EventRecord e;
//		e.event = event;
//		e.objid = objid;
//		if(data != nullptr)
//			e.data = data;
//		e.npcmob = npcmob;
//		e.iteminst = iteminst;
//		e.mob = mob;
//		e.extradata = extradata;
//		e.global = global;
//		eventQueue.push(e);
//		return;
//	}
//
//	bool isPlayerQuest = false;
//    bool isGlobalPlayerQuest = false;
//	bool isGlobalNPC = false;
//	bool isItemQuest = false;
//	bool isSpellQuest = false;
//	if(event == EVENT_SPELL_EFFECT_CLIENT || 
//		event == EVENT_SPELL_EFFECT_NPC || 
//		event == EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT ||
//		event == EVENT_SPELL_EFFECT_BUFF_TIC_NPC ||
//		event == EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE)
//	{
//		isSpellQuest = true;
//	}
//	else
//	{
//		if(!npcmob && mob) {
//			if(!iteminst) {
//                if(global) {
//                    isGlobalPlayerQuest = true;
//                } else {
//				    isPlayerQuest = true;
//                }
//            }
//			else 
//				isItemQuest = true;
//		}
//	}
//
//	string packagename;
//	if(!isPlayerQuest && !isGlobalPlayerQuest && !isItemQuest && !isSpellQuest){
//
//		if(global){
//			isGlobalNPC = true;
//			packagename = "global_npc";
//		} else {
//
//			packagename = GetPkgPrefix(objid);
//		}
//	}
//	else if(isItemQuest) {
//		const Item_Struct* item = iteminst->GetItem();
//		if (!item) return;
//
//		if (event == EVENT_SCALE_CALC || event == EVENT_ITEM_ENTERZONE) {
//			packagename = item->CharmFile;
//		}
//		else if (event == EVENT_ITEM_CLICK || event == EVENT_ITEM_CLICK_CAST) {
//			packagename = "script_";
//			packagename += itoa(item->ScriptFileID);
//		}
//		else {
//			packagename = "item_";
//			packagename += itoa(objid);
//		}
//	}
//	else if(isPlayerQuest) {
//		if(!zone || !zone->GetShortName()) // possible segfault fix
//			return;
//		packagename = "player";
//		packagename += "_";
//		packagename += zone->GetShortName();
//	}
//    else if(isGlobalPlayerQuest) {
//        packagename = "global_player";
//    }
//	else
//	{
//		packagename = "spell_effect_";
//		packagename += data;
//	}
//
//	const char *sub_name = QuestEventSubroutines[event];
//
//	//make sure the sub we need even exists before we even do all this crap.
//	if(!perl->SubExists(packagename.c_str(), sub_name)) {
//		return;
//	}
//
//	int charid = 0;
//	if (mob && mob->IsClient()) {  // some events like waypoint and spawn don't have a player involved
//		charid = mob->CastToClient()->CharacterID();
//	} else {
//		if(npcmob)
//		{
//			charid = -npcmob->GetNPCTypeID();  // make char id negative npc id as a fudge
//		}
//		else if(mob && mob->IsNPC())
//		{
//			charid = -mob->CastToNPC()->GetNPCTypeID();  // make char id negative npc id as a fudge
//		}
//	}
//	ExportVar(packagename.c_str(), "charid", charid);
//
//	//NPC quest
//	if(!isPlayerQuest && !isGlobalPlayerQuest && !isItemQuest && !isSpellQuest)
//	{
//		//only export for npcs that are global enabled.
//		if(npcmob && npcmob->GetQglobal())
//		{
//			map<string, string> globhash;
//			QGlobalCache *npc_c = nullptr;
//			QGlobalCache *char_c = nullptr;
//			QGlobalCache *zone_c = nullptr;
//
//			//retrieve our globals
//			npc_c = npcmob->GetQGlobals();
//			if(mob && mob->IsClient())
//				char_c = mob->CastToClient()->GetQGlobals();
//			zone_c = zone->GetQGlobals();
//
//			if(!npc_c)
//			{
//				npc_c = npcmob->CreateQGlobals();
//				npc_c->LoadByNPCID(npcmob->GetNPCTypeID());
//			}
//
//			if(!char_c)
//			{
//				if(mob && mob->IsClient())
//				{
//					char_c = mob->CastToClient()->CreateQGlobals();
//					char_c->LoadByCharID(mob->CastToClient()->CharacterID());
//				}
//			}
//
//			if(!zone_c)
//			{
//				zone_c = zone->CreateQGlobals();
//				zone_c->LoadByZoneID(zone->GetZoneID());
//				zone_c->LoadByGlobalContext();
//			}
//
//			std::list<QGlobal> globalMap;
//			if(npc_c)
//			{
//				QGlobalCache::Combine(globalMap, npc_c->GetBucket(), npcmob->GetNPCTypeID(), charid, zone->GetZoneID());
//			}
//
//			if(char_c)
//			{
//				QGlobalCache::Combine(globalMap, char_c->GetBucket(), npcmob->GetNPCTypeID(), charid, zone->GetZoneID());
//			}
//
//			if(zone_c)
//			{
//				QGlobalCache::Combine(globalMap, zone_c->GetBucket(), npcmob->GetNPCTypeID(), charid, zone->GetZoneID());
//			}
//			
//			std::list<QGlobal>::iterator iter = globalMap.begin();
//			while(iter != globalMap.end())
//			{
//				globhash[(*iter).name] = (*iter).value;
//				ExportVar(packagename.c_str(), (*iter).name.c_str(), (*iter).value.c_str());
//				++iter;
//			}
//			ExportHash(packagename.c_str(), "qglobals", globhash);
//		}
//	}
//	else
//	{
//		map<string, string> globhash;
//		QGlobalCache *char_c = nullptr;
//		QGlobalCache *zone_c = nullptr;
//
//		//retrieve our globals
//		if(mob && mob->IsClient())
//			char_c = mob->CastToClient()->GetQGlobals();
//		zone_c = zone->GetQGlobals();
//
//		if(!char_c)
//		{
//			if(mob && mob->IsClient())
//			{
//				char_c = mob->CastToClient()->CreateQGlobals();
//				char_c->LoadByCharID(mob->CastToClient()->CharacterID());
//			}
//		}
//
//		if(!zone_c)
//		{
//			zone_c = zone->CreateQGlobals();
//			zone_c->LoadByZoneID(zone->GetZoneID());
//			zone_c->LoadByGlobalContext();
//		}
//
//		std::list<QGlobal> globalMap;
//		if(char_c)
//		{
//			QGlobalCache::Combine(globalMap, char_c->GetBucket(), 0, charid, zone->GetZoneID());
//		}
//
//		if(zone_c)
//		{
//			QGlobalCache::Combine(globalMap, zone_c->GetBucket(), 0, charid, zone->GetZoneID());
//		}
//
//		std::list<QGlobal>::iterator iter = globalMap.begin();
//		while(iter != globalMap.end())
//		{
//			globhash[(*iter).name] = (*iter).value;
//			ExportVar(packagename.c_str(), (*iter).name.c_str(), (*iter).value.c_str());
//			++iter;
//		}
//		ExportHash(packagename.c_str(), "qglobals", globhash);
//	}
//
//	uint8 fac = 0;
//	if (mob && mob->IsClient()) {
//		ExportVar(packagename.c_str(), "uguild_id", mob->CastToClient()->GuildID());
//		ExportVar(packagename.c_str(), "uguildrank", mob->CastToClient()->GuildRank());
//		ExportVar(packagename.c_str(), "status", mob->CastToClient()->Admin());
//	}
//
//	if(!isPlayerQuest && !isGlobalPlayerQuest && !isItemQuest){
//		if (mob && npcmob && mob->IsClient() && npcmob->IsNPC()) {
//			Client* client = mob->CastToClient();
//			NPC* npc = npcmob->CastToNPC();
//
//			// Need to figure out why one of these casts would fail..
//			if (client && npc) {
//				fac = client->GetFactionLevel(client->CharacterID(), npcmob->GetID(), client->GetRace(), client->GetClass(), client->GetDeity(), npc->GetPrimaryFaction(), npcmob);
//			}
//			else if (!client) {
//				LogFile->write(EQEMuLog::Status, "WARNING: cast failure on mob->CastToClient()");
//			}
//			else if (!npc) {
//				LogFile->write(EQEMuLog::Status, "WARNING: cast failure on npcmob->CastToNPC()");
//			}
//		}
//	}
//	if (mob) {
//		ExportVar(packagename.c_str(), "name", mob->GetName());
//		ExportVar(packagename.c_str(), "race", GetRaceName(mob->GetRace()));
//		ExportVar(packagename.c_str(), "class", GetEQClassName(mob->GetClass()));
//		ExportVar(packagename.c_str(), "ulevel", mob->GetLevel());
//		ExportVar(packagename.c_str(), "userid", mob->GetID());
//	}
//
//	if(!isPlayerQuest && !isGlobalPlayerQuest && !isItemQuest && !isSpellQuest)
//	{
//		if (npcmob)
//		{
//			ExportVar(packagename.c_str(), "mname", npcmob->GetName());
//			ExportVar(packagename.c_str(), "mobid", npcmob->GetID());
//			ExportVar(packagename.c_str(), "mlevel", npcmob->GetLevel());
//			ExportVar(packagename.c_str(), "hpratio",npcmob->GetHPRatio());
//			ExportVar(packagename.c_str(), "x", npcmob->GetX() );
//			ExportVar(packagename.c_str(), "y", npcmob->GetY() );
//			ExportVar(packagename.c_str(), "z", npcmob->GetZ() );
//			ExportVar(packagename.c_str(), "h", npcmob->GetHeading() );
//			if ( npcmob->GetTarget() ) {
//				ExportVar(packagename.c_str(), "targetid", npcmob->GetTarget()->GetID());
//				ExportVar(packagename.c_str(), "targetname", npcmob->GetTarget()->GetName());
//			}
//		}
//
//		if (fac) {
//			ExportVar(packagename.c_str(), "faction", itoa(fac));
//		}
//	}
//
//	if (zone) {
//		ExportVar(packagename.c_str(), "zoneid", zone->GetZoneID());
//		ExportVar(packagename.c_str(), "zoneln", zone->GetLongName());
//		ExportVar(packagename.c_str(), "zonesn", zone->GetShortName());
//		ExportVar(packagename.c_str(), "instanceid", zone->GetInstanceID());
//		ExportVar(packagename.c_str(), "instanceversion", zone->GetInstanceVersion());
//		TimeOfDay_Struct eqTime;
//		zone->zone_time.getEQTimeOfDay( time(0), &eqTime);
//		ExportVar(packagename.c_str(), "zonehour", eqTime.hour - 1);
//		ExportVar(packagename.c_str(), "zonemin", eqTime.minute);
//		ExportVar(packagename.c_str(), "zonetime", (eqTime.hour - 1) * 100 + eqTime.minute);
//		ExportVar(packagename.c_str(), "zoneweather", zone->zone_weather);
//	}
//
//// $hasitem
//#define HASITEM_FIRST 0
//#define HASITEM_LAST 29 // this includes worn plus 8 base slots
//#define HASITEM_ISNULLITEM(item) ((item==-1) || (item==0))
//
//	if(mob && mob->IsClient())
//	{
//		string hashname = packagename + std::string("::hasitem");
//#if EQDEBUG >= 7
//		LogFile->write(EQEMuLog::Debug, "starting hasitem, on : %s",hashname.c_str() );
//#endif
//
//		//start with an empty hash
//		perl->eval(std::string("%").append(hashname).append(" = ();").c_str());
//
//		for(int slot=HASITEM_FIRST; slot<=HASITEM_LAST;slot++)
//		{
//			char *hi_decl=nullptr;
//			int itemid=mob->CastToClient()->GetItemIDAt(slot);
//			if(!HASITEM_ISNULLITEM(itemid))
//			{
//				MakeAnyLenString(&hi_decl, "push (@{$%s{%d}},%d);",hashname.c_str(),itemid,slot);
//// this is annoying
//#if EQDEBUG >= 7
//				LogFile->write(EQEMuLog::Debug, "declare hasitem : %s",hi_decl);
//#endif
//				perl->eval(hi_decl);
//				safe_delete_array(hi_decl);
//			}
//		}
//	}
//// $oncursor
//	if(mob && mob->IsClient()) {
//		string hashname = packagename + std::string("::oncursor");
//		perl->eval(std::string("%").append(hashname).append(" = ();").c_str());
//		char *hi_decl = nullptr;
//		int itemid = mob->CastToClient()->GetItemIDAt(30);
//		if(!HASITEM_ISNULLITEM(itemid)) {
//			MakeAnyLenString(&hi_decl, "push (@{$%s{%d}},%d);",hashname.c_str(),itemid,30);
//			perl->eval(hi_decl);
//			safe_delete_array(hi_decl);
//		}
//	}
//	//do any event-specific stuff...
//	switch (event) {
//		case EVENT_SAY: {
//			if (npcmob && npcmob->GetAppearance() != eaDead)
//			  npcmob->FaceTarget(mob);
//			ExportVar(packagename.c_str(), "data", objid);
//			ExportVar(packagename.c_str(), "text", data);
//			ExportVar(packagename.c_str(), "langid", extradata);
//			break;
//		}
//		case EVENT_ITEM: {
//			if (npcmob->GetAppearance() != eaDead)
//			  npcmob->FaceTarget(mob);
//			//this is such a hack... why aren't these just set directly..
//			ExportVar(packagename.c_str(), "item1", GetVar("item1", objid).c_str());
//			ExportVar(packagename.c_str(), "item2", GetVar("item2", objid).c_str());
//			ExportVar(packagename.c_str(), "item3", GetVar("item3", objid).c_str());
//			ExportVar(packagename.c_str(), "item4", GetVar("item4", objid).c_str());
//			ExportVar(packagename.c_str(), "item1_charges", GetVar("item1.charges", objid).c_str());
//			ExportVar(packagename.c_str(), "item2_charges", GetVar("item2.charges", objid).c_str());
//			ExportVar(packagename.c_str(), "item3_charges", GetVar("item3.charges", objid).c_str());
//			ExportVar(packagename.c_str(), "item4_charges", GetVar("item4.charges", objid).c_str());
//			ExportVar(packagename.c_str(), "item1_attuned", GetVar("item1.attuned", objid).c_str());
//			ExportVar(packagename.c_str(), "item2_attuned", GetVar("item2.attuned", objid).c_str());
//			ExportVar(packagename.c_str(), "item3_attuned", GetVar("item3.attuned", objid).c_str());
//			ExportVar(packagename.c_str(), "item4_attuned", GetVar("item4.attuned", objid).c_str());
//			ExportVar(packagename.c_str(), "copper", GetVar("copper", objid).c_str());
//			ExportVar(packagename.c_str(), "silver", GetVar("silver", objid).c_str());
//			ExportVar(packagename.c_str(), "gold", GetVar("gold", objid).c_str());
//			ExportVar(packagename.c_str(), "platinum", GetVar("platinum", objid).c_str());
//			string hashname = packagename + std::string("::itemcount");
//			perl->eval(std::string("%").append(hashname).append(" = ();").c_str());
//			perl->eval(std::string("++$").append(hashname).append("{$").append(packagename).append("::item1};").c_str());
//			perl->eval(std::string("++$").append(hashname).append("{$").append(packagename).append("::item2};").c_str());
//			perl->eval(std::string("++$").append(hashname).append("{$").append(packagename).append("::item3};").c_str());
//			perl->eval(std::string("++$").append(hashname).append("{$").append(packagename).append("::item4};").c_str());
//			break;
//		}
//		case EVENT_WAYPOINT_ARRIVE:
//		case EVENT_WAYPOINT_DEPART: {
//			ExportVar(packagename.c_str(), "wp", data);
//			break;
//		}
//		case EVENT_HP: {
//			if (extradata == 1) {
//				ExportVar(packagename.c_str(), "hpevent", "-1");
//				ExportVar(packagename.c_str(), "inchpevent", data);
//			}
//			else
//			{
//				ExportVar(packagename.c_str(), "hpevent", data);
//				ExportVar(packagename.c_str(), "inchpevent", "-1");
//			}
//			break;
//}
//		case EVENT_TIMER: {
//			ExportVar(packagename.c_str(), "timer", data);
//			break;
//		}
//		case EVENT_SIGNAL: {
//			ExportVar(packagename.c_str(), "signal", data);
//			break;
//		}
//		case EVENT_NPC_SLAY: {
//			ExportVar(packagename.c_str(), "killed", mob->GetNPCTypeID());
//			break;
//		}
//		case EVENT_COMBAT: {
//			ExportVar(packagename.c_str(), "combat_state", data);
//			break;
//		}
//
//		case EVENT_CLICKDOOR: {
//			Seperator *sep = new Seperator(data);
//			ExportVar(packagename.c_str(), "doorid", sep->arg[0]);
//			ExportVar(packagename.c_str(), "version", sep->arg[1]);
//			break;
//		}
//
//		case EVENT_LOOT:{
//			Seperator *sep = new Seperator(data);
//			ExportVar(packagename.c_str(), "looted_id", sep->arg[0]);
//			ExportVar(packagename.c_str(), "looted_charges", sep->arg[1]);
//			ExportVar(packagename.c_str(), "corpse", sep->arg[2]);
//			safe_delete(sep);
//			break;
//		}
//
//		case EVENT_ZONE:{
//			ExportVar(packagename.c_str(), "target_zone_id", data);
//			break;
//		}
//		
//		case EVENT_CAST_ON:
//		case EVENT_CAST:{
//			ExportVar(packagename.c_str(), "spell_id", data);
//			break;
//		}
//
//		case EVENT_TASKACCEPTED:{
//			ExportVar(packagename.c_str(), "task_id", data);
//			break;
//		}
//
//		case EVENT_TASK_STAGE_COMPLETE:{
//			Seperator *sep = new Seperator(data);
//			ExportVar(packagename.c_str(), "task_id", sep->arg[0]);
//			ExportVar(packagename.c_str(), "activity_id", sep->arg[1]);
//			safe_delete(sep);
//			break;
//		}
//		case EVENT_TASK_FAIL:{
//			Seperator *sep = new Seperator(data);
//			ExportVar(packagename.c_str(), "task_id", sep->arg[0]);
//			safe_delete(sep);
//			break;
//		}
//		case EVENT_TASK_COMPLETE:
//		case EVENT_TASK_UPDATE:{
//			Seperator *sep = new Seperator(data);
//			ExportVar(packagename.c_str(), "donecount", sep->arg[0]);
//			ExportVar(packagename.c_str(), "activity_id", sep->arg[1]);
//			ExportVar(packagename.c_str(), "task_id", sep->arg[2]);
//			safe_delete(sep);
//			break;
//		}
//		case EVENT_PLAYER_PICKUP:{
//			ExportVar(packagename.c_str(), "picked_up_id", data);
//			break;		
//		}
//
//		case EVENT_AGGRO_SAY: {
//			ExportVar(packagename.c_str(), "data", objid);
//			ExportVar(packagename.c_str(), "text", data);
//			ExportVar(packagename.c_str(), "langid", extradata);
//			break;
//		}
//		case EVENT_POPUPRESPONSE:{
//			ExportVar(packagename.c_str(), "popupid", data);
//			break;
//		}
//		case EVENT_PROXIMITY_SAY: {
//			ExportVar(packagename.c_str(), "data", objid);
//			ExportVar(packagename.c_str(), "text", data);
//			ExportVar(packagename.c_str(), "langid", extradata);
//			break;
//		}
//		case EVENT_SCALE_CALC:
//        case EVENT_ITEM_ENTERZONE: {
//			ExportVar(packagename.c_str(), "itemid", objid);
//			ExportVar(packagename.c_str(), "itemname", iteminst->GetItem()->Name);
//			break;
//		}
//		case EVENT_ITEM_CLICK_CAST:
//		case EVENT_ITEM_CLICK: {
//			ExportVar(packagename.c_str(), "itemid", objid);
//			ExportVar(packagename.c_str(), "itemname", iteminst->GetItem()->Name);
//			ExportVar(packagename.c_str(), "slotid", extradata);
//			break;
//		}
//		case EVENT_GROUP_CHANGE: {
//			if(mob && mob->IsClient())
//			{
//				ExportVar(packagename.c_str(), "grouped", mob->IsGrouped());
//				ExportVar(packagename.c_str(), "raided", mob->IsRaidGrouped());
//			}
//			break;
//		}
//		case EVENT_HATE_LIST: {
//			ExportVar(packagename.c_str(), "hate_state", data);
//			break;
//		}
//
//		case EVENT_SPELL_EFFECT_CLIENT:
//		case EVENT_SPELL_EFFECT_NPC:
//		case EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT:
//		case EVENT_SPELL_EFFECT_BUFF_TIC_NPC:
//		{
//			ExportVar(packagename.c_str(), "caster_id", extradata);
//			break;
//		}
//		//tradeskill events
//		case EVENT_COMBINE_SUCCESS:
//		case EVENT_COMBINE_FAILURE:
//		{
//			ExportVar(packagename.c_str(), "recipe_id", extradata);
//			ExportVar(packagename.c_str(), "recipe_name", data);
//			break;
//		}
//
//		case EVENT_FORAGE_SUCCESS: {
//			ExportVar(packagename.c_str(), "foraged_item", extradata);
//			break; 
//		}
//
//		case EVENT_FISH_SUCCESS: {
//			ExportVar(packagename.c_str(), "fished_item", extradata);
//			break; 
//		}
//
//		case EVENT_CLICK_OBJECT: {
//			ExportVar(packagename.c_str(), "objectid", data);
//			break;
//		}
//
//		case EVENT_DISCOVER_ITEM: {
//			ExportVar(packagename.c_str(), "itemid", extradata);
//			break;
//		}
//
//		//nothing special about these events
//		case EVENT_DEATH:
//		case EVENT_SPAWN:
//		case EVENT_ATTACK:
//		case EVENT_SLAY:
//		case EVENT_AGGRO:
//		case EVENT_ENTER:
//		case EVENT_EXIT:
//		case EVENT_ENTERZONE:
//		case EVENT_LEVEL_UP:
//		case EVENT_KILLED_MERIT:
//		case EVENT_TARGET_CHANGE:
//			break;
//
//		default: {
//			// should we do anything here?
//			break;
//		}
//	}
//
//	if(isPlayerQuest || isGlobalPlayerQuest){
//		SendCommands(packagename.c_str(), sub_name, 0, mob, mob, nullptr);
//	}
//	else if(isItemQuest) {
//		SendCommands(packagename.c_str(), sub_name, 0, mob, mob, iteminst);
//	}
//	else if(isSpellQuest)
//	{
//        if(mob) {
//            SendCommands(packagename.c_str(), sub_name, 0, mob, mob, nullptr);
//        } else {
//            SendCommands(packagename.c_str(), sub_name, 0, npcmob, mob, nullptr);
//        }
//	}
//	else {
//		SendCommands(packagename.c_str(), sub_name, objid, npcmob, mob, nullptr);
//	}
//
//	//now handle any events that cropped up...
//	HandleQueue();
//}
//
//bool PerlembParser::isloaded(const char *packagename) const {
//	char buffer[120];
//	snprintf(buffer, 120, "$%s::isloaded", packagename);
//	if(!perl->VarExists(packagename, "isloaded"))
//		return(false);
//	return perl->geti(buffer);
//}
//
//
////this function does NOT consider the default to be a quest
//int PerlembParser::HasQuestFile(uint32 npcid) {
//	int32 qstID = GetNPCqstID(npcid);
//	int success=1;
//
//	if(hasQuests.count(npcid) == 1) {
//		questMode mode = hasQuests[npcid];
//		if(mode == questDefault)
//			return(false);
//		return(true);
//	}
//
//	if (qstID==-1)
//		success = false;
//	if (!success)
//		return(false);
//
//	if(hasQuests.count(npcid) != 1)
//		return(false);
//
//	questMode mode = hasQuests[npcid];
//	if(mode == questDefault)
//		return(false);
//
//	return(true);
//}
//
//bool PerlembParser::HasQuestSub(uint32 npcid, const char *subname) {
//	int32 qstID = GetNPCqstID(npcid);
//
//	string packagename = GetPkgPrefix(npcid);
//
//	return(perl->SubExists(packagename.c_str(), subname));
//}
//
//bool PerlembParser::HasGlobalQuestSub(const char *subname) {
//	string packagename = "global_npc";
//
//	return(perl->SubExists(packagename.c_str(), subname));
//}
//
//bool PerlembParser::PlayerHasQuestSub(const char *subname) {
//
//	string packagename = "player_";
//	packagename += zone->GetShortName();
//		
//	if(subname == "EVENT_CAST")
//		return (playerQuestLoaded[zone->GetShortName()] == pQuestEventCast);
//	
//	return(perl->SubExists(packagename.c_str(), subname));
//}
//
//bool PerlembParser::GlobalPlayerHasQuestSub(const char *subname) {
//
//	string packagename = "global_player";
//		
//	if(subname == "EVENT_CAST")
//		return (globalPlayerQuestLoaded == pQuestEventCast);
//	
//	return(perl->SubExists(packagename.c_str(), subname));
//}
//
//bool PerlembParser::SpellHasQuestSub(uint32 id, const char *subname) 
//{
//	string packagename = "spell_effect_";
//	packagename += itoa(id);
//
//	return(perl->SubExists(packagename.c_str(), subname));
//}
//
//bool PerlembParser::ItemHasQuestSub(ItemInst *itm, const char *subname)
//{
//	string packagename;
//	const Item_Struct* item = itm->GetItem();
//	if(!item)
//		return false;
//
//	if(strcmp("EVENT_SCALE_CALC", subname) == 0 || strcmp("EVENT_ITEM_ENTERZONE", subname) == 0)
//	{
//		packagename = item->CharmFile;
//	}
//	else if(strcmp("EVENT_ITEM_CLICK", subname) == 0 || strcmp("EVENT_ITEM_CLICK_CAST", subname) == 0 )
//	{
//		packagename = "script_";
//		packagename += itoa(item->ScriptFileID);
//	}
//	else
//	{
//		packagename = "item_";
//		packagename += itoa(item->ID);
//	}
//
//	return perl->SubExists(packagename.c_str(), subname);
//}
//
////utility - return something of the form "qst1234"...
////will return "qst[DEFAULT_QUEST_PREFIX]" if the npc in question has no script of its own or failed to compile and defaultOK is set to true
//std::string PerlembParser::GetPkgPrefix(uint32 npcid, bool defaultOK)
//{
//	char buf[32];
//	snprintf(buf, 32, "qst%lu", (unsigned long) npcid);
////	std::string prefix = "qst";
////	std::string temp = prefix + (std::string)(itoa(npcid));
////	if(!npcid || (defaultOK && isdefault(temp.c_str())))
//	if(!npcid || (defaultOK && (hasQuests.count(npcid) == 1 && hasQuests[npcid] == questDefault)))
//	{
//		snprintf(buf, 32, "qst%s", DEFAULT_QUEST_PREFIX.c_str());
//	}
//
//	return(std::string(buf));
//}
//
//void PerlembParser::SendCommands(const char * pkgprefix, const char *event, uint32 npcid, Mob* other, Mob* mob, ItemInst* iteminst)
//{
//	if(!perl)
//		return;
//	_ZP(PerlembParser_SendCommands);
//
//	if(mob && mob->IsClient())
//		quest_manager.StartQuest(other, mob->CastToClient());
//	else
//		quest_manager.StartQuest(other, nullptr);
//
//	try
//	{
//		std::string cmd = "@quest::cmd_queue = (); package " + (std::string)(pkgprefix) + (std::string)(";");
//		perl->eval(cmd.c_str());
//		perl->dosub(std::string(pkgprefix).append("::").append(event).c_str());
//	}
//	catch(const char * err)
//	{
//		//try to reduce some of the console spam...
//		//todo: tweak this to be more accurate at deciding what to filter (we don't want to gag legit errors)
//		if(!strstr(err,"Undefined subroutine"))
//			LogFile->write(EQEMuLog::Status, "Script error: %s::%s - %s", pkgprefix, event, err);
//		return;
//	}
//
//	int numcoms = perl->geti("quest::qsize()");
//	for(int c = 0; c < numcoms; ++c)
//	{
//		char var[1024] = {0};
//		sprintf(var,"$quest::cmd_queue[%d]{func}",c);
//		std::string cmd = perl->getstr(var);
//		sprintf(var,"$quest::cmd_queue[%d]{args}",c);
//		std::string args = perl->getstr(var);
//		size_t num_args = std::count(args.begin(), args.end(), ',') + 1;
//	
//		ExCommands(cmd, args, num_args, npcid, other, mob);
//	}
//
//	quest_manager.EndQuest();
//}
//
//#ifdef EMBPERL_COMMANDS
//void PerlembParser::ExecCommand(Client *c, Seperator *sep) {
//#ifdef EMBPERL_XS_CLASSES
//	SV *client = get_sv("commands::client", true);
//	if(c != nullptr) {
//		sv_setref_pv(client, "Client", c);
//	} else {
//		//clear out the value, mainly to get rid of blessedness
//		//which prevents us from accessing an invalid pointer
//		sv_setsv(client, newSV(0));
//	}
//#endif
//
//	char namebuf[128];
//	snprintf(namebuf, 128, "commands::%s", sep->arg[0]+1);
//	namebuf[127] = '\0';
//	std::vector<std::string> args;
//	int i;
//	for(i = 1; i <= sep->argnum; i++) {
//		args.push_back(sep->arg[i]);
//	}
//
//	try
//	{
//		perl->dosub(namebuf, &args);
//	} catch(const char * err)
//	{
//		c->Message(13, "Error executing perl command, check the logs.");
//		LogFile->write(EQEMuLog::Quest, "Script error: %s", err);
//	}
//
//	//now handle any events that cropped up...
//	HandleQueue();
//}
//#endif
//
//void PerlembParser::map_funs()
//{
//	//map each "exported" function to a variable list that we can access from c
//	//todo:
//	//	break 1|settimer 2|stoptimer 1|dbspawnadd 2|flagcheck 1|write 2|
//	//	settarget 2|follow 1|sfollow 1|save 1|setallskill 1
//	//update/ensure that the api matches that of the native script engine
//	perl->eval(
//"{"
//"package quest;"
//"&boot_qc;"
//"@cmd_queue = ();"
//"sub qsize{return scalar(@cmd_queue)};"
//"sub say{push(@cmd_queue,{func=>'say',args=>join(',',@_)});}"
//"sub emote{push(@cmd_queue,{func=>'emote',args=>join(',',@_)});}"
//"sub shout{push(@cmd_queue,{func=>'shout',args=>join(',',@_)});}"
//"sub spawn{push(@cmd_queue,{func=>'spawn',args=>join(',',@_)});}"
//"sub spawn2{push(@cmd_queue,{func=>'spawn2',args=>join(',',@_)});}"
//"sub unique_spawn{push(@cmd_queue,{func=>'unique_spawn',args=>join(',',@_)});}"
//"sub echo{push(@cmd_queue,{func=>'echo',args=>join(',',@_)});}"
//"sub summonitem{push(@cmd_queue,{func=>'summonitem',args=>join(',',@_)});}"
//"sub castspell{push(@cmd_queue,{func=>'castspell',args=>join(',',@_)});}"
//"sub selfcast{push(@cmd_queue,{func=>'selfcast',args=>join(',',@_)});}"
//"sub depop{push(@cmd_queue,{func=>'depop'});}"
//"sub exp{push(@cmd_queue,{func=>'exp',args=>join(',',@_)});}"
//"sub level{push(@cmd_queue,{func=>'level',args=>join(',',@_)});}"
//"sub safemove{push(@cmd_queue,{func=>'safemove'});}"
//"sub rain{push(@cmd_queue,{func=>'rain',args=>join(',',@_)});}"
//"sub snow{push(@cmd_queue,{func=>'snow',args=>join(',',@_)});}"
//"sub givecash{push(@cmd_queue,{func=>'givecash',args=>join(',',@_)});}"
//"sub pvp{push(@cmd_queue,{func=>'pvp',args=>join(',',@_)});}"
//"sub doanim{push(@cmd_queue,{func=>'doanim',args=>join(',',@_)});}"
//"sub addskill{push(@cmd_queue,{func=>'addskill',args=>join(',',@_)});}"
//"sub me{push(@cmd_queue,{func=>'me',args=>join(',',@_)});}"
//"sub permagender{push(@cmd_queue,{func=>'permagender',args=>join(',',@_)});}"
//"sub permarace{push(@cmd_queue,{func=>'permarace',args=>join(',',@_)});}"
//"sub scribespells{push(@cmd_queue,{func=>'scribespells',args=>join(',',@_)});}"
//"sub permaclass{push(@cmd_queue,{func=>'permaclass',args=>join(',',@_)});}"
//"sub surname{push(@cmd_queue,{func=>'surname',args=>join(',',@_)});}"
//"sub addldonpoint{push(@cmd_queue,{func=>'addldonpoint',args=>join(',',@_)});}"
//"sub ding{push(@cmd_queue,{func=>'ding',args=>join(',',@_)});}"
//"sub faction{push(@cmd_queue,{func=>'faction',args=>join(',',@_)});}"
//"sub setguild{push(@cmd_queue,{func=>'setguild',args=>join(',',@_)});}"
//"sub rebind{push(@cmd_queue,{func=>'rebind',args=>join(',',@_)});}"
//"sub flagcheck{push(@cmd_queue,{func=>'flagcheck',args=>join(',',@_)});}"
//"sub write{push(@cmd_queue,{func=>'write',args=>join(',',@_)});}"
//"sub settime{push(@cmd_queue,{func=>'settime',args=>join(',',@_)});}"
//"sub setsky{push(@cmd_queue,{func=>'setsky',args=>join(',',@_)});}"
//"sub settimer{push(@cmd_queue,{func=>'settimer',args=>join(',',@_)});}"
//"sub stoptimer{push(@cmd_queue,{func=>'stoptimer',args=>join(',',@_)});}"
//"sub settarget{push(@cmd_queue,{func=>'settarget',args=>join(',',@_)});}"
//"sub follow{push(@cmd_queue,{func=>'follow',args=>join(',',@_)});}"
//"sub sfollow{push(@cmd_queue,{func=>'sfollow',args=>join(',',@_)});}"
//"sub movepc{push(@cmd_queue,{func=>'movepc',args=>join(',',@_)});}"
//"sub gmmove{push(@cmd_queue,{func=>'gmmove',args=>join(',',@_)});}"
//"sub movegrp{push(@cmd_queue,{func=>'movegrp',args=>join(',',@_)});}"
//"sub setlanguage{push(@cmd_queue,{func=>'setlanguage',args=>join(',',@_)});}"
//"sub setskill{push(@cmd_queue,{func=>'setskill',args=>join(',',@_)});}"
//"sub setallskill{push(@cmd_queue,{func=>'setallskill',args=>join(',',@_)});}"
//"sub attack{push(@cmd_queue,{func=>'attack',args=>join(',',@_)});}"
//"sub save{push(@cmd_queue,{func=>'save',args=>join(',',@_)});}"
//"sub linkitem{push(@cmd_queue,{func=>'linkitem',args=>join(',',@_)});}"
//"sub sethp{push(@cmd_queue,{func=>'sethp',args=>join(',',@_)});}"
//"sub signal{push(@cmd_queue,{func=>'signal',args=>join(',',@_)});}"
//"sub setglobal{push(@cmd_queue,{func=>'setglobal',args=>join(',',@_)});}"
//"sub targlobal{push(@cmd_queue,{func=>'targlobal',args=>join(',',@_)});}"
//"sub delglobal{push(@cmd_queue,{func=>'delglobal',args=>join(',',@_)});}"
//"sub setnexthpevent{push(@cmd_queue,{func=>'setnexthpevent',args=>join(',',@_)});}"
//"sub setnextinchpevent{push(@cmd_queue,{func=>'setnextinchpevent',args=>join(',',@_)});}"
//"sub respawn{push(@cmd_queue,{func=>'respawn',args=>join(',',@_)});}"
//"sub stop{push(@cmd_queue,{func=>'stop',args=>join(',',@_)});}"
//"sub pause{push(@cmd_queue,{func=>'pause',args=>join(',',@_)});}"
//"sub resume{push(@cmd_queue,{func=>'resume',args=>join(',',@_)});}"
//"sub start{push(@cmd_queue,{func=>'start',args=>join(',',@_)});}"
//"sub moveto{push(@cmd_queue,{func=>'moveto',args=>join(',',@_)});}"
//"sub warp{push(@cmd_queue,{func=>'warp',args=>join(',',@_)});}"
//"sub changedeity{push(@cmd_queue,{func=>'changedeity',args=>join(',',@_)});}"
//"sub addldonpoints{push(@cmd_queue,{func=>'addldonpoints',args=>join(',',@_)});}"
//"sub addloot{push(@cmd_queue,{func=>'addloot',args=>join(',',@_)});}"
//"sub traindisc{push(@cmd_queue,{func=>'traindisc',args=>join(',',@_)});}"
//"sub set_proximity{push(@cmd_queue,{func=>'set_proximity',args=>join(',',@_)});}"
//"sub clear_proximity{push(@cmd_queue,{func=>'clear_proximity',args=>join(',',@_)});}"
//"sub setanim{push(@cmd_queue,{func=>'setanim',args=>join(',',@_)});}"
//"sub showgrid{push(@cmd_queue,{func=>'showgrid',args=>join(',',@_)});}"
//"sub showpath{push(@cmd_queue,{func=>'showpath',args=>join(',',@_)});}"
//"sub pathto{push(@cmd_queue,{func=>'pathto',args=>join(',',@_)});}"
//"sub spawn_condition{push(@cmd_queue,{func=>'spawn_condition',args=>join(',',@_)});}"
//"sub toggle_spawn_event{push(@cmd_queue,{func=>'toggle_spawn_event',args=>join(',',@_)});}"
//"sub set_zone_flag{push(@cmd_queue,{func=>'set_zone_flag',args=>join(',',@_)});}"
//"sub clear_zone_flag{push(@cmd_queue,{func=>'clear_zone_flag',args=>join(',',@_)});}"
//"package main;"
//"}"
//);//eval
//}
//
//#endif //EMBPERL
//
//#endif //EMBPARSER_CPP
