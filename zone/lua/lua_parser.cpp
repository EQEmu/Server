#include <sol.hpp>
#include <ctype.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

#include "../../common/spdat.h"
#include "../masterentity.h"
#include "../questmgr.h"
#include "../zone.h"
#include "../zone_config.h"

#include "lua_parser.h"
#include "lua_events.h"
#include "lua_structs.h"
#include "lua_forward.h"

extern Zone *zone;
extern const ZoneConfig *Config;

struct LuaParser::Implementation
{
	std::map<std::string, std::string> vars;
	std::map<std::string, Script> loaded;
	std::unique_ptr<sol::state> state;
};

LuaParser::LuaParser() {
	mImpl.reset(new Implementation());
}

LuaParser::~LuaParser() {
}

int LuaParser::EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc) {
		return 0;
	}

	if(!HasQuestSub(npc->GetNPCTypeID(), evt)) {
		return 0;
	}

	std::string package_name = "npc_" + std::to_string(npc->GetNPCTypeID());
	return _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
							  std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!npc) {
		return 0;
	}

	if(!HasGlobalQuestSub(evt)) {
		return 0;
	}

	return _EventNPC("global_npc", evt, npc, init, data, extra_data, extra_pointers);
}

int LuaParser::_EventNPC(std::string package_name, QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
						 std::vector<EQEmu::Any> *extra_pointers, sol::function *l_func) {
	int r = 0;
	const char *sub_name = LuaEvents[evt];
	auto script = mImpl->loaded.find(package_name);
	
	sol::protected_function f;
	if (l_func) {
		f = *l_func;
	}
	else {
		f = script->second.Env[sub_name];
	}

	auto table = mImpl->state->create_table();
	table["self"] = npc;

	Client *c = (init && init->IsClient()) ? init->CastToClient() : nullptr;
	quest_manager.StartQuest(npc, c, nullptr);
	try {
		auto res = f(table);
		if (res.valid()) {
			r = res;
		}
		else {
			sol::error err = res;
			AddError(err.what());
		}
	}
	catch (sol::error &err) {
		AddError(err.what());
	}
	quest_manager.EndQuest();

	return r;
}

int LuaParser::EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!client) {
		return 0;
	}

	if(!PlayerHasQuestSub(evt)) {
		return 0;
	}

	return _EventPlayer("player", evt, client, data, extra_data, extra_pointers);
}

int LuaParser::EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!client) {
		return 0;
	}

	if(!GlobalPlayerHasQuestSub(evt)) {
		return 0;
	}

	return _EventPlayer("global_player", evt, client, data, extra_data, extra_pointers);
}

int LuaParser::_EventPlayer(std::string package_name, QuestEventID evt, Client *client, std::string data, uint32 extra_data,
							std::vector<EQEmu::Any> *extra_pointers, sol::function *l_func) {
	//const char *sub_name = LuaEvents[evt];
	//int start = lua_gettop(mImpl->state);
	//
	//try {
	//	int npop = 1;
	//	if(l_func != nullptr) {
	//		l_func->push(mImpl->state);
	//	} else {
	//		lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
	//		lua_getfield(mImpl->state, -1, sub_name);
	//		npop = 2;
	//	}
	//
	//	lua_createtable(mImpl->state, 0, 0);
	//	//push self
	//	Lua_Client l_client(client);
	//	luabind::adl::object l_client_o = luabind::adl::object(mImpl->state, l_client);
	//	l_client_o.push(mImpl->state);
	//	lua_setfield(mImpl->state, -2, "self");
	//
	//	auto arg_function = mImpl->PlayerArgumentDispatch[evt];
	//	arg_function(this, mImpl->state, client, data, extra_data, extra_pointers);
	//
	//	quest_manager.StartQuest(client, client, nullptr);
	//	if(lua_pcall(mImpl->state, 1, 1, 0)) {
	//		std::string error = lua_tostring(mImpl->state, -1);
	//		AddError(error);
	//		quest_manager.EndQuest();
	//		return 0;
	//	}
	//	quest_manager.EndQuest();
	//
	//	if(lua_isnumber(mImpl->state, -1)) {
	//		int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
	//		lua_pop(mImpl->state, npop);
	//		return ret;
	//	}
	//
	//	lua_pop(mImpl->state, npop);
	//} catch(std::exception &ex) {
	//	std::string error = "Lua Exception: ";
	//	error += std::string(ex.what());
	//	AddError(error);
	//
	//	//Restore our stack to the best of our ability
	//	int end = lua_gettop(mImpl->state);
	//	int n = end - start;
	//	if(n > 0) {
	//		lua_pop(mImpl->state, n);
	//	}
	//}

	return 0;
}

int LuaParser::EventItem(QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	if(!item) {
		return 0;
	}

	if(!ItemHasQuestSub(item, evt)) {
		return 0;
	}

	std::string package_name = "item_";
	package_name += std::to_string(item->GetID());
	return _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers);
}

int LuaParser::_EventItem(std::string package_name, QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob,
						  std::string data, uint32 extra_data, std::vector<EQEmu::Any> *extra_pointers, sol::function *l_func) {
	//const char *sub_name = LuaEvents[evt];
	//
	//int start = lua_gettop(mImpl->state);
	//
	//try {
	//	int npop = 1;
	//	if(l_func != nullptr) {
	//		l_func->push(mImpl->state);
	//	} else {
	//		lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
	//		lua_getfield(mImpl->state, -1, sub_name);
	//	}
	//
	//	lua_createtable(mImpl->state, 0, 0);
	//	//always push self
	//	Lua_ItemInst l_item(item);
	//	luabind::adl::object l_item_o = luabind::adl::object(mImpl->state, l_item);
	//	l_item_o.push(mImpl->state);
	//	lua_setfield(mImpl->state, -2, "self");
	//
	//	Lua_Client l_client(client);
	//	luabind::adl::object l_client_o = luabind::adl::object(mImpl->state, l_client);
	//	l_client_o.push(mImpl->state);
	//	lua_setfield(mImpl->state, -2, "owner");
	//
	//	//redo this arg function
	//	auto arg_function = mImpl->ItemArgumentDispatch[evt];
	//	arg_function(this, mImpl->state, client, item, mob, data, extra_data, extra_pointers);
	//
	//	quest_manager.StartQuest(client, client, item);
	//	if(lua_pcall(mImpl->state, 1, 1, 0)) {
	//		std::string error = lua_tostring(mImpl->state, -1);
	//		AddError(error);
	//		quest_manager.EndQuest();
	//		return 0;
	//	}
	//	quest_manager.EndQuest();
	//
	//	if(lua_isnumber(mImpl->state, -1)) {
	//		int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
	//		lua_pop(mImpl->state, npop);
	//		return ret;
	//	}
	//
	//	lua_pop(mImpl->state, npop);
	//} catch(std::exception &ex) {
	//	std::string error = "Lua Exception: ";
	//	error += std::string(ex.what());
	//	AddError(error);
	//
	//	//Restore our stack to the best of our ability
	//	int end = lua_gettop(mImpl->state);
	//	int n = end - start;
	//	if(n > 0) {
	//		lua_pop(mImpl->state, n);
	//	}
	//}

	return 0;
}

int LuaParser::EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
						  std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "spell_" + std::to_string(spell_id);

	if(!SpellHasQuestSub(spell_id, evt)) {
		return 0;
	}

	return _EventSpell(package_name, evt, npc, client, spell_id, extra_data, extra_pointers);
}

int LuaParser::_EventSpell(std::string package_name, QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
						   std::vector<EQEmu::Any> *extra_pointers, sol::function *l_func) {
	//const char *sub_name = LuaEvents[evt];
	//
	//int start = lua_gettop(mImpl->state);
	//
	//try {
	//	int npop = 1;
	//	if(l_func != nullptr) {
	//		l_func->push(mImpl->state);
	//	} else {
	//		lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
	//		lua_getfield(mImpl->state, -1, sub_name);
	//		npop = 2;
	//	}
	//
	//	lua_createtable(mImpl->state, 0, 0);
	//
	//	//always push self even if invalid
	//	if(IsValidSpell(spell_id)) {
	//		Lua_Spell l_spell(&spells[spell_id]);
	//		luabind::adl::object l_spell_o = luabind::adl::object(mImpl->state, l_spell);
	//		l_spell_o.push(mImpl->state);
	//	} else {
	//		Lua_Spell l_spell(nullptr);
	//		luabind::adl::object l_spell_o = luabind::adl::object(mImpl->state, l_spell);
	//		l_spell_o.push(mImpl->state);
	//	}
	//	lua_setfield(mImpl->state, -2, "self");
	//
	//	auto arg_function = mImpl->SpellArgumentDispatch[evt];
	//	arg_function(this, mImpl->state, npc, client, spell_id, extra_data, extra_pointers);
	//
	//	quest_manager.StartQuest(npc, client, nullptr);
	//	if(lua_pcall(mImpl->state, 1, 1, 0)) {
	//		std::string error = lua_tostring(mImpl->state, -1);
	//		AddError(error);
	//		quest_manager.EndQuest();
	//		return 0;
	//	}
	//	quest_manager.EndQuest();
	//
	//	if(lua_isnumber(mImpl->state, -1)) {
	//		int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
	//		lua_pop(mImpl->state, npop);
	//		return ret;
	//	}
	//
	//	lua_pop(mImpl->state, npop);
	//} catch(std::exception &ex) {
	//	std::string error = "Lua Exception: ";
	//	error += std::string(ex.what());
	//	AddError(error);
	//
	//	//Restore our stack to the best of our ability
	//	int end = lua_gettop(mImpl->state);
	//	int n = end - start;
	//	if(n > 0) {
	//		lua_pop(mImpl->state, n);
	//	}
	//}
	
	return 0;
}

int LuaParser::EventEncounter(QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data, std::vector<EQEmu::Any> *extra_pointers) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return 0;
	}

	std::string package_name = "encounter_" + encounter_name;

	if(!EncounterHasQuestSub(encounter_name, evt)) {
		return 0;
	}

	return _EventEncounter(package_name, evt, encounter_name, data, extra_data, extra_pointers);
}

int LuaParser::_EventEncounter(std::string package_name, QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data,
							   std::vector<EQEmu::Any> *extra_pointers) {
	//const char *sub_name = LuaEvents[evt];
	//
	//int start = lua_gettop(mImpl->state);
	//
	//try {
	//	lua_getfield(mImpl->state, LUA_REGISTRYINDEX, package_name.c_str());
	//	lua_getfield(mImpl->state, -1, sub_name);
	//
	//	lua_createtable(mImpl->state, 0, 0);
	//	lua_pushstring(mImpl->state, encounter_name.c_str());
	//	lua_setfield(mImpl->state, -2, "name");
	//
	//	Encounter *enc = lua_encounters[encounter_name];
	//
	//	auto arg_function = mImpl->EncounterArgumentDispatch[evt];
	//	arg_function(this, mImpl->state, enc, data, extra_data, extra_pointers);
	//
	//	quest_manager.StartQuest(enc, nullptr, nullptr, encounter_name);
	//	if(lua_pcall(mImpl->state, 1, 1, 0)) {
	//		std::string error = lua_tostring(mImpl->state, -1);
	//		AddError(error);
	//		quest_manager.EndQuest();
	//		return 0;
	//	}
	//	quest_manager.EndQuest();
	//
	//	if(lua_isnumber(mImpl->state, -1)) {
	//		int ret = static_cast<int>(lua_tointeger(mImpl->state, -1));
	//		lua_pop(mImpl->state, 2);
	//		return ret;
	//	}
	//
	//	lua_pop(mImpl->state, 2);
	//} catch(std::exception &ex) {
	//	std::string error = "Lua Exception: ";
	//	error += std::string(ex.what());
	//	AddError(error);
	//
	//	//Restore our stack to the best of our ability
	//	int end = lua_gettop(mImpl->state);
	//	int n = end - start;
	//	if(n > 0) {
	//		lua_pop(mImpl->state, n);
	//	}
	//}

	return 0;
}

bool LuaParser::HasQuestSub(uint32 npc_id, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "npc_" + std::to_string(npc_id);

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::HasGlobalQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_npc");
}

bool LuaParser::PlayerHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "player");
}

bool LuaParser::GlobalPlayerHasQuestSub(QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, "global_player");
}

bool LuaParser::SpellHasQuestSub(uint32 spell_id, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "spell_" + std::to_string(spell_id);

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::ItemHasQuestSub(EQEmu::ItemInstance *itm, QuestEventID evt) {
	if (itm == nullptr) {
		return false;
	}
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "item_";
	package_name += std::to_string(itm->GetID());

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

bool LuaParser::EncounterHasQuestSub(std::string encounter_name, QuestEventID evt) {
	evt = ConvertLuaEvent(evt);
	if(evt >= _LargestEventID) {
		return false;
	}

	std::string package_name = "encounter_" + encounter_name;

	const char *subname = LuaEvents[evt];
	return HasFunction(subname, package_name);
}

void LuaParser::LoadNPCScript(std::string filename, int npc_id) {
	std::string package_name = "npc_" + std::to_string(npc_id);

	LoadScript(filename, package_name);
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

void LuaParser::LoadItemScript(std::string filename, EQEmu::ItemInstance *item) {
	if (item == nullptr)
		return;
	std::string package_name = "item_";
	package_name += std::to_string(item->GetID());

	LoadScript(filename, package_name);
}

void LuaParser::LoadSpellScript(std::string filename, uint32 spell_id) {
	std::string package_name = "spell_" + std::to_string(spell_id);

	LoadScript(filename, package_name);
}

void LuaParser::LoadEncounterScript(std::string filename, std::string encounter_name) {
	std::string package_name = "encounter_" + encounter_name;

	LoadScript(filename, package_name);
}

void LuaParser::AddVar(std::string name, std::string val) {
	mImpl->vars[name] = val;
}

std::string LuaParser::GetVar(std::string name) {
	auto iter = mImpl->vars.find(name);
	if(iter != mImpl->vars.end()) {
		return iter->second;
	}

	return std::string();
}

void LuaParser::Init() {
	ReloadQuests();
}

void LuaParser::ReloadQuests() {
	mImpl->loaded.clear();
	errors_.clear();

	mImpl->state.reset(new sol::state());
	mImpl->state->open_libraries(sol::lib::base, 
		sol::lib::package, 
		sol::lib::coroutine, 
		sol::lib::string, 
		sol::lib::os, 
		sol::lib::math, 
		sol::lib::table, 
		sol::lib::debug, 
		sol::lib::bit32, 
		sol::lib::io);
	mImpl->state->script("math.randomseed(os.time())");

	auto globals = mImpl->state->globals();
#ifdef SANITIZE_LUA_LIBS
	globals["io"] = sol::nil;
	globals["collectgarbage"] = sol::nil;
	globals["loadfile"] = sol::nil;

	auto os = globals["os"];
	os["exit"] = sol::nil;
	os["execute"] = sol::nil;
	os["getenv"] = sol::nil;
	os["remove"] = sol::nil;
	os["rename"] = sol::nil;
	os["setlocale"] = sol::nil;
	os["tmpname"] = sol::nil;
#endif

#ifdef _WIN32
	const char libext[] = ".dll";
#else
	const char libext[] = ".so";
#endif

#if SOL_LUA_VERSION == 501
	const char lua_version[] = "5.1";
#elif SOL_LUA_VERSION == 502
	const char lua_version[] = "5.2";
#elif SOL_LUA_VERSION == 503
	const char lua_version[] = "5.3";
#else
#error Unsupported Lua version
#endif

	auto package = globals["package"];
	std::string path = package["path"];
	std::string cpath = package["cpath"];

	path += ";./" + Config->LuaModuleDir + "?.lua;./" + Config->LuaModuleDir + "?/init.lua";
	path += ";./" + Config->LuaModuleDir + "share/lua/" + lua_version + "/?.lua";
	path += ";./" + Config->LuaModuleDir + "share/lua/" + lua_version + "/?/init.lua";
	cpath += ";./" + Config->LuaModuleDir + "?" + libext;
	cpath += ";./" + Config->LuaModuleDir + "lib/lua/" + lua_version + "/?" + libext;

	package["path"] = path;
	package["cpath"] = cpath;

	MapFunctions();

//	mImpl->loaded.clear();
//	errors_.clear();
//	mImpl->mods.clear();
//	lua_encounter_events_registered.clear();
//	lua_encounters_loaded.clear();
//
//	for (auto encounter : lua_encounters) {
//		encounter.second->Depop();
//	}
//
//	lua_encounters.clear();
//	// so the Depop function above depends on the Process being called again so ...
//	// And there is situations where it wouldn't be :P
//	entity_list.EncounterProcess();
//
//	if(mImpl->state) {
//		lua_close(mImpl->state);
//	}
//
//	mImpl->state = luaL_newstate();
//	luaL_openlibs(mImpl->state);
//
//	auto top = lua_gettop(mImpl->state);
//
//	if(luaopen_bit(mImpl->state) != 1) {
//		std::string error = lua_tostring(mImpl->state, -1);
//		AddError(error);
//	}
//
//	if(luaL_dostring(mImpl->state, "math.randomseed(os.time())")) {
//		std::string error = lua_tostring(mImpl->state, -1);
//		AddError(error);
//	}
//
//#ifdef SANITIZE_LUA_LIBS
//	//io
//	lua_pushnil(mImpl->state);
//	//lua_setglobal(mImpl->state, "io");
//
//	//some os/debug are okay some are not
//	lua_getglobal(mImpl->state, "os");
//	lua_pushnil(mImpl->state);
//	lua_setfield(mImpl->state, -2, "exit");
//	lua_pushnil(mImpl->state);
//	lua_setfield(mImpl->state, -2, "execute");
//	lua_pushnil(mImpl->state);
//	lua_setfield(mImpl->state, -2, "getenv");
//	lua_pushnil(mImpl->state);
//	lua_setfield(mImpl->state, -2, "remove");
//	lua_pushnil(mImpl->state);
//	lua_setfield(mImpl->state, -2, "rename");
//	lua_pushnil(mImpl->state);
//	lua_setfield(mImpl->state, -2, "setlocale");
//	lua_pushnil(mImpl->state);
//	lua_setfield(mImpl->state, -2, "tmpname");
//	lua_pop(mImpl->state, 1);
//
//	lua_pushnil(mImpl->state);
//	lua_setglobal(mImpl->state, "collectgarbage");
//
//	lua_pushnil(mImpl->state);
//	lua_setglobal(mImpl->state, "loadfile");
//
//#endif
//
//	// lua 5.2+ defines these
//#if defined(LUA_VERSION_MAJOR) && defined(LUA_VERSION_MINOR)
//	const char lua_version[] = LUA_VERSION_MAJOR "." LUA_VERSION_MINOR;
//#elif LUA_VERSION_NUM == 501
//	const char lua_version[] = "5.1";
//#else
//#error Incompatible lua version
//#endif
//
//#ifdef WINDOWS
//	const char libext[] = ".dll";
//#else
//	// lua doesn't care OSX doesn't use sonames
//	const char libext[] = ".so";
//#endif
//
//	lua_getglobal(mImpl->state, "package");
//	lua_getfield(mImpl->state, -1, "path");
//	std::string module_path = lua_tostring(mImpl->state,-1);
//	module_path += ";./" + Config->LuaModuleDir + "?.lua;./" + Config->LuaModuleDir + "?/init.lua";
//	// luarock paths using lua_modules as tree
//	// to path it adds foo/share/lua/5.1/?.lua and foo/share/lua/5.1/?/init.lua
//	module_path += ";./" + Config->LuaModuleDir + "share/lua/" + lua_version + "/?.lua";
//	module_path += ";./" + Config->LuaModuleDir + "share/lua/" + lua_version + "/?/init.lua";
//	lua_pop(mImpl->state, 1);
//	lua_pushstring(mImpl->state, module_path.c_str());
//	lua_setfield(mImpl->state, -2, "path");
//	lua_pop(mImpl->state, 1);
//
//	lua_getglobal(mImpl->state, "package");
//	lua_getfield(mImpl->state, -1, "cpath");
//	module_path = lua_tostring(mImpl->state, -1);
//	module_path += ";./" + Config->LuaModuleDir + "?" + libext;
//	// luarock paths using lua_modules as tree
//	// luarocks adds foo/lib/lua/5.1/?.so for cpath
//	module_path += ";./" + Config->LuaModuleDir + "lib/lua/" + lua_version + "/?" + libext;
//	lua_pop(mImpl->state, 1);
//	lua_pushstring(mImpl->state, module_path.c_str());
//	lua_setfield(mImpl->state, -2, "cpath");
//	lua_pop(mImpl->state, 1);
//
//	MapFunctions(mImpl->state);
//
//	//load init
//	std::string path = Config->QuestDir;
//	path += "/";
//	path += QUEST_GLOBAL_DIRECTORY;
//	path += "/script_init.lua";
//
//	FILE *f = fopen(path.c_str(), "r");
//	if(f) {
//		fclose(f);
//
//		if(luaL_dofile(mImpl->state, path.c_str())) {
//			std::string error = lua_tostring(mImpl->state, -1);
//			AddError(error);
//		}
//	}
//
//	//zone init - always loads after global
//	if(zone) {
//		std::string zone_script = Config->QuestDir;
//		zone_script += "/";
//		zone_script += zone->GetShortName();
//		zone_script += "/script_init_v";
//		zone_script += std::to_string(zone->GetInstanceVersion());
//		zone_script += ".lua";
//		f = fopen(zone_script.c_str(), "r");
//		if(f) {
//			fclose(f);
//
//			if(luaL_dofile(mImpl->state, zone_script.c_str())) {
//				std::string error = lua_tostring(mImpl->state, -1);
//				AddError(error);
//			}
//		}
//		else {
//			zone_script = Config->QuestDir;
//			zone_script += "/";
//			zone_script += zone->GetShortName();
//			zone_script += "/script_init.lua";
//			f = fopen(zone_script.c_str(), "r");
//			if (f) {
//				fclose(f);
//
//				if (luaL_dofile(mImpl->state, zone_script.c_str())) {
//					std::string error = lua_tostring(mImpl->state, -1);
//					AddError(error);
//				}
//			}
//		}
//	}
//
//	FILE *load_order = fopen("mods/load_order.txt", "r");
//	if (load_order) {
//		char file_name[256] = { 0 };
//		while (fgets(file_name, 256, load_order) != nullptr) {
//			for (int i = 0; i < 256; ++i) {
//				auto c = file_name[i];
//				if (c == '\n' || c == '\r' || c == ' ') {
//					file_name[i] = 0;
//					break;
//				}
//			}
//
//			LoadScript("mods/" + std::string(file_name), file_name);
//			mImpl->mods.push_back(LuaMod(mImpl->state, this, file_name));
//		}
//
//		fclose(load_order);
//	}
//
//	auto end = lua_gettop(mImpl->state);
//	int n = end - top;
//	if (n > 0) {
//		lua_pop(mImpl->state, n);
//	}
}

void LuaParser::LoadScript(const std::string &filename)
{
}

void LuaParser::LoadScript(const std::string &filename, const std::string &package_name) {
	auto iter = mImpl->loaded.find(package_name);
	if(iter != mImpl->loaded.end()) {
		return;
	}

	Script s;
	s.Env = sol::environment(*mImpl->state, sol::create, mImpl->state->globals());
	try {
		auto res = mImpl->state->safe_script_file(filename);
		if (res.valid()) {
			s.Loaded = true;
		}
		else {
			sol::error err = res;
			AddError(err.what());
			s.Loaded = false;
		}
	}
	catch (sol::error &err) {
		AddError(err.what());
	}

	mImpl->loaded[package_name] = s;
}

bool LuaParser::HasFunction(const std::string &subname, const std::string &package_name) {
	auto iter = mImpl->loaded.find(package_name);
	if(iter == mImpl->loaded.end()) {
		return false;
	}

	auto f = iter->second.Env[subname];
	if (f.valid() && f.get_type() == sol::type::function) {
		return true;
	}

	return false;
}

void LuaParser::MapFunctions() {
	lua_register_entity(mImpl->state.get());
	lua_register_mob(mImpl->state.get());
	lua_register_npc(mImpl->state.get());
	lua_register_client(mImpl->state.get());
	lua_register_doors(mImpl->state.get());
	lua_register_corpse(mImpl->state.get());
	lua_register_object(mImpl->state.get());
	lua_register_general(mImpl->state.get());
}

int LuaParser::DispatchEventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
								 std::vector<EQEmu::Any> *extra_pointers) {
//	evt = ConvertLuaEvent(evt);
//	if(evt >= _LargestEventID) {
//		return 0;
//	}
//
//	if(!npc)
//		return 0;
//
//	std::string package_name = "npc_" + std::to_string(npc->GetNPCTypeID());
//	int ret = 0;
//
//	auto iter = lua_encounter_events_registered.find(package_name);
//	if(iter != lua_encounter_events_registered.end()) {
//		auto riter = iter->second.begin();
//		while(riter != iter->second.end()) {
//			if(riter->event_id == evt) {
//				std::string package_name = "encounter_" + riter->encounter_name;
//				int i = _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers, &riter->lua_reference);
//                if(i != 0)
//                    ret = i;
//			}
//			++riter;
//		}
//	}
//
//	iter = lua_encounter_events_registered.find("npc_-1");
//	if(iter == lua_encounter_events_registered.end()) {
//		return ret;
//	}
//
//	auto riter = iter->second.begin();
//	while(riter != iter->second.end()) {
//		if(riter->event_id == evt) {
//			std::string package_name = "encounter_" + riter->encounter_name;
//			int i = _EventNPC(package_name, evt, npc, init, data, extra_data, extra_pointers, &riter->lua_reference);
//            if(i != 0)
//                ret = i;
//		}
//		++riter;
//	}
//
//    return ret;
	return 0;
}

int LuaParser::DispatchEventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
									std::vector<EQEmu::Any> *extra_pointers) {
//	evt = ConvertLuaEvent(evt);
//	if(evt >= _LargestEventID) {
//		return 0;
//	}
//
//	std::string package_name = "player";
//
//	auto iter = lua_encounter_events_registered.find(package_name);
//	if(iter == lua_encounter_events_registered.end()) {
//		return 0;
//	}
//
//    int ret = 0;
//	auto riter = iter->second.begin();
//	while(riter != iter->second.end()) {
//		if(riter->event_id == evt) {
//			std::string package_name = "encounter_" + riter->encounter_name;
//			int i = _EventPlayer(package_name, evt, client, data, extra_data, extra_pointers, &riter->lua_reference);
//            if(i != 0)
//                ret = i;
//		}
//		++riter;
//	}
//
//    return ret;
	return 0;
}

int LuaParser::DispatchEventItem(QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
								  std::vector<EQEmu::Any> *extra_pointers) {
//	evt = ConvertLuaEvent(evt);
//	if(evt >= _LargestEventID) {
//		return 0;
//	}
//
//	if(!item)
//		return 0;
//
//	std::string package_name = "item_";
//	package_name += std::to_string(item->GetID());
//	int ret = 0;
//
//	auto iter = lua_encounter_events_registered.find(package_name);
//	if(iter != lua_encounter_events_registered.end()) {
//		auto riter = iter->second.begin();
//		while(riter != iter->second.end()) {
//			if(riter->event_id == evt) {
//				std::string package_name = "encounter_" + riter->encounter_name;
//				int i = _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers, &riter->lua_reference);
//                if(i != 0)
//                    ret = i;
//			}
//			++riter;
//		}
//	}
//
//	iter = lua_encounter_events_registered.find("item_-1");
//	if(iter == lua_encounter_events_registered.end()) {
//		return ret;
//	}
//
//	auto riter = iter->second.begin();
//	while(riter != iter->second.end()) {
//		if(riter->event_id == evt) {
//			std::string package_name = "encounter_" + riter->encounter_name;
//			int i = _EventItem(package_name, evt, client, item, mob, data, extra_data, extra_pointers, &riter->lua_reference);
//            if(i != 0)
//                ret = i;
//		}
//		++riter;
//	}
//    return ret;
	return 0;
}

int LuaParser::DispatchEventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
								   std::vector<EQEmu::Any> *extra_pointers) {
//	evt = ConvertLuaEvent(evt);
//	if(evt >= _LargestEventID) {
//		return 0;
//	}
//
//	std::string package_name = "spell_" + std::to_string(spell_id);
//
//    int ret = 0;
//	auto iter = lua_encounter_events_registered.find(package_name);
//	if(iter != lua_encounter_events_registered.end()) {
//	    auto riter = iter->second.begin();
//		while(riter != iter->second.end()) {
//			if(riter->event_id == evt) {
//				std::string package_name = "encounter_" + riter->encounter_name;
//				int i = _EventSpell(package_name, evt, npc, client, spell_id, extra_data, extra_pointers, &riter->lua_reference);
//                if(i != 0) {
//                    ret = i;
//                }
//			}
//			++riter;
//		}
//	}
//
//	iter = lua_encounter_events_registered.find("spell_-1");
//	if(iter == lua_encounter_events_registered.end()) {
//		return ret;
//	}
//
//	auto riter = iter->second.begin();
//	while(riter != iter->second.end()) {
//		if(riter->event_id == evt) {
//			std::string package_name = "encounter_" + riter->encounter_name;
//			int i = _EventSpell(package_name, evt, npc, client, spell_id, extra_data, extra_pointers, &riter->lua_reference);
//            if(i != 0)
//                ret = i;
//		}
//		++riter;
//	}
//    return ret;
	return 0;
}

QuestEventID LuaParser::ConvertLuaEvent(QuestEventID evt) {
	switch(evt) {
	case EVENT_SLAY:
	case EVENT_NPC_SLAY:
		return EVENT_SLAY;
		break;
	case EVENT_SPELL_EFFECT_CLIENT:
	case EVENT_SPELL_EFFECT_NPC:
		return EVENT_SPELL_EFFECT_CLIENT;
		break;
	case EVENT_SPELL_BUFF_TIC_CLIENT:
	case EVENT_SPELL_BUFF_TIC_NPC:
		return EVENT_SPELL_BUFF_TIC_CLIENT;
		break;
	case EVENT_AGGRO:
	case EVENT_ATTACK:
		return _LargestEventID;
		break;
	default:
		return evt;
	}
}

void LuaParser::MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	//for (auto &mod : mImpl->mods) {
	//	mod.MeleeMitigation(self, attacker, hit, opts, ignoreDefault);
	//}
}

void LuaParser::ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault)
{
	//for (auto &mod : mImpl->mods) {
	//	mod.ApplyDamageTable(self, hit, ignoreDefault);
	//}
}

bool LuaParser::AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool & ignoreDefault)
{
	bool retval = false;
	//for (auto &mod : mImpl->mods) {
	//	mod.AvoidDamage(self, other, hit, retval, ignoreDefault);
	//}
	return retval;
}

bool LuaParser::CheckHitChance(Mob *self, Mob *other, DamageHitInfo &hit, bool &ignoreDefault)
{
	bool retval = false;
	//for (auto &mod : mImpl->mods) {
	//	mod.CheckHitChance(self, other, hit, retval, ignoreDefault);
	//}
	return retval;
}

void LuaParser::TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	//for (auto &mod : mImpl->mods) {
	//	mod.TryCriticalHit(self, defender, hit, opts, ignoreDefault);
	//}
}

void LuaParser::CommonOutgoingHitSuccess(Mob *self, Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	//for (auto &mod : mImpl->mods) {
	//	mod.CommonOutgoingHitSuccess(self, other, hit, opts, ignoreDefault);
	//}
}

uint32 LuaParser::GetRequiredAAExperience(Client *self, bool &ignoreDefault)
{
	uint32 retval = 0;
	//for (auto &mod : mImpl->mods) {
	//	mod.GetRequiredAAExperience(self, retval, ignoreDefault);
	//}
	return retval;
}

uint32 LuaParser::GetEXPForLevel(Client *self, uint16 level, bool &ignoreDefault)
{
	uint32 retval = 0;
	//for (auto &mod : mImpl->mods) {
	//	mod.GetEXPForLevel(self, level, retval, ignoreDefault);
	//}
	return retval;
}

uint32 LuaParser::GetExperienceForKill(Client *self, Mob *against, bool &ignoreDefault)
{
	uint32 retval = 0;
	//for (auto &mod : mImpl->mods) {
	//	mod.GetExperienceForKill(self, against, retval, ignoreDefault);
	//}
	return retval;
}
