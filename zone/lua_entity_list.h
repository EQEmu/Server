#ifndef EQEMU_LUA_ENTITY_LIST_H
#define EQEMU_LUA_ENTITY_LIST_H
#ifdef LUA_EQEMU

#include "lua_ptr.h"

class EntityList;
class Lua_Mob;
class Lua_Client;
class Lua_NPC;
class Lua_Door;
class Lua_Corpse;
class Lua_Object;
class Lua_Group;
class Lua_Raid;
class Lua_Spawn;
struct Lua_Mob_List;
struct Lua_Client_List;
struct Lua_NPC_List;
struct Lua_Corpse_List;
struct Lua_Object_List;
struct Lua_Doors_List;
struct Lua_Spawn_List;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_entity_list();
luabind::scope lua_register_mob_list();
luabind::scope lua_register_client_list();
luabind::scope lua_register_npc_list();
luabind::scope lua_register_corpse_list();
luabind::scope lua_register_object_list();
luabind::scope lua_register_door_list();
luabind::scope lua_register_spawn_list();

class Lua_EntityList : public Lua_Ptr<EntityList>
{
	typedef EntityList NativeType;
public:
	Lua_EntityList() : Lua_Ptr(nullptr) { }
	Lua_EntityList(EntityList *d) : Lua_Ptr(d) { }
	virtual ~Lua_EntityList() { }

	operator EntityList*() {
		return reinterpret_cast<EntityList*>(GetLuaPtrData());
	}

	Lua_Mob GetMobID(int id);
	Lua_Mob GetMob(const char *name);
	Lua_Mob GetMob(int id);
	Lua_Mob GetMobByNpcTypeID(int npc_type);
	bool IsMobSpawnedByNpcTypeID(int npc_type);
	Lua_NPC GetNPCByID(int id);
	Lua_NPC GetNPCByNPCTypeID(int npc_type);
	Lua_Client GetClientByName(const char *name);
	Lua_Client GetClientByAccID(uint32 acct_id);
	Lua_Client GetClientByID(int id);
	Lua_Client GetClientByCharID(uint32 char_id);
	Lua_Client GetClientByWID(uint32 wid);
	Lua_Object GetObjectByID(int id);
	Lua_Object GetObjectByDBID(uint32 db_id);
	Lua_Door GetDoorsByID(int id);
	Lua_Door GetDoorsByDBID(uint32 db_id);
	Lua_Door GetDoorsByDoorID(uint32 door_id);
	Lua_Door FindDoor(uint32 id);
	Lua_Group GetGroupByMob(Lua_Mob mob);
	Lua_Group GetGroupByClient(Lua_Client client);
	Lua_Group GetGroupByID(int id);
	Lua_Group GetGroupByLeaderName(const char *name);
	Lua_Raid GetRaidByID(int id);
	Lua_Raid GetRaidByClient(Lua_Client client);
	Lua_Corpse GetCorpseByOwner(Lua_Client client);
	Lua_Corpse GetCorpseByID(int id);
	Lua_Corpse GetCorpseByName(const char *name);
	Lua_Spawn GetSpawnByID(uint32 id);
	void ClearClientPetitionQueue();
	bool CanAddHateForMob(Lua_Mob p);
	void Message(uint32 guild_dbid, uint32 type, const char *message);
	void MessageStatus(uint32 guild_dbid, int min_status, uint32 type, const char *message);
	void MessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, const char *message);
	void FilteredMessageClose(Lua_Mob sender, bool skip_sender, float dist, uint32 type, int filter, const char *message);
	void RemoveFromTargets(Lua_Mob mob);
	void RemoveFromTargets(Lua_Mob mob, bool RemoveFromXTargets);
	void ReplaceWithTarget(Lua_Mob target, Lua_Mob new_target);
	void OpenDoorsNear(Lua_NPC opener);
	std::string MakeNameUnique(const char *name);
	std::string RemoveNumbers(const char *name);
	void SignalMobsByNPCID(uint32 npc_id, int signal);
	int DeleteNPCCorpses();
	int DeletePlayerCorpses();
	void HalveAggro(Lua_Mob who);
	void DoubleAggro(Lua_Mob who);
	void ClearFeignAggro(Lua_Mob who);
	bool Fighting(Lua_Mob who);
	void RemoveFromHateLists(Lua_Mob who);
	void RemoveFromHateLists(Lua_Mob who, bool set_to_one);
	void MessageGroup(Lua_Mob who, bool skip_close, uint32 type, const char *message);
	Lua_Client GetRandomClient(float x, float y, float z, float dist);
	Lua_Client GetRandomClient(float x, float y, float z, float dist, Lua_Client exclude);
	Lua_Mob_List GetMobList();
	Lua_Client_List GetClientList();
	Lua_NPC_List GetNPCList();
	Lua_Corpse_List GetCorpseList();
	Lua_Object_List GetObjectList();
	Lua_Doors_List GetDoorsList();
	Lua_Spawn_List GetSpawnList();
	void SignalAllClients(int signal);
	void ChannelMessage(Lua_Mob from, int channel_num, int language, const char *message);
};

#endif
#endif
