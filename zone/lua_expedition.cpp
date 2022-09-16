#ifdef LUA_EQEMU

#include <sol/sol.hpp>
#include "lua_expedition.h"
#include "expedition.h"
#include "../common/zone_store.h"

void lua_register_expedition(sol::state_view &sv)
{
	auto expedition = sv.new_usertype<Lua_Expedition>("Expedition", sol::constructors<Lua_Expedition()>());
	expedition["null"] = sol::readonly_property(&Lua_Expedition::Null);
	expedition["valid"] = sol::readonly_property(&Lua_Expedition::Valid);
	expedition["AddLockout"] = (void(Lua_Expedition::*)(std::string, uint32_t))&Lua_Expedition::AddLockout;
	expedition["AddLockoutDuration"] =
	    sol::overload((void(Lua_Expedition::*)(std::string, int)) & Lua_Expedition::AddLockoutDuration,
			  (void(Lua_Expedition::*)(std::string, int, bool)) & Lua_Expedition::AddLockoutDuration);
	expedition["AddReplayLockout"] = (void(Lua_Expedition::*)(uint32_t))&Lua_Expedition::AddReplayLockout;
	expedition["AddReplayLockoutDuration"] =
	    sol::overload((void(Lua_Expedition::*)(int)) & Lua_Expedition::AddReplayLockoutDuration,
			  (void(Lua_Expedition::*)(int, bool)) & Lua_Expedition::AddReplayLockoutDuration);
	expedition["GetDynamicZoneID"] = &Lua_Expedition::GetDynamicZoneID;
	expedition["GetID"] = (uint32_t(Lua_Expedition::*)(void))&Lua_Expedition::GetID;
	expedition["GetInstanceID"] = (int(Lua_Expedition::*)(void))&Lua_Expedition::GetInstanceID;
	expedition["GetLeaderName"] = (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetLeaderName;
	expedition["GetLockouts"] = &Lua_Expedition::GetLockouts;
	expedition["GetLootEventByNPCTypeID"] = (std::string(Lua_Expedition::*)(uint32_t))&Lua_Expedition::GetLootEventByNPCTypeID;
	expedition["GetLootEventBySpawnID"] = (std::string(Lua_Expedition::*)(uint32_t))&Lua_Expedition::GetLootEventBySpawnID;
	expedition["GetMemberCount"] = (uint32_t(Lua_Expedition::*)(void))&Lua_Expedition::GetMemberCount;
	expedition["GetMembers"] = &Lua_Expedition::GetMembers;
	expedition["GetName"] = (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetName;
	expedition["GetSecondsRemaining"] = (int(Lua_Expedition::*)(void))&Lua_Expedition::GetSecondsRemaining;
	expedition["GetUUID"] = (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetUUID;
	expedition["GetZoneID"] = (int(Lua_Expedition::*)(void))&Lua_Expedition::GetZoneID;
	expedition["GetZoneName"] = &Lua_Expedition::GetZoneName;
	expedition["GetZoneVersion"] = &Lua_Expedition::GetZoneVersion;
	expedition["HasLockout"] = (bool(Lua_Expedition::*)(std::string))&Lua_Expedition::HasLockout;
	expedition["HasReplayLockout"] = (bool(Lua_Expedition::*)(void))&Lua_Expedition::HasReplayLockout;
	expedition["IsLocked"] = &Lua_Expedition::IsLocked;
	expedition["RemoveCompass"] = (void(Lua_Expedition::*)(void))&Lua_Expedition::RemoveCompass;
	expedition["RemoveLockout"] = (void(Lua_Expedition::*)(std::string))&Lua_Expedition::RemoveLockout;
	expedition["SetCompass"] =
	    sol::overload((void(Lua_Expedition::*)(uint32_t, float, float, float)) & Lua_Expedition::SetCompass,
			  (void(Lua_Expedition::*)(std::string, float, float, float)) & Lua_Expedition::SetCompass);
	expedition["SetLocked"] =
	    sol::overload((void(Lua_Expedition::*)(bool)) & Lua_Expedition::SetLocked,
			  (void(Lua_Expedition::*)(bool, int)) & Lua_Expedition::SetLocked,
			  (void(Lua_Expedition::*)(bool, int, uint32_t)) & Lua_Expedition::SetLocked);
	expedition["SetLootEventByNPCTypeID"] = (void(Lua_Expedition::*)(uint32_t, std::string))&Lua_Expedition::SetLootEventByNPCTypeID;
	expedition["SetLootEventBySpawnID"] = (void(Lua_Expedition::*)(uint32_t, std::string))&Lua_Expedition::SetLootEventBySpawnID;
	expedition["SetReplayLockoutOnMemberJoin"] = (void(Lua_Expedition::*)(bool))&Lua_Expedition::SetReplayLockoutOnMemberJoin;
	expedition["SetSafeReturn"] = sol::overload(
	    (void(Lua_Expedition::*)(uint32_t, float, float, float, float)) & Lua_Expedition::SetSafeReturn,
	    (void(Lua_Expedition::*)(std::string, float, float, float, float)) & Lua_Expedition::SetSafeReturn);
	expedition["SetSecondsRemaining"] = &Lua_Expedition::SetSecondsRemaining;
	expedition["SetSwitchID"] = &Lua_Expedition::SetSwitchID;
	expedition["SetZoneInLocation"] = (void(Lua_Expedition::*)(float, float, float, float))&Lua_Expedition::SetZoneInLocation;
	expedition["UpdateLockoutDuration"] = sol::overload(
	    (void(Lua_Expedition::*)(std::string, uint32_t)) & Lua_Expedition::UpdateLockoutDuration,
	    (void(Lua_Expedition::*)(std::string, uint32_t, bool)) & Lua_Expedition::UpdateLockoutDuration);
}

void lua_register_expedition_lock_messages(sol::state_view &sv)
{
	sv.new_enum<ExpeditionLockMessage>("ExpeditionLockMessage",
		{
			{ "None", ExpeditionLockMessage::None },
			{ "Close", ExpeditionLockMessage::Close },
			{ "Begin", ExpeditionLockMessage::Begin }
		}
	);
}

#endif // LUA_EQEMU
