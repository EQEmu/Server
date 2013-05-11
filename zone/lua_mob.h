#ifndef EQEMU_LUA_MOB_H
#define EQEMU_LUA_MOB_H
#ifdef LUA_EQEMU

#include "lua_entity.h"

class Mob;

class Lua_Mob : public Lua_Entity
{
public:
	Lua_Mob() { }
	Lua_Mob(Mob *d) { this->d_ = d; }
	virtual ~Lua_Mob() { }

	const char *GetName();

	void Depop();
	void Depop(bool start_spawn_timer);
};

#endif
#endif