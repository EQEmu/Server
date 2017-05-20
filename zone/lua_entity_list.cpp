#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/iterator_policy.hpp>

#include "masterentity.h"
#include "lua_entity_list.h"
#include "lua_entity.h"
#include "lua_mob.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_object.h"
#include "lua_door.h"
#include "lua_corpse.h"
#include "lua_group.h"
#include "lua_raid.h"
#include "lua_spawn.h"

struct Lua_Mob_List {
	std::vector<Lua_Mob> entries;
};

struct Lua_NPC_List {
	std::vector<Lua_NPC> entries;
};

struct Lua_Client_List {
	std::vector<Lua_Client> entries;
};

struct Lua_Corpse_List {
	std::vector<Lua_Corpse> entries;
};

struct Lua_Object_List {
	std::vector<Lua_Object> entries;
};

struct Lua_Doors_List {
	std::vector<Lua_Door> entries;
};

struct Lua_Spawn_List {
	std::vector<Lua_Spawn> entries;
};

Lua_Mob Lua_EntityList::GetMobID(int id) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetMobID(id));
}

Lua_Mob Lua_EntityList::GetMob(const char *name) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetMob(name));
}

Lua_Mob Lua_EntityList::GetMob(int id) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetMob(id));
}

Lua_Mob Lua_EntityList::GetMobByNpcTypeID(int npc_type) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetMobByNpcTypeID(npc_type));
}

bool Lua_EntityList::IsMobSpawnedByNpcTypeID(int npc_type) {
	Lua_Safe_Call_Bool();
	return self->IsMobSpawnedByNpcTypeID(npc_type);
}

Lua_NPC Lua_EntityList::GetNPCByID(int id) {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetNPCByID(id));
}

Lua_NPC Lua_EntityList::GetNPCByNPCTypeID(int npc_type) {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetNPCByNPCTypeID(npc_type));
}

Lua_Client Lua_EntityList::GetClientByName(const char *name) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByName(name));
}

Lua_Client Lua_EntityList::GetClientByAccID(uint32 acct_id) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByAccID(acct_id));
}

Lua_Client Lua_EntityList::GetClientByID(int id) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByID(id));
}

Lua_Client Lua_EntityList::GetClientByCharID(uint32 char_id) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByCharID(char_id));
}

Lua_Client Lua_EntityList::GetClientByWID(uint32 wid) {
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetClientByWID(wid));
}

Lua_Object Lua_EntityList::GetObjectByID(int id) {
	Lua_Safe_Call_Class(Lua_Object);
	return Lua_Object(self->GetObjectByID(id));
}

Lua_Object Lua_EntityList::GetObjectByDBID(uint32 db_id) {
	Lua_Safe_Call_Class(Lua_Object);
	return Lua_Object(self->GetObjectByDBID(db_id));
}

Lua_Door Lua_EntityList::GetDoorsByID(int id) {
	Lua_Safe_Call_Class(Lua_Door);
	return Lua_Door(self->GetDoorsByID(id));
}

Lua_Door Lua_EntityList::GetDoorsByDBID(uint32 db_id) {
	Lua_Safe_Call_Class(Lua_Door);
	return Lua_Door(self->GetDoorsByDBID(db_id));
}

Lua_Door Lua_EntityList::GetDoorsByDoorID(uint32 door_id) {
	Lua_Safe_Call_Class(Lua_Door);
	return Lua_Door(self->GetDoorsByDoorID(door_id));
}

Lua_Door Lua_EntityList::FindDoor(uint32 id) {
	Lua_Safe_Call_Class(Lua_Door);
	return Lua_Door(self->FindDoor(id));
}

Lua_Group Lua_EntityList::GetGroupByMob(Lua_Mob mob) {
	Lua_Safe_Call_Class(Lua_Group);
	return Lua_Group(self->GetGroupByMob(mob));
}

Lua_Group Lua_EntityList::GetGroupByClient(Lua_Client client) {
	Lua_Safe_Call_Class(Lua_Group);
	return Lua_Group(self->GetGroupByClient(client));
}

Lua_Group Lua_EntityList::GetGroupByID(int id) {
	Lua_Safe_Call_Class(Lua_Group);
	return Lua_Group(self->GetGroupByID(id));
}

Lua_Group Lua_EntityList::GetGroupByLeaderName(const char *name) {
	Lua_Safe_Call_Class(Lua_Group);
	return Lua_Group(self->GetGroupByLeaderName(name));
}

Lua_Raid Lua_EntityList::GetRaidByID(int id) {
	Lua_Safe_Call_Class(Lua_Raid);
	return Lua_Raid(self->GetRaidByID(id));
}

Lua_Raid Lua_EntityList::GetRaidByClient(Lua_Client client) {
	Lua_Safe_Call_Class(Lua_Raid);
	return Lua_Raid(self->GetRaidByClient(client));
}

Lua_Corpse Lua_EntityList::GetCorpseByOwner(Lua_Client client) {
	Lua_Safe_Call_Class(Lua_Corpse);
	return Lua_Corpse(self->GetCorpseByOwner(client));
}

Lua_Corpse Lua_EntityList::GetCorpseByID(int id) {
	Lua_Safe_Call_Class(Lua_Corpse);
	return Lua_Corpse(self->GetCorpseByID(id));
}

Lua_Corpse Lua_EntityList::GetCorpseByName(const char *name) {
	Lua_Safe_Call_Class(Lua_Corpse);
	return Lua_Corpse(self->GetCorpseByName(name));
}

Lua_Spawn Lua_EntityList::GetSpawnByID(uint32 id) {
	Lua_Safe_Call_Class(Lua_Spawn);
	return self->GetSpawnByID(id);
}

void Lua_EntityList::ClearClientPetitionQueue() {
	Lua_Safe_Call_Void();
	self->ClearClientPetitionQueue();
}

bool Lua_EntityList::CanAddHateForMob(Lua_Mob p) {
	Lua_Safe_Call_Bool();
	return self->CanAddHateForMob(p);
}

void Lua_EntityList::Message(uint32 guild_dbid, uint32 type, const char *message) {
	Lua_Safe_Call_Void();
	self->Message(guild_dbid, type, message);
}

void Lua_EntityList::MessageStatus(uint32 guild_dbid, int min_status, uint32 type, const char *message) {
	Lua_Safe_Call_Void();
	self->MessageStatus(guild_dbid, min_status, type, message);
}

void Lua_EntityList::MessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, const char *message) {
	Lua_Safe_Call_Void();
	self->MessageClose(sender, skip_sender, dist, type, message);
}

void Lua_EntityList::FilteredMessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, int filter, const char *message)
{
	Lua_Safe_Call_Void();
	self->FilteredMessageClose(sender, skip_sender, dist, type, (eqFilterType)filter, message);
}

void Lua_EntityList::RemoveFromTargets(Lua_Mob mob) {
	Lua_Safe_Call_Void();
	self->RemoveFromTargets(mob);
}

void Lua_EntityList::RemoveFromTargets(Lua_Mob mob, bool RemoveFromXTargets) {
	Lua_Safe_Call_Void();
	self->RemoveFromTargets(mob, RemoveFromXTargets);
}

void Lua_EntityList::ReplaceWithTarget(Lua_Mob target, Lua_Mob new_target) {
	Lua_Safe_Call_Void();
	self->ReplaceWithTarget(target, new_target);
}

void Lua_EntityList::OpenDoorsNear(Lua_NPC opener) {
	Lua_Safe_Call_Void();
	self->OpenDoorsNear(opener);
}

std::string Lua_EntityList::MakeNameUnique(const char *name) {
	Lua_Safe_Call_String();

	char t_name[64];
	strncpy(t_name, name, 64);
	return self->MakeNameUnique(t_name);
}

std::string Lua_EntityList::RemoveNumbers(const char *name) {
	Lua_Safe_Call_String();

	char t_name[64];
	strncpy(t_name, name, 64);
	return self->RemoveNumbers(t_name);
}

void Lua_EntityList::SignalMobsByNPCID(uint32 npc_id, int signal) {
	Lua_Safe_Call_Void();
	self->SignalMobsByNPCID(npc_id, signal);
}

int Lua_EntityList::DeleteNPCCorpses() {
	Lua_Safe_Call_Int();
	return self->DeleteNPCCorpses();
}

int Lua_EntityList::DeletePlayerCorpses() {
	Lua_Safe_Call_Int();
	return self->DeletePlayerCorpses();
}

void Lua_EntityList::HalveAggro(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->HalveAggro(who);
}

void Lua_EntityList::DoubleAggro(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->DoubleAggro(who);
}

void Lua_EntityList::ClearFeignAggro(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->ClearFeignAggro(who);
}

bool Lua_EntityList::Fighting(Lua_Mob who) {
	Lua_Safe_Call_Bool();
	return self->Fighting(who);
}

void Lua_EntityList::RemoveFromHateLists(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->RemoveFromHateLists(who);
}

void Lua_EntityList::RemoveFromHateLists(Lua_Mob who, bool set_to_one) {
	Lua_Safe_Call_Void();
	self->RemoveFromHateLists(who, set_to_one);
}

void Lua_EntityList::MessageGroup(Lua_Mob who, bool skip_close, uint32 type, const char *message) {
	Lua_Safe_Call_Void();
	self->MessageGroup(who, skip_close, type, message);
}

Lua_Client Lua_EntityList::GetRandomClient(float x, float y, float z, float dist) {
	Lua_Safe_Call_Class(Lua_Client);
	return self->GetRandomClient(glm::vec3(x, y, z), dist);
}

Lua_Client Lua_EntityList::GetRandomClient(float x, float y, float z, float dist, Lua_Client exclude) {
	Lua_Safe_Call_Class(Lua_Client);
	return self->GetRandomClient(glm::vec3(x, y, z), dist, exclude);
}

Lua_Mob_List Lua_EntityList::GetMobList() {
	Lua_Safe_Call_Class(Lua_Mob_List);
	Lua_Mob_List ret;
	auto &t_list = self->GetMobList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.push_back(Lua_Mob(iter->second));
		++iter;
	}

	return ret;
}

Lua_Client_List Lua_EntityList::GetClientList() {
	Lua_Safe_Call_Class(Lua_Client_List);
	Lua_Client_List ret;
	auto &t_list = self->GetClientList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.push_back(Lua_Client(iter->second));
		++iter;
	}

	return ret;
}

Lua_NPC_List Lua_EntityList::GetNPCList() {
	Lua_Safe_Call_Class(Lua_NPC_List);
	Lua_NPC_List ret;
	auto &t_list = self->GetNPCList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.push_back(Lua_NPC(iter->second));
		++iter;
	}

	return ret;
}

Lua_Corpse_List Lua_EntityList::GetCorpseList() {
	Lua_Safe_Call_Class(Lua_Corpse_List);
	Lua_Corpse_List ret;
	auto &t_list = self->GetCorpseList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.push_back(Lua_Corpse(iter->second));
		++iter;
	}

	return ret;
}

Lua_Object_List Lua_EntityList::GetObjectList() {
	Lua_Safe_Call_Class(Lua_Object_List);
	Lua_Object_List ret;
	auto &t_list = self->GetObjectList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.push_back(Lua_Object(iter->second));
		++iter;
	}

	return ret;
}

Lua_Doors_List Lua_EntityList::GetDoorsList() {
	Lua_Safe_Call_Class(Lua_Doors_List);
	Lua_Doors_List ret;
	auto &t_list = self->GetDoorsList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.push_back(Lua_Door(iter->second));
		++iter;
	}

	return ret;
}

Lua_Spawn_List Lua_EntityList::GetSpawnList() {
	Lua_Safe_Call_Class(Lua_Spawn_List);
	Lua_Spawn_List ret;
	std::list<Spawn2*> t_list;
	self->GetSpawnList(t_list);

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.push_back(Lua_Spawn(*iter));
		++iter;
	}

	return ret;
}

void Lua_EntityList::SignalAllClients(int signal) {
	Lua_Safe_Call_Void();
	self->SignalAllClients(signal);
}

void Lua_EntityList::ChannelMessage(Lua_Mob from, int channel_num, int language, const char *message) {
	Lua_Safe_Call_Void();
	self->ChannelMessage(from, channel_num, language, message);
}

luabind::scope lua_register_entity_list() {
	return luabind::class_<Lua_EntityList>("EntityList")
		.def(luabind::constructor<>())
		.property("null", &Lua_EntityList::Null)
		.property("valid", &Lua_EntityList::Valid)
		.def("GetMobID", (Lua_Mob(Lua_EntityList::*)(int))&Lua_EntityList::GetMobID)
		.def("GetMob", (Lua_Mob(Lua_EntityList::*)(const char*))&Lua_EntityList::GetMob)
		.def("GetMob", (Lua_Mob(Lua_EntityList::*)(int))&Lua_EntityList::GetMob)
		.def("GetMobByNpcTypeID", (Lua_Mob(Lua_EntityList::*)(int))&Lua_EntityList::GetMobByNpcTypeID)
		.def("IsMobSpawnedByNpcTypeID", (bool(Lua_EntityList::*)(int))&Lua_EntityList::IsMobSpawnedByNpcTypeID)
		.def("GetNPCByID", (Lua_NPC(Lua_EntityList::*)(int))&Lua_EntityList::GetNPCByID)
		.def("GetNPCByNPCTypeID", (Lua_NPC(Lua_EntityList::*)(int))&Lua_EntityList::GetNPCByNPCTypeID)
		.def("GetClientByName", (Lua_Client(Lua_EntityList::*)(const char*))&Lua_EntityList::GetClientByName)
		.def("GetClientByAccID", (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByAccID)
		.def("GetClientByID", (Lua_Client(Lua_EntityList::*)(int))&Lua_EntityList::GetClientByID)
		.def("GetClientByCharID", (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByCharID)
		.def("GetClientByWID", (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByWID)
		.def("GetObjectByID", (Lua_Object(Lua_EntityList::*)(int))&Lua_EntityList::GetObjectByID)
		.def("GetObjectByDBID", (Lua_Object(Lua_EntityList::*)(uint32))&Lua_EntityList::GetObjectByDBID)
		.def("GetDoorsByID", (Lua_Door(Lua_EntityList::*)(int))&Lua_EntityList::GetDoorsByID)
		.def("GetDoorsByDBID", (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::GetDoorsByDBID)
		.def("GetDoorsByDoorID", (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::GetDoorsByDoorID)
		.def("FindDoor", (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::FindDoor)
		.def("GetGroupByMob", (Lua_Group(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::GetGroupByMob)
		.def("GetGroupByClient", (Lua_Group(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetGroupByClient)
		.def("GetGroupByID", (Lua_Group(Lua_EntityList::*)(int))&Lua_EntityList::GetGroupByID)
		.def("GetGroupByLeaderName", (Lua_Group(Lua_EntityList::*)(const char*))&Lua_EntityList::GetGroupByLeaderName)
		.def("GetRaidByID", (Lua_Raid(Lua_EntityList::*)(int))&Lua_EntityList::GetRaidByID)
		.def("GetRaidByClient", (Lua_Raid(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetRaidByClient)
		.def("GetCorpseByOwner", (Lua_Corpse(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetCorpseByOwner)
		.def("GetCorpseByID", (Lua_Corpse(Lua_EntityList::*)(int))&Lua_EntityList::GetCorpseByID)
		.def("GetCorpseByName", (Lua_Corpse(Lua_EntityList::*)(const char*))&Lua_EntityList::GetCorpseByName)
		.def("GetSpawnByID", (Lua_Spawn(Lua_EntityList::*)(uint32))&Lua_EntityList::GetSpawnByID)
		.def("ClearClientPetitionQueue", (void(Lua_EntityList::*)(void))&Lua_EntityList::ClearClientPetitionQueue)
		.def("CanAddHateForMob", (bool(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::CanAddHateForMob)
		.def("Message", (void(Lua_EntityList::*)(uint32, uint32, const char*))&Lua_EntityList::Message)
		.def("MessageStatus", (void(Lua_EntityList::*)(uint32, uint32, uint32, const char*))&Lua_EntityList::MessageStatus)
		.def("MessageClose", (void(Lua_EntityList::*)(Lua_Mob, bool, float, uint32, const char*))&Lua_EntityList::MessageClose)
		.def("FilteredMessageClose", &Lua_EntityList::FilteredMessageClose)
		.def("RemoveFromTargets", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::RemoveFromTargets)
		.def("RemoveFromTargets", (void(Lua_EntityList::*)(Lua_Mob, bool))&Lua_EntityList::RemoveFromTargets)
		.def("ReplaceWithTarget", (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob))&Lua_EntityList::ReplaceWithTarget)
		.def("OpenDoorsNear", (void(Lua_EntityList::*)(Lua_NPC))&Lua_EntityList::OpenDoorsNear)
		.def("MakeNameUnique", (std::string(Lua_EntityList::*)(const char*))&Lua_EntityList::MakeNameUnique)
		.def("RemoveNumbers", (std::string(Lua_EntityList::*)(const char*))&Lua_EntityList::RemoveNumbers)
		.def("SignalMobsByNPCID", (void(Lua_EntityList::*)(uint32, int))&Lua_EntityList::SignalMobsByNPCID)
		.def("DeleteNPCCorpses", (int(Lua_EntityList::*)(void))&Lua_EntityList::DeleteNPCCorpses)
		.def("DeletePlayerCorpses", (int(Lua_EntityList::*)(void))&Lua_EntityList::DeletePlayerCorpses)
		.def("HalveAggro", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::HalveAggro)
		.def("DoubleAggro", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::DoubleAggro)
		.def("ClearFeignAggro", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::ClearFeignAggro)
		.def("Fighting", (bool(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::Fighting)
		.def("RemoveFromHateLists", (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::RemoveFromHateLists)
		.def("RemoveFromHateLists", (void(Lua_EntityList::*)(Lua_Mob, bool))&Lua_EntityList::RemoveFromHateLists)
		.def("MessageGroup", (void(Lua_EntityList::*)(Lua_Mob, bool, uint32, const char*))&Lua_EntityList::MessageGroup)
		.def("GetRandomClient", (Lua_Client(Lua_EntityList::*)(float, float, float, float))&Lua_EntityList::GetRandomClient)
		.def("GetRandomClient", (Lua_Client(Lua_EntityList::*)(float, float, float, float, Lua_Client))&Lua_EntityList::GetRandomClient)
		.def("GetMobList", (Lua_Mob_List(Lua_EntityList::*)(void))&Lua_EntityList::GetMobList)
		.def("GetClientList", (Lua_Client_List(Lua_EntityList::*)(void))&Lua_EntityList::GetClientList)
		.def("GetNPCList", (Lua_NPC_List(Lua_EntityList::*)(void))&Lua_EntityList::GetNPCList)
		.def("GetCorpseList", (Lua_Corpse_List(Lua_EntityList::*)(void))&Lua_EntityList::GetCorpseList)
		.def("GetObjectList", (Lua_Object_List(Lua_EntityList::*)(void))&Lua_EntityList::GetObjectList)
		.def("GetDoorsList", (Lua_Doors_List(Lua_EntityList::*)(void))&Lua_EntityList::GetDoorsList)
		.def("GetSpawnList", (Lua_Spawn_List(Lua_EntityList::*)(void))&Lua_EntityList::GetSpawnList)
		.def("SignalAllClients", (void(Lua_EntityList::*)(int))&Lua_EntityList::SignalAllClients)
		.def("ChannelMessage", (void(Lua_EntityList::*)(Lua_Mob, int, int, const char*))&Lua_EntityList::ChannelMessage);
}

luabind::scope lua_register_mob_list() {
	return luabind::class_<Lua_Mob_List>("MobList")
			.def_readwrite("entries", &Lua_Mob_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_client_list() {
	return luabind::class_<Lua_Client_List>("ClientList")
			.def_readwrite("entries", &Lua_Client_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_npc_list() {
	return luabind::class_<Lua_NPC_List>("NPCList")
			.def_readwrite("entries", &Lua_NPC_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_corpse_list() {
	return luabind::class_<Lua_Corpse_List>("CorpseList")
			.def_readwrite("entries", &Lua_Corpse_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_object_list() {
	return luabind::class_<Lua_Object_List>("ObjectList")
			.def_readwrite("entries", &Lua_Object_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_door_list() {
	return luabind::class_<Lua_Doors_List>("DoorList")
			.def_readwrite("entries", &Lua_Doors_List::entries, luabind::return_stl_iterator);
}

luabind::scope lua_register_spawn_list() {
	return luabind::class_<Lua_Spawn_List>("SpawnList")
			.def_readwrite("entries", &Lua_Spawn_List::entries, luabind::return_stl_iterator);
}

#endif
