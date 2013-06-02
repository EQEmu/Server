#ifndef EQEMU_LUA_RAID_H
#define EQEMU_LUA_RAID_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class Raid;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_raid();

class Lua_Raid : public Lua_Ptr<void>
{
	typedef Raid NativeType;
public:
	Lua_Raid() { }
	Lua_Raid(Raid *d) : Lua_Ptr(d) { }
	virtual ~Lua_Raid() { }

	operator Raid*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Raid*>(d);
		}

		return nullptr;
	}
};

#endif
#endif
