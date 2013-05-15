#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_item.h"

bool Lua_Item::Null() {
	return d_ == nullptr;
}

bool Lua_Item::Valid() {
	return d_ != nullptr;
}

#endif
