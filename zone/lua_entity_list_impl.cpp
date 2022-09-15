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

Lua_NPC Lua_EntityList::GetNPCBySpawnID(uint32 spawn_id) {
	Lua_Safe_Call_Class(Lua_NPC);
	return Lua_NPC(self->GetNPCBySpawnID(spawn_id));
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
	self->MessageStatus(
		guild_dbid,
		min_status,
		type,
		message
	);
}

void Lua_EntityList::MessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, const char *message) {
	Lua_Safe_Call_Void();

	if (RuleB(Chat, AutoInjectSaylinksToClientMessage))
	{
		std::string new_message = EQ::SayLinkEngine::InjectSaylinksIfNotExist(message);
		self->MessageClose(sender, skip_sender, dist, type, new_message.c_str());
	}
	else
	{
		self->MessageClose(sender, skip_sender, dist, type, message);
	}
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

void Lua_EntityList::OpenDoorsNear(Lua_Mob opener) {
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

uint32 Lua_EntityList::DeleteNPCCorpses() {
	Lua_Safe_Call_Int();
	return self->DeleteNPCCorpses();
}

uint32 Lua_EntityList::DeletePlayerCorpses() {
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

Lua_Client Lua_EntityList::GetRandomClient(float x, float y, float z, float distance) {
	Lua_Safe_Call_Class(Lua_Client);
	return self->GetRandomClient(glm::vec3(x, y, z), distance);
}

Lua_Client Lua_EntityList::GetRandomClient(float x, float y, float z, float distance, Lua_Client exclude_client) {
	Lua_Safe_Call_Class(Lua_Client);
	return self->GetRandomClient(glm::vec3(x, y, z), distance, exclude_client);
}

void Lua_EntityList::SignalAllClients(int signal) {
	Lua_Safe_Call_Void();
	self->SignalAllClients(signal);
}

void Lua_EntityList::ChannelMessage(Lua_Mob from, int channel_num, int language, const char *message) {
	Lua_Safe_Call_Void();
	self->ChannelMessage(from, channel_num, language, message);
}

Lua_Mob Lua_EntityList::GetRandomMob(float x, float y, float z, float distance) {
	Lua_Safe_Call_Class(Lua_Mob);
	return self->GetRandomMob(glm::vec3(x, y, z), distance);
}

Lua_Mob Lua_EntityList::GetRandomMob(float x, float y, float z, float distance, Lua_Mob exclude_mob) {
	Lua_Safe_Call_Class(Lua_Mob);
	return self->GetRandomMob(glm::vec3(x, y, z), distance, exclude_mob);
}

Lua_NPC Lua_EntityList::GetRandomNPC(float x, float y, float z, float distance) {
	Lua_Safe_Call_Class(Lua_NPC);
	return self->GetRandomNPC(glm::vec3(x, y, z), distance);
}

Lua_NPC Lua_EntityList::GetRandomNPC(float x, float y, float z, float distance, Lua_NPC exclude_npc) {
	Lua_Safe_Call_Class(Lua_NPC);
	return self->GetRandomNPC(glm::vec3(x, y, z), distance, exclude_npc);
}

#ifdef BOTS
Lua_Bot Lua_EntityList::GetBotByID(uint32 bot_id) {
	Lua_Safe_Call_Class(Lua_Bot);
	return Lua_Bot(self->GetBotByBotID(bot_id));
}

Lua_Bot Lua_EntityList::GetBotByName(std::string bot_name) {
	Lua_Safe_Call_Class(Lua_Bot);
	return Lua_Bot(self->GetBotByBotName(bot_name));
}
#endif

