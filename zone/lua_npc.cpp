#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "lua_npc.h"

luabind::scope lua_register_npc() {
	return luabind::class_<Lua_NPC, Lua_Mob>("NPC")
		.def(luabind::constructor<>());
}

#endif
