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

int Lua_Expedition::GetInstanceID() {
	Lua_Safe_Call_Int();
	return self->GetDynamicZone().GetInstanceID();
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

int Lua_Expedition::GetSecondsRemaining() {
	Lua_Safe_Call_Int();
	return self->GetDynamicZone().GetSecondsRemaining();
}

int Lua_Expedition::GetZoneID() {
	Lua_Safe_Call_Int();
	return self->GetDynamicZone().GetZoneID();
}

bool Lua_Expedition::HasLockout(std::string event_name) {
	Lua_Safe_Call_Bool();
	return self->HasLockout(event_name);
}

bool Lua_Expedition::HasReplayLockout() {
	Lua_Safe_Call_Bool();
	return self->HasReplayLockout();
}

void Lua_Expedition::RemoveCompass() {
	Lua_Safe_Call_Void();
	self->SetDzCompass(0, 0, 0, 0, true);
}

void Lua_Expedition::RemoveLockout(std::string event_name) {
	Lua_Safe_Call_Void();
	self->RemoveLockout(event_name);
}

void Lua_Expedition::SetCompass(uint32_t zone_id, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->SetDzCompass(zone_id, x, y, z, true);
}

void Lua_Expedition::SetCompass(std::string zone_name, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->SetDzCompass(zone_name, x, y, z, true);
}

void Lua_Expedition::SetLocked(bool lock_expedition) {
	Lua_Safe_Call_Void();
	self->SetLocked(lock_expedition, true);
}

void Lua_Expedition::SetReplayLockoutOnMemberJoin(bool enable) {
	Lua_Safe_Call_Void();
	self->SetReplayLockoutOnMemberJoin(enable, true);
}

void Lua_Expedition::SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->SetDzSafeReturn(zone_id, x, y, z, heading, true);
}

void Lua_Expedition::SetSafeReturn(std::string zone_name, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->SetDzSafeReturn(zone_name, x, y, z, heading, true);
}

void Lua_Expedition::SetZoneInLocation(float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->SetDzZoneInLocation(x, y, z, heading, true);
}

luabind::scope lua_register_expedition() {
	return luabind::class_<Lua_Expedition>("Expedition")
		.def(luabind::constructor<>())
		.property("null", &Lua_Expedition::Null)
		.property("valid", &Lua_Expedition::Valid)
		.def("AddLockout", (void(Lua_Expedition::*)(std::string, uint32_t))&Lua_Expedition::AddLockout)
		.def("AddReplayLockout", (void(Lua_Expedition::*)(uint32_t))&Lua_Expedition::AddReplayLockout)
		.def("GetID", (uint32_t(Lua_Expedition::*)(void))&Lua_Expedition::GetID)
		.def("GetInstanceID", (int(Lua_Expedition::*)(void))&Lua_Expedition::GetInstanceID)
		.def("GetLeaderName", (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetLeaderName)
		.def("GetLockouts", &Lua_Expedition::GetLockouts)
		.def("GetMemberCount", (uint32_t(Lua_Expedition::*)(void))&Lua_Expedition::GetMemberCount)
		.def("GetMembers", &Lua_Expedition::GetMembers)
		.def("GetName", (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetName)
		.def("GetSecondsRemaining", (int(Lua_Expedition::*)(void))&Lua_Expedition::GetSecondsRemaining)
		.def("GetZoneID", (int(Lua_Expedition::*)(void))&Lua_Expedition::GetZoneID)
		.def("HasLockout", (bool(Lua_Expedition::*)(std::string))&Lua_Expedition::HasLockout)
		.def("HasReplayLockout", (bool(Lua_Expedition::*)(void))&Lua_Expedition::HasReplayLockout)
		.def("RemoveCompass", (void(Lua_Expedition::*)(void))&Lua_Expedition::RemoveCompass)
		.def("RemoveLockout", (void(Lua_Expedition::*)(std::string))&Lua_Expedition::RemoveLockout)
		.def("SetCompass", (void(Lua_Expedition::*)(uint32_t, float, float, float))&Lua_Expedition::SetCompass)
		.def("SetCompass", (void(Lua_Expedition::*)(std::string, float, float, float))&Lua_Expedition::SetCompass)
		.def("SetLocked", (void(Lua_Expedition::*)(bool))&Lua_Expedition::SetLocked)
		.def("SetReplayLockoutOnMemberJoin", (void(Lua_Expedition::*)(bool))&Lua_Expedition::SetReplayLockoutOnMemberJoin)
		.def("SetSafeReturn", (void(Lua_Expedition::*)(uint32_t, float, float, float, float))&Lua_Expedition::SetSafeReturn)
		.def("SetSafeReturn", (void(Lua_Expedition::*)(std::string, float, float, float, float))&Lua_Expedition::SetSafeReturn)
		.def("SetZoneInLocation", (void(Lua_Expedition::*)(float, float, float, float))&Lua_Expedition::SetZoneInLocation);
}

#endif // LUA_EQEMU
