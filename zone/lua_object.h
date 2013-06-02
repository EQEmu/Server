#ifndef EQEMU_LUA_OBJECT_H
#define EQEMU_LUA_OBJECT_H
#ifdef LUA_EQEMU

#include "lua_entity.h"

class Object;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_object();

class Lua_Object : public Lua_Entity
{
	typedef Object NativeType;
public:
	Lua_Object() { }
	Lua_Object(Object *d) { SetLuaPtrData(d); }
	virtual ~Lua_Object() { }

	operator Object*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Object*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
