#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "lua_raid.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

bool Lua_Raid::IsRaidMember(const char *name) {
	Lua_Safe_Call_Bool();
	return self->IsRaidMember(name);
}

void Lua_Raid::CastGroupSpell(Lua_Mob caster, int spell_id, uint32 group_id) {
	Lua_Safe_Call_Void();
	self->CastGroupSpell(caster, spell_id, group_id);
}

int Lua_Raid::GroupCount(uint32 group_id) {
	Lua_Safe_Call_Int();
	return self->GroupCount(group_id);
}

int Lua_Raid::RaidCount() {
	Lua_Safe_Call_Int();
	return self->RaidCount();
}

int Lua_Raid::GetGroup(const char *c) {
	Lua_Safe_Call_Int();
	return self->GetGroup(c);
}

int Lua_Raid::GetGroup(Lua_Client c) {
	Lua_Safe_Call_Int();
	return self->GetGroup(c);
}

void Lua_Raid::SplitExp(uint32 exp, Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->SplitExp(exp, other);
}

uint32 Lua_Raid::GetTotalRaidDamage(Lua_Mob other) {
	Lua_Safe_Call_Int();
	return self->GetTotalRaidDamage(other);
}

void Lua_Raid::SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->SplitMoney(gid, copper, silver, gold, platinum);
}

void Lua_Raid::SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Lua_Client splitter) {
	Lua_Safe_Call_Void();
	self->SplitMoney(gid, copper, silver, gold, platinum, splitter);
}

void Lua_Raid::BalanceHP(int penalty, uint32 group_id) {
	Lua_Safe_Call_Void();
	self->BalanceHP(penalty, group_id);
}

bool Lua_Raid::IsLeader(const char *c) {
	Lua_Safe_Call_Bool();
	return self->IsLeader(c);
}

bool Lua_Raid::IsLeader(Lua_Client c) {
	Lua_Safe_Call_Bool();
	return self->IsLeader(c);
}

bool Lua_Raid::IsGroupLeader(const char *name) {
	Lua_Safe_Call_Bool();
	return self->IsGroupLeader(name);
}

int Lua_Raid::GetHighestLevel() {
	Lua_Safe_Call_Int();
	return self->GetHighestLevel();
}

int Lua_Raid::GetLowestLevel() {
	Lua_Safe_Call_Int();
	return self->GetLowestLevel();
}

Lua_Client Lua_Raid::GetClientByIndex(int index) {
	Lua_Safe_Call_Class(Lua_Client);
	return self->GetClientByIndex(index);
}

void Lua_Raid::TeleportGroup(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h, uint32 group_id) {
	Lua_Safe_Call_Void();
	self->TeleportGroup(sender, zone_id, instance_id, x, y, z, h, group_id);
}

void Lua_Raid::TeleportRaid(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h) {
	Lua_Safe_Call_Void();
	self->TeleportRaid(sender, zone_id, instance_id, x, y, z, h);
}

int Lua_Raid::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

Lua_Client Lua_Raid::GetMember(int index) {
	Lua_Safe_Call_Class(Lua_Client);

	if(index >= 72 || index < 0) {
		return Lua_Client();
	}

	return self->members[index].member;
}

int Lua_Raid::GetGroupNumber(int index) {
	Lua_Safe_Call_Int();

	if(index >= 72 || index < 0 || self->members[index].GroupNumber == RAID_GROUPLESS) {
		return -1;
	}

	return self->members[index].GroupNumber;
}

bool Lua_Raid::DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name)
{
	Lua_Safe_Call_Bool();
	return self->DoesAnyMemberHaveExpeditionLockout(expedition_name, event_name);
}

bool Lua_Raid::DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name, int max_check_count)
{
	Lua_Safe_Call_Bool();
	return self->DoesAnyMemberHaveExpeditionLockout(expedition_name, event_name, max_check_count);
}

luabind::scope lua_register_raid() {
	return luabind::class_<Lua_Raid>("Raid")
		.def(luabind::constructor<>())
		.property("null", &Lua_Raid::Null)
		.property("valid", &Lua_Raid::Valid)
		.def("IsRaidMember", (bool(Lua_Raid::*)(const char*))&Lua_Raid::IsRaidMember)
		.def("CastGroupSpell", (void(Lua_Raid::*)(Lua_Mob,int,uint32))&Lua_Raid::CastGroupSpell)
		.def("GroupCount", (int(Lua_Raid::*)(uint32))&Lua_Raid::GroupCount)
		.def("RaidCount", (int(Lua_Raid::*)(void))&Lua_Raid::RaidCount)
		.def("GetGroup", (int(Lua_Raid::*)(const char*))&Lua_Raid::GetGroup)
		.def("GetGroup", (int(Lua_Raid::*)(Lua_Client))&Lua_Raid::GetGroup)
		.def("SplitExp", (void(Lua_Raid::*)(uint32,Lua_Mob))&Lua_Raid::SplitExp)
		.def("GetTotalRaidDamage", (uint32(Lua_Raid::*)(Lua_Mob))&Lua_Raid::GetTotalRaidDamage)
		.def("SplitMoney", (void(Lua_Raid::*)(uint32,uint32,uint32,uint32,uint32))&Lua_Raid::SplitMoney)
		.def("SplitMoney", (void(Lua_Raid::*)(uint32,uint32,uint32,uint32,uint32,Lua_Client))&Lua_Raid::SplitMoney)
		.def("BalanceHP", (void(Lua_Raid::*)(int,uint32))&Lua_Raid::BalanceHP)
		.def("IsLeader", (bool(Lua_Raid::*)(const char*))&Lua_Raid::IsLeader)
		.def("IsGroupLeader", (bool(Lua_Raid::*)(const char*))&Lua_Raid::IsGroupLeader)
		.def("GetHighestLevel", (int(Lua_Raid::*)(void))&Lua_Raid::GetHighestLevel)
		.def("GetLowestLevel", (int(Lua_Raid::*)(void))&Lua_Raid::GetLowestLevel)
		.def("GetClientByIndex", (Lua_Client(Lua_Raid::*)(int))&Lua_Raid::GetClientByIndex)
		.def("TeleportGroup", (int(Lua_Raid::*)(Lua_Mob,uint32,uint32,float,float,float,float,uint32))&Lua_Raid::TeleportGroup)
		.def("TeleportRaid", (int(Lua_Raid::*)(Lua_Mob,uint32,uint32,float,float,float,float))&Lua_Raid::TeleportRaid)
		.def("GetID", (int(Lua_Raid::*)(void))&Lua_Raid::GetID)
		.def("GetMember", (Lua_Client(Lua_Raid::*)(int))&Lua_Raid::GetMember)
		.def("GetGroupNumber", (int(Lua_Raid::*)(int))&Lua_Raid::GetGroupNumber)
		.def("DoesAnyMemberHaveExpeditionLockout", (bool(Lua_Raid::*)(std::string, std::string))&Lua_Raid::DoesAnyMemberHaveExpeditionLockout)
		.def("DoesAnyMemberHaveExpeditionLockout", (bool(Lua_Raid::*)(std::string, std::string, int))&Lua_Raid::DoesAnyMemberHaveExpeditionLockout);
}

#endif
