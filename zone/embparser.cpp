/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2006  EQEMu Development Team (http://eqemulator.net)

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
#include "../common/seperator.h"
#include "../common/MiscFunctions.h"
#include "../common/features.h"
#include "masterentity.h"
#include "embparser.h"
#include "questmgr.h"
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
	"EVENT_POPUP_RESPONSE",
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

PerlembParser::PerlembParser() : perl(nullptr), event_queue_in_use_(false) {
	global_npc_quest_status_ = questUnloaded;
	player_quest_status_ = questUnloaded;
	global_player_quest_status_ = questUnloaded;
}

PerlembParser::~PerlembParser() {
	safe_delete(perl);
}

void PerlembParser::ReloadQuests() {

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

void PerlembParser::EventCommon(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, 
	uint32 extradata, bool global)
{
	if(!perl)
		return;

	if(event >= _LargestEventID)
		return;

	if(perl->InUse()) {
		AddQueueEvent(event, objid, data, npcmob, iteminst, mob, extradata, global);
		return;
	}

	bool isPlayerQuest = false;
	bool isGlobalPlayerQuest = false;
	bool isGlobalNPC = false;
	bool isItemQuest = false;
	bool isSpellQuest = false;
	std::string package_name;

	GetQuestTypes(isPlayerQuest, isGlobalPlayerQuest, isGlobalNPC, isItemQuest, isSpellQuest,
		event, npcmob, iteminst, mob, global);
	GetQuestPackageName(isPlayerQuest, isGlobalPlayerQuest, isGlobalNPC, isItemQuest, isSpellQuest,
		package_name, event, objid, data, npcmob, iteminst, global);

	const char *sub_name = QuestEventSubroutines[event];
	if(!perl->SubExists(package_name.c_str(), sub_name)) {
		return;
	}

	int char_id = 0;
	ExportCharID(package_name, char_id, npcmob, mob);
	ExportQGlobals(isPlayerQuest, isGlobalPlayerQuest, isGlobalNPC, isItemQuest, isSpellQuest,
		package_name, npcmob, mob, char_id);

	//ExportGenericVariables();
	ExportMobVariables(isPlayerQuest, isGlobalPlayerQuest, isGlobalNPC, isItemQuest, isSpellQuest, 
		package_name, mob, npcmob);
	ExportZoneVariables(package_name);
	ExportItemVariables(package_name, mob);
	ExportEventVariables(package_name, event, objid, data, npcmob, iteminst, mob, extradata);

	if(isPlayerQuest || isGlobalPlayerQuest){
		SendCommands(package_name.c_str(), sub_name, 0, mob, mob, nullptr);
	}
	else if(isItemQuest) {
		SendCommands(package_name.c_str(), sub_name, 0, mob, mob, iteminst);
	}
	else if(isSpellQuest)
	{
		if(mob) {
			SendCommands(package_name.c_str(), sub_name, 0, mob, mob, nullptr);
		} else {
			SendCommands(package_name.c_str(), sub_name, 0, npcmob, mob, nullptr);
		}
	}
	else {
		SendCommands(package_name.c_str(), sub_name, objid, npcmob, mob, nullptr);
	}

	HandleQueue();
}

int PerlembParser::EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	EventCommon(evt, npc->GetNPCTypeID(), data.c_str(), npc, nullptr, init, extra_data, false);
	return 0;
}

int PerlembParser::EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	EventCommon(evt, npc->GetNPCTypeID(), data.c_str(), npc, nullptr, init, extra_data, true);
	return 0;
}

int PerlembParser::EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	EventCommon(evt, 0, data.c_str(), nullptr, nullptr, client, extra_data, false);
	return 0;
}

int PerlembParser::EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	EventCommon(evt, 0, data.c_str(), nullptr, nullptr, client, extra_data, true);
	return 0;
}

int PerlembParser::EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) {
	EventCommon(evt, objid, nullptr, nullptr, item, client, extra_data, false);
	return 0;
}

int PerlembParser::EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) {
	EventCommon(evt, 0, itoa(spell_id), npc, nullptr, client, extra_data, false);
	return 0;
}

bool PerlembParser::HasQuestSub(uint32 npcid, const char *subname) {
	std::stringstream package_name;
	package_name << "qst_npc_" << npcid;

	if(!perl)
		return false;

	auto iter = npc_quest_status_.find(npcid);
	if(iter == npc_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	return(perl->SubExists(package_name.str().c_str(), subname));
}

bool PerlembParser::HasGlobalQuestSub(const char *subname) {
	if(!perl)
		return false;

	if(global_npc_quest_status_ != questLoaded) {
		return false;
	}

	return(perl->SubExists("qst_global_npc", subname));
}

bool PerlembParser::PlayerHasQuestSub(const char *subname) {
	if(!perl)
		return false;

	if(player_quest_status_ != questLoaded) {
		return false;
	}

	return(perl->SubExists("qst_player", subname));
}

bool PerlembParser::GlobalPlayerHasQuestSub(const char *subname) {
	if(!perl)
		return false;

	if(global_player_quest_status_ != questLoaded) {
		return false;
	}

	return(perl->SubExists("qst_global_player", subname));
}

bool PerlembParser::SpellHasQuestSub(uint32 spell_id, const char *subname) {
	std::stringstream package_name;
	package_name << "qst_spell_" << spell_id;

	if(!perl)
		return false;

	auto iter = spell_quest_status_.find(spell_id);
	if(iter == spell_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	return(perl->SubExists(package_name.str().c_str(), subname));
}

bool PerlembParser::ItemHasQuestSub(ItemInst *itm, const char *subname) {
	std::stringstream package_name;
	package_name << "qst_item_";

	std::string item_name;
	const Item_Struct* item = itm->GetItem();
	if(strcmp("EVENT_SCALE_CALC", subname) == 0 || strcmp("EVENT_ITEM_ENTERZONE", subname) == 0)
	{
		item_name = item->CharmFile;
	}
	else if(strcmp("EVENT_ITEM_CLICK", subname) == 0 || strcmp("EVENT_ITEM_CLICK_CAST", subname) == 0 )
	{
		item_name = "script_";
		item_name += itoa(item->ScriptFileID);
	}
	else
	{
		item_name = "item_";
		item_name += itoa(item->ID);
	}

	package_name << item_name;

	if(!perl)
		return false;

	auto iter = item_quest_status_.find(item_name);
	if(iter == item_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	return(perl->SubExists(package_name.str().c_str(), subname));
}

void PerlembParser::LoadNPCScript(std::string filename, int npc_id) {
	std::stringstream package_name;
	package_name << "qst_npc_" << npc_id;

	if(!perl)
		return;

	if(perl->InUse())
	{
		return;
	}

	auto iter = npc_quest_status_.find(npc_id);
	if(iter != npc_quest_status_.end()) {
		return;
	}

	try {
		perl->eval_file(package_name.str().c_str(), filename.c_str());
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "WARNING: error compiling quest file %s: %s", filename.c_str(), err);
		npc_quest_status_[npc_id] = questFailedToLoad;
		return;
	}

	npc_quest_status_[npc_id] = questLoaded;
}

void PerlembParser::LoadGlobalNPCScript(std::string filename) {
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
	std::stringstream package_name;
	package_name << "qst_item_" << item_script;

	if(!perl)
		return;

	if(perl->InUse())
	{
		return;
	}

	auto iter = item_quest_status_.find(item_script);
	if(iter != item_quest_status_.end()) {
		return;
	}

	try {
		perl->eval_file(package_name.str().c_str(), filename.c_str());
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "WARNING: error compiling quest file %s: %s", filename.c_str(), err);
		item_quest_status_[item_script] = questFailedToLoad;
		return;
	}

	item_quest_status_[item_script] = questLoaded;
}

void PerlembParser::LoadSpellScript(std::string filename, uint32 spell_id) {
	std::stringstream package_name;
	package_name << "qst_spell_" << spell_id;

	if(!perl)
		return;

	if(perl->InUse())
	{
		return;
	}

	auto iter = spell_quest_status_.find(spell_id);
	if(iter != spell_quest_status_.end()) {
		return;
	}

	try {
		perl->eval_file(package_name.str().c_str(), filename.c_str());
	}
	catch(const char *err)
	{
		LogFile->write(EQEMuLog::Quest, "WARNING: error compiling quest file %s: %s", filename.c_str(), err);
		spell_quest_status_[spell_id] = questFailedToLoad;
		return;
	}

	spell_quest_status_[spell_id] = questLoaded;
}

void PerlembParser::AddVar(std::string name, std::string val) {
	vars_[name] = val;
}

std::string PerlembParser::GetVar(std::string name) {
	auto iter = vars_.find(name);
	if(iter != vars_.end()) {
		return iter->second;
	}

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
	if(!perl)
		return;

	if(mob && mob->IsClient())
		quest_manager.StartQuest(other, mob->CastToClient(), iteminst);
	else
		quest_manager.StartQuest(other, nullptr, nullptr);

	try {

		std::string cmd = "package " + (std::string)(pkgprefix) + (std::string)(";");
		perl->eval(cmd.c_str());

#ifdef EMBPERL_XS_CLASSES
		char namebuf[64];

		//init a couple special vars: client, npc, entity_list
		Client *curc = quest_manager.GetInitiator();
		snprintf(namebuf, 64, "%s::client", pkgprefix);
		SV *client = get_sv(namebuf, true);
		if(curc != nullptr) {
			sv_setref_pv(client, "Client", curc);
		} else {
			//clear out the value, mainly to get rid of blessedness
			sv_setsv(client, _empty_sv);
		}

		//only export NPC if it's a npc quest
		if(!other->IsClient()){
			NPC *curn = quest_manager.GetNPC();
			snprintf(namebuf, 64, "%s::npc", pkgprefix);
			SV *npc = get_sv(namebuf, true);
			sv_setref_pv(npc, "NPC", curn);
		}

		//only export QuestItem if it's an item quest
		if(iteminst) {
			ItemInst* curi = quest_manager.GetQuestItem();
			snprintf(namebuf, 64, "%s::questitem", pkgprefix);
			SV *questitem = get_sv(namebuf, true);
			sv_setref_pv(questitem, "QuestItem", curi);
		}

		snprintf(namebuf, 64, "%s::entity_list", pkgprefix);
		SV *el = get_sv(namebuf, true);
		sv_setref_pv(el, "EntityList", &entity_list);
#endif

		//now call the requested sub
		perl->dosub(std::string(pkgprefix).append("::").append(event).c_str());

#ifdef EMBPERL_XS_CLASSES
		std::string eval_str = (std::string)"$" + (std::string)pkgprefix + (std::string)"::client = undef;";
		eval_str += (std::string)"$" + (std::string)pkgprefix + (std::string)"::npc = undef;";
		eval_str += (std::string)"$" + (std::string)pkgprefix + (std::string)"::questitem = undef;";
		eval_str += (std::string)"$" + (std::string)pkgprefix + (std::string)"::entity_list = undef;";
		perl->eval(eval_str.c_str());
#endif

	} catch(const char * err) {

		//try to reduce some of the console spam...
		//todo: tweak this to be more accurate at deciding what to filter (we don't want to gag legit errors)
		if(!strstr(err,"Undefined subroutine"))
			LogFile->write(EQEMuLog::Status, "Script error: %s::%s - %s", pkgprefix, event, err);
	}

	quest_manager.EndQuest();
}

void PerlembParser::MapFunctions() {
	_empty_sv = newSV(0);

	perl->eval(
	"{"
	"package quest;"
	"&boot_quest;"			//load our quest XS
#ifdef EMBPERL_XS_CLASSES
	"package Mob;"
	"&boot_Mob;"			//load our Mob XS

	"package Client;"
	"our @ISA = qw(Mob);"	//client inherits mob.
	"&boot_Mob;"			//load our Mob XS
	"&boot_Client;"			//load our Client XS

	"package NPC;"
	"our @ISA = qw(Mob);"	//NPC inherits mob.
	"&boot_Mob;"			//load our Mob XS
	"&boot_NPC;"			//load our NPC XS

	"package Corpse;"
	"our @ISA = qw(Mob);"	//Corpse inherits mob.
	"&boot_Mob;"			//load our Mob XS
	"&boot_Corpse;"			//load our Mob XS

	"package EntityList;"
	"&boot_EntityList;"		//load our EntityList XS

	"package PerlPacket;"
	"&boot_PerlPacket;"		//load our PerlPacket XS

	"package Group;"
	"&boot_Group;"		//load our Group XS

	"package Raid;"
	"&boot_Raid;"		//load our Raid XS

	"package QuestItem;"
	"&boot_QuestItem;"	// load quest Item XS

	"package HateEntry;"
	"&boot_HateEntry;"	// load quest Hate XS

	"package Object;"
	"&boot_Object;"	// load quest Object XS

	"package Doors;"
	"&boot_Doors;"	// load quest Doors XS

#endif
	"package main;"
	"}"
	);
}

void PerlembParser::HandleQueue() {
	if(event_queue_in_use_)
		return;

	event_queue_in_use_ = true;

	while(!event_queue_.empty()) {
		EventRecord e = event_queue_.front();
		event_queue_.pop();

		EventCommon(e.event, e.objid, e.data.c_str(), e.npcmob, e.iteminst, e.mob, e.extradata, e.global);
	}

	event_queue_in_use_ = false;
}

void PerlembParser::AddQueueEvent(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, 
		uint32 extradata, bool global) 
{
	EventRecord e;
	e.event = event;
	e.objid = objid;
	e.data = data;
	e.npcmob = npcmob;
	e.iteminst = iteminst;
	e.mob = mob;
	e.extradata = extradata;
	e.global = global;
	event_queue_.push(e);
}

void PerlembParser::GetQuestTypes(bool &isPlayerQuest, bool &isGlobalPlayerQuest, bool &isGlobalNPC, bool &isItemQuest, 
		bool &isSpellQuest, QuestEventID event, NPC* npcmob, ItemInst* iteminst, Mob* mob, bool global)
{
	if(event == EVENT_SPELL_EFFECT_CLIENT || 
		event == EVENT_SPELL_EFFECT_NPC || 
		event == EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT ||
		event == EVENT_SPELL_EFFECT_BUFF_TIC_NPC ||
		event == EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE)
	{
		isSpellQuest = true;
	}
	else
	{
		if(!npcmob && mob) {
			if(!iteminst) {
				if(global) {
					isGlobalPlayerQuest = true;
				} else {
					isPlayerQuest = true;
				}
			}
			else 
				isItemQuest = true;
		}
	}
}

void PerlembParser::GetQuestPackageName(bool &isPlayerQuest, bool &isGlobalPlayerQuest, bool &isGlobalNPC, bool &isItemQuest, 
		bool &isSpellQuest, std::string &package_name, QuestEventID event, uint32 objid, const char * data, 
		NPC* npcmob, ItemInst* iteminst, bool global)
{
	if(!isPlayerQuest && !isGlobalPlayerQuest && !isItemQuest && !isSpellQuest) {
		if(global) {
			isGlobalNPC = true;
			package_name = "qst_global_npc";
		} else {
			package_name = "qst_npc_";
			package_name += itoa(npcmob->GetNPCTypeID());
		}
	}
	else if(isItemQuest) {
		const Item_Struct* item = iteminst->GetItem();
		package_name = "qst_item_";

		if (event == EVENT_SCALE_CALC || event == EVENT_ITEM_ENTERZONE) {
			package_name += item->CharmFile;
		}
		else if (event == EVENT_ITEM_CLICK || event == EVENT_ITEM_CLICK_CAST) {
			package_name += "script_";
			package_name += itoa(item->ScriptFileID);
		}
		else {
			package_name += "item_";
			package_name += itoa(objid);
		}
	}
	else if(isPlayerQuest) {
		package_name = "qst_player";
	}
	else if(isGlobalPlayerQuest) {
		package_name = "qst_global_player";
	}
	else
	{
		package_name = "qst_spell_";
		package_name += data;
	}
}

void PerlembParser::ExportCharID(const std::string &package_name, int &char_id, NPC *npcmob, Mob *mob) {
	if (mob && mob->IsClient()) {  // some events like waypoint and spawn don't have a player involved
		char_id = mob->CastToClient()->CharacterID();
	} else {
		if(npcmob)
		{
			char_id = -static_cast<int>(npcmob->GetNPCTypeID());  // make char id negative npc id as a fudge
		}
		else if(mob && mob->IsNPC())
		{
			char_id = -static_cast<int>(mob->CastToNPC()->GetNPCTypeID());  // make char id negative npc id as a fudge
		}
	}
	ExportVar(package_name.c_str(), "charid", char_id);
}

void PerlembParser::ExportQGlobals(bool isPlayerQuest, bool isGlobalPlayerQuest, bool isGlobalNPC, bool isItemQuest, 
	bool isSpellQuest, std::string &package_name, NPC *npcmob, Mob *mob, int char_id) {
	//NPC quest
	if(!isPlayerQuest && !isGlobalPlayerQuest && !isItemQuest && !isSpellQuest)
	{
		//only export for npcs that are global enabled.
		if(npcmob && npcmob->GetQglobal())
		{
			map<string, string> globhash;
			QGlobalCache *npc_c = nullptr;
			QGlobalCache *char_c = nullptr;
			QGlobalCache *zone_c = nullptr;

			//retrieve our globals
			npc_c = npcmob->GetQGlobals();
			if(mob && mob->IsClient())
				char_c = mob->CastToClient()->GetQGlobals();
			zone_c = zone->GetQGlobals();

			if(!npc_c)
			{
				npc_c = npcmob->CreateQGlobals();
				npc_c->LoadByNPCID(npcmob->GetNPCTypeID());
			}

			if(!char_c)
			{
				if(mob && mob->IsClient())
				{
					char_c = mob->CastToClient()->CreateQGlobals();
					char_c->LoadByCharID(mob->CastToClient()->CharacterID());
				}
			}

			if(!zone_c)
			{
				zone_c = zone->CreateQGlobals();
				zone_c->LoadByZoneID(zone->GetZoneID());
				zone_c->LoadByGlobalContext();
			}

			std::list<QGlobal> globalMap;
			if(npc_c)
			{
				QGlobalCache::Combine(globalMap, npc_c->GetBucket(), npcmob->GetNPCTypeID(), char_id, zone->GetZoneID());
			}

			if(char_c)
			{
				QGlobalCache::Combine(globalMap, char_c->GetBucket(), npcmob->GetNPCTypeID(), char_id, zone->GetZoneID());
			}

			if(zone_c)
			{
				QGlobalCache::Combine(globalMap, zone_c->GetBucket(), npcmob->GetNPCTypeID(), char_id, zone->GetZoneID());
			}
			
			std::list<QGlobal>::iterator iter = globalMap.begin();
			while(iter != globalMap.end())
			{
				globhash[(*iter).name] = (*iter).value;
				ExportVar(package_name.c_str(), (*iter).name.c_str(), (*iter).value.c_str());
				++iter;
			}
			ExportHash(package_name.c_str(), "qglobals", globhash);
		}
	}
	else
	{
		map<string, string> globhash;
		QGlobalCache *char_c = nullptr;
		QGlobalCache *zone_c = nullptr;

		//retrieve our globals
		if(mob && mob->IsClient())
			char_c = mob->CastToClient()->GetQGlobals();
		zone_c = zone->GetQGlobals();

		if(!char_c)
		{
			if(mob && mob->IsClient())
			{
				char_c = mob->CastToClient()->CreateQGlobals();
				char_c->LoadByCharID(mob->CastToClient()->CharacterID());
			}
		}

		if(!zone_c)
		{
			zone_c = zone->CreateQGlobals();
			zone_c->LoadByZoneID(zone->GetZoneID());
			zone_c->LoadByGlobalContext();
		}

		std::list<QGlobal> globalMap;
		if(char_c)
		{
			QGlobalCache::Combine(globalMap, char_c->GetBucket(), 0, char_id, zone->GetZoneID());
		}

		if(zone_c)
		{
			QGlobalCache::Combine(globalMap, zone_c->GetBucket(), 0, char_id, zone->GetZoneID());
		}

		std::list<QGlobal>::iterator iter = globalMap.begin();
		while(iter != globalMap.end())
		{
			globhash[(*iter).name] = (*iter).value;
			ExportVar(package_name.c_str(), (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
		}
		ExportHash(package_name.c_str(), "qglobals", globhash);
	}
}

void PerlembParser::ExportMobVariables(bool isPlayerQuest, bool isGlobalPlayerQuest, bool isGlobalNPC, bool isItemQuest, 
		bool isSpellQuest, std::string &package_name, Mob *mob, NPC *npcmob) 
{
	uint8 fac = 0;
	if (mob && mob->IsClient()) {
		ExportVar(package_name.c_str(), "uguild_id", mob->CastToClient()->GuildID());
		ExportVar(package_name.c_str(), "uguildrank", mob->CastToClient()->GuildRank());
		ExportVar(package_name.c_str(), "status", mob->CastToClient()->Admin());
	}

	if(!isPlayerQuest && !isGlobalPlayerQuest && !isItemQuest) {
		if (mob && npcmob && mob->IsClient()) {
			Client* client = mob->CastToClient();

			fac = client->GetFactionLevel(client->CharacterID(), npcmob->GetID(), client->GetRace(), 
				client->GetClass(), client->GetDeity(), npcmob->GetPrimaryFaction(), npcmob);
		}
	}

	if(mob) {
		ExportVar(package_name.c_str(), "name", mob->GetName());
		ExportVar(package_name.c_str(), "race", GetRaceName(mob->GetRace()));
		ExportVar(package_name.c_str(), "class", GetEQClassName(mob->GetClass()));
		ExportVar(package_name.c_str(), "ulevel", mob->GetLevel());
		ExportVar(package_name.c_str(), "userid", mob->GetID());
	}

	if(!isPlayerQuest && !isGlobalPlayerQuest && !isItemQuest && !isSpellQuest)
	{
		if (npcmob)
		{
			ExportVar(package_name.c_str(), "mname", npcmob->GetName());
			ExportVar(package_name.c_str(), "mobid", npcmob->GetID());
			ExportVar(package_name.c_str(), "mlevel", npcmob->GetLevel());
			ExportVar(package_name.c_str(), "hpratio",npcmob->GetHPRatio());
			ExportVar(package_name.c_str(), "x", npcmob->GetX() );
			ExportVar(package_name.c_str(), "y", npcmob->GetY() );
			ExportVar(package_name.c_str(), "z", npcmob->GetZ() );
			ExportVar(package_name.c_str(), "h", npcmob->GetHeading() );
			if(npcmob->GetTarget()) {
				ExportVar(package_name.c_str(), "targetid", npcmob->GetTarget()->GetID());
				ExportVar(package_name.c_str(), "targetname", npcmob->GetTarget()->GetName());
			}
		}

		if (fac) {
			ExportVar(package_name.c_str(), "faction", itoa(fac));
		}
	}
}

void PerlembParser::ExportZoneVariables(std::string &package_name) {
	if (zone) {
		ExportVar(package_name.c_str(), "zoneid", zone->GetZoneID());
		ExportVar(package_name.c_str(), "zoneln", zone->GetLongName());
		ExportVar(package_name.c_str(), "zonesn", zone->GetShortName());
		ExportVar(package_name.c_str(), "instanceid", zone->GetInstanceID());
		ExportVar(package_name.c_str(), "instanceversion", zone->GetInstanceVersion());
		TimeOfDay_Struct eqTime;
		zone->zone_time.getEQTimeOfDay( time(0), &eqTime);
		ExportVar(package_name.c_str(), "zonehour", eqTime.hour - 1);
		ExportVar(package_name.c_str(), "zonemin", eqTime.minute);
		ExportVar(package_name.c_str(), "zonetime", (eqTime.hour - 1) * 100 + eqTime.minute);
		ExportVar(package_name.c_str(), "zoneweather", zone->zone_weather);
	}
}

#define HASITEM_FIRST 0
#define HASITEM_LAST 29 // this includes worn plus 8 base slots
#define HASITEM_ISNULLITEM(item) ((item==-1) || (item==0))

void PerlembParser::ExportItemVariables(std::string &package_name, Mob *mob) {
	if(mob && mob->IsClient())
	{
		string hashname = package_name + std::string("::hasitem");

		//start with an empty hash
		perl->eval(std::string("%").append(hashname).append(" = ();").c_str());

		for(int slot = HASITEM_FIRST; slot <= HASITEM_LAST; slot++)
		{
			char *hi_decl=nullptr;
			int itemid = mob->CastToClient()->GetItemIDAt(slot);
			if(!HASITEM_ISNULLITEM(itemid))
			{
				MakeAnyLenString(&hi_decl, "push (@{$%s{%d}},%d);", hashname.c_str(), itemid, slot);
				perl->eval(hi_decl);
				safe_delete_array(hi_decl);
			}
		}
	}

	if(mob && mob->IsClient()) {
		string hashname = package_name + std::string("::oncursor");
		perl->eval(std::string("%").append(hashname).append(" = ();").c_str());
		char *hi_decl = nullptr;
		int itemid = mob->CastToClient()->GetItemIDAt(30);
		if(!HASITEM_ISNULLITEM(itemid)) {
			MakeAnyLenString(&hi_decl, "push (@{$%s{%d}},%d);",hashname.c_str(), itemid, 30);
			perl->eval(hi_decl);
			safe_delete_array(hi_decl);
		}
	}
}

#undef HASITEM_FIRST
#undef HASITEM_LAST
#undef HASITEM_ISNULLITEM

void PerlembParser::ExportEventVariables(std::string &package_name, QuestEventID event, uint32 objid, const char * data, 
	NPC* npcmob, ItemInst* iteminst, Mob* mob, uint32 extradata) 
{
	switch (event) {
		case EVENT_SAY: {
			if (npcmob && npcmob->GetAppearance() != eaDead)
			  npcmob->FaceTarget(mob);
			ExportVar(package_name.c_str(), "data", objid);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extradata);
			break;
		}

		case EVENT_ITEM: {
			if (npcmob->GetAppearance() != eaDead) {
			  npcmob->FaceTarget(mob);
			}

			//this is such a hack... why aren't these just set directly..
			ExportVar(package_name.c_str(), "item1", GetVar("item1." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item2", GetVar("item2." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item3", GetVar("item3." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item4", GetVar("item4." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item1_charges", GetVar("item1.charges." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item2_charges", GetVar("item2.charges." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item3_charges", GetVar("item3.charges." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item4_charges", GetVar("item4.charges." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item1_attuned", GetVar("item1.attuned." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item2_attuned", GetVar("item2.attuned." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item3_attuned", GetVar("item3.attuned." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "item4_attuned", GetVar("item4.attuned." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "copper", GetVar("copper." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "silver", GetVar("silver." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "gold", GetVar("gold." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "platinum", GetVar("platinum." + std::string(itoa(objid))).c_str());
			string hashname = package_name + std::string("::itemcount");
			perl->eval(std::string("%").append(hashname).append(" = ();").c_str());
			perl->eval(std::string("++$").append(hashname).append("{$").append(package_name).append("::item1};").c_str());
			perl->eval(std::string("++$").append(hashname).append("{$").append(package_name).append("::item2};").c_str());
			perl->eval(std::string("++$").append(hashname).append("{$").append(package_name).append("::item3};").c_str());
			perl->eval(std::string("++$").append(hashname).append("{$").append(package_name).append("::item4};").c_str());
			break;
		}

		case EVENT_WAYPOINT_ARRIVE:
		case EVENT_WAYPOINT_DEPART: {
			ExportVar(package_name.c_str(), "wp", data);
			break;
		}

		case EVENT_HP: {
			if (extradata == 1) {
				ExportVar(package_name.c_str(), "hpevent", "-1");
				ExportVar(package_name.c_str(), "inchpevent", data);
			}
			else
			{
				ExportVar(package_name.c_str(), "hpevent", data);
				ExportVar(package_name.c_str(), "inchpevent", "-1");
			}
			break;
		}

		case EVENT_TIMER: {
			ExportVar(package_name.c_str(), "timer", data);
			break;
		}

		case EVENT_SIGNAL: {
			ExportVar(package_name.c_str(), "signal", data);
			break;
		}

		case EVENT_NPC_SLAY: {
			ExportVar(package_name.c_str(), "killed", mob->GetNPCTypeID());
			break;
		}

		case EVENT_COMBAT: {
			ExportVar(package_name.c_str(), "combat_state", data);
			break;
		}

		case EVENT_CLICKDOOR: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "doorid", sep.arg[0]);
			ExportVar(package_name.c_str(), "version", sep.arg[1]);
			break;
		}

		case EVENT_LOOT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "looted_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "looted_charges", sep.arg[1]);
			ExportVar(package_name.c_str(), "corpse", sep.arg[2]);
			break;
		}

		case EVENT_ZONE:{
			ExportVar(package_name.c_str(), "target_zone_id", data);
			break;
		}
		
		case EVENT_CAST_ON:
		case EVENT_CAST:{
			ExportVar(package_name.c_str(), "spell_id", data);
			break;
		}

		case EVENT_TASKACCEPTED:{
			ExportVar(package_name.c_str(), "task_id", data);
			break;
		}

		case EVENT_TASK_STAGE_COMPLETE:{
			Seperator sep(data);
			ExportVar(package_name.c_str(), "task_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "activity_id", sep.arg[1]);
			break;
		}

		case EVENT_TASK_FAIL:{
			Seperator sep(data);
			ExportVar(package_name.c_str(), "task_id", sep.arg[0]);
			break;
		}

		case EVENT_TASK_COMPLETE:
		case EVENT_TASK_UPDATE:{
			Seperator sep(data);
			ExportVar(package_name.c_str(), "donecount", sep.arg[0]);
			ExportVar(package_name.c_str(), "activity_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "task_id", sep.arg[2]);
			break;
		}

		case EVENT_PLAYER_PICKUP:{
			ExportVar(package_name.c_str(), "picked_up_id", data);
			break;		
		}

		case EVENT_AGGRO_SAY: {
			ExportVar(package_name.c_str(), "data", objid);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extradata);
			break;
		}

		case EVENT_POPUP_RESPONSE:{
			ExportVar(package_name.c_str(), "popupid", data);
			break;
		}

		case EVENT_PROXIMITY_SAY: {
			ExportVar(package_name.c_str(), "data", objid);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extradata);
			break;
		}

		case EVENT_SCALE_CALC:
		case EVENT_ITEM_ENTERZONE: {
			ExportVar(package_name.c_str(), "itemid", objid);
			ExportVar(package_name.c_str(), "itemname", iteminst->GetItem()->Name);
			break;
		}

		case EVENT_ITEM_CLICK_CAST:
		case EVENT_ITEM_CLICK: {
			ExportVar(package_name.c_str(), "itemid", objid);
			ExportVar(package_name.c_str(), "itemname", iteminst->GetItem()->Name);
			ExportVar(package_name.c_str(), "slotid", extradata);
			break;
		}

		case EVENT_GROUP_CHANGE: {
			if(mob && mob->IsClient())
			{
				ExportVar(package_name.c_str(), "grouped", mob->IsGrouped());
				ExportVar(package_name.c_str(), "raided", mob->IsRaidGrouped());
			}
			break;
		}

		case EVENT_HATE_LIST: {
			ExportVar(package_name.c_str(), "hate_state", data);
			break;
		}

		case EVENT_SPELL_EFFECT_CLIENT:
		case EVENT_SPELL_EFFECT_NPC:
		case EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT:
		case EVENT_SPELL_EFFECT_BUFF_TIC_NPC:
		{
			ExportVar(package_name.c_str(), "caster_id", extradata);
			break;
		}

		//tradeskill events
		case EVENT_COMBINE_SUCCESS:
		case EVENT_COMBINE_FAILURE:
		{
			ExportVar(package_name.c_str(), "recipe_id", extradata);
			ExportVar(package_name.c_str(), "recipe_name", data);
			break;
		}

		case EVENT_FORAGE_SUCCESS: {
			ExportVar(package_name.c_str(), "foraged_item", extradata);
			break; 
		}

		case EVENT_FISH_SUCCESS: {
			ExportVar(package_name.c_str(), "fished_item", extradata);
			break; 
		}

		case EVENT_CLICK_OBJECT: {
			ExportVar(package_name.c_str(), "objectid", data);
			break;
		}

		case EVENT_DISCOVER_ITEM: {
			ExportVar(package_name.c_str(), "itemid", extradata);
			break;
		}

		default: {
			break;
		}
	}
}

#endif
