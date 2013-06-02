#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "lua_object.h"

luabind::scope lua_register_object() {
	return luabind::class_<Lua_Object, Lua_Entity>("Object")
		.def(luabind::constructor<>())
		.property("null", &Lua_Object::Null)
		.property("valid", &Lua_Object::Valid);
}

#endif
