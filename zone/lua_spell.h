#ifndef EQEMU_LUA_SPELL_H
#define EQEMU_LUA_SPELL_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

struct SPDat_Spell_Struct;

class Lua_Spell : public Lua_Ptr<const void>
{
	typedef const SPDat_Spell_Struct NativeType;
public:
	Lua_Spell() { }
	Lua_Spell(const SPDat_Spell_Struct *d) : Lua_Ptr(d) { }
	virtual ~Lua_Spell() { }

	operator const SPDat_Spell_Struct*() {
		const void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<const SPDat_Spell_Struct*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
