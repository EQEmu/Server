/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#ifdef EMBPERL

#include "../common/global_define.h"
#include "../common/seperator.h"
#include "../common/misc_functions.h"
#include "../common/strings.h"
#include "../common/features.h"
#include "masterentity.h"
#include "embparser.h"
#include "questmgr.h"
#include "qglobals.h"
#include "zone.h"
#include <algorithm>
#include <sstream>

extern Zone *zone;

#ifdef EMBPERL_XS
void perl_register_quest();
#ifdef EMBPERL_XS_CLASSES
void perl_register_mob();
void perl_register_npc();
void perl_register_client();
void perl_register_corpse();
void perl_register_entitylist();
void perl_register_perlpacket();
void perl_register_group();
void perl_register_raid();
void perl_register_inventory();
void perl_register_questitem();
void perl_register_spell();
void perl_register_hateentry();
void perl_register_object();
void perl_register_doors();
void perl_register_expedition();
void perl_register_expedition_lock_messages();
#ifdef BOTS
void perl_register_bot();
#endif // BOTS
#endif // EMBPERL_XS_CLASSES
#endif // EMBPERL_XS

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
	"EVENT_ENVIRONMENTAL_DAMAGE",
	"EVENT_PROXIMITY_SAY",
	"EVENT_CAST",
	"EVENT_CAST_BEGIN",
	"EVENT_SCALE_CALC",
	"EVENT_ITEM_ENTER_ZONE",
	"EVENT_TARGET_CHANGE",
	"EVENT_HATE_LIST",
	"EVENT_SPELL_EFFECT_CLIENT",
	"EVENT_SPELL_EFFECT_NPC",
	"EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT",
	"EVENT_SPELL_EFFECT_BUFF_TIC_NPC",
	"EVENT_SPELL_FADE",
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
	"EVENT_DUEL_LOSE",
	"EVENT_ENCOUNTER_LOAD",
	"EVENT_ENCOUNTER_UNLOAD",
	"EVENT_COMMAND",
	"EVENT_DROP_ITEM",
	"EVENT_DESTROY_ITEM",
	"EVENT_FEIGN_DEATH",
	"EVENT_WEAPON_PROC",
	"EVENT_EQUIP_ITEM",
	"EVENT_UNEQUIP_ITEM",
	"EVENT_AUGMENT_ITEM",
	"EVENT_UNAUGMENT_ITEM",
	"EVENT_AUGMENT_INSERT",
	"EVENT_AUGMENT_REMOVE",
	"EVENT_ENTER_AREA",
	"EVENT_LEAVE_AREA",
	"EVENT_RESPAWN",
	"EVENT_DEATH_COMPLETE",
	"EVENT_UNHANDLED_OPCODE",
	"EVENT_TICK",
	"EVENT_SPAWN_ZONE",
	"EVENT_DEATH_ZONE",
	"EVENT_USE_SKILL",
	"EVENT_COMBINE_VALIDATE",
	"EVENT_BOT_COMMAND",
	"EVENT_WARP",
	"EVENT_TEST_BUFF",
	"EVENT_COMBINE",
	"EVENT_CONSIDER",
	"EVENT_CONSIDER_CORPSE",
	"EVENT_LOOT_ZONE",
	"EVENT_EQUIP_ITEM_CLIENT",
	"EVENT_UNEQUIP_ITEM_CLIENT",
	"EVENT_SKILL_UP",
	"EVENT_LANGUAGE_SKILL_UP",
	"EVENT_ALT_CURRENCY_MERCHANT_BUY",
	"EVENT_ALT_CURRENCY_MERCHANT_SELL",
	"EVENT_MERCHANT_BUY",
	"EVENT_MERCHANT_SELL",
	"EVENT_INSPECT",
	"EVENT_TASK_BEFORE_UPDATE",
	"EVENT_AA_BUY",
	"EVENT_AA_GAIN"
  #ifdef BOTS
	,
	"EVENT_SPELL_EFFECT_BOT",
	"EVENT_SPELL_EFFECT_BUFF_TIC_BOT"
#endif
};

PerlembParser::PerlembParser() : perl(nullptr)
{
	global_npc_quest_status_    = questUnloaded;
	player_quest_status_        = questUnloaded;
	global_player_quest_status_ = questUnloaded;

#ifdef BOTS
	bot_quest_status_ = questUnloaded;
	global_bot_quest_status_ = questUnloaded;
#endif
}

PerlembParser::~PerlembParser()
{
	safe_delete(perl);
}

void PerlembParser::ReloadQuests()
{
	try {
		if (perl == nullptr) {
			perl = new Embperl;
		}
		else {
			perl->Reinit();
		}
		MapFunctions();
	}
	catch (std::exception &e) {
		if (perl != nullptr) {
			delete perl;
			perl = nullptr;
		}

		LogInfo("Error Re-Initializing PerlEmbed: [{}]", e.what());
		throw e.what();
	}

	errors_.clear();
	npc_quest_status_.clear();
	global_npc_quest_status_    = questUnloaded;
	player_quest_status_        = questUnloaded;
	global_player_quest_status_ = questUnloaded;

#ifdef BOTS
	bot_quest_status_ = questUnloaded;
	global_bot_quest_status_ = questUnloaded;
#endif

	item_quest_status_.clear();
	spell_quest_status_.clear();
}

int PerlembParser::EventCommon(
	QuestEventID event,
	uint32 objid,
	const char *data,
	Mob *npcmob,
	EQ::ItemInstance *item_inst,
	const SPDat_Spell_Struct* spell,
	Mob *mob,
	uint32 extradata,
	bool global,
	std::vector<std::any> *extra_pointers
) {
	if (!perl) {
		return 0;
	}

	if (event >= _LargestEventID) {
		return 0;
	}

	bool isPlayerQuest = false;
	bool isGlobalPlayerQuest = false;
	bool isGlobalNPC = false;
	bool isBotQuest = false;
	bool isGlobalBotQuest = false;
	bool isItemQuest = false;
	bool isSpellQuest = false;

	std::string package_name;

	GetQuestTypes(
		isPlayerQuest,
		isGlobalPlayerQuest,
		isBotQuest,
		isGlobalBotQuest,
		isGlobalNPC,
		isItemQuest,
		isSpellQuest,
		event,
		npcmob,
		item_inst,
		mob,
		global
	);

	GetQuestPackageName(
		isPlayerQuest,
		isGlobalPlayerQuest,
		isBotQuest,
		isGlobalBotQuest,
		isGlobalNPC,
		isItemQuest,
		isSpellQuest,
		package_name,
		event,
		objid,
		data,
		npcmob,
		item_inst,
		global
	);

	const char *sub_name = QuestEventSubroutines[event];
	if (!perl->SubExists(package_name.c_str(), sub_name)) {
		return 0;
	}

	int char_id = 0;
	ExportCharID(package_name, char_id, npcmob, mob);

	/* Check for QGlobal export event enable */
	if (parse->perl_event_export_settings[event].qglobals) {
		ExportQGlobals(
			isPlayerQuest,
			isGlobalPlayerQuest,
			isBotQuest,
			isGlobalBotQuest,
			isGlobalNPC,
			isItemQuest,
			isSpellQuest,
			package_name,
			npcmob,
			mob,
			char_id
		);
	}

	/* Check for Mob export event enable */
	if (parse->perl_event_export_settings[event].mob) {
		ExportMobVariables(
			isPlayerQuest,
			isGlobalPlayerQuest,
			isBotQuest,
			isGlobalBotQuest,
			isGlobalNPC,
			isItemQuest,
			isSpellQuest,
			package_name,
			mob,
			npcmob
		);
	}

	/* Check for Zone export event enable */
	if (parse->perl_event_export_settings[event].zone) {
		ExportZoneVariables(package_name);
	}

	/* Check for Item export event enable */
	if (parse->perl_event_export_settings[event].item) {
		ExportItemVariables(package_name, mob);
	}

	/* Check for Event export event enable */
	if (parse->perl_event_export_settings[event].event_variables) {
		ExportEventVariables(package_name, event, objid, data, npcmob, item_inst, mob, extradata, extra_pointers);
	}

	if (isPlayerQuest || isGlobalPlayerQuest) {
		return SendCommands(package_name.c_str(), sub_name, 0, mob, mob, nullptr, nullptr);
	} else if (isBotQuest || isGlobalBotQuest) {
		return SendCommands(package_name.c_str(), sub_name, 0, npcmob, mob, nullptr, nullptr);
	} else if (isItemQuest) {
		return SendCommands(package_name.c_str(), sub_name, 0, mob, mob, item_inst, nullptr);
	} else if (isSpellQuest) {
		if (mob) {
			return SendCommands(package_name.c_str(), sub_name, 0, mob, mob, nullptr, spell);
		} else {
			return SendCommands(package_name.c_str(), sub_name, 0, npcmob, mob, nullptr, spell);
		}
	} else {
		return SendCommands(package_name.c_str(), sub_name, objid, npcmob, mob, nullptr, nullptr);
	}
}

int PerlembParser::EventNPC(
	QuestEventID evt, NPC *npc, Mob *mob, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers
)
{
	return EventCommon(evt, npc->GetNPCTypeID(), data.c_str(), npc, nullptr, nullptr, mob, extra_data, false, extra_pointers);
}

int PerlembParser::EventGlobalNPC(
	QuestEventID evt, NPC *npc, Mob *mob, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers
)
{
	return EventCommon(evt, npc->GetNPCTypeID(), data.c_str(), npc, nullptr, nullptr, mob, extra_data, true, extra_pointers);
}

int PerlembParser::EventPlayer(
	QuestEventID evt, Client *client, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers
)
{
	return EventCommon(evt, 0, data.c_str(), nullptr, nullptr, nullptr, client, extra_data, false, extra_pointers);
}

int PerlembParser::EventGlobalPlayer(
	QuestEventID evt, Client *client, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers
)
{
	return EventCommon(evt, 0, data.c_str(), nullptr, nullptr, nullptr, client, extra_data, true, extra_pointers);
}

int PerlembParser::EventItem(
	QuestEventID evt, Client *client, EQ::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers
)
{
	// needs pointer validation on 'item' argument
	return EventCommon(evt, item->GetID(), nullptr, nullptr, item, nullptr, client, extra_data, false, extra_pointers);
}

int PerlembParser::EventSpell(
	QuestEventID evt, Mob *mob, Client *client, uint32 spell_id, std::string data, uint32 extra_data,
	std::vector<std::any> *extra_pointers
)
{
	return EventCommon(evt, spell_id, data.c_str(), mob, nullptr, &spells[spell_id], client, extra_data, false, extra_pointers);
}

bool PerlembParser::HasQuestSub(uint32 npcid, QuestEventID evt)
{
	std::stringstream package_name;
	package_name << "qst_npc_" << npcid;

	if (!perl) {
		return false;
	}

	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = QuestEventSubroutines[evt];

	auto iter = npc_quest_status_.find(npcid);
	if (iter == npc_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	return (perl->SubExists(package_name.str().c_str(), subname));
}

bool PerlembParser::HasGlobalQuestSub(QuestEventID evt)
{
	if (!perl) {
		return false;
	}

	if (global_npc_quest_status_ != questLoaded) {
		return false;
	}

	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = QuestEventSubroutines[evt];

	return (perl->SubExists("qst_global_npc", subname));
}

bool PerlembParser::PlayerHasQuestSub(QuestEventID evt)
{
	if (!perl) {
		return false;
	}

	if (player_quest_status_ != questLoaded) {
		return false;
	}

	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = QuestEventSubroutines[evt];

	return (perl->SubExists("qst_player", subname));
}

bool PerlembParser::GlobalPlayerHasQuestSub(QuestEventID evt)
{
	if (!perl) {
		return false;
	}

	if (global_player_quest_status_ != questLoaded) {
		return false;
	}

	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = QuestEventSubroutines[evt];

	return (perl->SubExists("qst_global_player", subname));
}

bool PerlembParser::SpellHasQuestSub(uint32 spell_id, QuestEventID evt)
{
	std::stringstream package_name;
	package_name << "qst_spell_" << spell_id;

	if (!perl) {
		return false;
	}

	auto iter = spell_quest_status_.find(spell_id);
	if (iter == spell_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = QuestEventSubroutines[evt];

	return (perl->SubExists(package_name.str().c_str(), subname));
}

bool PerlembParser::ItemHasQuestSub(EQ::ItemInstance *itm, QuestEventID evt)
{
	std::stringstream package_name;
	package_name << "qst_item_" << itm->GetID();

	if (!perl) {
		return false;
	}

	if (itm == nullptr) {
		return false;
	}

	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = QuestEventSubroutines[evt];

	auto iter = item_quest_status_.find(itm->GetID());
	if (iter == item_quest_status_.end() || iter->second == QuestFailedToLoad) {
		return false;
	}

	return (perl->SubExists(package_name.str().c_str(), subname));
}

void PerlembParser::LoadNPCScript(std::string filename, int npc_id)
{
	std::stringstream package_name;
	package_name << "qst_npc_" << npc_id;

	if (!perl) {
		return;
	}

	auto iter = npc_quest_status_.find(npc_id);
	if (iter != npc_quest_status_.end()) {
		return;
	}

	try {
		perl->eval_file(package_name.str().c_str(), filename.c_str());
	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling NPC Quest File [{}] NPC ID [{}] Error [{}]",
				filename,
				npc_id,
				e
			)
		);

		npc_quest_status_[npc_id] = questFailedToLoad;
		return;
	}

	npc_quest_status_[npc_id] = questLoaded;
}

void PerlembParser::LoadGlobalNPCScript(std::string filename)
{
	if (!perl) {
		return;
	}

	if (global_npc_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_npc", filename.c_str());
	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Global NPC Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		global_npc_quest_status_ = questFailedToLoad;
		return;
	}

	global_npc_quest_status_ = questLoaded;
}

void PerlembParser::LoadPlayerScript(std::string filename)
{
	if (!perl) {
		return;
	}

	if (player_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_player", filename.c_str());
	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Player Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		player_quest_status_ = questFailedToLoad;
		return;
	}

	player_quest_status_ = questLoaded;
}

void PerlembParser::LoadGlobalPlayerScript(std::string filename)
{
	if (!perl) {
		return;
	}

	if (global_player_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_player", filename.c_str());
	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Global Player Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		global_player_quest_status_ = questFailedToLoad;
		return;
	}

	global_player_quest_status_ = questLoaded;
}

void PerlembParser::LoadItemScript(std::string filename, EQ::ItemInstance *item)
{
	if (item == nullptr) {
		return;
	}

	std::stringstream package_name;
	package_name << "qst_item_" << item->GetID();

	if (!perl) {
		return;
	}

	auto iter = item_quest_status_.find(item->GetID());
	if (iter != item_quest_status_.end()) {
		return;
	}

	try {
		perl->eval_file(package_name.str().c_str(), filename.c_str());
	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Item Quest File [{}] Item ID [{}] Error [{}]",
				filename,
				item->GetID(),
				e
			)
		);

		item_quest_status_[item->GetID()] = questFailedToLoad;
		return;
	}

	item_quest_status_[item->GetID()] = questLoaded;
}

void PerlembParser::LoadSpellScript(std::string filename, uint32 spell_id)
{
	std::stringstream package_name;
	package_name << "qst_spell_" << spell_id;

	if (!perl) {
		return;
	}

	auto iter = spell_quest_status_.find(spell_id);
	if (iter != spell_quest_status_.end()) {
		return;
	}

	try {
		perl->eval_file(package_name.str().c_str(), filename.c_str());
	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Spell Quest File [{}] Spell ID [{}] Error [{}]",
				filename,
				spell_id,
				e
			)
		);

		spell_quest_status_[spell_id] = questFailedToLoad;
		return;
	}

	spell_quest_status_[spell_id] = questLoaded;
}

void PerlembParser::AddVar(std::string name, std::string val)
{
	vars_[name] = val;
}

std::string PerlembParser::GetVar(std::string name)
{
	auto iter = vars_.find(name);
	if (iter != vars_.end()) {
		return iter->second;
	}

	return std::string();
}

void PerlembParser::ExportHash(const char *pkgprefix, const char *hashname, std::map<std::string, std::string> &vals)
{
	if (!perl) {
		return;
	}

	try {
		perl->sethash(
			std::string(pkgprefix).append("::").append(hashname).c_str(),
			vals
		);
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl hash [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVar(const char *pkgprefix, const char *varname, int value)
{

	if (!perl) {
		return;
	}

	try {
		perl->seti(std::string(pkgprefix).append("::").append(varname).c_str(), value);

	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVar(const char *pkgprefix, const char *varname, unsigned int value)
{

	if (!perl) {
		return;
	}

	try {
		perl->seti(std::string(pkgprefix).append("::").append(varname).c_str(), value);

	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVar(const char *pkgprefix, const char *varname, float value)
{

	if (!perl) {
		return;
	}

	try {
		perl->setd(std::string(pkgprefix).append("::").append(varname).c_str(), value);
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVarComplex(const char *pkgprefix, const char *varname, const char *value)
{

	if (!perl) {
		return;
	}
	try {
		perl->eval(std::string("$").append(pkgprefix).append("::").append(varname).append("=").append(value).append(";").c_str());
	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}

void PerlembParser::ExportVar(const char *pkgprefix, const char *varname, const char *value)
{
	if (!perl) {
		return;
	}

	try {
		perl->setstr(std::string(pkgprefix).append("::").append(varname).c_str(), value);
	}
	catch (std::string e) {
		AddError(
			fmt::format(
				"Error exporting Perl variable [{}]",
				e
			)
		);
	}
}


void PerlembParser::ExportVar(const char* pkgprefix, const char* varname, const char* classname, void* value)
{
	if (!perl) {
		return;
	}

	// todo: try/catch shouldn't be necessary here (called perl apis don't throw)
	try {
		perl->setptr(std::string(pkgprefix).append("::").append(varname).c_str(), classname, value);
	}
	catch (std::string e) {
		AddError(fmt::format("Error exporting Perl variable [{}]", e));
	}
}

int PerlembParser::SendCommands(
	const char *pkgprefix,
	const char *event,
	uint32 object_id,
	Mob *other,
	Mob *mob,
	EQ::ItemInstance *item_inst,
	const SPDat_Spell_Struct *spell
)
{
	if (!perl) {
		return 0;
	}

	int ret_value = 0;
	if (mob && mob->IsClient()) {
		quest_manager.StartQuest(other, mob->CastToClient(), item_inst, spell);
	}
	else {
		quest_manager.StartQuest(other);
	}

	try {

		std::string cmd = "package " + (std::string) (pkgprefix) + (std::string) (";");
		perl->eval(cmd.c_str());

#ifdef EMBPERL_XS_CLASSES
		dTHX;
		{
			std::string cl  = (std::string) "$" + (std::string) pkgprefix + (std::string) "::client";
			std::string np  = (std::string) "$" + (std::string) pkgprefix + (std::string) "::npc";
			std::string qi  = (std::string) "$" + (std::string) pkgprefix + (std::string) "::questitem";
			std::string sp  = (std::string) "$" + (std::string) pkgprefix + (std::string) "::spell";
			std::string enl = (std::string) "$" + (std::string) pkgprefix + (std::string) "::entity_list";

#ifdef BOTS
			std::string bot = (std::string) "$" + (std::string) pkgprefix + (std::string) "::bot";
#endif

			if (clear_vars_.find(cl) != clear_vars_.end()) {
				auto e = fmt::format("{} = undef;", cl);
				perl->eval(e.c_str());
			}

			if (clear_vars_.find(np) != clear_vars_.end()) {
				auto e = fmt::format("{} = undef;", np);
				perl->eval(e.c_str());
			}

			if (clear_vars_.find(qi) != clear_vars_.end()) {
				auto e = fmt::format("{} = undef;", qi);
				perl->eval(e.c_str());
			}

			if (clear_vars_.find(sp) != clear_vars_.end()) {
				auto e = fmt::format("{} = undef;", sp);
				perl->eval(e.c_str());
			}

			if (clear_vars_.find(enl) != clear_vars_.end()) {
				auto e = fmt::format("{} = undef;", enl);
				perl->eval(e.c_str());
			}

#ifdef BOTS
			if (clear_vars_.find(bot) != clear_vars_.end()) {
				auto e = fmt::format("{} = undef;", bot);
				perl->eval(e.c_str());
			}
#endif
		}

		std::string buf;

		//init a couple special vars: client, npc, entity_list
		Client *curc = quest_manager.GetInitiator();
		buf = fmt::format("{}::client", pkgprefix);
		SV *client = get_sv(buf.c_str(), true);
		if (curc) {
			sv_setref_pv(client, "Client", curc);
		} else {
			//clear out the value, mainly to get rid of blessedness
			sv_setsv(client, _empty_sv);
		}

		//only export NPC if it's a npc quest
		if (!other->IsClient() && other->IsNPC()) {
			NPC *curn = quest_manager.GetNPC();
			buf = fmt::format("{}::npc", pkgprefix);
			SV *npc = get_sv(buf.c_str(), true);
			sv_setref_pv(npc, "NPC", curn);
		}

#ifdef BOTS
		if (!other->IsClient() && other->IsBot()) {
			Bot *curb = quest_manager.GetBot();
			buf = fmt::format("{}::bot", pkgprefix);
			SV *bot = get_sv(buf.c_str(), true);
			sv_setref_pv(bot, "Bot", curb);
		}
#endif

		//only export QuestItem if it's an item quest
		if (item_inst) {
			EQ::ItemInstance *curi = quest_manager.GetQuestItem();
			buf = fmt::format("{}::questitem", pkgprefix);
			SV *questitem = get_sv(buf.c_str(), true);
			sv_setref_pv(questitem, "QuestItem", curi);
		}

		if (spell) {
			const SPDat_Spell_Struct* current_spell = quest_manager.GetQuestSpell();
			SPDat_Spell_Struct* real_spell = const_cast<SPDat_Spell_Struct*>(current_spell);
			buf = fmt::format("{}::spell", pkgprefix);
			SV *spell = get_sv(buf.c_str(), true);
			sv_setref_pv(spell, "Spell", (void *) real_spell);
		}


		buf = fmt::format("{}::entity_list", pkgprefix);
		SV *el = get_sv(buf.c_str(), true);
		sv_setref_pv(el, "EntityList", &entity_list);
#endif

		//now call the requested sub
		ret_value = perl->dosub(std::string(pkgprefix).append("::").append(event).c_str());

#ifdef EMBPERL_XS_CLASSES
		{
			std::string cl  = (std::string) "$" + (std::string) pkgprefix + (std::string) "::client";
			std::string np  = (std::string) "$" + (std::string) pkgprefix + (std::string) "::npc";
			std::string qi  = (std::string) "$" + (std::string) pkgprefix + (std::string) "::questitem";
			std::string sp  = (std::string) "$" + (std::string) pkgprefix + (std::string) "::spell";
			std::string enl = (std::string) "$" + (std::string) pkgprefix + (std::string) "::entity_list";

#ifdef BOTS
			std::string bot = (std::string) "$" + (std::string) pkgprefix + (std::string) "::bot";
#endif

			clear_vars_[cl]  = 1;
			clear_vars_[np]  = 1;
			clear_vars_[qi]  = 1;
			clear_vars_[sp]  = 1;
			clear_vars_[enl] = 1;

#ifdef BOTS
			clear_vars_[bot] = 1;
#endif
		}
#endif

	} catch (std::string e) {
		AddError(
			fmt::format(
				"Script Error | Package [{}] Event [{}] Error [{}]",
				pkgprefix,
				event,
				Strings::Trim(e)
			)
		);
	}

	quest_manager.EndQuest();

#ifdef EMBPERL_XS_CLASSES
	if (!quest_manager.QuestsRunning()) {
		std::string eval_str;
		for (const auto &v : clear_vars_) {
			eval_str += fmt::format("{} = undef;", v.first);
		}

		clear_vars_.clear();

		try {
			perl->eval(eval_str.c_str());
		} catch (std::string e) {
			AddError(
				fmt::format(
					"Script Clear Error | Error [{}]",
					e
				)
			);
		}
	}
#endif

	return ret_value;
}

void PerlembParser::MapFunctions()
{
	dTHX;
	_empty_sv = newSV(0);

	perl_register_quest();
#ifdef EMBPERL_XS_CLASSES
	perl_register_mob();
	perl_register_npc();
	perl_register_client();
	perl_register_corpse();
	perl_register_entitylist();
	perl_register_perlpacket();
	perl_register_group();
	perl_register_raid();
	perl_register_inventory();
	perl_register_questitem();
	perl_register_spell();
	perl_register_hateentry();
	perl_register_object();
	perl_register_doors();
	perl_register_expedition();
	perl_register_expedition_lock_messages();
#ifdef BOTS
	perl_register_bot();
#endif // BOTS
#endif // EMBPERL_XS_CLASSES
}

void PerlembParser::GetQuestTypes(
	bool &isPlayerQuest,
	bool &isGlobalPlayerQuest,
	bool &isBotQuest,
	bool &isGlobalBotQuest,
	bool &isGlobalNPC,
	bool &isItemQuest,
	bool &isSpellQuest,
	QuestEventID event,
	Mob *npcmob,
	EQ::ItemInstance *item_inst,
	Mob *mob,
	bool global
) {
	if (
		event == EVENT_SPELL_EFFECT_CLIENT ||
		event == EVENT_SPELL_EFFECT_NPC ||
#ifdef BOTS		
		event == EVENT_SPELL_EFFECT_BOT ||
#endif
		event == EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT ||
		event == EVENT_SPELL_EFFECT_BUFF_TIC_NPC ||
#ifdef BOTS		
		event == EVENT_SPELL_EFFECT_BUFF_TIC_BOT ||
#endif
		event == EVENT_SPELL_FADE ||
		event == EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE
	) {
		isSpellQuest = true;
	} else {
		if (npcmob) {
			if (!item_inst) {
				if (global) {
					if (npcmob->IsBot()) {
						isGlobalBotQuest = true;
					}
				} else {
					if (npcmob->IsBot()) {
						isBotQuest = true;
					}
				}
			} else {
				isItemQuest = true;
			}
		} else if (!npcmob && mob) {
			if (!item_inst) {
				if (global) {
					if (mob->IsClient()) {
						isGlobalPlayerQuest = true;
					}
				} else {
					if (mob->IsClient()) {
						isPlayerQuest = true;
					}
				}
			} else {
				isItemQuest = true;
			}
		}
	}
}

void PerlembParser::GetQuestPackageName(
	bool &isPlayerQuest,
	bool &isGlobalPlayerQuest,
	bool &isBotQuest,
	bool &isGlobalBotQuest,
	bool &isGlobalNPC,
	bool &isItemQuest,
	bool &isSpellQuest,
	std::string &package_name,
	QuestEventID event,
	uint32 objid,
	const char *data,
	Mob *npcmob,
	EQ::ItemInstance *item_inst,
	bool global
)
{
	if (
		!isPlayerQuest &&
		!isGlobalPlayerQuest &&
		!isBotQuest &&
		!isGlobalBotQuest &&
		!isItemQuest &&
		!isSpellQuest
	) {
		if (global) {
			isGlobalNPC  = true;
			package_name = "qst_global_npc";
		} else {
			package_name = fmt::format("qst_npc_{}", npcmob->GetNPCTypeID());
		}
	} else if (isItemQuest) {
		// need a valid EQ::ItemInstance pointer check here..unsure how to cancel this process
		const EQ::ItemData *item = item_inst->GetItem();
		package_name = fmt::format("qst_item_{}", item->ID);
	} else if (isPlayerQuest) {
		package_name = "qst_player";
	} else if (isGlobalPlayerQuest) {
		package_name = "qst_global_player";
	} else if (isBotQuest) {
		package_name = "qst_bot";
	} else if (isGlobalBotQuest) {
		package_name = "qst_global_bot";
	} else {
		package_name = fmt::format("qst_spell_{}", objid);
	}
}

void PerlembParser::ExportCharID(const std::string &package_name, int &char_id, Mob *npcmob, Mob *mob)
{
	if (mob && mob->IsClient()) {  // some events like waypoint and spawn don't have a player involved
		char_id = mob->CastToClient()->CharacterID();
	} else {
		if (npcmob) {
			char_id = -static_cast<int>(npcmob->GetNPCTypeID());  // make char id negative npc id as a fudge
		}
		else if (mob && mob->IsNPC()) {
			char_id = -static_cast<int>(mob->CastToNPC()->GetNPCTypeID());  // make char id negative npc id as a fudge
		}
	}
	ExportVar(package_name.c_str(), "charid", char_id);
}

void PerlembParser::ExportQGlobals(
	bool isPlayerQuest,
	bool isGlobalPlayerQuest,
	bool isBotQuest,
	bool isGlobalBotQuest,
	bool isGlobalNPC,
	bool isItemQuest,
	bool isSpellQuest,
	std::string &package_name,
	Mob *npcmob,
	Mob *mob,
	int char_id
) {
	//NPC quest
	if (
		!isPlayerQuest &&
		!isGlobalPlayerQuest &&
		!isBotQuest && 
		!isGlobalBotQuest &&
		!isItemQuest &&
		!isSpellQuest
	) {
		//only export for npcs that are global enabled.
		if (npcmob && npcmob->GetQglobal()) {
			std::map<std::string, std::string> globhash;
			QGlobalCache *npc_c  = nullptr;
			QGlobalCache *char_c = nullptr;
			QGlobalCache *zone_c = nullptr;

			//retrieve our globals
			if (npcmob) {
				if (npcmob->IsNPC()) {
					npc_c = npcmob->CastToNPC()->GetQGlobals();
				} else if (npcmob->IsClient()) {
					char_c = npcmob->CastToClient()->GetQGlobals();
				}
			}

			if (mob && mob->IsClient()) {
				char_c = mob->CastToClient()->GetQGlobals();
			}

			zone_c = zone->GetQGlobals();

			if (!npc_c) {
				if (npcmob && npcmob->IsNPC()) {
					npc_c = npcmob->CastToNPC()->CreateQGlobals();
					npc_c->LoadByNPCID(npcmob->GetNPCTypeID());
				}
			}

			if (!char_c) {
				if (mob && mob->IsClient()) {
					char_c = mob->CastToClient()->CreateQGlobals();
					char_c->LoadByCharID(mob->CastToClient()->CharacterID());
				}
			}

			if (!zone_c) {
				zone_c = zone->CreateQGlobals();
				zone_c->LoadByZoneID(zone->GetZoneID());
				zone_c->LoadByGlobalContext();
			}

			std::list<QGlobal> globalMap;
			if (npc_c) {
				QGlobalCache::Combine(
					globalMap,
					npc_c->GetBucket(),
					npcmob->GetNPCTypeID(),
					char_id,
					zone->GetZoneID())
				;
			}

			if (char_c) {
				QGlobalCache::Combine(
					globalMap,
					char_c->GetBucket(),
					npcmob->GetNPCTypeID(),
					char_id,
					zone->GetZoneID()
				);
			}

			if (zone_c) {
				QGlobalCache::Combine(
					globalMap,
					zone_c->GetBucket(),
					npcmob->GetNPCTypeID(),
					char_id,
					zone->GetZoneID()
				);
			}

			auto iter = globalMap.begin();
			while (iter != globalMap.end()) {
				globhash[(*iter).name] = (*iter).value;
				ExportVar(package_name.c_str(), (*iter).name.c_str(), (*iter).value.c_str());
				++iter;
			}

			ExportHash(package_name.c_str(), "qglobals", globhash);
		}
	} else {
		std::map<std::string, std::string> globhash;
		QGlobalCache *char_c = nullptr;
		QGlobalCache *zone_c = nullptr;

		//retrieve our globals
		if (mob && mob->IsClient()) {
			char_c = mob->CastToClient()->GetQGlobals();
		}

		zone_c = zone->GetQGlobals();

		if (!char_c) {
			if (mob && mob->IsClient()) {
				char_c = mob->CastToClient()->CreateQGlobals();
				char_c->LoadByCharID(mob->CastToClient()->CharacterID());
			}
		}

		if (!zone_c) {
			zone_c = zone->CreateQGlobals();
			zone_c->LoadByZoneID(zone->GetZoneID());
			zone_c->LoadByGlobalContext();
		}

		std::list<QGlobal> globalMap;
		if (char_c) {
			QGlobalCache::Combine(globalMap, char_c->GetBucket(), 0, char_id, zone->GetZoneID());
		}

		if (zone_c) {
			QGlobalCache::Combine(globalMap, zone_c->GetBucket(), 0, char_id, zone->GetZoneID());
		}

		auto iter = globalMap.begin();
		while (iter != globalMap.end()) {
			globhash[(*iter).name] = (*iter).value;
			ExportVar(package_name.c_str(), (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
		}

		ExportHash(package_name.c_str(), "qglobals", globhash);
	}
}

void PerlembParser::ExportMobVariables(
	bool isPlayerQuest,
	bool isGlobalPlayerQuest,
	bool isBotQuest,
	bool isGlobalBotQuest,
	bool isGlobalNPC,
	bool isItemQuest,
	bool isSpellQuest,
	std::string &package_name,
	Mob *mob,
	Mob *npcmob
) {
	uint8 fac = 0;
	if (mob && mob->IsClient()) {
		ExportVar(package_name.c_str(), "uguild_id", mob->CastToClient()->GuildID());
		ExportVar(package_name.c_str(), "uguildrank", mob->CastToClient()->GuildRank());
		ExportVar(package_name.c_str(), "status", mob->CastToClient()->Admin());
	}

#ifdef BOTS
	if (mob && mob->IsBot()) {
		ExportVar(package_name.c_str(), "bot_id", mob->CastToBot()->GetBotID());
		ExportVar(package_name.c_str(), "bot_owner_char_id", mob->CastToBot()->GetBotOwnerCharacterID());
	}
#endif

	if (
		!isPlayerQuest &&
		!isGlobalPlayerQuest &&
		!isBotQuest &&
		!isGlobalBotQuest &&
		!isItemQuest
	) {
		if (mob && mob->IsClient() && npcmob && npcmob->IsNPC()) {
			Client *client = mob->CastToClient();

			fac = client->GetFactionLevel(
				client->CharacterID(), npcmob->GetID(), client->GetFactionRace(),
				client->GetClass(), client->GetDeity(), npcmob->GetPrimaryFaction(), npcmob
			);
		}
	}

	if (mob) {
		ExportVar(package_name.c_str(), "name", mob->GetName());
		ExportVar(package_name.c_str(), "race", GetRaceIDName(mob->GetRace()));
		ExportVar(package_name.c_str(), "class", GetClassIDName(mob->GetClass()));
		ExportVar(package_name.c_str(), "ulevel", mob->GetLevel());
		ExportVar(package_name.c_str(), "userid", mob->GetID());
	}

	if (
		!isPlayerQuest &&
		!isGlobalPlayerQuest &&
		!isBotQuest && 
		!isGlobalBotQuest &&
		!isItemQuest &&
		!isSpellQuest
	) {
		if (npcmob->IsNPC()) {
			ExportVar(package_name.c_str(), "mname", npcmob->GetName());
			ExportVar(package_name.c_str(), "mobid", npcmob->GetID());
			ExportVar(package_name.c_str(), "mlevel", npcmob->GetLevel());
			ExportVar(package_name.c_str(), "hpratio", npcmob->GetHPRatio());
			ExportVar(package_name.c_str(), "x", npcmob->GetX());
			ExportVar(package_name.c_str(), "y", npcmob->GetY());
			ExportVar(package_name.c_str(), "z", npcmob->GetZ());
			ExportVar(package_name.c_str(), "h", npcmob->GetHeading());
			if (npcmob->GetTarget()) {
				ExportVar(package_name.c_str(), "targetid", npcmob->GetTarget()->GetID());
				ExportVar(package_name.c_str(), "targetname", npcmob->GetTarget()->GetName());
			}
		}

		if (fac) {
			ExportVar(package_name.c_str(), "faction", itoa(fac));
		}
	}
}

void PerlembParser::ExportZoneVariables(std::string &package_name)
{
	if (zone) {
		ExportVar(package_name.c_str(), "zoneid", zone->GetZoneID());
		ExportVar(package_name.c_str(), "zoneln", zone->GetLongName());
		ExportVar(package_name.c_str(), "zonesn", zone->GetShortName());
		ExportVar(package_name.c_str(), "instanceid", zone->GetInstanceID());
		ExportVar(package_name.c_str(), "instanceversion", zone->GetInstanceVersion());
		TimeOfDay_Struct eqTime;
		zone->zone_time.GetCurrentEQTimeOfDay(time(0), &eqTime);
		ExportVar(package_name.c_str(), "zonehour", eqTime.hour - 1);
		ExportVar(package_name.c_str(), "zonemin", eqTime.minute);
		ExportVar(package_name.c_str(), "zonetime", (eqTime.hour - 1) * 100 + eqTime.minute);
		ExportVar(package_name.c_str(), "zoneweather", zone->zone_weather);
	}
}

void PerlembParser::ExportItemVariables(std::string &package_name, Mob *mob)
{
	if (mob && mob->IsClient()) {
		std::string hashname = package_name + std::string("::hasitem");

		//start with an empty hash
		perl->eval(std::string("%").append(hashname).append(" = ();").c_str());

		for (int slot = EQ::invslot::EQUIPMENT_BEGIN; slot <= EQ::invslot::GENERAL_END; slot++) {
			int  itemid   = mob->CastToClient()->GetItemIDAt(slot);
			if (itemid != -1 && itemid != 0) {
				// this is really ugly with fmtlib, I think I did it right
				auto hi_decl = fmt::format("push (@{{${0}{{{1}}}}},{2});", hashname, itemid, slot);
				perl->eval(hi_decl.c_str());
			}
		}
	}

	if (mob && mob->IsClient()) {
		std::string hashname = package_name + std::string("::oncursor");
		perl->eval(std::string("%").append(hashname).append(" = ();").c_str());
		int  itemid   = mob->CastToClient()->GetItemIDAt(EQ::invslot::slotCursor);
		if (itemid != -1 && itemid != 0) {
			// this is really ugly with fmtlib, I think I did it right
			auto hi_decl = fmt::format("push (@{{${0}{{{1}}}}},{2});", hashname, itemid, EQ::invslot::slotCursor);
			perl->eval(hi_decl.c_str());
		}
	}
}

void PerlembParser::ExportEventVariables(
	std::string &package_name,
	QuestEventID event,
	uint32 objid,
	const char *data,
	Mob *npcmob,
	EQ::ItemInstance *item_inst,
	Mob *mob,
	uint32 extradata,
	std::vector<std::any> *extra_pointers
) {
	switch (event) {
		case EVENT_SAY: {
			if (npcmob && npcmob->IsNPC() && mob) {
				npcmob->CastToNPC()->DoQuestPause(mob);
			}

			ExportVar(package_name.c_str(), "data", objid);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extradata);
			break;
		}

		case EVENT_TRADE: {
			if (extra_pointers) {
				size_t      sz = extra_pointers->size();
				for (size_t i  = 0; i < sz; ++i) {
					EQ::ItemInstance *inst = std::any_cast<EQ::ItemInstance *>(extra_pointers->at(i));

					std::string var_name = "item";
					var_name += std::to_string(i + 1);

					if (inst) {
						ExportVar(package_name.c_str(), var_name.c_str(), inst->GetItem()->ID);

						std::string temp_var_name = var_name;
						temp_var_name += "_charges";
						ExportVar(package_name.c_str(), temp_var_name.c_str(), inst->GetCharges());

						temp_var_name = var_name;
						temp_var_name += "_attuned";
						ExportVar(package_name.c_str(), temp_var_name.c_str(), inst->IsAttuned());

						temp_var_name = var_name;
						temp_var_name += "_inst";
						ExportVar(package_name.c_str(), temp_var_name.c_str(), "QuestItem", inst);
					}
					else {
						ExportVar(package_name.c_str(), var_name.c_str(), 0);

						std::string temp_var_name = var_name;
						temp_var_name += "_charges";
						ExportVar(package_name.c_str(), temp_var_name.c_str(), 0);

						temp_var_name = var_name;
						temp_var_name += "_attuned";
						ExportVar(package_name.c_str(), temp_var_name.c_str(), 0);

						temp_var_name = var_name;
						temp_var_name += "_inst";
						ExportVar(package_name.c_str(), temp_var_name.c_str(), 0);
					}
				}
			}

			ExportVar(package_name.c_str(), "copper", GetVar("copper." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "silver", GetVar("silver." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "gold", GetVar("gold." + std::string(itoa(objid))).c_str());
			ExportVar(package_name.c_str(), "platinum", GetVar("platinum." + std::string(itoa(objid))).c_str());
			std::string hashname = package_name + std::string("::itemcount");
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
			else {
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

		case EVENT_CLICK_DOOR: {
			ExportVar(package_name.c_str(), "doorid", data);
			ExportVar(package_name.c_str(), "version", zone->GetInstanceVersion());
			break;
		}

		case EVENT_LOOT_ZONE:
		case EVENT_LOOT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "looted_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "looted_charges", sep.arg[1]);
			ExportVar(package_name.c_str(), "corpse", sep.arg[2]);
			ExportVar(package_name.c_str(), "corpse_id", sep.arg[3]);
			break;
		}

		case EVENT_ZONE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "from_zone_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "from_instance_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "from_instance_version", sep.arg[2]);
			ExportVar(package_name.c_str(), "target_zone_id", sep.arg[3]);
			ExportVar(package_name.c_str(), "target_instance_id", sep.arg[4]);
			ExportVar(package_name.c_str(), "target_instance_version", sep.arg[5]);
			break;
		}

		case EVENT_CAST_ON:
		case EVENT_CAST:
		case EVENT_CAST_BEGIN: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "spell_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "caster_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "caster_level", sep.arg[2]);
			break;
		}

		case EVENT_TASK_ACCEPTED: {
			ExportVar(package_name.c_str(), "task_id", data);
			break;
		}

		case EVENT_TASK_STAGE_COMPLETE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "task_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "activity_id", sep.arg[1]);
			break;
		}

		case EVENT_TASK_FAIL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "task_id", sep.arg[0]);
			break;
		}

		case EVENT_TASK_COMPLETE:
		case EVENT_TASK_BEFORE_UPDATE:
		case EVENT_TASK_UPDATE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "donecount", sep.arg[0]);
			ExportVar(package_name.c_str(), "activity_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "task_id", sep.arg[2]);
			break;
		}

		case EVENT_PLAYER_PICKUP: {
			ExportVar(package_name.c_str(), "picked_up_id", data);
			ExportVar(package_name.c_str(), "picked_up_entity_id", extradata);
			break;
		}

		case EVENT_AGGRO_SAY: {
			ExportVar(package_name.c_str(), "data", objid);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extradata);
			break;
		}

		case EVENT_POPUP_RESPONSE: {
			ExportVar(package_name.c_str(), "popupid", data);
			break;
		}
		case EVENT_ENVIRONMENTAL_DAMAGE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "env_damage", sep.arg[0]);
			ExportVar(package_name.c_str(), "env_damage_type", sep.arg[1]);
			ExportVar(package_name.c_str(), "env_final_damage", sep.arg[2]);
			break;
		}

		case EVENT_PROXIMITY_SAY: {
			ExportVar(package_name.c_str(), "data", objid);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extradata);
			break;
		}

		case EVENT_SCALE_CALC:
		case EVENT_ITEM_ENTER_ZONE: {
			// need a valid EQ::ItemInstance pointer check here..unsure how to cancel this process
			ExportVar(package_name.c_str(), "itemid", objid);
			ExportVar(package_name.c_str(), "itemname", item_inst->GetItem()->Name);
			break;
		}

		case EVENT_ITEM_CLICK_CAST:
		case EVENT_ITEM_CLICK: {
			// need a valid EQ::ItemInstance pointer check here..unsure how to cancel this process
			ExportVar(package_name.c_str(), "itemid", objid);
			ExportVar(package_name.c_str(), "itemname", item_inst->GetItem()->Name);
			ExportVar(package_name.c_str(), "slotid", extradata);
			ExportVar(package_name.c_str(), "spell_id", item_inst->GetItem()->Click.Effect);
			break;
		}

		case EVENT_GROUP_CHANGE: {
			if (mob && mob->IsClient()) {
				ExportVar(package_name.c_str(), "grouped", mob->IsGrouped());
				ExportVar(package_name.c_str(), "raided", mob->IsRaidGrouped());
			}
			break;
		}

		case EVENT_HATE_LIST: {
			ExportVar(package_name.c_str(), "hate_state", data);
			break;
		}

		
#ifdef BOTS
		case EVENT_SPELL_EFFECT_BUFF_TIC_BOT:
#endif
		case EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT:
		case EVENT_SPELL_EFFECT_BUFF_TIC_NPC:
#ifdef BOTS
		case EVENT_SPELL_EFFECT_BOT:
#endif
		case EVENT_SPELL_EFFECT_CLIENT:
		case EVENT_SPELL_EFFECT_NPC:
		case EVENT_SPELL_FADE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "spell_id", objid);
			ExportVar(package_name.c_str(), "caster_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "tics_remaining", sep.arg[1]);
			ExportVar(package_name.c_str(), "caster_level", sep.arg[2]);
			ExportVar(package_name.c_str(), "buff_slot", sep.arg[3]);
			break;
		}

			//tradeskill events
		case EVENT_COMBINE_SUCCESS:
		case EVENT_COMBINE_FAILURE: {
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
			ExportVar(package_name.c_str(), "clicker_id", extradata);
			break;
		}

		case EVENT_DISCOVER_ITEM: {
			ExportVar(package_name.c_str(), "itemid", extradata);
			break;
		}

		case EVENT_COMMAND: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "command", (sep.arg[0] + 1));
			ExportVar(package_name.c_str(), "args", (sep.argnum >= 1 ? (&data[strlen(sep.arg[0]) + 1]) : "0"));
			ExportVar(package_name.c_str(), "data", objid);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extradata);
			break;
		}

		case EVENT_RESPAWN: {
			ExportVar(package_name.c_str(), "option", data);
			ExportVar(package_name.c_str(), "resurrect", extradata);
			break;
		}

		case EVENT_DEATH_ZONE:
		case EVENT_DEATH:
		case EVENT_DEATH_COMPLETE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "killer_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "killer_damage", sep.arg[1]);
			ExportVar(package_name.c_str(), "killer_spell", sep.arg[2]);
			ExportVar(package_name.c_str(), "killer_skill", sep.arg[3]);
			if (extra_pointers && extra_pointers->size() >= 1)
			{
				Corpse* corpse = std::any_cast<Corpse*>(extra_pointers->at(0));
				if (corpse)
				{
					ExportVar(package_name.c_str(), "killed_corpse_id", corpse->GetID());
				}
			}
			if (extra_pointers && extra_pointers->size() >= 2)
			{
				NPC* killed = std::any_cast<NPC*>(extra_pointers->at(1));
				if (killed)
				{
					ExportVar(package_name.c_str(), "killed_npc_id", killed->GetNPCTypeID());
					ExportVar(package_name.c_str(), "killed_x", killed->GetX());
					ExportVar(package_name.c_str(), "killed_y", killed->GetY());
					ExportVar(package_name.c_str(), "killed_z", killed->GetZ());
					ExportVar(package_name.c_str(), "killed_h", killed->GetHeading());
				}
			}
			break;
		}
		case EVENT_DROP_ITEM: {
			ExportVar(package_name.c_str(), "quantity", item_inst->IsStackable() ? item_inst->GetCharges() : 1);
			ExportVar(package_name.c_str(), "itemname", item_inst->GetItem()->Name);
			ExportVar(package_name.c_str(), "itemid", item_inst->GetItem()->ID);
			ExportVar(package_name.c_str(), "spell_id", item_inst->GetItem()->Click.Effect);
			ExportVar(package_name.c_str(), "slotid", extradata);
			break;
		}
		case EVENT_SPAWN_ZONE: {
			ExportVar(package_name.c_str(), "spawned_entity_id", mob->GetID());
			ExportVar(package_name.c_str(), "spawned_npc_id", mob->GetNPCTypeID());
			break;
		}
		case EVENT_USE_SKILL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "skill_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "skill_level", sep.arg[1]);
			break;
		}
		case EVENT_COMBINE_VALIDATE: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "recipe_id", extradata);
			ExportVar(package_name.c_str(), "validate_type", sep.arg[0]);

			std::string zone_id       = "-1";
			std::string tradeskill_id = "-1";
			if (strcmp(sep.arg[0], "check_zone") == 0) {
				zone_id = sep.arg[1];
			}
			else if (strcmp(sep.arg[0], "check_tradeskill") == 0) {
				tradeskill_id = sep.arg[1];
			}

			ExportVar(package_name.c_str(), "zone_id", zone_id.c_str());
			ExportVar(package_name.c_str(), "tradeskill_id", tradeskill_id.c_str());
			break;
		}
		case EVENT_BOT_COMMAND: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "bot_command", (sep.arg[0] + 1));
			ExportVar(package_name.c_str(), "args", (sep.argnum >= 1 ? (&data[strlen(sep.arg[0]) + 1]) : "0"));
			ExportVar(package_name.c_str(), "data", objid);
			ExportVar(package_name.c_str(), "text", data);
			ExportVar(package_name.c_str(), "langid", extradata);
			break;
		}
		case EVENT_WARP: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "from_x", sep.arg[0]);
			ExportVar(package_name.c_str(), "from_y", sep.arg[1]);
			ExportVar(package_name.c_str(), "from_z", sep.arg[2]);
			break;
		}

		case EVENT_CONSIDER: {
			ExportVar(package_name.c_str(), "entity_id", std::stoi(data));
			break;
		}

		case EVENT_CONSIDER_CORPSE: {
			ExportVar(package_name.c_str(), "corpse_entity_id", std::stoi(data));
			break;
		}

		case EVENT_COMBINE: {
			ExportVar(package_name.c_str(), "container_slot", std::stoi(data));
			break;
		}

		case EVENT_EQUIP_ITEM_CLIENT:
		case EVENT_UNEQUIP_ITEM_CLIENT: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "item_id", extradata);
			ExportVar(package_name.c_str(), "item_quantity", sep.arg[0]);
			ExportVar(package_name.c_str(), "slot_id", sep.arg[1]);
			break;
		}

		case EVENT_SKILL_UP: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "skill_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "skill_value", sep.arg[1]);
			ExportVar(package_name.c_str(), "skill_max", sep.arg[2]);
			ExportVar(package_name.c_str(), "is_tradeskill", sep.arg[3]);
			break;
		}

		case EVENT_LANGUAGE_SKILL_UP: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "skill_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "skill_value", sep.arg[1]);
			ExportVar(package_name.c_str(), "skill_max", sep.arg[2]);
			break;
		}

		case EVENT_ALT_CURRENCY_MERCHANT_BUY:
		case EVENT_ALT_CURRENCY_MERCHANT_SELL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "currency_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "npc_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "merchant_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "item_id", sep.arg[3]);
			ExportVar(package_name.c_str(), "item_cost", sep.arg[4]);
			break;
		}

		case EVENT_MERCHANT_BUY:
		case EVENT_MERCHANT_SELL: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "npc_id", sep.arg[0]);
			ExportVar(package_name.c_str(), "merchant_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "item_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "item_quantity", sep.arg[3]);
			ExportVar(package_name.c_str(), "item_cost", sep.arg[4]);
			break;
		}

		case EVENT_AA_BUY: {
			Seperator sep(data);
			ExportVar(package_name.c_str(), "aa_cost", sep.arg[0]);
			ExportVar(package_name.c_str(), "aa_id", sep.arg[1]);
			ExportVar(package_name.c_str(), "aa_previous_id", sep.arg[2]);
			ExportVar(package_name.c_str(), "aa_next_id", sep.arg[3]);
			break;
		}

		case EVENT_AA_GAIN: {
			ExportVar(package_name.c_str(), "aa_gained", data);
			break;
		}

		case EVENT_INSPECT: {
			ExportVar(package_name.c_str(), "target_id", extradata);
			break;
		}

		default: {
			break;
		}
	}
}

#ifdef BOTS
void PerlembParser::LoadBotScript(std::string filename)
{
	if (!perl) {
		return;
	}

	if (bot_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_bot", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Bot Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		bot_quest_status_ = questFailedToLoad;
		return;
	}

	bot_quest_status_ = questLoaded;
}

void PerlembParser::LoadGlobalBotScript(std::string filename)
{
	if (!perl) {
		return;
	}

	if (global_bot_quest_status_ != questUnloaded) {
		return;
	}

	try {
		perl->eval_file("qst_global_bot", filename.c_str());
	} catch (std::string e) {
		AddError(
			fmt::format(
				"Error Compiling Global Bot Quest File [{}] Error [{}]",
				filename,
				e
			)
		);

		global_bot_quest_status_ = questFailedToLoad;
		return;
	}

	global_bot_quest_status_ = questLoaded;
}

bool PerlembParser::BotHasQuestSub(QuestEventID evt)
{
	if (!perl) {
		return false;
	}

	if (bot_quest_status_ != questLoaded) {
		return false;
	}

	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = QuestEventSubroutines[evt];

	return (perl->SubExists("qst_bot", subname));
}

bool PerlembParser::GlobalBotHasQuestSub(QuestEventID evt)
{
	if (!perl) {
		return false;
	}

	if (global_bot_quest_status_ != questLoaded) {
		return false;
	}

	if (evt >= _LargestEventID) {
		return false;
	}

	const char *subname = QuestEventSubroutines[evt];

	return (perl->SubExists("qst_global_bot", subname));
}

int PerlembParser::EventBot(
	QuestEventID evt,
	Bot *bot,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	return EventCommon(evt, 0, data.c_str(), bot, nullptr, nullptr, mob, extra_data, false, extra_pointers);
}

int PerlembParser::EventGlobalBot(
	QuestEventID evt,
	Bot *bot,
	Mob *mob,
	std::string data,
	uint32 extra_data,
	std::vector<std::any> *extra_pointers
) {
	return EventCommon(evt, 0, data.c_str(), bot, nullptr, nullptr, mob, extra_data, true, extra_pointers);
}
#endif

#endif
