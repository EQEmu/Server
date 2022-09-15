#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "spawn2.h"
#include "lua_npc.h"
#include "lua_spawn.h"

void lua_register_spawn(sol::state_view &sv)
{
	auto spawn = sv.new_usertype<Lua_Spawn>("Spawn", sol::constructors<Lua_Spawn()>());
	spawn["null"] = sol::property(&Lua_Spawn::Null);
	spawn["valid"] = sol::property(&Lua_Spawn::Valid);
	spawn["CurrentNPCID"] = (uint32(Lua_Spawn::*)(void))&Lua_Spawn::CurrentNPCID;
	spawn["Depop"] = (void(Lua_Spawn::*)(void))&Lua_Spawn::Depop;
	spawn["Disable"] = (void(Lua_Spawn::*)(void))&Lua_Spawn::Disable;
	spawn["Enable"] = (void(Lua_Spawn::*)(void))&Lua_Spawn::Enable;
	spawn["Enabled"] = (bool(Lua_Spawn::*)(void))&Lua_Spawn::Enabled;
	spawn["ForceDespawn"] = (void(Lua_Spawn::*)(void))&Lua_Spawn::ForceDespawn;
	spawn["GetHeading"] = (float(Lua_Spawn::*)(void))&Lua_Spawn::GetHeading;
	spawn["GetID"] = (uint32(Lua_Spawn::*)(void))&Lua_Spawn::GetID;
	spawn["GetKillCount"] = (uint32(Lua_Spawn::*)(void))&Lua_Spawn::GetKillCount;
	spawn["GetSpawnCondition"] = (uint32(Lua_Spawn::*)(void))&Lua_Spawn::GetSpawnCondition;
	spawn["GetVariance"] = (uint32(Lua_Spawn::*)(void))&Lua_Spawn::GetVariance;
	spawn["GetX"] = (float(Lua_Spawn::*)(void))&Lua_Spawn::GetX;
	spawn["GetY"] = (float(Lua_Spawn::*)(void))&Lua_Spawn::GetY;
	spawn["GetZ"] = (float(Lua_Spawn::*)(void))&Lua_Spawn::GetZ;
	spawn["LoadGrid"] = (void(Lua_Spawn::*)(void))&Lua_Spawn::LoadGrid;
	spawn["NPCPointerValid"] = (bool(Lua_Spawn::*)(void))&Lua_Spawn::NPCPointerValid;
	spawn["Repop"] = sol::overload((void(Lua_Spawn::*)(uint32)) & Lua_Spawn::Repop,
				       (void(Lua_Spawn::*)(void)) & Lua_Spawn::Repop);
	spawn["Reset"] = (void(Lua_Spawn::*)(void))&Lua_Spawn::Reset;
	spawn["RespawnTimer"] = (uint32(Lua_Spawn::*)(void))&Lua_Spawn::RespawnTimer;
	spawn["SetCurrentNPCID"] = (void(Lua_Spawn::*)(uint32))&Lua_Spawn::SetCurrentNPCID;
	spawn["SetNPCPointer"] = (void(Lua_Spawn::*)(Lua_NPC))&Lua_Spawn::SetNPCPointer;
	spawn["SetRespawnTimer"] = (void(Lua_Spawn::*)(uint32))&Lua_Spawn::SetRespawnTimer;
	spawn["SetTimer"] = (void(Lua_Spawn::*)(uint32))&Lua_Spawn::SetTimer;
	spawn["SetVariance"] = (void(Lua_Spawn::*)(uint32))&Lua_Spawn::SetVariance;
	spawn["SpawnGroupID"] = (uint32(Lua_Spawn::*)(void))&Lua_Spawn::SpawnGroupID;
}

#endif
