#ifdef BOTS
#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "bot.h"
#include "lua_bot.h"

luabind::scope lua_register_bot() {
	return luabind::class_<Lua_Bot, Lua_Mob>("Bot").def(luabind::constructor<>());
}

#endif
#endif