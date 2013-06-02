#ifndef EQEMU_LUA_GROUP_H
#define EQEMU_LUA_GROUP_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class Group;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_group();

class Lua_Group : public Lua_Ptr<void>
{
	typedef Group NativeType;
public:
	Lua_Group() { }
	Lua_Group(Group *d) : Lua_Ptr(d) { }
	virtual ~Lua_Group() { }

	operator Group*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Group*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
