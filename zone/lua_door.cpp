#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "doors.h"
#include "lua_mob.h"
#include "lua_door.h"

void lua_register_door(sol::state_view &sv)
{
	auto door =
	    sv.new_usertype<Lua_Door>("Door", sol::constructors<Lua_Door()>(), sol::base_classes, sol::bases<Lua_Entity>());
	door["null"] = sol::readonly_property(&Lua_Door::Null);
	door["valid"] = sol::readonly_property(&Lua_Door::Valid);
	door["CreateDatabaseEntry"] = &Lua_Door::CreateDatabaseEntry;
	door["ForceClose"] = sol::overload((void(Lua_Door::*)(Lua_Mob))&Lua_Door::ForceClose,
					   (void(Lua_Door::*)(Lua_Mob, bool)) & Lua_Door::ForceClose);
	door["ForceOpen"] = sol::overload((void(Lua_Door::*)(Lua_Mob))&Lua_Door::ForceOpen,
					  (void(Lua_Door::*)(Lua_Mob, bool))&Lua_Door::ForceOpen);
	door["GetDisableTimer"] = &Lua_Door::GetDisableTimer;
	door["GetDoorDBID"] = &Lua_Door::GetDoorDBID;
	door["GetDoorID"] = &Lua_Door::GetDoorID;
	door["GetDoorName"] = &Lua_Door::GetDoorName;
	door["GetHeading"] = &Lua_Door::GetHeading;
	door["GetIncline"] = &Lua_Door::GetIncline;
	door["GetKeyItem"] = &Lua_Door::GetKeyItem;
	door["GetLockPick"] = &Lua_Door::GetLockPick;
	door["GetNoKeyring"] = &Lua_Door::GetNoKeyring;
	door["GetOpenType"] = &Lua_Door::GetOpenType;
	door["GetSize"] = &Lua_Door::GetSize;
	door["GetX"] = &Lua_Door::GetX;
	door["GetY"] = &Lua_Door::GetY;
	door["GetZ"] = &Lua_Door::GetZ;
	door["SetDisableTimer"] = &Lua_Door::SetDisableTimer;
	door["SetDoorName"] = &Lua_Door::SetDoorName;
	door["SetHeading"] = &Lua_Door::SetHeading;
	door["SetIncline"] = &Lua_Door::SetIncline;
	door["SetKeyItem"] = &Lua_Door::SetKeyItem;
	door["SetLocation"] = &Lua_Door::SetLocation;
	door["SetLockPick"] = &Lua_Door::SetLockPick;
	door["SetNoKeyring"] = &Lua_Door::SetNoKeyring;
	door["SetOpenType"] = &Lua_Door::SetOpenType;
	door["SetSize"] = &Lua_Door::SetSize;
	door["SetX"] = &Lua_Door::SetX;
	door["SetY"] = &Lua_Door::SetY;
	door["SetZ"] = &Lua_Door::SetZ;
}

#endif
