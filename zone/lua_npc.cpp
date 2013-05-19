#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_npc.h"

#include "lua.hpp"
#include <luabind/luabind.hpp>

luabind::scope lua_register_npc() {
	return luabind::class_<Lua_NPC, Lua_Mob>("NPC")
		.def(luabind::constructor<>());
}

#endif
