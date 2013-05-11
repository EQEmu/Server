#ifdef LUA_EQEMU

#include "lua_parser.h"
#include <ctype.h>
#include <sstream>

#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <boost/any.hpp>
//#include <LuaBridge.h>

#include "masterentity.h"
#include "lua_entity.h"
#include "lua_mob.h"

const char *LuaEvents[_LargestEventID] = {
	"event_say",
	"event_item",
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
	"event_enterzone",
	"event_clickdoor",
	"event_loot",
	"event_zone",
	"event_level_up",
	"event_killed_merit",
	"event_cast_on",
	"event_taskaccepted",
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
	"event_item_enterzone",
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
	"event_duel_lose"
};

LuaParser::LuaParser() {
}

LuaParser::~LuaParser() {
	ClearStates();
}

double LuaParser::EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	if(evt >= _LargestEventID) {
		return 100.0;
	}

	if(!npc) {
		return 100.0;
	}

	std::stringstream package_name;
	package_name << "npc_" << npc->GetNPCTypeID();
	
	lua_State *L = nullptr;
	auto iter = states_.find(package_name.str());
	if(iter == states_.end()) {
		return 100.0;
	}
	L = iter->second;
	
	Lua_Entity ent(npc);

	try {
		double val = luabind::call_function<double>(L, LuaEvents[evt], ent);
		return val;
	} catch(std::exception) {
		return 100.0;
	}

	return 100.0;
}

double LuaParser::EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	return 100.0;
}

double LuaParser::EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	return 100.0;
}

double LuaParser::EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	return 100.0;
}

double LuaParser::EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) {
	return 100.0;
}

double LuaParser::EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) {
	return 100.0;
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
	if(strcmp("EVENT_SCALE_CALC", subname) == 0 || strcmp("EVENT_ITEM_ENTERZONE", subname) == 0)
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
	ClearStates();
}

void LuaParser::LoadScript(std::string filename, std::string package_name) {
	auto iter = states_.find(package_name);
	if(iter != states_.end()) {
		return;
	}

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	//lua_pushnil(L);
	//lua_setglobal(L, "os");
	//
	//lua_pushnil(L);
	//lua_setglobal(L, "io");

	MapFunctions(L);

	if(luaL_dofile(L, filename.c_str())) {
		printf("Lua Error: %s\n", lua_tostring(L, -1));
		lua_close(L);
		return;
	}

	states_[package_name] = L;
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

	auto iter = states_.find(package_name);
	if(iter == states_.end()) {
		return false;
	}

	lua_getfield(iter->second, LUA_GLOBALSINDEX, subname.c_str());
	if(lua_isfunction(iter->second, -1)) {
		return true;
	}

	return false;
}

void LuaParser::ClearStates() {
	auto iter = states_.begin();
	while(iter != states_.end()) {
		if(iter->second) {
			lua_close(iter->second);
		}
		++iter;
	}

	states_.clear();
}

void LuaParser::MapFunctions(lua_State *L) {

	try {
		luabind::open(L);

		luabind::module(L)
		[
			luabind::class_<Lua_Entity>("Entity")
				.def("IsClient", &Lua_Entity::IsClient)
				.def("IsNPC", &Lua_Entity::IsNPC)
				.def("IsMob", &Lua_Entity::IsMob)
				.def("IsMerc", &Lua_Entity::IsMerc)
				.def("IsCorpse", &Lua_Entity::IsCorpse)
				.def("IsPlayerCorpse", &Lua_Entity::IsPlayerCorpse)
				.def("IsNPCCorpse", &Lua_Entity::IsNPCCorpse)
				.def("IsObject", &Lua_Entity::IsObject)
				.def("IsDoor", &Lua_Entity::IsDoor)
				.def("IsTrap", &Lua_Entity::IsTrap)
				.def("IsBeacon", &Lua_Entity::IsBeacon)
				.def("GetID", &Lua_Entity::GetID)
				.def("CastToMob", &Lua_Entity::CastToMob),
			luabind::class_<Lua_Mob>("Mob")
				.def("GetName", &Lua_Mob::GetName)
				.def("Depop", (void(Lua_Mob::*)(void))&Lua_Mob::Depop)
				.def("Depop", (void(Lua_Mob::*)(bool))&Lua_Mob::Depop)
		];
	
	} catch(std::exception &ex) {
		printf("Error: %s\n", ex.what());
	}
}

#endif
