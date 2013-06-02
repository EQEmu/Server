#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "doors.h"
#include "lua_door.h"

luabind::scope lua_register_door() {
	return luabind::class_<Lua_Door, Lua_Entity>("Door")
		.def(luabind::constructor<>())
		.property("null", &Lua_Door::Null)
		.property("valid", &Lua_Door::Valid);
}

#endif
