#include <sol.hpp>
#include "../client.h"

void lua_register_inventory_where(sol::state *state) {
	state->create_named_table("InventoryWhere",
		"Personal", static_cast<int>(invWherePersonal),
		"Bank", static_cast<int>(invWhereBank),
		"SharedBank", static_cast<int>(invWhereSharedBank),
		"Trading", static_cast<int>(invWhereTrading),
		"Cursor", static_cast<int>(invWhereCursor)
	);
}
