#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "lua_client.h"

luabind::scope lua_register_client() {
	return luabind::class_<Lua_Client, Lua_Mob>("Client")
		.def(luabind::constructor<>());
}

#endif
