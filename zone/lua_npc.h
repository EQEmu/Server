#ifndef EQEMU_LUA_NPC_H
#define EQEMU_LUA_NPC_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class NPC;
namespace luabind {
	struct scope;
}

luabind::scope lua_register_npc();

class Lua_NPC : public Lua_Mob
{
	typedef NPC NativeType;
public:
	Lua_NPC() { }
	Lua_NPC(NPC *d) { SetLuaPtrData(d); }
	virtual ~Lua_NPC() { }

	operator NPC*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<NPC*>(d);
		}

		return nullptr;
	}
};

#endif
#endif