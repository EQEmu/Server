#ifndef EQEMU_LUA_ENTITY_H
#define EQEMU_LUA_ENTITY_H
#ifdef LUA_EQEMU

class Entity;
class Lua_Client;
class Lua_NPC;
class Lua_Mob;
//class Lua_Merc;
//class Lua_Corpse;
//class Lua_Object;
//class Lua_Doors;
//class Lua_Trap;
//class Lua_Item;

//TODO: Remove the error checking by a flag since this adds significant overhead to each c call
#define Lua_Safe_Call_Void(Type) if(!d_) { return; } Type *self = reinterpret_cast<Type*>(d_);
#define Lua_Safe_Call_Bool(Type) if(!d_) { return false; } Type *self = reinterpret_cast<Type*>(d_);
#define Lua_Safe_Call_Int(Type) if(!d_) { return 0; } Type *self = reinterpret_cast<Type*>(d_);
#define Lua_Safe_Call_Real(Type) if(!d_) { return 0.0; } Type *self = reinterpret_cast<Type*>(d_);
#define Lua_Safe_Call_String(Type) if(!d_) { return ""; } Type *self = reinterpret_cast<Type*>(d_);
#define Lua_Safe_Call_Entity(Type) if(!d_) { return Lua_Entity(); } Type *self = reinterpret_cast<Type*>(d_);
#define Lua_Safe_Call_Mob(Type) if(!d_) { return Lua_Mob(); } Type *self = reinterpret_cast<Type*>(d_);
#define Lua_Safe_Call_NPC(Type) if(!d_) { return Lua_NPC(); } Type *self = reinterpret_cast<Type*>(d_);
#define Lua_Safe_Call_Client(Type) if(!d_) { return Lua_Client(); } Type *self = reinterpret_cast<Type*>(d_);

class Lua_Entity
{
public:
	Lua_Entity() { d_ = nullptr; }
	Lua_Entity(Entity *d) : d_(d) { }
	virtual ~Lua_Entity() { }

	operator Entity* () {
		if(d_) {
			return reinterpret_cast<Entity*>(d_);
		}

		return nullptr;
	}

	bool Null();
	bool Valid();
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

	Lua_Client CastToClient();
	Lua_NPC CastToNPC();
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