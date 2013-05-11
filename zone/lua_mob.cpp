#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_mob.h"

const char *Lua_Mob::GetName() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetName();
}

void Lua_Mob::Depop() {
	Depop(true);
}

void Lua_Mob::Depop(bool start_spawn_timer) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->Depop(start_spawn_timer);
}

#endif
