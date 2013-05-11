#ifndef EQEMU_LUA_ENTITY_H
#define EQEMU_LUA_ENTITY_H
#ifdef LUA_EQEMU

class Entity;
//class Lua_Client;
//class Lua_NPC;
class Lua_Mob;
//class Lua_Merc;
//class Lua_Corpse;
//class Lua_Object;
//class Lua_Doors;
//class Lua_Trap;
//class Lua_Beacon;

class Lua_Entity
{
public:
	Lua_Entity() { }
	Lua_Entity(Entity *d) : d_(d) { }
	virtual ~Lua_Entity() { }

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
	int GetID();

	//Lua_Client CastToClient();
	//Lua_NPC CastToNPC();
	Lua_Mob CastToMob();
	//Lua_Merc CastToMerc();
	//Lua_Corpse CastToCorpse();
	//Lua_Object CastToObject();
	//Lua_Doors CastToDoors();
	//Lua_Trap CastToTrap();
	//Lua_Beacon CastToBeacon();

	void *d_;
};

#endif
#endif