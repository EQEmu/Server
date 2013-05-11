#ifndef EQEMU_LUA_CLIENT_H
#define EQEMU_LUA_CLIENT_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Client;

class Lua_Client : public Lua_Mob
{
public:
	Lua_Client() { d_ = nullptr; }
	Lua_Client(Client *d) { d_ = d; }
	virtual ~Lua_Client() { }
};

#endif
#endif