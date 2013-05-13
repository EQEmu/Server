#ifndef EQEMU_LUA_CLIENT_H
#define EQEMU_LUA_CLIENT_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Client;

class Lua_Client : public Lua_Mob
{
	typedef Client NativeType;
public:
	Lua_Client() { d_ = nullptr; }
	Lua_Client(NativeType *d) { d_ = d; }
	virtual ~Lua_Client() { }

	operator NativeType* () {
		if(d_) {
			return reinterpret_cast<NativeType*>(d_);
		}

		return nullptr;
	}
};

#endif
#endif