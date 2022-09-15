#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "groups.h"
#include "masterentity.h"
#include "lua_group.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

void lua_register_group(sol::state_view &sv)
{
	auto group = sv.new_usertype<Lua_Group>("Group", sol::constructors<Lua_Group()>());
	group["null"] = sol::readonly_property(&Lua_Group::Null);
	group["valid"] = sol::readonly_property(&Lua_Group::Valid);
	group["CastGroupSpell"] = (void(Lua_Group::*)(Lua_Mob,int))&Lua_Group::CastGroupSpell;
	group["DisbandGroup"] = (void(Lua_Group::*)(void))&Lua_Group::DisbandGroup;
	group["DoesAnyMemberHaveExpeditionLockout"] = sol::overload(
	    (bool(Lua_Group::*)(std::string, std::string)) & Lua_Group::DoesAnyMemberHaveExpeditionLockout,
	    (bool(Lua_Group::*)(std::string, std::string, int)) & Lua_Group::DoesAnyMemberHaveExpeditionLockout);
	group["GetHighestLevel"] = (int(Lua_Group::*)(void))&Lua_Group::GetHighestLevel;
	group["GetID"] = (int(Lua_Group::*)(void))&Lua_Group::GetID;
	group["GetLeader"] = (Lua_Mob(Lua_Group::*)(void))&Lua_Group::GetLeader;
	group["GetLeaderName"] = (const char*(Lua_Group::*)(void))&Lua_Group::GetLeaderName;
	group["GetLowestLevel"] = (int(Lua_Group::*)(void))&Lua_Group::GetLowestLevel;
	group["GetMember"] = (Lua_Mob(Lua_Group::*)(int))&Lua_Group::GetMember;
	group["GetTotalGroupDamage"] = (uint32(Lua_Group::*)(Lua_Mob))&Lua_Group::GetTotalGroupDamage;
	group["GroupCount"] = (int(Lua_Group::*)(void))&Lua_Group::GroupCount;
	group["GroupMessage"] = (void(Lua_Group::*)(Lua_Mob,int,const char* message))&Lua_Group::GroupMessage;
	group["IsGroupMember"] = (bool(Lua_Group::*)(Lua_Mob))&Lua_Group::IsGroupMember;
	group["IsLeader"] = (bool(Lua_Group::*)(Lua_Mob))&Lua_Group::IsLeader;
	group["SetLeader"] = (void(Lua_Group::*)(Lua_Mob))&Lua_Group::SetLeader;
	group["SplitExp"] = (void(Lua_Group::*)(uint32,Lua_Mob))&Lua_Group::SplitExp;
	group["SplitMoney"] =
	    sol::overload((void(Lua_Group::*)(uint32, uint32, uint32, uint32)) & Lua_Group::SplitMoney,
			  (void(Lua_Group::*)(uint32, uint32, uint32, uint32, Lua_Client)) & Lua_Group::SplitMoney);
	group["TeleportGroup"] = (void(Lua_Group::*)(Lua_Mob,uint32,uint32,float,float,float,float))&Lua_Group::TeleportGroup;
}

#endif
