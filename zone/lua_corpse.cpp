#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "lua_corpse.h"

luabind::scope lua_register_corpse() {
	return luabind::class_<Lua_Corpse, Lua_Mob>("Corpse")
		.def(luabind::constructor<>())
		.property("null", &Lua_Corpse::Null)
		.property("valid", &Lua_Corpse::Valid);
}

#endif
