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
#include "lua_mob.h"
#include "lua_hate_entry.h"
#include "lua_hate_list.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_spell.h"
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
	"event_duel_lose"
};

extern Zone *zone;

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

int LuaParser::_EventNPC(std::string package_name, QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(L);

	try {
		lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
		lua_getfield(L, -1, sub_name);
		
		lua_createtable(L, 0, 0);
		//always push self
		Lua_NPC l_npc(npc);
		luabind::object l_npc_o = luabind::object(L, l_npc);
		l_npc_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = NPCArgumentDispatch[evt];
		arg_function(this, L, npc, init, data, extra_data);
		
		if(lua_pcall(L, 1, 1, 0)) {
			printf("Error: %s\n", lua_tostring(L, -1));
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, 2);
			return ret;
		}
		
		lua_pop(L, 2);
	} catch(std::exception &ex) {
		printf("Lua call exception: %s\n", ex.what());

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

int LuaParser::_EventPlayer(std::string package_name, QuestEventID evt, Client *client, std::string data, uint32 extra_data) {
	const char *sub_name = LuaEvents[evt];
	int start = lua_gettop(L);

	try {
		lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
		lua_getfield(L, -1, sub_name);
	
		lua_createtable(L, 0, 0);
		//push self
		Lua_Client l_client(client);
		luabind::object l_client_o = luabind::object(L, l_client);
		l_client_o.push(L);
		lua_setfield(L, -2, "self");
		
		auto arg_function = PlayerArgumentDispatch[evt];
		arg_function(this, L, client, data, extra_data);
	
		if(lua_pcall(L, 1, 1, 0)) {
			printf("Error: %s\n", lua_tostring(L, -1));
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, 2);
			return ret;
		}
		
		lua_pop(L, 2);
	} catch(std::exception &ex) {
		printf("Lua call exception: %s\n", ex.what());

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

int LuaParser::_EventItem(std::string package_name, QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) {
	const char *sub_name = LuaEvents[evt];

	int start = lua_gettop(L);

	try {
		lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
		lua_getfield(L, -1, sub_name);
		
		lua_createtable(L, 0, 0);
		//always push self
		Lua_ItemInst l_item(item);
		luabind::object l_item_o = luabind::object(L, l_item);
		l_item_o.push(L);
		lua_setfield(L, -2, "self");

		auto arg_function = ItemArgumentDispatch[evt];
		arg_function(this, L, client, item, objid, extra_data);
		
		if(lua_pcall(L, 1, 1, 0)) {
			printf("Error: %s\n", lua_tostring(L, -1));
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, 2);
			return ret;
		}
		
		lua_pop(L, 2);
	} catch(std::exception &ex) {
		printf("Lua call exception: %s\n", ex.what());

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

int LuaParser::_EventSpell(std::string package_name, QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) {
	const char *sub_name = LuaEvents[evt];
	
	int start = lua_gettop(L);

	try {
		lua_getfield(L, LUA_REGISTRYINDEX, package_name.c_str());
		lua_getfield(L, -1, sub_name);
		
		lua_createtable(L, 0, 0);

		//always push self
		Lua_Spell l_spell(&spells[spell_id]);
		luabind::object l_spell_o = luabind::object(L, l_spell);
		l_spell_o.push(L);
		lua_setfield(L, -2, "self");
		
		auto arg_function = SpellArgumentDispatch[evt];
		arg_function(this, L, npc, client, spell_id, extra_data);
		
		if(lua_pcall(L, 1, 1, 0)) {
			printf("Error: %s\n", lua_tostring(L, -1));
			return 0;
		}
		
		if(lua_isnumber(L, -1)) {
			int ret = static_cast<int>(lua_tointeger(L, -1));
			lua_pop(L, 2);
			return ret;
		}
		
		lua_pop(L, 2);
	} catch(std::exception &ex) {
		printf("Lua call exception: %s\n", ex.what());

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
	if(L) {
		lua_close(L);
	}

	loaded_.clear();

	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushnil(L);
	lua_setglobal(L, "os");

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	char module_path[1024] = { 0 };
	snprintf(module_path, 1023, "%s;%s", lua_tostring(L,-1), "quests/plugins/?.lua");
	lua_pop(L, 1);
	lua_pushstring(L, module_path);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	//load init
	FILE *f = fopen("quests/global/script_init.lua", "r");
	if(f) {
		fclose(f);
	
		if(luaL_dofile(L, "quests/global/script_init.lua")) {
			printf("Lua Error in Global Init: %s\n", lua_tostring(L, -1));
			lua_close(L);
			return;
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
				printf("Lua Error in Zone Init: %s\n", lua_tostring(L, -1));
				lua_close(L);
				return;
			}
		}
	}
	
	MapFunctions(L);
}

void LuaParser::LoadScript(std::string filename, std::string package_name) {
	auto iter = loaded_.find(package_name);
	if(iter != loaded_.end()) {
		return;
	}
	
	if(luaL_loadfile(L, filename.c_str())) {
		printf("Lua Load Error: %s\n", lua_tostring(L, -1));
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
		printf("Lua Load Error: %s\n", lua_tostring(L, -1));
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
			luabind::class_<Lua_Entity>("Entity")
				.def(luabind::constructor<>())
				.property("null", &Lua_Entity::Null)
				.property("valid", &Lua_Entity::Valid)
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
				.def("CastToClient", &Lua_Entity::CastToClient)
				.def("CastToNPC", &Lua_Entity::CastToNPC)
				.def("CastToMob", &Lua_Entity::CastToMob),

			luabind::class_<Lua_Mob, Lua_Entity>("Mob")
				.def(luabind::constructor<>())
				.def("GetName", &Lua_Mob::GetName)
				.def("Depop", (void(Lua_Mob::*)(void))&Lua_Mob::Depop)
				.def("Depop", (void(Lua_Mob::*)(bool))&Lua_Mob::Depop)
				.def("BehindMob", (bool(Lua_Mob::*)(void))&Lua_Mob::BehindMob)
				.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::BehindMob)
				.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob,float))&Lua_Mob::BehindMob)
				.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob,float,float))&Lua_Mob::BehindMob)
				.def("SetLevel", (void(Lua_Mob::*)(int))&Lua_Mob::SetLevel)
				.def("SetLevel", (void(Lua_Mob::*)(int,bool))&Lua_Mob::SetLevel)
				.def("IsMoving", &Lua_Mob::IsMoving)
				.def("GotoBind", &Lua_Mob::GotoBind)
				.def("Attack", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::Attack)
				.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int))&Lua_Mob::Attack)
				.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool))&Lua_Mob::Attack)
				.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool,bool))&Lua_Mob::Attack)
				.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool,bool,bool))&Lua_Mob::Attack)
				.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int,int,int))&Lua_Mob::Damage)
				.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,bool))&Lua_Mob::Damage)
				.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,bool,int))&Lua_Mob::Damage)
				.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,bool,int,bool))&Lua_Mob::Damage)
				.def("RangedAttack", &Lua_Mob::RangedAttack)
				.def("ThrowingAttack", &Lua_Mob::ThrowingAttack)
				.def("Heal", &Lua_Mob::Heal)
				.def("HealDamage", (void(Lua_Mob::*)(uint32))&Lua_Mob::HealDamage)
				.def("HealDamage", (void(Lua_Mob::*)(uint32,Lua_Mob))&Lua_Mob::HealDamage)
				.def("GetLevelCon", (uint32(Lua_Mob::*)(int))&Lua_Mob::GetLevelCon)
				.def("GetLevelCon", (uint32(Lua_Mob::*)(int,int))&Lua_Mob::GetLevelCon)
				.def("SetHP", &Lua_Mob::SetHP)
				.def("DoAnim", (void(Lua_Mob::*)(int))&Lua_Mob::DoAnim)
				.def("DoAnim", (void(Lua_Mob::*)(int,int))&Lua_Mob::DoAnim)
				.def("DoAnim", (void(Lua_Mob::*)(int,int,bool))&Lua_Mob::DoAnim)
				.def("DoAnim", (void(Lua_Mob::*)(int,int,bool,int))&Lua_Mob::DoAnim)
				.def("ChangeSize", (void(Lua_Mob::*)(double))&Lua_Mob::ChangeSize)
				.def("ChangeSize", (void(Lua_Mob::*)(double,bool))&Lua_Mob::ChangeSize)
				.def("GMMove", (void(Lua_Mob::*)(double,double,double))&Lua_Mob::GMMove)
				.def("GMMove", (void(Lua_Mob::*)(double,double,double,double))&Lua_Mob::GMMove)
				.def("GMMove", (void(Lua_Mob::*)(double,double,double,double,bool))&Lua_Mob::GMMove)
				.def("HasProcs", &Lua_Mob::HasProcs)
				.def("IsInvisible", (bool(Lua_Mob::*)(void))&Lua_Mob::IsInvisible)
				.def("IsInvisible", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::IsInvisible)
				.def("SetInvisible", &Lua_Mob::SetInvisible)
				.def("FindBuff", &Lua_Mob::FindBuff)
				.def("FindType", (bool(Lua_Mob::*)(int))&Lua_Mob::FindType)
				.def("FindType", (bool(Lua_Mob::*)(int,bool))&Lua_Mob::FindType)
				.def("FindType", (bool(Lua_Mob::*)(int,bool,int))&Lua_Mob::FindType)
				.def("GetBuffSlotFromType", &Lua_Mob::GetBuffSlotFromType)
				.def("GetBaseRace", &Lua_Mob::GetBaseRace)
				.def("GetBaseGender", &Lua_Mob::GetBaseGender)
				.def("GetDeity", &Lua_Mob::GetDeity)
				.def("GetRace", &Lua_Mob::GetRace)
				.def("GetGender", &Lua_Mob::GetGender)
				.def("GetTexture", &Lua_Mob::GetTexture)
				.def("GetHelmTexture", &Lua_Mob::GetHelmTexture)
				.def("GetHairColor", &Lua_Mob::GetHairColor)
				.def("GetBeardColor", &Lua_Mob::GetBeardColor)
				.def("GetEyeColor1", &Lua_Mob::GetEyeColor1)
				.def("GetEyeColor2", &Lua_Mob::GetEyeColor2)
				.def("GetHairStyle", &Lua_Mob::GetHairStyle)
				.def("GetLuclinFace", &Lua_Mob::GetLuclinFace)
				.def("GetBeard", &Lua_Mob::GetBeard)
				.def("GetDrakkinHeritage", &Lua_Mob::GetDrakkinHeritage)
				.def("GetDrakkinTattoo", &Lua_Mob::GetDrakkinTattoo)
				.def("GetDrakkinDetails", &Lua_Mob::GetDrakkinDetails)
				.def("GetClass", &Lua_Mob::GetClass)
				.def("GetLevel", &Lua_Mob::GetLevel)
				.def("GetCleanName", &Lua_Mob::GetCleanName)
				.def("GetTarget", &Lua_Mob::GetTarget)
				.def("SetTarget", &Lua_Mob::SetTarget)
				.def("GetHPRatio", &Lua_Mob::GetHPRatio)
				.def("IsWarriorClass", &Lua_Mob::IsWarriorClass)
				.def("GetHP", &Lua_Mob::GetHP)
				.def("GetMaxHP", &Lua_Mob::GetMaxHP)
				.def("GetItemHPBonuses", &Lua_Mob::GetItemHPBonuses)
				.def("GetSpellHPBonuses", &Lua_Mob::GetSpellHPBonuses)
				.def("GetWalkspeed", &Lua_Mob::GetWalkspeed)
				.def("GetRunspeed", &Lua_Mob::GetRunspeed)
				.def("GetCasterLevel", &Lua_Mob::GetCasterLevel)
				.def("GetMaxMana", &Lua_Mob::GetMaxMana)
				.def("GetMana", &Lua_Mob::GetMana)
				.def("SetMana", &Lua_Mob::SetMana)
				.def("GetManaRatio", &Lua_Mob::GetManaRatio)
				.def("GetAC", &Lua_Mob::GetAC)
				.def("GetATK", &Lua_Mob::GetATK)
				.def("GetSTR", &Lua_Mob::GetSTR)
				.def("GetSTA", &Lua_Mob::GetSTA)
				.def("GetDEX", &Lua_Mob::GetDEX)
				.def("GetAGI", &Lua_Mob::GetAGI)
				.def("GetINT", &Lua_Mob::GetINT)
				.def("GetWIS", &Lua_Mob::GetWIS)
				.def("GetCHA", &Lua_Mob::GetCHA)
				.def("GetMR", &Lua_Mob::GetMR)
				.def("GetFR", &Lua_Mob::GetFR)
				.def("GetDR", &Lua_Mob::GetDR)
				.def("GetPR", &Lua_Mob::GetPR)
				.def("GetCR", &Lua_Mob::GetCR)
				.def("GetCorruption", &Lua_Mob::GetCorruption)
				.def("GetMaxSTR", &Lua_Mob::GetMaxSTR)
				.def("GetMaxSTA", &Lua_Mob::GetMaxSTA)
				.def("GetMaxDEX", &Lua_Mob::GetMaxDEX)
				.def("GetMaxAGI", &Lua_Mob::GetMaxAGI)
				.def("GetMaxINT", &Lua_Mob::GetMaxINT)
				.def("GetMaxWIS", &Lua_Mob::GetMaxWIS)
				.def("GetMaxCHA", &Lua_Mob::GetMaxCHA)
				.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob))&Lua_Mob::ResistSpell)
				.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool))&Lua_Mob::ResistSpell)
				.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool,int))&Lua_Mob::ResistSpell)
				.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool,int,bool))&Lua_Mob::ResistSpell)
				.def("GetSpecializeSkillValue", &Lua_Mob::GetSpecializeSkillValue)
				.def("GetNPCTypeID", &Lua_Mob::GetNPCTypeID)
				.def("IsTargeted", &Lua_Mob::IsTargeted)
				.def("GetX", &Lua_Mob::GetX)
				.def("GetY", &Lua_Mob::GetY)
				.def("GetZ", &Lua_Mob::GetZ)
				.def("GetHeading", &Lua_Mob::GetHeading)
				.def("GetWaypointX", &Lua_Mob::GetWaypointX)
				.def("GetWaypointY", &Lua_Mob::GetWaypointY)
				.def("GetWaypointZ", &Lua_Mob::GetWaypointZ)
				.def("GetWaypointH", &Lua_Mob::GetWaypointH)
				.def("GetWaypointPause", &Lua_Mob::GetWaypointPause)
				.def("GetWaypointID", &Lua_Mob::GetWaypointID)
				.def("SetCurrentWP", &Lua_Mob::SetCurrentWP)
				.def("GetSize", &Lua_Mob::GetSize)
				.def("Message", &Lua_Mob::Message)
				.def("Message_StringID", &Lua_Mob::Message_StringID)
				.def("Say", &Lua_Mob::Say)
				.def("Shout", &Lua_Mob::Shout)
				.def("Emote", &Lua_Mob::Emote)
				.def("InterruptSpell", (void(Lua_Mob::*)(void))&Lua_Mob::InterruptSpell)
				.def("InterruptSpell", (void(Lua_Mob::*)(int))&Lua_Mob::InterruptSpell)
				.def("CastSpell", (bool(Lua_Mob::*)(int,int))&Lua_Mob::CastSpell)
				.def("CastSpell", (bool(Lua_Mob::*)(int,int,int))&Lua_Mob::CastSpell)
				.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int))&Lua_Mob::CastSpell)
				.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int))&Lua_Mob::CastSpell)
				.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int))&Lua_Mob::CastSpell)
				.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int,int,int))&Lua_Mob::CastSpell)
				.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int,int,int,int))&Lua_Mob::CastSpell)
				.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob))&Lua_Mob::SpellFinished)
				.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int))&Lua_Mob::SpellFinished)
				.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int))&Lua_Mob::SpellFinished)
				.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32))&Lua_Mob::SpellFinished)
				.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32,int))&Lua_Mob::SpellFinished)
				.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32,int,bool))&Lua_Mob::SpellFinished)
				.def("SpellEffect", &Lua_Mob::SpellEffect)
				.def("GetHateList", &Lua_Mob::GetHateList),

			luabind::class_<Lua_Client, Lua_Mob>("Client")
				.def(luabind::constructor<>()),

			luabind::class_<Lua_NPC, Lua_Mob>("NPC")
				.def(luabind::constructor<>()),

			luabind::class_<Lua_ItemInst>("ItemInst")
				.def(luabind::constructor<>())
				.property("null", &Lua_ItemInst::Null)
				.property("valid", &Lua_ItemInst::Valid),

			luabind::class_<Lua_Item>("Item")
				.def(luabind::constructor<>())
				.property("null", &Lua_Item::Null)
				.property("valid", &Lua_Item::Valid),

			luabind::class_<Lua_Spell>("Spell")
				.def(luabind::constructor<>())
				.property("null", &Lua_Spell::Null)
				.property("valid", &Lua_Spell::Valid),

			luabind::class_<Lua_HateEntry>("HateEntry")
				.property("null", &Lua_HateEntry::Null)
				.property("valid", &Lua_HateEntry::Valid)
				.property("ent", &Lua_HateEntry::GetEnt, &Lua_HateEntry::SetEnt)
				.property("damage", &Lua_HateEntry::GetDamage, &Lua_HateEntry::SetDamage)
				.property("hate", &Lua_HateEntry::GetHate, &Lua_HateEntry::SetHate)
				.property("frenzy", &Lua_HateEntry::GetFrenzy, &Lua_HateEntry::SetFrenzy),

			luabind::class_<Lua_HateList>("HateList")
				.def_readwrite("entries", &Lua_HateList::entries, luabind::return_stl_iterator)
		];
	
	} catch(std::exception &ex) {
		printf("Error: %s\n", ex.what());
	}
}

#endif
