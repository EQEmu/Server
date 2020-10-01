/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef ENTITY_H
#define ENTITY_H

#include <unordered_map>
#include <queue>

#include "../common/types.h"
#include "../common/linked_list.h"
#include "../common/servertalk.h"
#include "../common/bodytypes.h"
#include "../common/eq_constants.h"

#include "position.h"
#include "zonedump.h"
#include "common.h"

class Encounter;
class Beacon;
class Client;
class Corpse;
class Doors;
class EQApplicationPacket;
class Entity;
class EntityList;
class Group;
class Merc;
class Mob;
class NPC;
class Object;
class Petition;
class Raid;
class Spawn2;
class Trap;

struct GuildBankItemUpdate_Struct;
struct NewSpawn_Struct;
struct QGlobal;
struct UseAA_Struct;
struct Who_All_Struct;

#ifdef BOTS
class Bot;
class BotRaids;
#endif

extern EntityList entity_list;

class Entity
{
public:
	Entity();
	virtual ~Entity();

	virtual bool IsClient()			const { return false; }
	virtual bool IsNPC()			const { return false; }
	virtual bool IsMob()			const { return false; }
	virtual bool IsMerc()			const { return false; }
	virtual bool IsCorpse()			const { return false; }
	virtual bool IsPlayerCorpse()	const { return false; }
	virtual bool IsNPCCorpse()		const { return false; }
	virtual bool IsObject()			const { return false; }
	virtual bool IsDoor()			const { return false; }
	virtual bool IsTrap()			const { return false; }
	virtual bool IsBeacon()			const { return false; }
	virtual bool IsEncounter()		const { return false; }
	virtual bool IsBot()            const { return false; }
	virtual bool IsAura()			const { return false; }

	virtual bool Process() { return false; }
	virtual bool Save() { return true; }
	virtual void Depop(bool StartSpawnTimer = false) {}

	Client	*CastToClient();
	NPC		*CastToNPC();
	Mob		*CastToMob();
	Merc	*CastToMerc();
	Corpse	*CastToCorpse();
	Object	*CastToObject();
	Doors	*CastToDoors();
	Trap	*CastToTrap();
	Beacon	*CastToBeacon();
	Encounter *CastToEncounter();

	const Client	*CastToClient() const;
	const NPC		*CastToNPC() const;
	const Mob		*CastToMob() const;
	const Merc		*CastToMerc() const;
	const Corpse	*CastToCorpse() const;
	const Object	*CastToObject() const;
	const Doors		*CastToDoors() const;
	const Trap		*CastToTrap() const;
	const Beacon	*CastToBeacon() const;
	const Encounter *CastToEncounter() const;

	inline const uint16& GetInitialId() const { return initial_id; }
	inline const uint16& GetID() const { return id; }
	inline const time_t& GetSpawnTimeStamp() const { return spawn_timestamp; }

	virtual const char* GetName() { return ""; }
	bool CheckCoordLosNoZLeaps(float cur_x, float cur_y, float cur_z, float trg_x, float trg_y, float trg_z, float perwalk=1);

#ifdef BOTS
	Bot* CastToBot();
	const Bot* CastToBot() const;
#endif

protected:
	friend class EntityList;
	inline virtual void SetID(uint16 set_id) {
		id = set_id;

		if (initial_id == 0 && set_id > 0) {
			initial_id = set_id;
		}
	}
	uint32 pDBAsyncWorkID;
private:
	uint16 id;
	uint16 initial_id;
	time_t spawn_timestamp;
};

class EntityList
{
public:
	struct Area {
		int id;
		int type;
		float min_x, max_x;
		float min_y, max_y;
		float min_z, max_z;
	};

	EntityList();
	~EntityList();

	Entity* GetID(uint16 id);
	Mob *GetMob(uint16 id);
	inline Mob *GetMobID(uint16 id) { return(GetMob(id)); }	//for perl
	Mob *GetMob(const char* name);
	Mob *GetMobByNpcTypeID(uint32 get_id);
	bool IsMobSpawnedByNpcTypeID(uint32 get_id);
	Mob *GetTargetForVirus(Mob* spreader, int range);
	inline NPC *GetNPCByID(uint16 id)
	{
		auto it = npc_list.find(id);
		if (it != npc_list.end())
			return it->second;
		return nullptr;
	}
	NPC *GetNPCByNPCTypeID(uint32 npc_id);
	NPC *GetNPCBySpawnID(uint32 spawn_id);
	inline Merc *GetMercByID(uint16 id)
	{
		auto it = merc_list.find(id);
		if (it != merc_list.end())
			return it->second;
		return nullptr;
	}
	Client *GetClientByName(const char *name);
	Client *GetClientByAccID(uint32 accid);
	inline Client *GetClientByID(uint16 id)
	{
		auto it = client_list.find(id);
		if (it != client_list.end())
			return it->second;
		return nullptr;
	}
	Client *GetClientByCharID(uint32 iCharID);
	Client *GetClientByWID(uint32 iWID);
	Client *GetClientByLSID(uint32 iLSID);
	Client *GetClient(uint32 ip, uint16 port);
	Client *GetRandomClient(const glm::vec3& location, float Distance, Client *ExcludeClient = nullptr);
	Group *GetGroupByMob(Mob* mob);
	Group *GetGroupByClient(Client* client);
	Group *GetGroupByID(uint32 id);
	Group *GetGroupByLeaderName(const char* leader);
	Raid *GetRaidByMob(Mob* mob);
	Raid *GetRaidByClient(Client* client);
	Raid *GetRaidByID(uint32 id);
	Raid *GetRaidByLeaderName(const char *leader);

	Corpse *GetCorpseByOwner(Client* client);
	Corpse *GetCorpseByOwnerWithinRange(Client* client, Mob* center, int range);
	inline Corpse *GetCorpseByID(uint16 id)
	{
		auto it = corpse_list.find(id);
		if (it != corpse_list.end())
			return it->second;
		return nullptr;
	}
	Corpse *GetCorpseByDBID(uint32 dbid);
	Corpse *GetCorpseByName(const char* name);

	Spawn2* GetSpawnByID(uint32 id);

	Client* FindCorpseDragger(uint16 CorpseID);

	inline Object *GetObjectByID(uint16 id)
	{
		auto it = object_list.find(id);
		if (it != object_list.end())
			return it->second;
		return nullptr;
	}
	Object *GetObjectByDBID(uint32 id);
	inline Doors *GetDoorsByID(uint16 id)
	{
		auto it = door_list.find(id);
		if (it != door_list.end())
			return it->second;
		return nullptr;
	}
	Doors *GetDoorsByDoorID(uint32 id);
	Doors *GetDoorsByDBID(uint32 id);
	void RemoveAllCorpsesByCharID(uint32 charid);
	void RemoveCorpseByDBID(uint32 dbid);
	int RezzAllCorpsesByCharID(uint32 charid);
	bool IsMobInZone(Mob *who);
	void ClearClientPetitionQueue();
	bool CanAddHateForMob(Mob *p);
	void	SendGuildMOTD(uint32 guild_id);
	void	SendGuildSpawnAppearance(uint32 guild_id);
	void	SendGuildMembers(uint32 guild_id);
	void	RefreshAllGuildInfo(uint32 guild_id);
	void	SendGuildList();
	void	CheckGroupList (const char *fname, const int fline);
	void	GroupProcess();
	void	RaidProcess();
	void	DoorProcess();
	void	ObjectProcess();
	void	CorpseProcess();
	void	MobProcess();
	void	TrapProcess();
	void	BeaconProcess();
	void	EncounterProcess();
	void	ProcessMove(Client *c, const glm::vec3& location);
	void	ProcessMove(NPC *n, float x, float y, float z);
	void	AddArea(int id, int type, float min_x, float max_x, float min_y, float max_y, float min_z, float max_z);
	void	RemoveArea(int id);
	void	ClearAreas();
	void	ReloadMerchants();
	void	ProcessProximitySay(const char *Message, Client *c, uint8 language = 0);
	void	SendAATimer(uint32 charid,UseAA_Struct* uaa);
	Doors *FindDoor(uint8 door_id);
	Object *FindObject(uint32 object_id);
	Object*	FindNearbyObject(float x, float y, float z, float radius);
	bool	MakeDoorSpawnPacket(EQApplicationPacket* app, Client *client);
	bool	MakeTrackPacket(Client* client);
	void	SendTraders(Client* client);
	void	AddClient(Client*);
	void	AddNPC(NPC*, bool SendSpawnPacket = true, bool dontqueue = false);
	void	AddMerc(Merc*, bool SendSpawnPacket = true, bool dontqueue = false);
	void	AddCorpse(Corpse* pc, uint32 in_id = 0xFFFFFFFF);
	void	AddObject(Object*, bool SendSpawnPacket = true);
	void	AddGroup(Group*);
	void	AddGroup(Group*, uint32 id);
	void	AddRaid(Raid *raid);
	void	AddRaid(Raid*, uint32 id);
	void	AddDoor(Doors* door);
	void	AddTrap(Trap* trap);
	void	AddBeacon(Beacon *beacon);
	void	AddEncounter(Encounter *encounter);
	void	AddProximity(NPC *proximity_for);
	void	Clear();
	bool	RemoveMob(uint16 delete_id);
	bool	RemoveMob(Mob* delete_mob);
	bool	RemoveClient(uint16 delete_id);
	bool	RemoveClient(Client* delete_client);
	bool	RemoveNPC(uint16 delete_id);
	bool	RemoveMerc(uint16 delete_id);
	bool	RemoveGroup(uint32 delete_id);
	bool	RemoveRaid(uint32 delete_id);
	bool	RemoveCorpse(uint16 delete_id);
	bool	RemoveDoor(uint16 delete_id);
	bool	RemoveTrap(uint16 delete_id);
	bool	RemoveObject(uint16 delete_id);
	bool	RemoveProximity(uint16 delete_npc_id);
	bool	RemoveMobFromCloseLists(Mob *mob);
	void    RemoveAuraFromMobs(Mob *aura);
	void	RemoveAllMobs();
	void	RemoveAllClients();
	void	RemoveAllNPCs();
	void	RemoveAllMercs();
	void	RemoveAllGroups();
	void	RemoveAllCorpses();
	void	RemoveAllDoors();
	void	DespawnAllDoors();
	void	RespawnAllDoors();
	void	RemoveAllTraps();
	void	RemoveAllObjects();
	void	RemoveAllLocalities();
	void	RemoveAllRaids();
	void	RemoveAllEncounters();
	void	DestroyTempPets(Mob *owner);
	int16	CountTempPets(Mob *owner);
	void	AddTempPetsToHateList(Mob *owner, Mob* other, bool bFrenzy = false);
	Entity *GetEntityMob(uint16 id);
	Entity *GetEntityMerc(uint16 id);
	Entity *GetEntityDoor(uint16 id);
	Entity *GetEntityObject(uint16 id);
	Entity *GetEntityCorpse(uint16 id);
	Entity *GetEntityTrap(uint16 id);
	Entity *GetEntityBeacon(uint16 id);
	Entity *GetEntityEncounter(uint16 id);
	Entity *GetEntityMob(const char *name);
	Entity *GetEntityCorpse(const char *name);

	void	StopMobAI();

	void DescribeAggro(Client *towho, NPC *from_who, float dist, bool verbose);

	void	Message(uint32 to_guilddbid, uint32 type, const char* message, ...);
	void	MessageStatus(uint32 to_guilddbid, int to_minstatus, uint32 type, const char* message, ...);
	void	MessageClose(Mob* sender, bool skipsender, float dist, uint32 type, const char* message, ...);
	void	FilteredMessageClose(Mob* sender, bool skipsender, float dist, uint32 type, eqFilterType filter, const char* message, ...);
	void	MessageString(Mob *sender, bool skipsender, uint32 type, uint32 string_id, const char* message1=0,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0);
	void	FilteredMessageString(Mob *sender, bool skipsender, uint32 type, eqFilterType filter, uint32 string_id, const char* message1=0,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0);
	void	MessageCloseString(
		Mob *sender,
		bool skipsender,
		float dist,
		uint32 type,
		uint32 string_id,
		const char *message1 = 0,
		const char *message2 = 0,
		const char *message3 = 0,
		const char *message4 = 0,
		const char *message5 = 0,
		const char *message6 = 0,
		const char *message7 = 0,
		const char *message8 = 0,
		const char *message9 = 0);
	void	FilteredMessageCloseString(
		Mob *sender,
		bool skipsender,
		float dist,
		uint32 type,
		eqFilterType filter,
		uint32 string_id,
		const char *message1 = 0,
		const char *message2 = 0,
		const char *message3 = 0,
		const char *message4 = 0,
		const char *message5 = 0,
		const char *message6 = 0,
		const char *message7 = 0,
		const char *message8 = 0,
		const char *message9 = 0);
	void	ChannelMessageFromWorld(const char* from, const char* to, uint8 chan_num, uint32 guilddbid, uint8 language, uint8 lang_skill, const char* message);
	void	ChannelMessage(Mob* from, uint8 chan_num, uint8 language, const char* message, ...);
	void	ChannelMessage(Mob* from, uint8 chan_num, uint8 language, uint8 lang_skill, const char* message, ...);
	void	SendZoneSpawns(Client*);
	void	SendZonePVPUpdates(Client *);
	void	SendZoneSpawnsBulk(Client* client);
	void	Save();
	void	SendZoneCorpses(Client*);
	void	SendZoneCorpsesBulk(Client*);
	void	SendZoneObjects(Client* client);
	void	SendZoneAppearance(Client *c);
	void	SendNimbusEffects(Client *c);
	void	SendUntargetable(Client *c);
	void	DuelMessage(Mob* winner, Mob* loser, bool flee);
	void	QuestJournalledSayClose(Mob *sender, float dist, const char* mobname, const char* message, Journal::Options &opts);
	void	GroupMessage(uint32 gid, const char *from, const char *message);
	void	ExpeditionWarning(uint32 minutes_left);

	void	RemoveFromTargets(Mob* mob, bool RemoveFromXTargets = false);
	void	RemoveFromXTargets(Mob* mob);
	void	RemoveFromAutoXTargets(Mob* mob);
	void	ReplaceWithTarget(Mob* pOldMob, Mob*pNewTarget);
	void	QueueCloseClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender=false, float distance=200, Mob* skipped_mob = 0, bool is_ack_required = true, eqFilterType filter=FilterNone);
	void	QueueClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender=false, bool ackreq = true);
	void	QueueClientsStatus(Mob* sender, const EQApplicationPacket* app, bool ignore_sender = false, uint8 minstatus = 0, uint8 maxstatus = 0);
	void	QueueClientsGuild(Mob* sender, const EQApplicationPacket* app, bool ignore_sender = false, uint32 guildeqid = 0);
	void	QueueClientsGuildBankItemUpdate(const GuildBankItemUpdate_Struct *gbius, uint32 GuildID);
	void	QueueClientsByTarget(Mob* sender, const EQApplicationPacket* app, bool iSendToSender = true, Mob* SkipThisMob = 0, bool ackreq = true,
						bool HoTT = true, uint32 ClientVersionBits = 0xFFFFFFFF, bool inspect_buffs = false);

	void	QueueClientsByXTarget(Mob* sender, const EQApplicationPacket* app, bool iSendToSender = true, EQ::versions::ClientVersionBitmask client_version_bits = EQ::versions::ClientVersionBitmask::maskAllClients);
	void	QueueToGroupsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app);
	void	QueueManaged(Mob* sender, const EQApplicationPacket* app, bool ignore_sender=false, bool ackreq = true);

	void AEAttack(
		Mob *attacker,
		float distance,
		int Hand = EQ::invslot::slotPrimary,
		int count = 0,
		bool is_from_spell = false
	);
	void AETaunt(Client *caster, float range = 0, int32 bonus_hate = 0);
	void AESpell(
		Mob *caster,
		Mob *center,
		uint16 spell_id,
		bool affect_caster = true,
		int16 resist_adjust = 0,
		int *max_targets = nullptr
	);
	void MassGroupBuff(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster = true);
	void AEBardPulse(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster = true);

	//trap stuff
	Mob*	GetTrapTrigger(Trap* trap);
	void	SendAlarm(Trap* trap, Mob* currenttarget, uint8 kos);
	Trap*	FindNearbyTrap(Mob* searcher, float max_dist, float &curdist, bool detected = false);

	void	AddHealAggro(Mob* target, Mob* caster, uint16 hate);
	Mob*	FindDefenseNPC(uint32 npcid);
	void	OpenDoorsNear(Mob* opener);
	void	UpdateWho(bool iSendFullUpdate = false);
	char*	MakeNameUnique(char* name);
	static char* RemoveNumbers(char* name);
	void	SignalMobsByNPCID(uint32 npc_type, int signal_id);
	void	CountNPC(uint32* NPCCount, uint32* NPCLootCount, uint32* gmspawntype_count);
	void	RemoveEntity(uint16 id);
	void	SendPetitionToAdmins(Petition* pet);
	void	SendPetitionToAdmins();
	void	AddLootToNPCS(uint32 item_id, uint32 count);

	void	ListNPCCorpses(Client* client);
	void	ListPlayerCorpses(Client* client);
	int32	DeleteNPCCorpses();
	int32	DeletePlayerCorpses();
	void	CorpseFix(Client* c);
	void	WriteEntityIDs();
	void	HalveAggro(Mob* who);
	void	DoubleAggro(Mob* who);
	void	Evade(Mob *who);
	void	UpdateHoTT(Mob* target);

	void	Process();
	void	ClearAggro(Mob* targ);
	void    ClearWaterAggro(Mob* targ);
	void	ClearFeignAggro(Mob* targ);
	void	ClearZoneFeignAggro(Client* targ);
	void	AggroZone(Mob* who, uint32 hate = 0);

	bool	Fighting(Mob* targ);
	void	RemoveFromHateLists(Mob* mob, bool settoone = false);
	void	RemoveDebuffs(Mob* caster);


	void	MessageGroup(Mob* sender, bool skipclose, uint32 type, const char* message, ...);

	void	LimitAddNPC(NPC *npc);
	void	LimitRemoveNPC(NPC *npc);
	bool	LimitCheckType(uint32 npc_type, int count);
	bool	LimitCheckGroup(uint32 spawngroup_id, int count);
	bool	LimitCheckBoth(uint32 npc_type, uint32 spawngroup_id, int group_count, int type_count);
	bool	LimitCheckName(const char* npc_name);

	int		GetHatedCount(Mob *attacker, Mob *exclude, bool inc_gray_con);
	bool	Merc_AICheckCloseBeneficialSpells(Merc* caster, uint8 iChance, float iRange, uint32 iSpellTypes);
	Mob*	GetTargetForMez(Mob* caster);
	uint32	CheckNPCsClose(Mob *center);

	Corpse* GetClosestCorpse(Mob* sender, const char *Name);
	NPC* GetClosestBanker(Mob* sender, uint32 &distance);
	void	CameraEffect(uint32 duration, uint32 intensity);
	Mob*	GetClosestMobByBodyType(Mob* sender, bodyType BodyType);
	void	ForceGroupUpdate(uint32 gid);
	void	SendGroupLeave(uint32 gid, const char *name);
	void	SendGroupJoin(uint32 gid, const char *name);

	void	SaveAllClientsTaskState();
	void	ReloadAllClientsTaskState(int TaskID=0);
	uint16	CreateGroundObject(uint32 itemid, const glm::vec4& position, uint32 decay_time = 300000);
	uint16	CreateGroundObjectFromModel(const char *model, const glm::vec4& position, uint8 type = 0x00, uint32 decay_time = 0);
	uint16	CreateDoor(const char *model, const glm::vec4& position, uint8 type = 0, uint16 size = 100);
	void	ZoneWho(Client *c, Who_All_Struct* Who);
	void	UnMarkNPC(uint16 ID);

	void	GateAllClients();
	void	SignalAllClients(uint32 data);
	void	UpdateQGlobal(uint32 qid, QGlobal newGlobal);
	void	DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID);
	void	SendFindableNPCList(Client *c);
	void	UpdateFindableNPCState(NPC *n, bool Remove);
	void	HideCorpses(Client *c, uint8 CurrentMode, uint8 NewMode);

	uint16 GetClientCount();
	void GetMobList(std::list<Mob*> &m_list);
	void GetNPCList(std::list<NPC*> &n_list);
	void GetMercList(std::list<Merc*> &n_list);
	void GetClientList(std::list<Client*> &c_list);
	void GetCorpseList(std::list<Corpse*> &c_list);
	void GetObjectList(std::list<Object*> &o_list);
	void GetDoorsList(std::list<Doors*> &d_list);
	void GetSpawnList(std::list<Spawn2*> &d_list);
	void GetTargetsForConeArea(Mob *start, float min_radius, float radius, float height, int pcnpc, std::list<Mob*> &m_list);

	inline const std::unordered_map<uint16, Mob *> &GetMobList() { return mob_list; }
	inline const std::unordered_map<uint16, NPC *> &GetNPCList() { return npc_list; }
	inline const std::unordered_map<uint16, Merc *> &GetMercList() { return merc_list; }
	inline const std::unordered_map<uint16, Client *> &GetClientList() { return client_list; }
	inline const std::unordered_map<uint16, Corpse *> &GetCorpseList() { return corpse_list; }
	inline const std::unordered_map<uint16, Object *> &GetObjectList() { return object_list; }
	inline const std::unordered_map<uint16, Doors *> &GetDoorsList() { return door_list; }

	std::unordered_map<uint16, Mob *> &GetCloseMobList(Mob *mob, float distance = 0);

	void	DepopAll(int NPCTypeID, bool StartSpawnTimer = true);

	uint16 GetFreeID();
	void RefreshAutoXTargets(Client *c);
	void RefreshClientXTargets(Client *c);
	void SendAlternateAdvancementStats();
	void ScanCloseMobs(
		std::unordered_map<uint16, Mob *> &close_mobs,
		Mob *scanning_mob,
		bool add_self_to_other_lists = false
	);

	void GetTrapInfo(Client* client);
	bool IsTrapGroupSpawned(uint32 trap_id, uint8 group);
	void UpdateAllTraps(bool respawn, bool repopnow = false);
	void ClearTrapPointers();

protected:
	friend class Zone;
	void	Depop(bool StartSpawnTimer = false);

private:
	void	AddToSpawnQueue(uint16 entityid, NewSpawn_Struct** app);
	void	CheckSpawnQueue();

	//used for limiting spawns
	class SpawnLimitRecord { public: uint32 spawngroup_id; uint32 npc_type; };
	std::map<uint16, SpawnLimitRecord> npc_limit_list;		//entity id -> npc type

	uint32	tsFirstSpawnOnQueue; // timestamp that the top spawn on the spawnqueue was added, should be 0xFFFFFFFF if queue is empty
	uint32	NumSpawnsOnQueue;
	LinkedList<NewSpawn_Struct*> SpawnQueue;

	std::unordered_map<uint16, Client *> client_list;
	std::unordered_map<uint16, Mob *> mob_list;
	std::unordered_map<uint16, NPC *> npc_list;
	std::unordered_map<uint16, Merc *> merc_list;
	std::unordered_map<uint16, Corpse *> corpse_list;
	std::unordered_map<uint16, Object *> object_list;
	std::unordered_map<uint16, Doors *> door_list;
	std::unordered_map<uint16, Trap *> trap_list;
	std::unordered_map<uint16, Beacon *> beacon_list;
	std::unordered_map<uint16, Encounter *> encounter_list;
	std::list<NPC *> proximity_list;
	std::list<Group *> group_list;
	std::list<Raid *> raid_list;
	std::list<Area> area_list;
	std::queue<uint16> free_ids;

	Timer object_timer;
	Timer door_timer;
	Timer corpse_timer;
	Timer group_timer;
	Timer raid_timer;
	Timer trap_timer;

	// Please Do Not Declare Any EntityList Class Members After This Comment
#ifdef BOTS
	public:
		void AddBot(Bot* newBot, bool SendSpawnPacket = true, bool dontqueue = false);
		bool RemoveBot(uint16 entityID);
		Mob* GetMobByBotID(uint32 botID);
		Bot* GetBotByBotID(uint32 botID);
		Bot* GetBotByBotName(std::string botName);
		Client* GetBotOwnerByBotEntityID(uint16 entityID);
		std::list<Bot*> GetBotsByBotOwnerCharacterID(uint32 botOwnerCharacterID);

		bool Bot_AICheckCloseBeneficialSpells(Bot* caster, uint8 iChance, float iRange, uint32 iSpellTypes); // TODO: Evaluate this closesly in hopes to eliminate
		void ShowSpawnWindow(Client* client, int Distance, bool NamedOnly); // TODO: Implement ShowSpawnWindow in the bot class but it needs entity list stuff

		void ScanCloseClientMobs(std::unordered_map<uint16, Mob*>& close_mobs, Mob* scanning_mob);
	private:
		std::list<Bot*> bot_list;
#endif
};

class BulkZoneSpawnPacket {
public:
	BulkZoneSpawnPacket(Client* iSendTo, uint32 iMaxSpawnsPerPacket);	// 0 = send zonewide
	virtual ~BulkZoneSpawnPacket();

	bool	AddSpawn(NewSpawn_Struct* ns);
	void	SendBuffer();	// Sends the buffer and cleans up everything - can safely re-use the object after this function call (no need to free and do another new)
private:
	uint32	pMaxSpawnsPerPacket;
	uint32	index;
	NewSpawn_Struct* data;
	Client* pSendTo;
};

#endif

