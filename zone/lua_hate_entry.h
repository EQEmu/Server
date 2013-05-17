#ifndef EQEMU_LUA_HATE_ENTRY_H
#define EQEMU_LUA_HATE_ENTRY_H
#ifdef LUA_EQEMU

class Lua_Mob;

struct Lua_HateEntry
{
	Lua_HateEntry() { }
	virtual ~Lua_HateEntry() { }
	
	Lua_Mob ent;
	int damage;
	int hate;
	bool frenzy;
};

#endif
#endif
