#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

bool Lua_Entity::IsClient() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsClient();
}

bool Lua_Entity::IsNPC() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsNPC();
}

bool Lua_Entity::IsMob() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsMob();
}

bool Lua_Entity::IsMerc() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsMerc();
}

bool Lua_Entity::IsCorpse() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsCorpse();
}

bool Lua_Entity::IsPlayerCorpse() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsPlayerCorpse();
}

bool Lua_Entity::IsNPCCorpse() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsNPCCorpse();
}

bool Lua_Entity::IsObject() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsObject();
}

bool Lua_Entity::IsDoor() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsDoor();
}

bool Lua_Entity::IsTrap() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsTrap();
}

bool Lua_Entity::IsBeacon() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->IsBeacon();
}

int Lua_Entity::GetID() {
	Entity *ent = reinterpret_cast<Entity*>(d_);
	return ent->GetID();
}

Lua_Client Lua_Entity::CastToClient() {
	Client *m = reinterpret_cast<Client*>(d_);
	return Lua_Client(m);
}

Lua_NPC Lua_Entity::CastToNPC() {
	NPC *m = reinterpret_cast<NPC*>(d_);
	return Lua_NPC(m);
}

Lua_Mob Lua_Entity::CastToMob() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return Lua_Mob(m);
}

#endif
