/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/types.h"
#include "../common/linked_list.h"
#include "zonedb.h"
#include "../common/eq_constants.h"
#include "zonedump.h"
#include "zonedbasync.h"
#include "../common/servertalk.h"
#include "../common/bodytypes.h"
#include "QGlobals.h"

// max number of newspawns to send per bulk packet
#define SPAWNS_PER_POINT_DATARATE 10
#define MAX_SPAWNS_PER_PACKET	100

//#ifdef _WINDOWS
	class	EQApplicationPacket;
//#else
//	struct	EQApplicationPacket;
//#endif

class Client;
class Mob;
class NPC;
class Merc;
class Corpse;
class Beacon;
class Petition;
class Object;
class Group;
class Raid;
class Doors;
class Trap;
class Entity;
class EntityList;

#ifdef BOTS
class Bot;
class BotRaids;
#endif

extern EntityList entity_list;

void ProcessClientThreadSpawn(void *tmp);

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
//	virtual bool IsGroup()			const { return false; }
	virtual bool IsDoor()			const { return false; }
	virtual bool IsTrap()			const { return false; }
	virtual bool IsBeacon()			const { return false; }

	virtual bool Process() { return false; }
	virtual bool Save() { return true; }
	virtual void Depop(bool StartSpawnTimer = false) {}

	Client* CastToClient();
	NPC*	CastToNPC();
	Mob*	CastToMob();
	Merc*	CastToMerc();
	Corpse*	CastToCorpse();
	Object* CastToObject();
	Doors*	CastToDoors();
	Trap*	CastToTrap();
	Beacon*	CastToBeacon();

	const Client* CastToClient() const;
	const NPC*		CastToNPC() const;
	const Mob*		CastToMob() const;
	const Merc*		CastToMerc() const;
	const Corpse*	CastToCorpse() const;
	const Object*	CastToObject() const;
//	const Group*	CastToGroup() const;
	const Doors*	CastToDoors() const;
	const Trap*		CastToTrap() const;
	const Beacon*	CastToBeacon() const;

	inline const uint16& GetID() const{ return id; }
	virtual const char* GetName() { return ""; }
	virtual void DBAWComplete(uint8 workpt_b1, DBAsyncWork* dbaw) { pDBAsyncWorkID = 0; }
	bool CheckCoordLosNoZLeaps(float cur_x, float cur_y, float cur_z, float trg_x, float trg_y, float trg_z, float perwalk=1);

#ifdef BOTS
	virtual bool IsBot() const { return false; }
	Bot* CastToBot();
#endif

protected:
	friend class EntityList;
	virtual void SetID(uint16 set_id);
	uint32 pDBAsyncWorkID;
private:
	uint16 id;
};

class EntityList
{
public:
	EntityList();
	~EntityList();

	Entity* GetID(uint16 id);
	Mob*	GetMob(uint16 id);
	inline Mob*	GetMobID(uint16 id) { return(GetMob(id)); }	//for perl
	Mob*	GetMob(const char* name);
	Mob*	GetMobByNpcTypeID(uint32 get_id);
	Mob*	GetTargetForVirus(Mob* spreader);
	NPC*	GetNPCByID(uint16 id);
	NPC*	GetNPCByNPCTypeID(uint32 npc_id);
	Merc*	GetMercByID(uint16 id);
	Client* GetClientByName(const char *name);
	Client* GetClientByAccID(uint32 accid);
	Client* GetClientByID(uint16 id);
	Client* GetClientByCharID(uint32 iCharID);
	Client* GetClientByWID(uint32 iWID);
	Client* GetClient(uint32 ip, uint16 port);
	Client* GetRandomClient(float x, float y, float z, float Distance, Client *ExcludeClient = nullptr);
	Group*	GetGroupByMob(Mob* mob);
	Group*	GetGroupByClient(Client* client);
	Group*	GetGroupByID(uint32 id);
	Group*	GetGroupByLeaderName(char* leader);
	Raid*	GetRaidByMob(Mob* mob);
	Raid*	GetRaidByClient(Client* client);
	Raid*	GetRaidByID(uint32 id);
	Raid*	GetRaidByLeaderName(const char *leader);

	Corpse*	GetCorpseByOwner(Client* client);
	Corpse*	GetCorpseByOwnerWithinRange(Client* client, Mob* center, int range);
	Corpse* GetCorpseByID(uint16 id);
	Corpse* GetCorpseByDBID(uint32 dbid);
	Corpse* GetCorpseByName(const char* name);

	Client* FindCorpseDragger(const char *CorpseName);

	Object*	GetObjectByID(uint16 id);
	Object*	GetObjectByDBID(uint32 id);
	Doors*	GetDoorsByID(uint16 id);
	Doors* GetDoorsByDoorID(uint32 id);
	Doors*	GetDoorsByDBID(uint32 id);
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
//	void	SendGuildJoin(GuildJoin_Struct* gj);
	// Check group list for nullptr entries
	void	CheckGroupList (const char *fname, const int fline);
	void	GroupProcess();
	void	RaidProcess();
	void	DoorProcess();
	void	ObjectProcess();
	void	CorpseProcess();
	void	MobProcess();
	void	TrapProcess();
	void	BeaconProcess();
	void	ProcessMove(Client *c, float x, float y, float z);
	void	ProcessProximitySay(const char *Message, Client *c, uint8 language = 0);
	void	SendAATimer(uint32 charid,UseAA_Struct* uaa);
	Doors*	FindDoor(uint8 door_id);
	Object*	FindObject(uint32 object_id);
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
	void	DestroyTempPets(Mob *owner);
	Entity*	GetEntityMob(uint16 id);
	Entity* GetEntityMob(const char *name);
	Entity*	GetEntityMerc(uint16 id);
	Entity*	GetEntityDoor(uint16 id);
	Entity*	GetEntityObject(uint16 id);
	Entity*	GetEntityCorpse(uint16 id);
	Entity* GetEntityCorpse(const char *name);
//	Entity*	GetEntityGroup(uint32 id);
	Entity*	GetEntityTrap(uint16 id);
	Entity*	GetEntityBeacon(uint16 id);

	void DescribeAggro(Client *towho, NPC *from_who, float dist, bool verbose);

	void	Message(uint32 to_guilddbid, uint32 type, const char* message, ...);
	void	MessageStatus(uint32 to_guilddbid, int to_minstatus, uint32 type, const char* message, ...);
	void	MessageClose(Mob* sender, bool skipsender, float dist, uint32 type, const char* message, ...);
	void	Message_StringID(Mob *sender, bool skipsender, uint32 type, uint32 string_id, const char* message1=0,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0);
	void	MessageClose_StringID(Mob *sender, bool skipsender, float dist, uint32 type, uint32 string_id, const char* message1=0,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0);
	void	ChannelMessageFromWorld(const char* from, const char* to, uint8 chan_num, uint32 guilddbid, uint8 language, const char* message);
	void	ChannelMessage(Mob* from, uint8 chan_num, uint8 language, const char* message, ...);
	void	ChannelMessage(Mob* from, uint8 chan_num, uint8 language, uint8 lang_skill, const char* message, ...);
	void	ChannelMessageSend(Mob* to, uint8 chan_num, uint8 language, const char* message, ...);
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
	void	QuestJournalledSayClose(Mob *sender, Client *QuestIntiator, float dist, const char* mobname, const char* message);
	void	GroupMessage(uint32 gid, const char *from, const char *message);
	void	ExpeditionWarning(uint32 minutes_left);

	void	RemoveFromTargets(Mob* mob, bool RemoveFromXTargets = false);
	void	RemoveFromXTargets(Mob* mob);
	void	RemoveFromAutoXTargets(Mob* mob);
	void	ReplaceWithTarget(Mob* pOldMob, Mob*pNewTarget);
	void	QueueCloseClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender=false, float dist=200, Mob* SkipThisMob = 0, bool ackreq = true,eqFilterType filter=FilterNone);
	void	QueueClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender=false, bool ackreq = true);
	void	QueueClientsStatus(Mob* sender, const EQApplicationPacket* app, bool ignore_sender = false, uint8 minstatus = 0, uint8 maxstatus = 0);
	void	QueueClientsGuild(Mob* sender, const EQApplicationPacket* app, bool ignore_sender = false, uint32 guildeqid = 0);
	void	QueueClientsGuildBankItemUpdate(const GuildBankItemUpdate_Struct *gbius, uint32 GuildID);
	void	QueueClientsByTarget(Mob* sender, const EQApplicationPacket* app, bool iSendToSender = true, Mob* SkipThisMob = 0, bool ackreq = true,
						bool HoTT = true, uint32 ClientVersionBits = 0xFFFFFFFF);

	void	QueueClientsByXTarget(Mob* sender, const EQApplicationPacket* app, bool iSendToSender = true);
	void	QueueToGroupsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app);
	void	QueueManaged(Mob* sender, const EQApplicationPacket* app, bool ignore_sender=false, bool ackreq = true);

	void	AEAttack(Mob *attacker, float dist, int Hand = 13, int count = 0, bool IsFromSpell = false);
	void	AETaunt(Client *caster, float range = 0);
	void	AESpell(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster = true, int16 resist_adjust = 0);
	void	MassGroupBuff(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster = true);
	void	AEBardPulse(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster = true);

	void	RadialSetLogging(Mob *around, bool enabled, bool clients, bool non_clients, float range = 0);

	//trap stuff
	Mob*	GetTrapTrigger(Trap* trap);
	void	SendAlarm(Trap* trap, Mob* currenttarget, uint8 kos);
	Trap*	FindNearbyTrap(Mob* searcher, float max_dist);

	void	AddHealAggro(Mob* target, Mob* caster, uint16 thedam);
	Mob*	FindDefenseNPC(uint32 npcid);
	void	OpenDoorsNear(NPC* opener);
	void	UpdateWho(bool iSendFullUpdate = false);
	void	SendPositionUpdates(Client* client, uint32 cLastUpdate = 0, float range = 0, Entity* alwayssend = 0, bool iSendEvenIfNotChanged = false);
	char*	MakeNameUnique(char* name);
	static char* RemoveNumbers(char* name);
	void	SignalMobsByNPCID(uint32 npc_type, int signal_id);
	void	CountNPC(uint32* NPCCount, uint32* NPCLootCount, uint32* gmspawntype_count);
	void	DoZoneDump(ZSDump_Spawn2* spawn2dump, ZSDump_NPC* npcdump, ZSDump_NPC_Loot* npclootdump, NPCType* gmspawntype_dump);
	void	RemoveEntity(uint16 id);
	void	SendPetitionToAdmins(Petition* pet);
	void	SendPetitionToAdmins();
	void	AddLootToNPCS(uint32 item_id, uint32 count);

	void	ListNPCs(Client* client, const char* arg1 = 0, const char* arg2 = 0, uint8 searchtype = 0);
	void	ListNPCCorpses(Client* client);
	void	ListPlayerCorpses(Client* client);
	void	FindPathsToAllNPCs();
	int32	DeleteNPCCorpses();
	int32	DeletePlayerCorpses();
	void	WriteEntityIDs();
	void	HalveAggro(Mob* who);
	void	DoubleAggro(Mob* who);
	void	Evade(Mob *who);
	void	UpdateHoTT(Mob* target);

	void	Process();
	void	ClearAggro(Mob* targ);
	void	ClearFeignAggro(Mob* targ);
	void	ClearZoneFeignAggro(Client* targ);
	void	AggroZone(Mob* who, int hate = 0);

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

	void	CheckClientAggro(Client *around);
	Mob*	AICheckCloseAggro(Mob* sender, float iAggroRange, float iAssistRange);
	int	GetHatedCount(Mob *attacker, Mob *exclude);
	void	AIYellForHelp(Mob* sender, Mob* attacker);
	bool	AICheckCloseBeneficialSpells(NPC* caster, uint8 iChance, float iRange, uint16 iSpellTypes);
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

	uint16	CreateGroundObject(uint32 itemid, float x, float y, float z, float heading, uint32 decay_time = 300000);
	uint16	CreateGroundObjectFromModel(const char *model, float x, float y, float z, float heading, uint8 type = 0x00, uint32 decay_time = 0);
	uint16	CreateDoor(const char *model, float x, float y, float z, float heading, uint8 type = 0, uint16 size = 100);
	void	ZoneWho(Client *c, Who_All_Struct* Who);
	void	UnMarkNPC(uint16 ID);

	void	GateAllClients();
	void	SignalAllClients(uint32 data);
	void	UpdateQGlobal(uint32 qid, QGlobal newGlobal);
	void	DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID);
	void	SendFindableNPCList(Client *c);
	void	UpdateFindableNPCState(NPC *n, bool Remove);
	void	HideCorpses(Client *c, uint8 CurrentMode, uint8 NewMode);

	void GetMobList(std::list<Mob*> &m_list);
	void GetNPCList(std::list<NPC*> &n_list);
	void GetMercList(std::list<Merc*> &n_list);
	void GetClientList(std::list<Client*> &c_list);
	void GetCorpseList(std::list<Corpse*> &c_list);
	void GetObjectList(std::list<Object*> &o_list);
	void GetDoorsList(std::list<Doors*> &d_list);
	void GetTargetsForConeArea(Mob *start, uint32 radius, uint32 height, std::list<Mob*> &m_list);

	void	DepopAll(int NPCTypeID, bool StartSpawnTimer = true);

	uint16 GetFreeID();
	void RefreshAutoXTargets(Client *c);
	void RefreshClientXTargets(Client *c);

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

	LinkedList<Client*> client_list;
	LinkedList<Mob*> mob_list;
	LinkedList<NPC*> npc_list;
	LinkedList<Merc *> merc_list;
	std::list<Group*> group_list;
	LinkedList<Corpse*> corpse_list;
	LinkedList<Object*> object_list;
	LinkedList<Doors*> door_list;
	LinkedList<Trap*> trap_list;
	LinkedList<Beacon*> beacon_list;
	LinkedList<NPC *> proximity_list;
	std::list<Raid *> raid_list;
	uint16 last_insert_id;

	// Please Do Not Declare Any EntityList Class Members After This Comment
#ifdef BOTS
	public:
		void AddBot(Bot* newBot, bool SendSpawnPacket = true, bool dontqueue = false);
		void BotPickLock(Bot* rogue);
		bool RemoveBot(uint16 entityID);
		Mob* GetMobByBotID(uint32 botID);
		Bot* GetBotByBotID(uint32 botID);
		Bot* GetBotByBotName(std::string botName);
		std::list<Bot*> GetBotsByBotOwnerCharacterID(uint32 botOwnerCharacterID);

		bool Bot_AICheckCloseBeneficialSpells(Bot* caster, uint8 iChance, float iRange, uint16 iSpellTypes); // TODO: Evaluate this closesly in hopes to eliminate
		void ShowSpawnWindow(Client* client, int Distance, bool NamedOnly); // TODO: Implement ShowSpawnWindow in the bot class but it needs entity list stuff
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

