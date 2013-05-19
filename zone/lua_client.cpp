#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_client.h"

#include "lua.hpp"
#include <luabind/luabind.hpp>

luabind::scope lua_register_client() {
	return luabind::class_<Lua_Client, Lua_Mob>("Client")
		.def(luabind::constructor<>());
}

#endif
