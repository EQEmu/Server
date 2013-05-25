#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "lua_client.h"
#include "../common/item.h"

struct InventoryWhere { };

luabind::scope lua_register_client() {
	return luabind::class_<Lua_Client, Lua_Mob>("Client")
		.def(luabind::constructor<>());
}

luabind::scope lua_register_inventory_where() {
	return luabind::class_<InventoryWhere>("InventoryWhere")
		.enum_("constants")
		[
			luabind::value("Personal", static_cast<int>(invWherePersonal)),
			luabind::value("Bank", static_cast<int>(invWhereBank)),
			luabind::value("SharedBank", static_cast<int>(invWhereSharedBank)),
			luabind::value("Trading", static_cast<int>(invWhereTrading)),
			luabind::value("Cursor", static_cast<int>(invWhereCursor))
		];
}

#endif
