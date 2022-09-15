#ifdef LUA_EQEMU

#include <sol/sol.hpp>

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

#ifdef BOTS
#include "lua_bot.h"
#endif

struct Lua_Mob_List {
	std::vector<Lua_Mob> entries;
	sol::as_table_t<std::vector<Lua_Mob>> get_entries() { return sol::as_table(entries); }
};

struct Lua_NPC_List {
	std::vector<Lua_NPC> entries;
	sol::as_table_t<std::vector<Lua_NPC>> get_entries() { return sol::as_table(entries); }
};

struct Lua_Client_List {
	std::vector<Lua_Client> entries;
	sol::as_table_t<std::vector<Lua_Client>> get_entries() { return sol::as_table(entries); }
};

#ifdef BOTS
struct Lua_Bot_List {
	std::vector<Lua_Bot> entries;
	sol::as_table_t<std::vector<Lua_Bot>> get_entries() { return sol::as_table(entries); }
};
#endif

struct Lua_Corpse_List {
	std::vector<Lua_Corpse> entries;
	sol::as_table_t<std::vector<Lua_Corpse>> get_entries() { return sol::as_table(entries); }
};

struct Lua_Object_List {
	std::vector<Lua_Object> entries;
	sol::as_table_t<std::vector<Lua_Object>> get_entries() { return sol::as_table(entries); }
};

struct Lua_Doors_List {
	std::vector<Lua_Door> entries;
	sol::as_table_t<std::vector<Lua_Door>> get_entries() { return sol::as_table(entries); }
};

struct Lua_Spawn_List {
	std::vector<Lua_Spawn> entries;
	sol::as_table_t<std::vector<Lua_Spawn>> get_entries() { return sol::as_table(entries); }
};

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

#ifdef BOTS
Lua_Bot_List Lua_EntityList::GetBotList() {
	Lua_Safe_Call_Class(Lua_Bot_List);
	Lua_Bot_List ret;
	auto &bot_list = self->GetBotList();

	if (bot_list.size()) {
		for (auto bot : bot_list) {
			ret.entries.push_back(Lua_Bot(bot));
		}
	}

	return ret;
}

Lua_Bot_List Lua_EntityList::GetBotListByCharacterID(uint32 character_id) {
	Lua_Safe_Call_Class(Lua_Bot_List);
	Lua_Bot_List ret;
	auto bot_list = self->GetBotListByCharacterID(character_id);

	if (bot_list.size()) {
		for (auto bot : bot_list) {
			ret.entries.push_back(Lua_Bot(bot));
		}
	}

	return ret;
}

Lua_Bot_List Lua_EntityList::GetBotListByClientName(std::string client_name) {
	Lua_Safe_Call_Class(Lua_Bot_List);
	Lua_Bot_List ret;
	auto bot_list = self->GetBotListByClientName(client_name);

	if (bot_list.size()) {
		for (auto bot : bot_list) {
			ret.entries.push_back(Lua_Bot(bot));
		}
	}

	return ret;
}
#endif

Lua_Client_List Lua_EntityList::GetShuffledClientList() {
	Lua_Safe_Call_Class(Lua_Client_List);
	Lua_Client_List ret;
	auto &t_list = self->GetClientList();

	auto iter = t_list.begin();
	while(iter != t_list.end()) {
		ret.entries.push_back(Lua_Client(iter->second));
		++iter;
	}

	zone->random.Shuffle(ret.entries.begin(), ret.entries.end());

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

void lua_register_entity_list(sol::state_view &sv)
{
	auto entity_list = sv.new_usertype<Lua_EntityList>("EntityList", sol::constructors<Lua_EntityList()>());
	entity_list["null"] = sol::readonly_property(&Lua_EntityList::Null);
	entity_list["valid"] = sol::readonly_property(&Lua_EntityList::Valid);
	entity_list["CanAddHateForMob"] = (bool(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::CanAddHateForMob;
	entity_list["ChannelMessage"] = (void(Lua_EntityList::*)(Lua_Mob, int, int, const char*))&Lua_EntityList::ChannelMessage;
	entity_list["ClearClientPetitionQueue"] = (void(Lua_EntityList::*)(void))&Lua_EntityList::ClearClientPetitionQueue;
	entity_list["ClearFeignAggro"] = (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::ClearFeignAggro;
	entity_list["DeleteNPCCorpses"] = (uint32(Lua_EntityList::*)(void))&Lua_EntityList::DeleteNPCCorpses;
	entity_list["DeletePlayerCorpses"] = (uint32(Lua_EntityList::*)(void))&Lua_EntityList::DeletePlayerCorpses;
	entity_list["DoubleAggro"] = (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::DoubleAggro;
	entity_list["Fighting"] = (bool(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::Fighting;
	entity_list["FilteredMessageClose"] = &Lua_EntityList::FilteredMessageClose;
	entity_list["FindDoor"] = (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::FindDoor;
#ifdef BOTS
	entity_list["GetBotByID"] = (Lua_Bot(Lua_EntityList::*)(uint32))&Lua_EntityList::GetBotByID;
	entity_list["GetBotByName"] = (Lua_Bot(Lua_EntityList::*)(std::string))&Lua_EntityList::GetBotByName;
	entity_list["GetBotList"] = (Lua_Bot_List(Lua_EntityList::*)(void))&Lua_EntityList::GetBotList;
	entity_list["GetBotListByCharacterID"] = (Lua_Bot_List(Lua_EntityList::*)(uint32))&Lua_EntityList::GetBotListByCharacterID;
	entity_list["GetBotListByClientName"] = (Lua_Bot_List(Lua_EntityList::*)(std::string))&Lua_EntityList::GetBotListByClientName;
#endif
	entity_list["GetClientByAccID"] = (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByAccID;
	entity_list["GetClientByCharID"] = (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByCharID;
	entity_list["GetClientByID"] = (Lua_Client(Lua_EntityList::*)(int))&Lua_EntityList::GetClientByID;
	entity_list["GetClientByName"] = (Lua_Client(Lua_EntityList::*)(const char*))&Lua_EntityList::GetClientByName;
	entity_list["GetClientByWID"] = (Lua_Client(Lua_EntityList::*)(uint32))&Lua_EntityList::GetClientByWID;
	entity_list["GetClientList"] = (Lua_Client_List(Lua_EntityList::*)(void))&Lua_EntityList::GetClientList;
	entity_list["GetCorpseByID"] = (Lua_Corpse(Lua_EntityList::*)(int))&Lua_EntityList::GetCorpseByID;
	entity_list["GetCorpseByName"] = (Lua_Corpse(Lua_EntityList::*)(const char*))&Lua_EntityList::GetCorpseByName;
	entity_list["GetCorpseByOwner"] = (Lua_Corpse(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetCorpseByOwner;
	entity_list["GetCorpseList"] = (Lua_Corpse_List(Lua_EntityList::*)(void))&Lua_EntityList::GetCorpseList;
	entity_list["GetDoorsByDBID"] = (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::GetDoorsByDBID;
	entity_list["GetDoorsByDoorID"] = (Lua_Door(Lua_EntityList::*)(uint32))&Lua_EntityList::GetDoorsByDoorID;
	entity_list["GetDoorsByID"] = (Lua_Door(Lua_EntityList::*)(int))&Lua_EntityList::GetDoorsByID;
	entity_list["GetDoorsList"] = (Lua_Doors_List(Lua_EntityList::*)(void))&Lua_EntityList::GetDoorsList;
	entity_list["GetGroupByClient"] = (Lua_Group(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetGroupByClient;
	entity_list["GetGroupByID"] = (Lua_Group(Lua_EntityList::*)(int))&Lua_EntityList::GetGroupByID;
	entity_list["GetGroupByLeaderName"] = (Lua_Group(Lua_EntityList::*)(const char*))&Lua_EntityList::GetGroupByLeaderName;
	entity_list["GetGroupByMob"] = (Lua_Group(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::GetGroupByMob;
	entity_list["GetMob"] = sol::overload((Lua_Mob(Lua_EntityList::*)(const char *)) & Lua_EntityList::GetMob,
					      (Lua_Mob(Lua_EntityList::*)(int)) & Lua_EntityList::GetMob);
	entity_list["GetMobByNpcTypeID"] = (Lua_Mob(Lua_EntityList::*)(int))&Lua_EntityList::GetMobByNpcTypeID;
	entity_list["GetMobID"] = (Lua_Mob(Lua_EntityList::*)(int))&Lua_EntityList::GetMobID;
	entity_list["GetMobList"] = (Lua_Mob_List(Lua_EntityList::*)(void))&Lua_EntityList::GetMobList;
	entity_list["GetNPCByID"] = (Lua_NPC(Lua_EntityList::*)(int))&Lua_EntityList::GetNPCByID;
	entity_list["GetNPCByNPCTypeID"] = (Lua_NPC(Lua_EntityList::*)(int))&Lua_EntityList::GetNPCByNPCTypeID;
	entity_list["GetNPCBySpawnID"] = (Lua_NPC(Lua_EntityList::*)(int))&Lua_EntityList::GetNPCBySpawnID;
	entity_list["GetNPCList"] = (Lua_NPC_List(Lua_EntityList::*)(void))&Lua_EntityList::GetNPCList;
	entity_list["GetObjectByDBID"] = (Lua_Object(Lua_EntityList::*)(uint32))&Lua_EntityList::GetObjectByDBID;
	entity_list["GetObjectByID"] = (Lua_Object(Lua_EntityList::*)(int))&Lua_EntityList::GetObjectByID;
	entity_list["GetObjectList"] = (Lua_Object_List(Lua_EntityList::*)(void))&Lua_EntityList::GetObjectList;
	entity_list["GetRaidByClient"] = (Lua_Raid(Lua_EntityList::*)(Lua_Client))&Lua_EntityList::GetRaidByClient;
	entity_list["GetRaidByID"] = (Lua_Raid(Lua_EntityList::*)(int))&Lua_EntityList::GetRaidByID;
	entity_list["GetRandomClient"] = sol::overload(
	    (Lua_Client(Lua_EntityList::*)(float, float, float, float)) & Lua_EntityList::GetRandomClient,
	    (Lua_Client(Lua_EntityList::*)(float, float, float, float, Lua_Client)) & Lua_EntityList::GetRandomClient);
	entity_list["GetRandomMob"] = sol::overload(
	    (Lua_Mob(Lua_EntityList::*)(float, float, float, float)) & Lua_EntityList::GetRandomMob,
	    (Lua_Mob(Lua_EntityList::*)(float, float, float, float, Lua_Mob)) & Lua_EntityList::GetRandomMob);
	entity_list["GetRandomNPC"] = sol::overload(
	    (Lua_NPC(Lua_EntityList::*)(float, float, float, float)) & Lua_EntityList::GetRandomNPC,
	    (Lua_NPC(Lua_EntityList::*)(float, float, float, float, Lua_NPC)) & Lua_EntityList::GetRandomNPC);
	entity_list["GetShuffledClientList"] = (Lua_Client_List(Lua_EntityList::*)(void))&Lua_EntityList::GetShuffledClientList;
	entity_list["GetSpawnByID"] = (Lua_Spawn(Lua_EntityList::*)(uint32))&Lua_EntityList::GetSpawnByID;
	entity_list["GetSpawnList"] = (Lua_Spawn_List(Lua_EntityList::*)(void))&Lua_EntityList::GetSpawnList;
	entity_list["HalveAggro"] = (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::HalveAggro;
	entity_list["IsMobSpawnedByNpcTypeID"] = (bool(Lua_EntityList::*)(int))&Lua_EntityList::IsMobSpawnedByNpcTypeID;
	entity_list["MakeNameUnique"] = (std::string(Lua_EntityList::*)(const char*))&Lua_EntityList::MakeNameUnique;
	entity_list["Message"] = (void(Lua_EntityList::*)(uint32, uint32, const char*))&Lua_EntityList::Message;
	entity_list["MessageClose"] = (void(Lua_EntityList::*)(Lua_Mob, bool, float, uint32, const char*))&Lua_EntityList::MessageClose;
	entity_list["MessageGroup"] = (void(Lua_EntityList::*)(Lua_Mob, bool, uint32, const char*))&Lua_EntityList::MessageGroup;
	entity_list["MessageStatus"] = (void(Lua_EntityList::*)(uint32, uint32, uint32, const char*))&Lua_EntityList::MessageStatus;
	entity_list["OpenDoorsNear"] = (void(Lua_EntityList::*)(Lua_Mob))&Lua_EntityList::OpenDoorsNear;
	entity_list["RemoveFromHateLists"] =
	    sol::overload((void(Lua_EntityList::*)(Lua_Mob)) & Lua_EntityList::RemoveFromHateLists,
			  (void(Lua_EntityList::*)(Lua_Mob, bool)) & Lua_EntityList::RemoveFromHateLists);
	entity_list["RemoveFromTargets"] =
	    sol::overload((void(Lua_EntityList::*)(Lua_Mob)) & Lua_EntityList::RemoveFromTargets,
			  (void(Lua_EntityList::*)(Lua_Mob, bool)) & Lua_EntityList::RemoveFromTargets);
	entity_list["RemoveNumbers"] = (std::string(Lua_EntityList::*)(const char*))&Lua_EntityList::RemoveNumbers;
	entity_list["ReplaceWithTarget"] = (void(Lua_EntityList::*)(Lua_Mob, Lua_Mob))&Lua_EntityList::ReplaceWithTarget;
	entity_list["SignalAllClients"] = (void(Lua_EntityList::*)(int))&Lua_EntityList::SignalAllClients;
	entity_list["SignalMobsByNPCID"] = (void(Lua_EntityList::*)(uint32, int))&Lua_EntityList::SignalMobsByNPCID;
}

void lua_register_mob_list(sol::state_view &sv)
{
	auto mob_list = sv.new_usertype<Lua_Mob_List>("MobList");
	mob_list["entries"] = sol::readonly_property(&Lua_Mob_List::get_entries);
}

void lua_register_client_list(sol::state_view &sv)
{
	auto client_list = sv.new_usertype<Lua_Client_List>("ClientList");
	client_list["entries"] = sol::readonly_property(&Lua_Client_List::get_entries);
}

#ifdef BOTS
void lua_register_bot_list(sol::state_view &sv)
{
	auto bot_list = sv.new_usertype<Lua_Bot_List>("BotList");
	bot_list["entries"] = sol::readonly_property(&Lua_Bot_List::get_entries);
}
#endif

void lua_register_npc_list(sol::state_view &sv)
{
	auto npc_list = sv.new_usertype<Lua_NPC_List>("NPCList");
	npc_list["entries"] = sol::readonly_property(&Lua_NPC_List::get_entries);
}

void lua_register_corpse_list(sol::state_view &sv)
{
	auto corpse_list = sv.new_usertype<Lua_Corpse_List>("CorpseList");
	corpse_list["entries"] = sol::readonly_property(&Lua_Corpse_List::get_entries);
}

void lua_register_object_list(sol::state_view &sv)
{
	auto object_list = sv.new_usertype<Lua_Object_List>("ObjectList");
	object_list["entries"] = sol::readonly_property(&Lua_Object_List::get_entries);
}

void lua_register_door_list(sol::state_view &sv)
{
	auto doors_list = sv.new_usertype<Lua_Doors_List>("DoorList");
	doors_list["entries"] = sol::readonly_property(&Lua_Doors_List::get_entries);
}

void lua_register_spawn_list(sol::state_view &sv)
{
	auto spawn_list = sv.new_usertype<Lua_Spawn_List>("SpawnList");
	spawn_list["entries"] = sol::readonly_property(&Lua_Spawn_List::get_entries);
}

#endif
