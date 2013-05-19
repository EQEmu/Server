#ifndef EQEMU_LUA_CLIENT_H
#define EQEMU_LUA_CLIENT_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Client;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_client();

class Lua_Client : public Lua_Mob
{
	typedef Client NativeType;
public:
	Lua_Client() { }
	Lua_Client(Client *d) { SetLuaPtrData(d); }
	virtual ~Lua_Client() { }

	operator Client*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Client*>(d);
		}

		return nullptr;
	}
};

#endif
#endif