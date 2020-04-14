/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifdef LUA_EQEMU

#include "lua_expedition.h"
#include "expedition.h"
#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

void Lua_Expedition::AddLockout(std::string event_name, uint32_t seconds) {
	Lua_Safe_Call_Void();
	self->AddLockout(event_name, seconds);
}

void Lua_Expedition::AddReplayLockout(uint32_t seconds) {
	Lua_Safe_Call_Void();
	self->AddReplayLockout(seconds);
}

uint32_t Lua_Expedition::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

std::string Lua_Expedition::GetLeaderName() {
	Lua_Safe_Call_String();
	return self->GetLeaderName();
}

luabind::object Lua_Expedition::GetLockouts(lua_State* L) {
	luabind::object lua_table = luabind::newtable(L);

	if (d_)
	{
		auto self = reinterpret_cast<NativeType*>(d_);
		auto lockouts = self->GetLockouts();
		for (const auto& lockout : lockouts)
		{
			lua_table[lockout.first] = lockout.second.GetSecondsRemaining();
		}
	}
	return lua_table;
}

uint32_t Lua_Expedition::GetMemberCount() {
	Lua_Safe_Call_Int();
	return self->GetMemberCount();
}

luabind::object Lua_Expedition::GetMembers(lua_State* L) {
	luabind::object lua_table = luabind::newtable(L);

	if (d_)
	{
		auto self = reinterpret_cast<NativeType*>(d_);
		for (const auto& member : self->GetMembers())
		{
			lua_table[member.name] = member.char_id;
		}
	}
	return lua_table;
}

std::string Lua_Expedition::GetName() {
	Lua_Safe_Call_String();
	return self->GetName();
}

int Lua_Expedition::GetType() {
	Lua_Safe_Call_Int();
	return static_cast<int>(self->GetType());
}

bool Lua_Expedition::HasLockout(std::string event_name) {
	Lua_Safe_Call_Bool();
	return self->HasLockout(event_name);
}

bool Lua_Expedition::HasReplayLockout() {
	Lua_Safe_Call_Bool();
	return self->HasReplayLockout();
}

void Lua_Expedition::RemoveLockout(std::string event_name) {
	Lua_Safe_Call_Void();
	self->RemoveLockout(event_name);
}

luabind::scope lua_register_expedition() {
	return luabind::class_<Lua_Expedition>("Expedition")
		.def(luabind::constructor<>())
		.property("null", &Lua_Expedition::Null)
		.property("valid", &Lua_Expedition::Valid)
		.def("AddLockout", (void(Lua_Expedition::*)(std::string, uint32_t))&Lua_Expedition::AddLockout)
		.def("AddReplayLockout", (void(Lua_Expedition::*)(uint32_t))&Lua_Expedition::AddReplayLockout)
		.def("GetID", (uint32_t(Lua_Expedition::*)(void))&Lua_Expedition::GetID)
		.def("GetLeaderName", (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetLeaderName)
		.def("GetLockouts", &Lua_Expedition::GetLockouts)
		.def("GetMemberCount", (uint32_t(Lua_Expedition::*)(void))&Lua_Expedition::GetMemberCount)
		.def("GetMembers", &Lua_Expedition::GetMembers)
		.def("GetName", (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetName)
		.def("GetType", (int(Lua_Expedition::*)(void))&Lua_Expedition::GetType)
		.def("HasLockout", (bool(Lua_Expedition::*)(std::string))&Lua_Expedition::HasLockout)
		.def("HasReplayLockout", (bool(Lua_Expedition::*)())&Lua_Expedition::HasReplayLockout)
		.def("RemoveLockout", (void(Lua_Expedition::*)(std::string))&Lua_Expedition::RemoveLockout);
}

luabind::scope lua_register_expedition_member_status() {
	return luabind::class_<ExpeditionMemberStatus>("ExpeditionMemberStatus")
		.enum_("constants")
		[
			luabind::value("Unknown", static_cast<int>(ExpeditionMemberStatus::Unknown)),
			luabind::value("Online", static_cast<int>(ExpeditionMemberStatus::Online)),
			luabind::value("Offline", static_cast<int>(ExpeditionMemberStatus::Offline)),
			luabind::value("InDynamicZone", static_cast<int>(ExpeditionMemberStatus::InDynamicZone)),
			luabind::value("LinkDead", static_cast<int>(ExpeditionMemberStatus::LinkDead))
		];
}

luabind::scope lua_register_dynamiczone_types() {
	return luabind::class_<DynamicZoneType>("DynamicZoneType")
		.enum_("constants")
		[
			luabind::value("None", static_cast<int>(DynamicZoneType::None)),
			luabind::value("Expedition", static_cast<int>(DynamicZoneType::Expedition)),
			luabind::value("Tutorial", static_cast<int>(DynamicZoneType::Tutorial)),
			luabind::value("Task", static_cast<int>(DynamicZoneType::Task)),
			luabind::value("Mission", static_cast<int>(DynamicZoneType::Mission)),
			luabind::value("Quest", static_cast<int>(DynamicZoneType::Quest))
		];
}

#endif // LUA_EQEMU
