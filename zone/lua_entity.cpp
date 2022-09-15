#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "entity.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_corpse.h"
#include "lua_object.h"
#include "lua_door.h"

#ifdef BOTS
#include "lua_bot.h"
#endif

void lua_register_entity(sol::state_view &sv)
{
	auto entity = sv.new_usertype<Lua_Entity>("Entity", sol::constructors<Lua_Entity()>());
	entity["null"] = sol::readonly_property(&Lua_Entity::Null);
	entity["valid"] = sol::readonly_property(&Lua_Entity::Valid);
#ifdef BOTS
	entity["CastToBot"] = &Lua_Entity::CastToBot;
#endif
	entity["CastToClient"] = &Lua_Entity::CastToClient;
	entity["CastToCorpse"] = &Lua_Entity::CastToCorpse;
	entity["CastToDoor"] = &Lua_Entity::CastToDoor;
	entity["CastToMob"] = &Lua_Entity::CastToMob;
	entity["CastToNPC"] = &Lua_Entity::CastToNPC;
	entity["CastToObject"] = &Lua_Entity::CastToObject;
	entity["GetID"] = &Lua_Entity::GetID;
	entity["IsBeacon"] = &Lua_Entity::IsBeacon;
	entity["IsBot"] = &Lua_Entity::IsBot;
	entity["IsClient"] = &Lua_Entity::IsClient;
	entity["IsCorpse"] = &Lua_Entity::IsCorpse;
	entity["IsDoor"] = &Lua_Entity::IsDoor;
	entity["IsEncounter"] = &Lua_Entity::IsEncounter;
	entity["IsMerc"] = &Lua_Entity::IsMerc;
	entity["IsMob"] = &Lua_Entity::IsMob;
	entity["IsNPC"] = &Lua_Entity::IsNPC;
	entity["IsNPCCorpse"] = &Lua_Entity::IsNPCCorpse;
	entity["IsObject"] = &Lua_Entity::IsObject;
	entity["IsPlayerCorpse"] = &Lua_Entity::IsPlayerCorpse;
	entity["IsTrap"] = &Lua_Entity::IsTrap;
}

#endif
