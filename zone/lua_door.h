#ifndef EQEMU_LUA_DOOR_H
#define EQEMU_LUA_DOOR_H
#ifdef LUA_EQEMU

#include "lua_entity.h"

class Doors;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_door();

class Lua_Door : public Lua_Entity
{
	typedef Doors NativeType;
public:
	Lua_Door() { }
	Lua_Door(Doors *d) { SetLuaPtrData(d); }
	virtual ~Lua_Door() { }

	operator Doors*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Doors*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
