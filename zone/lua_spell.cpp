#ifdef LUA_EQEMU

#include "../common/spdat.h"
#include "lua_spell.h"

#include "lua.hpp"
#include <luabind/luabind.hpp>

luabind::scope lua_register_spell() {
	return luabind::class_<Lua_Spell>("Spell")
		.def(luabind::constructor<>())
		.property("null", &Lua_Spell::Null)
		.property("valid", &Lua_Spell::Valid);
}

#endif
