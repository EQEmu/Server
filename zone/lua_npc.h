#ifndef EQEMU_LUA_NPC_H
#define EQEMU_LUA_NPC_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class NPC;

class Lua_NPC : public Lua_Mob
{
public:
	Lua_NPC() { d_ = nullptr; }
	Lua_NPC(NPC *d) { d_ = d; }
	virtual ~Lua_NPC() { }
};

#endif
#endif