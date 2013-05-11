#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_mob.h"

const char *Lua_Mob::GetName() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetName();
}

#endif
