#ifndef EQEMU_LUA_HATE_LIST_H
#define EQEMU_LUA_HATE_LIST_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class Lua_Mob;
struct tHateEntry;

luabind::scope lua_register_hate_entry();
luabind::scope lua_register_hate_list();

class Lua_HateEntry : public Lua_Ptr<tHateEntry>
{
	typedef tHateEntry NativeType;
public:
	Lua_HateEntry() : Lua_Ptr(nullptr) { }
	Lua_HateEntry(tHateEntry *d) : Lua_Ptr(d) { }
	virtual ~Lua_HateEntry() { }
	
	Lua_Mob GetEnt();
	void SetEnt(Lua_Mob e);
	int GetDamage();
	void SetDamage(int value);
	int GetHate();
	void SetHate(int value);
	int GetFrenzy();
	void SetFrenzy(bool value);
};

struct Lua_HateList
{
	std::vector<Lua_HateEntry> entries;
};

#endif
#endif
