#ifdef LUA_EQEMU
#include "lua_behavior_interface.h"

#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "../common/spdat.h"
#include "lua_bit.h"
#include "lua_entity.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_hate_list.h"
#include "lua_client.h"
#include "lua_inventory.h"
#include "lua_npc.h"
#include "lua_spell.h"
#include "lua_entity_list.h"
#include "lua_group.h"
#include "lua_raid.h"
#include "lua_corpse.h"
#include "lua_object.h"
#include "lua_door.h"
#include "lua_spawn.h"
#include "lua_packet.h"
#include "lua_general.h"

LuaBehaviorInterface::LuaBehaviorInterface()
{
	L = nullptr;
}

LuaBehaviorInterface::~LuaBehaviorInterface()
{
	if (L) {
		lua_close(L);
	}
}

bool LuaBehaviorInterface::Init()
{
	if (L) {
		return false;
	}

	L = luaL_newstate();
	luaL_openlibs(L);

	if (luaopen_bit(L) != 1) {
		return false;
	}

	if (luaL_dostring(L, "math.randomseed(os.time())")) {
		return false;
	}

#ifdef SANITIZE_LUA_LIBS
	lua_pushnil(L);
	lua_setglobal(L, "io");

	lua_getglobal(L, "os");
	lua_pushnil(L);
	lua_setfield(L, -2, "exit");
	lua_pushnil(L);
	lua_setfield(L, -2, "execute");
	lua_pushnil(L);
	lua_setfield(L, -2, "getenv");
	lua_pushnil(L);
	lua_setfield(L, -2, "remove");
	lua_pushnil(L);
	lua_setfield(L, -2, "rename");
	lua_pushnil(L);
	lua_setfield(L, -2, "setlocale");
	lua_pushnil(L);
	lua_setfield(L, -2, "tmpname");
	lua_pop(L, 1);

	lua_pushnil(L);
	lua_setglobal(L, "collectgarbage");

	lua_pushnil(L);
	lua_setglobal(L, "loadfile");
#endif

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	std::string module_path = lua_tostring(L, -1);
	module_path += ";./lua_modules/?.lua";
	lua_pop(L, 1);
	lua_pushstring(L, module_path.c_str());
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	if (!MapFunctions()) {
		return false;
	}

	std::string path = "ai/script_init.lua";

	FILE *f = fopen(path.c_str(), "r");
	if (f) {
		fclose(f);

		if (luaL_dofile(L, path.c_str())) {
			return false;
		}
	}

	return true;
}

bool LuaBehaviorInterface::MapFunctions()
{
	try {
		luabind::open(L);

		luabind::module(L)
			[
				lua_register_general(),
				lua_register_events(),
				lua_register_faction(),
				lua_register_slot(),
				lua_register_material(),
				lua_register_client_version(),
				lua_register_appearance(),
				lua_register_entity(),
				lua_register_mob(),
				lua_register_special_abilities(),
				lua_register_npc(),
				lua_register_client(),
				lua_register_inventory(),
				lua_register_inventory_where(),
				lua_register_iteminst(),
				lua_register_item(),
				lua_register_spell(),
				lua_register_spawn(),
				lua_register_hate_entry(),
				lua_register_hate_list(),
				lua_register_entity_list(),
				lua_register_mob_list(),
				lua_register_client_list(),
				lua_register_npc_list(),
				lua_register_corpse_list(),
				lua_register_object_list(),
				lua_register_door_list(),
				lua_register_spawn_list(),
				lua_register_group(),
				lua_register_raid(),
				lua_register_corpse(),
				lua_register_door(),
				lua_register_object(),
				lua_register_packet(),
				lua_register_packet_opcodes()
			];

	}
	catch (std::exception &ex) {
		return false;
	}

	return true;
}

#endif
