#ifndef EQEMU_LUA_CORPSE_H
#define EQEMU_LUA_CORPSE_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Corpse;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_corpse();

class Lua_Corpse : public Lua_Mob
{
	typedef Corpse NativeType;
public:
	Lua_Corpse() { }
	Lua_Corpse(Corpse *d) { SetLuaPtrData(d); }
	virtual ~Lua_Corpse() { }

	operator Corpse*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Corpse*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
