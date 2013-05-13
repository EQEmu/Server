#ifndef EQEMU_LUA_NPC_H
#define EQEMU_LUA_NPC_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class NPC;

class Lua_NPC : public Lua_Mob
{
	typedef NPC NativeType;
public:
	Lua_NPC() { d_ = nullptr; }
	Lua_NPC(NativeType *d) { d_ = d; }
	virtual ~Lua_NPC() { }

	operator NativeType* () {
		if(d_) {
			return reinterpret_cast<NativeType*>(d_);
		}

		return nullptr;
	}
};

#endif
#endif