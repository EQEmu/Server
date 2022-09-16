#ifdef LUA_EQEMU

#include <sol/sol.hpp>
#include "lua_encounter.h"
#include "encounter.h"


void lua_register_encounter(sol::state_view &sv)
{
	sv.new_usertype<Lua_Encounter>("Encounter");
}

#endif
