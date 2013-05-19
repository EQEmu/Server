#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/iterator_policy.hpp>
#include <boost/any.hpp>
#include <ctype.h>
#include <stdio.h>
#include <sstream>
#include <vector>

#include "masterentity.h"
#include "../common/spdat.h"
#include "../common/seperator.h"
#include "lua_entity.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_hate_entry.h"
#include "lua_hate_list.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_spell.h"
#include "lua_general.h"
#include "zone.h"

#include "lua_parser.h"

const char *LuaEvents[_LargestEventID] = {
	"event_say",
	"event_trade",
	"event_death",
	"event_spawn",
	"event_attack",
	"event_combat",
	"event_aggro",
	"event_slay",
	"event_npc_slay",
	"event_waypoint_arrive",
	"event_waypoint_depart",
	"event_timer",
	"event_signal",
	"event_hp",
	"event_enter",
	"event_exit",
	"event_enter_zone",
	"event_click_door",
	"event_loot",
	"event_zone",
	"event_level_up",
	"event_killed_merit",
	"event_cast_on",
	"event_task_accepted",
	"event_task_stage_complete",
	"event_task_update",
	"event_task_complete",
	"event_task_fail",
	"event_aggro_say",
	"event_player_pickup",
	"event_popup_response",
	"event_proximity_say",
	"event_cast",
	"event_scale_calc",
	"event_item_enter_zone",
	"event_target_change",
	"event_hate_list",
	"event_spell_effect_client",
	"event_spell_effect_npc",
	"event_spell_effect_buff_tic_client",
	"event_spell_effect_buff_tic_npc",
	"event_spell_effect_translocate_complete",
	"event_combine_success",
	"event_combine_failure",
	"event_item_click",
	"event_item_click_cast",
	"event_group_change",
	"event_forage_success",
	"event_forage_failure",
	"event_fish_start",
	"event_fish_success",
	"event_fish_failure",
	"event_click_object",
	"event_discover_item",
	"event_disconnect",
	"event_connect",
	"event_item_tick",
	"event_duel_win",
	"event_duel_lose",
	"event_encounter_load",
	"event_encounter_unload"
};

extern Zone *zone;

struct lua_registered_event {
	std::string encounter_name;
	luabind::object lua_reference;
	QuestEventID event_id;
};

std::map<std::string, std::list<lua_registered_event>> lua_encounter_events_registered;

LuaParser::LuaParser() {
	for(int i = 0; i < _LargestEventID; ++i) {
		NPCArgumentDispatch[i] = handle_npc_null;
		PlayerArgumentDispatch[i] = handle_player_null;
		ItemArgumentDispatch[i] = handle_item_null;
		SpellArgumentDispatch[i] = handle_spell_null;
	}

	NPCArgumentDispatch[EVENT_SAY] = handle_npc_event_say;
	NPCArgumentDispatch[EVENT_AGGRO_SAY] = handle_npc_event_say;
	NPCArgumentDispatch[EVENT_PROXIMITY_SAY] = handle_npc_event_say;
	NPCArgumentDispatch[EVENT_TRADE] = handle_npc_event_trade;
	NPCArgumentDispatch[EVENT_HP] = handle_npc_event_hp;
	NPCArgumentDispatch[EVENT_ATTACK] = handle_npc_single_mob;
	NPCArgumentDispatch[EVENT_AGGRO] = handle_npc_single_mob;
	NPCArgumentDispatch[EVENT_TARGET_CHANGE] = handle_npc_single_mob;
	NPCArgumentDispatch[EVENT_CAST_ON] = handle_npc_single_mob;
	NPCArgumentDispatch[EVENT_KILLED_MERIT] = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_SLAY] = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_ENTER] = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_EXIT] = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_TASK_ACCEPTED] = handle_npc_single_client;
	NPCArgumentDispatch[EVENT_NPC_SLAY] = handle_npc_single_npc;
	NPCArgumentDispatch[EVENT_POPUP_RESPONSE] = handle_npc_popup;
	NPCArgumentDispatch[EVENT_WAYPOINT_ARRIVE] = handle_npc_waypoint;
	NPCArgumentDispatch[EVENT_WAYPOINT_DEPART] = handle_npc_waypoint;
	NPCArgumentDispatch[EVENT_HATE_LIST] = handle_npc_hate;
	NPCArgumentDispatch[EVENT_COMBAT] = handle_npc_hate;
	NPCArgumentDispatch[EVENT_SIGNAL] = handle_npc_signal;
	NPCArgumentDispatch[EVENT_TIMER] = handle_npc_timer;
	NPCArgumentDispatch[EVENT_DEATH] = handle_npc_death;

	L = nullptr;
}

LuaParser::~LuaParser() {
	if(L) {
		lua_close(L);
	}
}

int LuaParser::EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc) {
		return 0;
	}

	if(!HasQuestSub(npc->GetNPCTypeID(), LuaEvents[evt])) {
		return 0;
	}

	std::stringstream package_name;
	package_name << "npc_" << npc->GetNPCTypeID();
	
	return _EventNPC(package_name.str(), evt, npc, init, data, extra_data);
}

int LuaParser::EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc) {
		return 0;
	}

	if(!HasGlobalQuestSub(LuaEvents[evt])) {
		return 0;
	}

	return _EventNPC("global_npc", evt, npc, init, data, extra_data);
}

int LuaParser::_EventNPC(std::string package_name, QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						 luabind::object *l_func) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(L);

	try {
		int npop = 1;
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 2;
		}
		
		lua_createtable(L, 0, 0);
		//always push self
		Lua_NPC l_npc(npc);
		luabind::object l_npc_o = luabind::object(L, l_npc);
		l_npc_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = NPCArgumentDispatch[evt];
		arg_function(this, L, npc, init, data, extra_data);
		
		if(lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}
		
		lua_pop(L, npop);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!client) {
		return 0;
	}

	if(!PlayerHasQuestSub(LuaEvents[evt])) {
		return 0;
	}

	return _EventPlayer("player", evt, client, data, extra_data);
}

int LuaParser::EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!client) {
		return 0;
	}

	if(!GlobalPlayerHasQuestSub(LuaEvents[evt])) {
		return 0;
	}

	return _EventPlayer("global_player", evt, client, data, extra_data);
}

int LuaParser::_EventPlayer(std::string package_name, QuestEventID evt, Client *client, std::string data, uint32 extra_data,
							luabind::object *l_func) {
	const char *sub_name = LuaEvents[evt];
	int start = lua_gettop(L);

	try {
		int npop = 1;
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 2;
		}
	
		lua_createtable(L, 0, 0);
		//push self
		Lua_Client l_client(client);
		luabind::object l_client_o = luabind::object(L, l_client);
		l_client_o.push(L);
		lua_setfield(L, -2, "self");
		
		auto arg_function = PlayerArgumentDispatch[evt];
		arg_function(this, L, client, data, extra_data);
	
		if(lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}
		
		lua_pop(L, npop);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) {
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!item) {
		return 0;
	}

	if(!ItemHasQuestSub(item, LuaEvents[evt])) {
		return 0;
	}

	std::stringstream package_name;
	package_name << "item_";

	std::stringstream item_name;
	const Item_Struct* itm = item->GetItem();
	if(evt == EVENT_SCALE_CALC || evt == EVENT_ITEM_ENTER_ZONE)
	{
		item_name << itm->CharmFile;
	}
	else if(evt == EVENT_ITEM_CLICK || evt == EVENT_ITEM_CLICK_CAST)
	{
		item_name << "script_";
		item_name << itm->ScriptFileID;
	}
	else
	{
		item_name << "item_";
		item_name << itm->ID;
	}
	package_name << item_name;

	return _EventItem(package_name.str(), evt, client, item, objid, extra_data);
}

int LuaParser::_EventItem(std::string package_name, QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data,
						  luabind::object *l_func) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(L);

	try {
		int npop = 1;
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
		}
		
		lua_createtable(L, 0, 0);
		//always push self
		Lua_ItemInst l_item(item);
		luabind::object l_item_o = luabind::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = ItemArgumentDispatch[evt];
		arg_function(this, L, client, item, objid, extra_data);
		
		if(lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}
		
		lua_pop(L, npop);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) {
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	if(!SpellHasQuestSub(spell_id, LuaEvents[evt])) {
		return 0;
	}
	
	return _EventSpell(package_name.str(), evt, npc, client, spell_id, extra_data);
}

int LuaParser::_EventSpell(std::string package_name, QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
						   luabind::object *l_func) {
	const char *sub_name = LuaEvents[evt];
	
	int start = lua_gettop(L);

	try {
		int npop = 1;
		if(l_func != nullptr) {
			l_func->push(L);
		} else {
			lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
			lua_getfield(L, -1, sub_name);
			npop = 2;
		}
		
		lua_createtable(L, 0, 0);

		//always push self
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "self");
		
		auto arg_function = SpellArgumentDispatch[evt];
		arg_function(this, L, npc, client, spell_id, extra_data);
		
		if(lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, npop);
			return ret;
		}
		
		lua_pop(L, npop);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

int LuaParser::EventEncounter(QuestEventID evt, std::string encounter_name, uint32 extra_data) {
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "encounter_" + encounter_name;

	if(!EncounterHasQuestSub(encounter_name, LuaEvents[evt])) {
		return 0;
	}
	
	return _EventEncounter(package_name, evt, encounter_name, extra_data);
}

int LuaParser::_EventEncounter(std::string package_name, QuestEventID evt, std::string encounter_name, uint32 extra_data) {
	const char *sub_name = LuaEvents[evt];
	
	int start = lua_gettop(L);

	try {
		lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
		lua_getfield(L, -1, sub_name);
	
		//For now encounters just call event_encounter_load/event_encounter_unload with no arguments
		//So we dont pass anything

		if(lua_pcall(L, 0, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, 2);
			return ret;
		}
		
		lua_pop(L, 2);
	} catch(std::exception &ex) {
		std::string error = "Lua Exception: ";
		error += std::string(ex.what());
		AddError(error);

		//Restore our stack to the best of our ability
		int end = lua_gettop(L);
		int n = end - start;
		if(n > 0) {
			lua_pop(L, n);
		}
	}

	return 0;
}

bool LuaParser::HasQuestSub(uint32 npc_id, const char *subname) {
	std::stringstream package_name;
	package_name << "npc_" << npc_id;

	return HasFunction(subname, package_name.str());
}

bool LuaParser::HasGlobalQuestSub(const char *subname) {
	return HasFunction(subname, "global_npc");
}

bool LuaParser::PlayerHasQuestSub(const char *subname) {
	return HasFunction(subname, "player");
}

bool LuaParser::GlobalPlayerHasQuestSub(const char *subname) {
	return HasFunction(subname, "global_player");
}

bool LuaParser::SpellHasQuestSub(uint32 spell_id, const char *subname) {
	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	return HasFunction(subname, package_name.str());
}

bool LuaParser::ItemHasQuestSub(ItemInst *itm, const char *subname) {
	std::stringstream package_name;
	package_name << "item_";

	std::stringstream item_name;
	const Item_Struct* item = itm->GetItem();
	if(strcmp("EVENT_SCALE_CALC", subname) == 0 || strcmp("EVENT_ITEM_ENTER_ZONE", subname) == 0)
	{
		item_name << item->CharmFile;
	}
	else if(strcmp("EVENT_ITEM_CLICK", subname) == 0 || strcmp("EVENT_ITEM_CLICK_CAST", subname) == 0 )
	{
		item_name << "script_";
		item_name << item->ScriptFileID;
	}
	else
	{
		item_name << "item_";
		item_name << item->ID;
	}

	package_name << item_name;
	return HasFunction(subname, package_name.str());
}

bool LuaParser::EncounterHasQuestSub(std::string encounter_name, const char *subname) {
	std::string package_name = "encounter_" + encounter_name;

	return HasFunction(subname, package_name);
}

void LuaParser::LoadNPCScript(std::string filename, int npc_id) {
	std::stringstream package_name;
	package_name << "npc_" << npc_id;

	LoadScript(filename, package_name.str());
}

void LuaParser::LoadGlobalNPCScript(std::string filename) {
	LoadScript(filename, "global_npc");
}

void LuaParser::LoadPlayerScript(std::string filename) {
	LoadScript(filename, "player");
}

void LuaParser::LoadGlobalPlayerScript(std::string filename) {
	LoadScript(filename, "global_player");
}

void LuaParser::LoadItemScript(std::string filename, std::string item_script) {
	std::stringstream package_name;
	package_name << "item_" << item_script;

	LoadScript(filename, package_name.str());
}

void LuaParser::LoadSpellScript(std::string filename, uint32 spell_id) {
	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	LoadScript(filename, package_name.str());
}

void LuaParser::LoadEncounterScript(std::string filename, std::string encounter_name) {
	std::string package_name = "encounter_" + encounter_name;

	LoadScript(filename, package_name);
}

void LuaParser::AddVar(std::string name, std::string val) {
	vars_[name] = val;
}

std::string LuaParser::GetVar(std::string name) {
	auto iter = vars_.find(name);
	if(iter != vars_.end()) {
		return iter->second;
	}

	return std::string();
}

void LuaParser::ReloadQuests() {
	loaded_.clear();
	errors_.clear();
	lua_encounter_events_registered.clear();

	if(L) {
		lua_close(L);
	}

	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushnil(L);
	lua_setglobal(L, "os");

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	std::string module_path = lua_tostring(L,-1);
	module_path += "quests/plugins/?.lua";
	lua_pop(L, 1);
	lua_pushstring(L, module_path.c_str());
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	MapFunctions(L);

	//load init
	FILE *f = fopen("quests/global/script_init.lua", "r");
	if(f) {
		fclose(f);
	
		if(luaL_dofile(L, "quests/global/script_init.lua")) {
			std::string error = lua_tostring(L, -1);
			AddError(error);
		}
	}
	
	//zone init - always loads after global
	if(zone) {
		std::string zone_script = "quests/" + std::string(zone->GetShortName());
		zone_script += "/script_init.lua";
		f = fopen(zone_script.c_str(), "r");
		if(f) {
			fclose(f);
		
			if(luaL_dofile(L, zone_script.c_str())) {
				std::string error = lua_tostring(L, -1);
				AddError(error);
			}
		}
	}
}

void LuaParser::LoadScript(std::string filename, std::string package_name) {
	auto iter = loaded_.find(package_name);
	if(iter != loaded_.end()) {
		return;
	}
	
	if(luaL_loadfile(L, filename.c_str())) {
		std::string error = lua_tostring(L, -1);
		AddError(error);
		lua_pop(L, 1);
		return;
	}

	//This makes an env table named: package_name
	//And makes it so we can see the global table _G from it
	//Then sets it so this script is called from that table as an env

	lua_createtable(L, 0, 0); // anon table
	lua_getglobal(L, "_G"); // get _G
	lua_setfield(L, -2, "__index"); //anon table.__index = _G

	lua_pushvalue(L, -1); //copy table to top of stack
	lua_setmetatable(L, -2); //setmetatable(anon_table, copied table)

	lua_pushvalue(L, -1); //put the table we made into the registry
	lua_setfield(L, LUA_REGISTRYINDEX, package_name.c_str());

	lua_setfenv(L, -2); //set the env to the table we made

	if(lua_pcall(L, 0, 0, 0)) {
		std::string error = lua_tostring(L, -1);
		AddError(error);
		lua_pop(L, 1);
		return;
	}

	loaded_[package_name] = true;
}

bool LuaParser::HasFunction(std::string subname, std::string package_name) {
	size_t sz = subname.length();
	for(size_t i = 0; i < sz; ++i) {
		char c = subname[i];
		if(65 <= c && c <= 90) {
			c += 32;
		}
		subname[i] = c;
	}

	auto iter = loaded_.find(package_name);
	if(iter == loaded_.end()) {
		return false;
	}

	lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
	lua_getfield(L, -1, subname.c_str());

	if(lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return true;
	}

	lua_pop(L, 2);
	return false;
}

void LuaParser::MapFunctions(lua_State *L) {

	try {
		luabind::open(L);

		luabind::module(L)
		[
			lua_register_general(),
			lua_register_events(),
			lua_register_entity(),
			lua_register_mob(),
			lua_register_npc(),
			lua_register_client(),
			lua_register_iteminst(),
			lua_register_item(),
			lua_register_spell(),

			luabind::class_<Lua_HateEntry>("HateEntry")
				.property("null", &Lua_HateEntry::Null)
				.property("valid", &Lua_HateEntry::Valid)
				.property("ent", &Lua_HateEntry::GetEnt, &Lua_HateEntry::SetEnt)
				.property("damage", &Lua_HateEntry::GetDamage, &Lua_HateEntry::SetDamage)
				.property("hate", &Lua_HateEntry::GetHate, &Lua_HateEntry::SetHate)
				.property("frenzy", &Lua_HateEntry::GetFrenzy, &Lua_HateEntry::SetFrenzy),

			luabind::class_<Lua_HateList>("HateList")
				.def_readwrite("entries", &Lua_HateList::entries, luabind::return_stl_iterator),

			luabind::class_<Lua_Mob::Lua_Illusion>("Illusion")
				.def(luabind::constructor<>())
				.def_readwrite("race", &Lua_Mob::Lua_Illusion::in_race)
				.def_readwrite("gender", &Lua_Mob::Lua_Illusion::in_gender)
				.def_readwrite("texture", &Lua_Mob::Lua_Illusion::in_texture)
				.def_readwrite("helmtexture", &Lua_Mob::Lua_Illusion::in_helmtexture)
				.def_readwrite("haircolor", &Lua_Mob::Lua_Illusion::in_haircolor)
				.def_readwrite("beardcolor", &Lua_Mob::Lua_Illusion::in_beardcolor)
				.def_readwrite("eyecolor1", &Lua_Mob::Lua_Illusion::in_eyecolor1)
				.def_readwrite("eyecolor2", &Lua_Mob::Lua_Illusion::in_eyecolor2)
				.def_readwrite("hairstyle", &Lua_Mob::Lua_Illusion::in_hairstyle)
				.def_readwrite("luclinface", &Lua_Mob::Lua_Illusion::in_luclinface)
				.def_readwrite("beard", &Lua_Mob::Lua_Illusion::in_beard)
				.def_readwrite("aa_title", &Lua_Mob::Lua_Illusion::in_aa_title)
				.def_readwrite("drakkin_heritage", &Lua_Mob::Lua_Illusion::in_drakkin_heritage)
				.def_readwrite("drakkin_tattoo", &Lua_Mob::Lua_Illusion::in_drakkin_tattoo)
				.def_readwrite("drakkin_details", &Lua_Mob::Lua_Illusion::in_drakkin_details)
				.def_readwrite("size", &Lua_Mob::Lua_Illusion::in_size)
		];
	
	} catch(std::exception &ex) {
		std::string error = ex.what();
		AddError(error);
	}
}

void LuaParser::GetErrors(std::list<std::string> &err) {
	err.insert(err.end(), errors_.begin(), errors_.end());
}

void LuaParser::AddError(std::string error) {
	errors_.push_back(error);
	if(errors_.size() > 30) {
		errors_.pop_front();
	}
}

void LuaParser::DispatchEventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	if(!npc)
		return;

	std::stringstream package_name;
	package_name << "npc_" << npc->GetNPCTypeID();

	auto iter = lua_encounter_events_registered.find(package_name.str());
	if(iter == lua_encounter_events_registered.end()) {
		return;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			_EventNPC(package_name, evt, npc, init, data, extra_data, &riter->lua_reference);
		}
		++riter;
	}
}

void LuaParser::DispatchEventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	std::string package_name = "player";

	auto iter = lua_encounter_events_registered.find(package_name);
	if(iter == lua_encounter_events_registered.end()) {
		return;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			_EventPlayer(package_name, evt, client, data, extra_data, &riter->lua_reference);
		}
		++riter;
	}
}

void LuaParser::DispatchEventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) {
	if(!item)
		return;

	std::stringstream package_name;
	package_name << "item_";

	std::stringstream item_name;
	const Item_Struct* itm = item->GetItem();
	if(evt == EVENT_SCALE_CALC || evt == EVENT_ITEM_ENTER_ZONE)
	{
		item_name << itm->CharmFile;
	}
	else if(evt == EVENT_ITEM_CLICK || evt == EVENT_ITEM_CLICK_CAST)
	{
		item_name << "script_";
		item_name << itm->ScriptFileID;
	}
	else
	{
		item_name << "item_";
		item_name << itm->ID;
	}
	package_name << item_name;

	auto iter = lua_encounter_events_registered.find(package_name.str());
	if(iter == lua_encounter_events_registered.end()) {
		return;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			_EventItem(package_name, evt, client, item, objid, extra_data, &riter->lua_reference);
		}
		++riter;
	}
}

void LuaParser::DispatchEventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) {
	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	auto iter = lua_encounter_events_registered.find(package_name.str());
	if(iter == lua_encounter_events_registered.end()) {
		return;
	}

	auto riter = iter->second.begin();
	while(riter != iter->second.end()) {
		if(riter->event_id == evt) {
			std::string package_name = "encounter_" + riter->encounter_name;
			_EventSpell(package_name, evt, npc, client, spell_id, extra_data, &riter->lua_reference);
		}
		++riter;
	}
}

#endif
