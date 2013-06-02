#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "groups.h"
#include "masterentity.h"
#include "lua_group.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

luabind::scope lua_register_group() {
	return luabind::class_<Lua_Group>("Group")
		.def(luabind::constructor<>())
		.property("null", &Lua_Group::Null)
		.property("valid", &Lua_Group::Valid);
}

#endif
