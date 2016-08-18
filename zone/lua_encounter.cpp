#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include "lua_encounter.h"
#include "encounter.h"


luabind::scope lua_register_encounter() {
	return luabind::class_<Lua_Encounter>("Encounter");
}

#endif