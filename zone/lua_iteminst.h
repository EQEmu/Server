#ifndef EQEMU_LUA_ITEMINST_H
#define EQEMU_LUA_ITEMINST_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class ItemInst;

class Lua_ItemInst : public Lua_Ptr<void>
{
	typedef ItemInst NativeType;
public:
	Lua_ItemInst() { }
	Lua_ItemInst(ItemInst *d) : Lua_Ptr(d) { }
	virtual ~Lua_ItemInst() { }

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
