#ifndef EQEMU_LUA_ITEM_H
#define EQEMU_LUA_ITEM_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

struct SPDat_Spell_Struct;

class Lua_Spell : public Lua_Ptr
{
	typedef SPDat_Spell_Struct NativeType;
public:
	Lua_Spell() { }
	Lua_Spell(SPDat_Spell_Struct *d) : Lua_Ptr(d) { }
	virtual ~Lua_Spell() { }

	operator SPDat_Spell_Struct*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<SPDat_Spell_Struct*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
