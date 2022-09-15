#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "masterentity.h"
#include "lua_raid.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

void lua_register_raid(sol::state_view &sv) {
	auto raid = sv.new_usertype<Lua_Raid>("Raid", sol::constructors<Lua_Raid()>());
	raid["null"] = sol::property(&Lua_Raid::Null);
	raid["valid"] = sol::property(&Lua_Raid::Valid);
	raid["BalanceHP"] = (void(Lua_Raid::*)(int,uint32))&Lua_Raid::BalanceHP;
	raid["CastGroupSpell"] = (void(Lua_Raid::*)(Lua_Mob,int,uint32))&Lua_Raid::CastGroupSpell;
	raid["DoesAnyMemberHaveExpeditionLockout"] = sol::overload(
	    (bool(Lua_Raid::*)(std::string, std::string)) & Lua_Raid::DoesAnyMemberHaveExpeditionLockout,
	    (bool(Lua_Raid::*)(std::string, std::string, int)) & Lua_Raid::DoesAnyMemberHaveExpeditionLockout);
	raid["GetClientByIndex"] = (Lua_Client(Lua_Raid::*)(int))&Lua_Raid::GetClientByIndex;
	raid["GetGroup"] = sol::overload((int(Lua_Raid::*)(Lua_Client)) & Lua_Raid::GetGroup,
					 (int(Lua_Raid::*)(const char *)) & Lua_Raid::GetGroup);
	raid["GetGroupNumber"] = (int(Lua_Raid::*)(int))&Lua_Raid::GetGroupNumber;
	raid["GetHighestLevel"] = (int(Lua_Raid::*)(void))&Lua_Raid::GetHighestLevel;
	raid["GetID"] = (int(Lua_Raid::*)(void))&Lua_Raid::GetID;
	raid["GetLowestLevel"] = (int(Lua_Raid::*)(void))&Lua_Raid::GetLowestLevel;
	raid["GetMember"] = (Lua_Client(Lua_Raid::*)(int))&Lua_Raid::GetMember;
	raid["GetTotalRaidDamage"] = (uint32(Lua_Raid::*)(Lua_Mob))&Lua_Raid::GetTotalRaidDamage;
	raid["GroupCount"] = (int(Lua_Raid::*)(uint32))&Lua_Raid::GroupCount;
	raid["IsGroupLeader"] = (bool(Lua_Raid::*)(const char*))&Lua_Raid::IsGroupLeader;
	raid["IsLeader"] = (bool(Lua_Raid::*)(const char*))&Lua_Raid::IsLeader;
	raid["IsRaidMember"] = (bool(Lua_Raid::*)(const char*))&Lua_Raid::IsRaidMember;
	raid["RaidCount"] = (int(Lua_Raid::*)(void))&Lua_Raid::RaidCount;
	raid["SplitExp"] = (void(Lua_Raid::*)(uint32,Lua_Mob))&Lua_Raid::SplitExp;
	raid["SplitMoney"] = sol::overload(
	    (void(Lua_Raid::*)(uint32, uint32, uint32, uint32, uint32)) & Lua_Raid::SplitMoney,
	    (void(Lua_Raid::*)(uint32, uint32, uint32, uint32, uint32, Lua_Client)) & Lua_Raid::SplitMoney);
	raid["TeleportGroup"] = (int(Lua_Raid::*)(Lua_Mob,uint32,uint32,float,float,float,float,uint32))&Lua_Raid::TeleportGroup;
	raid["TeleportRaid"] = (int(Lua_Raid::*)(Lua_Mob,uint32,uint32,float,float,float,float))&Lua_Raid::TeleportRaid;
}

#endif
