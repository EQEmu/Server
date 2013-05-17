#ifndef EQEMU_LUA_ITEM_H
#define EQEMU_LUA_ITEM_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class ItemInst;

class Lua_Item : public Lua_Ptr
{
	typedef ItemInst NativeType;
public:
	Lua_Item() { }
	Lua_Item(ItemInst *d) : Lua_Ptr(d) { }
	virtual ~Lua_Item() { }

	operator ItemInst*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<ItemInst*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
