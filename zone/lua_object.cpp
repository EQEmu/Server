#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "object.h"
#include "lua_object.h"

void lua_register_object(sol::state_view &sv)
{
	auto object = sv.new_usertype<Lua_Object>("Object", sol::constructors<Lua_Object()>(), sol::base_classes, sol::bases<Lua_Entity>());
	object["null"] = sol::readonly_property(&Lua_Object::Null);
	object["valid"] = sol::readonly_property(&Lua_Object::Valid);
	object["ClearUser"] = (void(Lua_Object::*)(void))&Lua_Object::ClearUser;
	object["Close"] = (void(Lua_Object::*)(void))&Lua_Object::Close;
	object["Delete"] = sol::overload((void(Lua_Object::*)(bool)) & Lua_Object::Delete,
					 (void(Lua_Object::*)(void)) & Lua_Object::Delete);
	object["DeleteItem"] = (void(Lua_Object::*)(int))&Lua_Object::DeleteItem;
	object["Depop"] = (void(Lua_Object::*)(void))&Lua_Object::Depop;
	object["EntityVariableExists"] = (bool(Lua_Object::*)(const char*))&Lua_Object::EntityVariableExists;
	object["GetDBID"] = (uint32(Lua_Object::*)(void))&Lua_Object::GetDBID;
	object["GetEntityVariable"] = (const char*(Lua_Object::*)(const char*))&Lua_Object::GetEntityVariable;
	object["GetHeading"] = (float(Lua_Object::*)(void))&Lua_Object::GetHeading;
	object["GetID"] = (int(Lua_Object::*)(void))&Lua_Object::GetID;
	object["GetIcon"] = (uint32(Lua_Object::*)(void))&Lua_Object::GetIcon;
	object["GetItemID"] = (uint32(Lua_Object::*)(void))&Lua_Object::GetItemID;
	object["GetModelName"] = (const char*(Lua_Object::*)(void))&Lua_Object::GetModelName;
	object["GetType"] = (uint32(Lua_Object::*)(void))&Lua_Object::GetType;
	object["GetX"] = (float(Lua_Object::*)(void))&Lua_Object::GetX;
	object["GetY"] = (float(Lua_Object::*)(void))&Lua_Object::GetY;
	object["GetZ"] = (float(Lua_Object::*)(void))&Lua_Object::GetZ;
	object["IsGroundSpawn"] = (bool(Lua_Object::*)(void))&Lua_Object::IsGroundSpawn;
	object["Repop"] = (void(Lua_Object::*)(void))&Lua_Object::Repop;
	object["Save"] = (bool(Lua_Object::*)(void))&Lua_Object::Save;
	object["SetEntityVariable"] = (void(Lua_Object::*)(const char*,const char*))&Lua_Object::SetEntityVariable;
	object["SetHeading"] = (void(Lua_Object::*)(float))&Lua_Object::SetHeading;
	object["SetID"] = (void(Lua_Object::*)(int))&Lua_Object::SetID;
	object["SetIcon"] = (void(Lua_Object::*)(uint32))&Lua_Object::SetIcon;
	object["SetItemID"] = (void(Lua_Object::*)(uint32))&Lua_Object::SetItemID;
	object["SetLocation"] = (void(Lua_Object::*)(float,float,float))&Lua_Object::SetLocation;
	object["SetModelName"] = (void(Lua_Object::*)(const char*))&Lua_Object::SetModelName;
	object["SetType"] = (void(Lua_Object::*)(uint32))&Lua_Object::SetType;
	object["SetX"] = (void(Lua_Object::*)(float))&Lua_Object::SetX;
	object["SetY"] = (void(Lua_Object::*)(float))&Lua_Object::SetY;
	object["SetZ"] = (void(Lua_Object::*)(float))&Lua_Object::SetZ;
	object["StartDecay"] = (void(Lua_Object::*)(void))&Lua_Object::StartDecay;
	object["VarSave"] = (uint32(Lua_Object::*)(void))&Lua_Object::VarSave;
}

#endif
