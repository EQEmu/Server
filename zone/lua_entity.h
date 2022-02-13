#ifndef EQEMU_LUA_ENTITY_H
#define EQEMU_LUA_ENTITY_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class Entity;
class Lua_Client;
#ifdef BOTS
class Lua_Bot;
#endif
class Lua_NPC;
class Lua_Mob;
struct Lua_HateList;
class Lua_Item;
class Lua_ItemInst;
class Lua_Corpse;
class Lua_Object;
class Lua_Door;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_entity();

class Lua_Entity : public Lua_Ptr<Entity>
{
	typedef Entity NativeType;
public:
	Lua_Entity() : Lua_Ptr(nullptr) { }
	Lua_Entity(Entity *d) : Lua_Ptr(d) { }
	virtual ~Lua_Entity() { }

	operator Entity*() {
		return reinterpret_cast<Entity*>(GetLuaPtrData());
	}

	bool IsClient();
	bool IsNPC();
	bool IsMob();
	bool IsMerc();
	bool IsCorpse();
	bool IsPlayerCorpse();
	bool IsNPCCorpse();
	bool IsObject();
	bool IsDoor();
	bool IsTrap();
	bool IsBeacon();
	bool IsEncounter();
	bool IsBot();
	int GetID();

	Lua_Client CastToClient();
	Lua_NPC CastToNPC();
	Lua_Mob CastToMob();
	Lua_Corpse CastToCorpse();
	Lua_Object CastToObject();
	Lua_Door CastToDoor();
#ifdef BOTS
	Lua_Bot CastToBot();
#endif
};

#endif
#endif