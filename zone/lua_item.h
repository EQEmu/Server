#ifndef EQEMU_LUA_ITEM_H
#define EQEMU_LUA_ITEM_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

struct Item_Struct;

class Lua_Item : public Lua_Ptr<const void>
{
	typedef Item_Struct NativeType;
public:
	Lua_Item() { }
	Lua_Item(const Item_Struct *d) : Lua_Ptr(d) { }
	virtual ~Lua_Item() { }

	operator const Item_Struct*() {
		const void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<const Item_Struct*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
