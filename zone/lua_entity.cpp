#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

bool Lua_Entity::IsClient() {
	Lua_Safe_Call_Bool();
	return self->IsClient();
}

bool Lua_Entity::IsNPC() {
	Lua_Safe_Call_Bool();
	return self->IsNPC();
}

bool Lua_Entity::IsMob() {
	Lua_Safe_Call_Bool();
	return self->IsMob();
}

bool Lua_Entity::IsMerc() {
	Lua_Safe_Call_Bool();
	return self->IsMerc();
}

bool Lua_Entity::IsCorpse() {
	Lua_Safe_Call_Bool();
	return self->IsCorpse();
}

bool Lua_Entity::IsPlayerCorpse() {
	Lua_Safe_Call_Bool();
	return self->IsPlayerCorpse();
}

bool Lua_Entity::IsNPCCorpse() {
	Lua_Safe_Call_Bool();
	return self->IsNPCCorpse();
}

bool Lua_Entity::IsObject() {
	Lua_Safe_Call_Bool();
	return self->IsObject();
}

bool Lua_Entity::IsDoor() {
	Lua_Safe_Call_Bool();
	return self->IsDoor();
}

bool Lua_Entity::IsTrap() {
	Lua_Safe_Call_Bool();
	return self->IsTrap();
}

bool Lua_Entity::IsBeacon() {
	Lua_Safe_Call_Bool();
	return self->IsBeacon();
}

int Lua_Entity::GetID() {
	Lua_Safe_Call_Bool();
	return self->GetID();
}

Lua_Client Lua_Entity::CastToClient() {
	void *d = GetLuaPtrData();
	Client *m = reinterpret_cast<Client*>(d);
	return Lua_Client(m);
}

Lua_NPC Lua_Entity::CastToNPC() {
	void *d = GetLuaPtrData();
	NPC *m = reinterpret_cast<NPC*>(d);
	return Lua_NPC(m);
}

Lua_Mob Lua_Entity::CastToMob() {
	void *d = GetLuaPtrData();
	Mob *m = reinterpret_cast<Mob*>(d);
	return Lua_Mob(m);
}

luabind::scope lua_register_entity() {
	return luabind::class_<Lua_Entity>("Entity")
		.def(luabind::constructor<>())
		.property("null", &Lua_Entity::Null)
		.property("valid", &Lua_Entity::Valid)
		.def("IsClient", &Lua_Entity::IsClient)
		.def("IsNPC", &Lua_Entity::IsNPC)
		.def("IsMob", &Lua_Entity::IsMob)
		.def("IsMerc", &Lua_Entity::IsMerc)
		.def("IsCorpse", &Lua_Entity::IsCorpse)
		.def("IsPlayerCorpse", &Lua_Entity::IsPlayerCorpse)
		.def("IsNPCCorpse", &Lua_Entity::IsNPCCorpse)
		.def("IsObject", &Lua_Entity::IsObject)
		.def("IsDoor", &Lua_Entity::IsDoor)
		.def("IsTrap", &Lua_Entity::IsTrap)
		.def("IsBeacon", &Lua_Entity::IsBeacon)
		.def("GetID", &Lua_Entity::GetID)
		.def("CastToClient", &Lua_Entity::CastToClient)
		.def("CastToNPC", &Lua_Entity::CastToNPC)
		.def("CastToMob", &Lua_Entity::CastToMob);
}

#endif
