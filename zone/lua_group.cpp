#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "groups.h"
#include "masterentity.h"
#include "lua_group.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"

void Lua_Group::DisbandGroup() {
	Lua_Safe_Call_Void();
	self->DisbandGroup();
}

bool Lua_Group::IsGroupMember(Lua_Mob mob) {
	Lua_Safe_Call_Bool();
	return self->IsGroupMember(mob);
}

void Lua_Group::CastGroupSpell(Lua_Mob caster, int spell_id) {
	Lua_Safe_Call_Void();
	self->CastGroupSpell(caster, spell_id);
}

void Lua_Group::SplitExp(uint32 exp, Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->SplitExp(exp, other);
}

void Lua_Group::GroupMessage(Lua_Mob sender, int language, const char *message) {
	Lua_Safe_Call_Void();
	self->GroupMessage(sender, language, 100, message);
}

uint32 Lua_Group::GetTotalGroupDamage(Lua_Mob other) {
	Lua_Safe_Call_Int();
	return self->GetTotalGroupDamage(other);
}

void Lua_Group::SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->SplitMoney(copper, silver, gold, platinum);
}

void Lua_Group::SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Lua_Client splitter) {
	Lua_Safe_Call_Void();
	self->SplitMoney(copper, silver, gold, platinum, splitter);
}

void Lua_Group::SetLeader(Lua_Mob leader) {
	Lua_Safe_Call_Void();
	self->SetLeader(leader);
}

Lua_Mob Lua_Group::GetLeader() {
	Lua_Safe_Call_Class(Lua_Mob);
	return self->GetLeader();
}

const char *Lua_Group::GetLeaderName() {
	Lua_Safe_Call_String();
	return self->GetLeaderName();
}

bool Lua_Group::IsLeader(Lua_Mob leader) {
	Lua_Safe_Call_Bool();
	return self->IsLeader(leader);
}

int Lua_Group::GroupCount() {
	Lua_Safe_Call_Int();
	return self->GroupCount();
}

int Lua_Group::GetHighestLevel() {
	Lua_Safe_Call_Int();
	return self->GetHighestLevel();
}

int Lua_Group::GetLowestLevel() {
	Lua_Safe_Call_Int();
	return self->GetLowestLevel();
}

void Lua_Group::TeleportGroup(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h) {
	Lua_Safe_Call_Void();
	self->TeleportGroup(sender, zone_id, instance_id, x, y, z, h);
}

int Lua_Group::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

Lua_Mob Lua_Group::GetMember(int index) {
	Lua_Safe_Call_Class(Lua_Mob);

	if(index >= 6 || index < 0) {
		return Lua_Mob();
	}

	return self->members[index];
}

bool Lua_Group::DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name)
{
	Lua_Safe_Call_Bool();
	return self->DoesAnyMemberHaveExpeditionLockout(expedition_name, event_name);
}

bool Lua_Group::DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name, int max_check_count)
{
	Lua_Safe_Call_Bool();
	return self->DoesAnyMemberHaveExpeditionLockout(expedition_name, event_name, max_check_count);
}

luabind::scope lua_register_group() {
	return luabind::class_<Lua_Group>("Group")
		.def(luabind::constructor<>())
		.property("null", &Lua_Group::Null)
		.property("valid", &Lua_Group::Valid)
		.def("DisbandGroup", (void(Lua_Group::*)(void))&Lua_Group::DisbandGroup)
		.def("IsGroupMember", (bool(Lua_Group::*)(Lua_Mob))&Lua_Group::IsGroupMember)
		.def("CastGroupSpell", (void(Lua_Group::*)(Lua_Mob,int))&Lua_Group::CastGroupSpell)
		.def("SplitExp", (void(Lua_Group::*)(uint32,Lua_Mob))&Lua_Group::SplitExp)
		.def("GroupMessage", (void(Lua_Group::*)(Lua_Mob,int,const char* message))&Lua_Group::GroupMessage)
		.def("GetTotalGroupDamage", (uint32(Lua_Group::*)(Lua_Mob))&Lua_Group::GetTotalGroupDamage)
		.def("SplitMoney", (void(Lua_Group::*)(uint32,uint32,uint32,uint32))&Lua_Group::SplitMoney)
		.def("SplitMoney", (void(Lua_Group::*)(uint32,uint32,uint32,uint32,Lua_Client))&Lua_Group::SplitMoney)
		.def("SetLeader", (void(Lua_Group::*)(Lua_Mob))&Lua_Group::SetLeader)
		.def("GetLeader", (Lua_Mob(Lua_Group::*)(void))&Lua_Group::GetLeader)
		.def("GetLeaderName", (const char*(Lua_Group::*)(void))&Lua_Group::GetLeaderName)
		.def("IsLeader", (bool(Lua_Group::*)(Lua_Mob))&Lua_Group::IsLeader)
		.def("GroupCount", (int(Lua_Group::*)(void))&Lua_Group::GroupCount)
		.def("GetHighestLevel", (int(Lua_Group::*)(void))&Lua_Group::GetHighestLevel)
		.def("GetLowestLevel", (int(Lua_Group::*)(void))&Lua_Group::GetLowestLevel)
		.def("TeleportGroup", (void(Lua_Group::*)(Lua_Mob,uint32,uint32,float,float,float,float))&Lua_Group::TeleportGroup)
		.def("GetID", (int(Lua_Group::*)(void))&Lua_Group::GetID)
		.def("GetMember", (Lua_Mob(Lua_Group::*)(int))&Lua_Group::GetMember)
		.def("DoesAnyMemberHaveExpeditionLockout", (bool(Lua_Group::*)(std::string, std::string))&Lua_Group::DoesAnyMemberHaveExpeditionLockout)
		.def("DoesAnyMemberHaveExpeditionLockout", (bool(Lua_Group::*)(std::string, std::string, int))&Lua_Group::DoesAnyMemberHaveExpeditionLockout);
}

#endif
