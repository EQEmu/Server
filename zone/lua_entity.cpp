#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

bool Lua_Entity::Null() {
	return d_ == nullptr;
}

bool Lua_Entity::Valid() {
	return d_ != nullptr;
}

bool Lua_Entity::IsClient() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsClient();
}

bool Lua_Entity::IsNPC() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsNPC();
}

bool Lua_Entity::IsMob() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsMob();
}

bool Lua_Entity::IsMerc() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsMerc();
}

bool Lua_Entity::IsCorpse() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsCorpse();
}

bool Lua_Entity::IsPlayerCorpse() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsPlayerCorpse();
}

bool Lua_Entity::IsNPCCorpse() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsNPCCorpse();
}

bool Lua_Entity::IsObject() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsObject();
}

bool Lua_Entity::IsDoor() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsDoor();
}

bool Lua_Entity::IsTrap() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsTrap();
}

bool Lua_Entity::IsBeacon() {
	Lua_Safe_Call_Bool(Entity);
	return self->IsBeacon();
}

int Lua_Entity::GetID() {
	Lua_Safe_Call_Bool(Entity);
	return self->GetID();
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
