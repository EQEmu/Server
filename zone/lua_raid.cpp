#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "raid.h"
#include "masterentity.h"
#include "lua_raid.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

luabind::scope lua_register_raid() {
	return luabind::class_<Lua_Raid>("Raid")
		.def(luabind::constructor<>())
		.property("null", &Lua_Raid::Null)
		.property("valid", &Lua_Raid::Valid);
}

#endif
