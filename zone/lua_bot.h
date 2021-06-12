#ifdef BOTS
#ifndef EQEMU_LUA_BOT_H
#define EQEMU_LUA_BOT_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Bot;
class Lua_Bot;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_bot();

class Lua_Bot : public Lua_Mob
{
	typedef Bot NativeType;
public:
	Lua_Bot() { SetLuaPtrData(nullptr); }
	Lua_Bot(Bot *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_Bot() { }

	operator Bot*() {
		return reinterpret_cast<Bot*>(GetLuaPtrData());
	}
};

#endif
#endif
#endif