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
#include "zone_store.h"
#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

void Lua_Expedition::AddLockout(std::string event_name, uint32_t seconds) {
	Lua_Safe_Call_Void();
	self->AddLockout(event_name, seconds);
}

void Lua_Expedition::AddLockoutDuration(std::string event_name, int seconds) {
	Lua_Safe_Call_Void();
	self->AddLockoutDuration(event_name, seconds);
}

void Lua_Expedition::AddLockoutDuration(std::string event_name, int seconds, bool members_only) {
	Lua_Safe_Call_Void();
	self->AddLockoutDuration(event_name, seconds, members_only);
}

void Lua_Expedition::AddReplayLockout(uint32_t seconds) {
	Lua_Safe_Call_Void();
	self->AddReplayLockout(seconds);
}

void Lua_Expedition::AddReplayLockoutDuration(int seconds) {
	Lua_Safe_Call_Void();
	self->AddReplayLockoutDuration(seconds);
}

void Lua_Expedition::AddReplayLockoutDuration(int seconds, bool members_only) {
	Lua_Safe_Call_Void();
	self->AddReplayLockoutDuration(seconds, members_only);
}

uint32_t Lua_Expedition::GetDynamicZoneID() {
	Lua_Safe_Call_Int();
	return self->GetDynamicZone().GetID();
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

std::string Lua_Expedition::GetLootEventByNPCTypeID(uint32_t npc_type_id) {
	Lua_Safe_Call_String();
	return self->GetLootEventByNPCTypeID(npc_type_id);
}

std::string Lua_Expedition::GetLootEventBySpawnID(uint32_t spawn_id) {
	Lua_Safe_Call_String();
	return self->GetLootEventBySpawnID(spawn_id);
}

uint32_t Lua_Expedition::GetMemberCount() {
	Lua_Safe_Call_Int();
	return self->GetDynamicZone().GetMemberCount();
}

luabind::object Lua_Expedition::GetMembers(lua_State* L) {
	luabind::object lua_table = luabind::newtable(L);

	if (d_)
	{
		auto self = reinterpret_cast<NativeType*>(d_);
		for (const auto& member : self->GetDynamicZone().GetMembers())
		{
			lua_table[member.name] = member.id;
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

std::string Lua_Expedition::GetUUID() {
	Lua_Safe_Call_String();
	return self->GetUUID();
}

int Lua_Expedition::GetZoneID() {
	Lua_Safe_Call_Int();
	return self->GetDynamicZone().GetZoneID();
}

std::string Lua_Expedition::GetZoneName() {
	Lua_Safe_Call_String();
	return ZoneName(self->GetDynamicZone().GetZoneID());
}

int Lua_Expedition::GetZoneVersion() {
	Lua_Safe_Call_Int();
	return self->GetDynamicZone().GetZoneVersion();
}

bool Lua_Expedition::HasLockout(std::string event_name) {
	Lua_Safe_Call_Bool();
	return self->HasLockout(event_name);
}

bool Lua_Expedition::HasReplayLockout() {
	Lua_Safe_Call_Bool();
	return self->HasReplayLockout();
}

bool Lua_Expedition::IsLocked() {
	Lua_Safe_Call_Bool();
	return self->IsLocked();
}

void Lua_Expedition::RemoveCompass() {
	Lua_Safe_Call_Void();
	self->GetDynamicZone().SetCompass(0, 0, 0, 0, true);
}

void Lua_Expedition::RemoveLockout(std::string event_name) {
	Lua_Safe_Call_Void();
	self->RemoveLockout(event_name);
}

void Lua_Expedition::SetCompass(uint32_t zone_id, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->GetDynamicZone().SetCompass(zone_id, x, y, z, true);
}

void Lua_Expedition::SetCompass(std::string zone_name, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->GetDynamicZone().SetCompass(ZoneID(zone_name), x, y, z, true);
}

void Lua_Expedition::SetLocked(bool lock_expedition) {
	Lua_Safe_Call_Void();
	self->SetLocked(lock_expedition, ExpeditionLockMessage::None, true);
}

void Lua_Expedition::SetLocked(bool lock_expedition, int lock_msg) {
	Lua_Safe_Call_Void();
	self->SetLocked(lock_expedition, static_cast<ExpeditionLockMessage>(lock_msg), true);
}

void Lua_Expedition::SetLocked(bool lock_expedition, int lock_msg, uint32_t msg_color) {
	Lua_Safe_Call_Void();
	self->SetLocked(lock_expedition, static_cast<ExpeditionLockMessage>(lock_msg), true, msg_color);
}

void Lua_Expedition::SetLootEventByNPCTypeID(uint32_t npc_type_id, std::string event_name) {
	Lua_Safe_Call_Void();
	self->SetLootEventByNPCTypeID(npc_type_id, event_name);
}

void Lua_Expedition::SetLootEventBySpawnID(uint32_t spawn_id, std::string event_name) {
	Lua_Safe_Call_Void();
	self->SetLootEventBySpawnID(spawn_id, event_name);
}

void Lua_Expedition::SetReplayLockoutOnMemberJoin(bool enable) {
	Lua_Safe_Call_Void();
	self->SetReplayLockoutOnMemberJoin(enable, true);
}

void Lua_Expedition::SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->GetDynamicZone().SetSafeReturn(zone_id, x, y, z, heading, true);
}

void Lua_Expedition::SetSafeReturn(std::string zone_name, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->GetDynamicZone().SetSafeReturn(ZoneID(zone_name), x, y, z, heading, true);
}

void Lua_Expedition::SetSecondsRemaining(uint32_t seconds_remaining)
{
	Lua_Safe_Call_Void();
	self->GetDynamicZone().SetSecondsRemaining(seconds_remaining);
}

void Lua_Expedition::SetZoneInLocation(float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->GetDynamicZone().SetZoneInLocation(x, y, z, heading, true);
}

void Lua_Expedition::UpdateLockoutDuration(std::string event_name, uint32_t duration) {
	Lua_Safe_Call_Void();
	self->UpdateLockoutDuration(event_name, duration);
}

void Lua_Expedition::UpdateLockoutDuration(std::string event_name, uint32_t duration, bool members_only) {
	Lua_Safe_Call_Void();
	self->UpdateLockoutDuration(event_name, duration, members_only);
}

luabind::scope lua_register_expedition() {
	return luabind::class_<Lua_Expedition>("Expedition")
		.def(luabind::constructor<>())
		.property("null", &Lua_Expedition::Null)
		.property("valid", &Lua_Expedition::Valid)
		.def("AddLockout", (void(Lua_Expedition::*)(std::string, uint32_t))&Lua_Expedition::AddLockout)
		.def("AddLockoutDuration", (void(Lua_Expedition::*)(std::string, int))&Lua_Expedition::AddLockoutDuration)
		.def("AddLockoutDuration", (void(Lua_Expedition::*)(std::string, int, bool))&Lua_Expedition::AddLockoutDuration)
		.def("AddReplayLockout", (void(Lua_Expedition::*)(uint32_t))&Lua_Expedition::AddReplayLockout)
		.def("AddReplayLockoutDuration", (void(Lua_Expedition::*)(int))&Lua_Expedition::AddReplayLockoutDuration)
		.def("AddReplayLockoutDuration", (void(Lua_Expedition::*)(int, bool))&Lua_Expedition::AddReplayLockoutDuration)
		.def("GetDynamicZoneID", &Lua_Expedition::GetDynamicZoneID)
		.def("GetID", (uint32_t(Lua_Expedition::*)(void))&Lua_Expedition::GetID)
		.def("GetInstanceID", (int(Lua_Expedition::*)(void))&Lua_Expedition::GetInstanceID)
		.def("GetLeaderName", (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetLeaderName)
		.def("GetLockouts", &Lua_Expedition::GetLockouts)
		.def("GetLootEventByNPCTypeID", (std::string(Lua_Expedition::*)(uint32_t))&Lua_Expedition::GetLootEventByNPCTypeID)
		.def("GetLootEventBySpawnID", (std::string(Lua_Expedition::*)(uint32_t))&Lua_Expedition::GetLootEventBySpawnID)
		.def("GetMemberCount", (uint32_t(Lua_Expedition::*)(void))&Lua_Expedition::GetMemberCount)
		.def("GetMembers", &Lua_Expedition::GetMembers)
		.def("GetName", (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetName)
		.def("GetSecondsRemaining", (int(Lua_Expedition::*)(void))&Lua_Expedition::GetSecondsRemaining)
		.def("GetUUID", (std::string(Lua_Expedition::*)(void))&Lua_Expedition::GetUUID)
		.def("GetZoneID", (int(Lua_Expedition::*)(void))&Lua_Expedition::GetZoneID)
		.def("GetZoneName", &Lua_Expedition::GetZoneName)
		.def("GetZoneVersion", &Lua_Expedition::GetZoneVersion)
		.def("HasLockout", (bool(Lua_Expedition::*)(std::string))&Lua_Expedition::HasLockout)
		.def("HasReplayLockout", (bool(Lua_Expedition::*)(void))&Lua_Expedition::HasReplayLockout)
		.def("IsLocked", &Lua_Expedition::IsLocked)
		.def("RemoveCompass", (void(Lua_Expedition::*)(void))&Lua_Expedition::RemoveCompass)
		.def("RemoveLockout", (void(Lua_Expedition::*)(std::string))&Lua_Expedition::RemoveLockout)
		.def("SetCompass", (void(Lua_Expedition::*)(uint32_t, float, float, float))&Lua_Expedition::SetCompass)
		.def("SetCompass", (void(Lua_Expedition::*)(std::string, float, float, float))&Lua_Expedition::SetCompass)
		.def("SetLocked", (void(Lua_Expedition::*)(bool))&Lua_Expedition::SetLocked)
		.def("SetLocked", (void(Lua_Expedition::*)(bool, int))&Lua_Expedition::SetLocked)
		.def("SetLocked", (void(Lua_Expedition::*)(bool, int, uint32_t))&Lua_Expedition::SetLocked)
		.def("SetLootEventByNPCTypeID", (void(Lua_Expedition::*)(uint32_t, std::string))&Lua_Expedition::SetLootEventByNPCTypeID)
		.def("SetLootEventBySpawnID", (void(Lua_Expedition::*)(uint32_t, std::string))&Lua_Expedition::SetLootEventBySpawnID)
		.def("SetReplayLockoutOnMemberJoin", (void(Lua_Expedition::*)(bool))&Lua_Expedition::SetReplayLockoutOnMemberJoin)
		.def("SetSafeReturn", (void(Lua_Expedition::*)(uint32_t, float, float, float, float))&Lua_Expedition::SetSafeReturn)
		.def("SetSafeReturn", (void(Lua_Expedition::*)(std::string, float, float, float, float))&Lua_Expedition::SetSafeReturn)
		.def("SetSecondsRemaining", &Lua_Expedition::SetSecondsRemaining)
		.def("SetZoneInLocation", (void(Lua_Expedition::*)(float, float, float, float))&Lua_Expedition::SetZoneInLocation)
		.def("UpdateLockoutDuration", (void(Lua_Expedition::*)(std::string, uint32_t))&Lua_Expedition::UpdateLockoutDuration)
		.def("UpdateLockoutDuration", (void(Lua_Expedition::*)(std::string, uint32_t, bool))&Lua_Expedition::UpdateLockoutDuration);
}

luabind::scope lua_register_expedition_lock_messages() {
	return luabind::class_<ExpeditionLockMessage>("ExpeditionLockMessage")
		.enum_("constants")
		[
			luabind::value("None", static_cast<int>(ExpeditionLockMessage::None)),
			luabind::value("Close", static_cast<int>(ExpeditionLockMessage::Close)),
			luabind::value("Begin", static_cast<int>(ExpeditionLockMessage::Begin))
		];
}

#endif // LUA_EQEMU
