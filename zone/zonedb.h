#ifndef ZONEDB_H_
#define ZONEDB_H_

#include "../common/shareddb.h"
#include "../common/eq_packet_structs.h"
#include "../common/loottable.h"
#include "zonedump.h"
#include "../common/faction.h"
//#include "doors.h"

struct wplist {
	int index;
	float x;
	float y;
	float z;
	int pause;
	float heading;
};

#pragma pack(1)
struct DBnpcspells_entries_Struct {
	int16	spellid;
	uint16	type;
	uint8	minlevel;
	uint8	maxlevel;
	int16	manacost;
	int32	recast_delay;
	int16	priority;
	int16	resist_adjust;
};
#pragma pack()

struct DBnpcspells_Struct {
	uint32	parent_list;
	int16	attack_proc;
	uint8	proc_chance;
	uint32	numentries;
	DBnpcspells_entries_Struct entries[0];
};

struct DBTradeskillRecipe_Struct {
	SkillType tradeskill;
	int16 skill_needed;
	uint16 trivial;
	bool nofail;
	bool replace_container;
	vector< pair<uint32,uint8> > onsuccess;
	vector< pair<uint32,uint8> > onfail;
	vector< pair<uint32,uint8> > salvage;
	string name;
	uint8 must_learn;
	bool has_learnt;
	uint32 madecount;
	uint32 recipe_id;
	bool quest;
};

struct PetRecord {
	uint32 npc_type;	// npc_type id for the pet data to use
	bool temporary;
	int16 petpower;
	uint8 petcontrol;	// What kind of control over the pet is possible (Animation, familiar, ...)
	uint8 petnaming;		// How to name the pet (Warder, pet, random name, familiar, ...)
	bool monsterflag;	// flag for if a random monster appearance should get picked
	uint32 equipmentset;	// default equipment for the pet
};

// Actual pet info for a client.
struct PetInfo {
	uint16	SpellID;
	int16	petpower;
	uint32	HP;
	uint32	Mana;
	SpellBuff_Struct	Buffs[BUFF_COUNT];
	uint32	Items[MAX_WORN_INVENTORY];
	char	Name[64];
};

struct ZoneSpellsBlocked {
	uint32 spellid;
	int8 type;
	float x;
	float y;
	float z;
	float xdiff;
	float ydiff;
	float zdiff;
	char message[256];
};

struct TraderCharges_Struct {
	uint32 ItemID[80];
	int32 SerialNumber[80];
	uint32 ItemCost[80];
	int32 Charges[80];
};

const int MaxMercStanceID = 9;

struct MercStanceInfo {
	uint8	ProficiencyID;
	uint8	ClassID;
	uint32	StanceID;
	uint8	IsDefault;
};

struct MercTemplate {
	uint32	MercTemplateID;
	uint32	MercType;				// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
	uint32	MercSubType;			// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
	uint16	RaceID;
	uint8	ClassID;
	uint32	MercNPCID;
	uint8	ProficiencyID;
	uint8	TierID;
	uint8	CostFormula;			// To determine cost to client
	uint32	ClientVersion;				// Only send valid mercs per expansion
	uint8	MercNameType;			// Determines if merc gets random name or default text
	char	MercNamePrefix[25];
	char	MercNameSuffix[25];
	uint32	Stances[MaxMercStanceID];
};

struct MercInfo {
	uint32	mercid;
	uint8	slot;
	char	merc_name[64];
	uint32	MercTemplateID;
	const	MercTemplate* myTemplate;
	uint32	SuspendedTime;
	bool	IsSuspended;
	uint32	MercTimerRemaining;
	uint8	Gender;
	int32	State;
	uint32	Stance;
	int32	hp;
	int32	mana;
	int32	endurance;
	uint8	face;
	uint8	luclinHairStyle;
	uint8	luclinHairColor;
	uint8	luclinEyeColor;
	uint8	luclinEyeColor2;
	uint8	luclinBeardColor;
	uint8	luclinBeard;
	uint32	drakkinHeritage;
	uint32	drakkinTattoo;
	uint32	drakkinDetails;
};

struct MercSpellEntry {
	uint8	proficiencyid;
	uint16	spellid;		// <= 0 = no spell
	uint32	type;			// 0 = never, must be one (and only one) of the defined values
	int16	stance;			// 0 = all, + = only this stance, - = all except this stance
	uint8	minlevel;
	uint8	maxlevel;
	int16	slot;
	uint16	proc_chance;
	uint32	time_cancast;	// when we can cast this spell next
};

struct ClientMercEntry {
	uint32 id;
	uint32 npcid;
};

class ItemInst;
struct FactionMods;
struct FactionValue;
struct LootTable_Struct;


class ZoneDatabase : public SharedDatabase {
	typedef list<ServerLootItem_Struct*> ItemList;
public:
	ZoneDatabase();
	ZoneDatabase(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	virtual ~ZoneDatabase();

	/*
	* Objects and World Containers
	*/
	void	LoadWorldContainer(uint32 parentid, ItemInst* container);
	void	SaveWorldContainer(uint32 zone_id, uint32 parent_id, const ItemInst* container);
	void	DeleteWorldContainer(uint32 parent_id,uint32 zone_id);
	uint32	AddObject(uint32 type, uint32 icon, const Object_Struct& object, const ItemInst* inst);
	void	UpdateObject(uint32 id, uint32 type, uint32 icon, const Object_Struct& object, const ItemInst* inst);
	void	DeleteObject(uint32 id);
	Ground_Spawns*	LoadGroundSpawns(uint32 zone_id, int16 version, Ground_Spawns* gs);

	/*
	* Traders
	*/

	void	SaveTraderItem(uint32 char_id,uint32 itemid,uint32 uniqueid, int32 charges,uint32 itemcost,uint8 slot);
	void	UpdateTraderItemCharges(int char_id, uint32 ItemInstID, int32 charges);
	void	UpdateTraderItemPrice(int CharID, uint32 ItemID, uint32 Charges, uint32 NewPrice);
	ItemInst* LoadSingleTraderItem(uint32 char_id, int uniqueid);
	void	DeleteTraderItem(uint32 char_id);
	void	DeleteTraderItem(uint32 char_id,uint16 slot_id);
	Trader_Struct* LoadTraderItem(uint32 char_id);
	TraderCharges_Struct* LoadTraderItemWithCharges(uint32 char_id);

	// Buyer/Barter
	//
	void AddBuyLine(uint32 CharID, uint32 BuySlot, uint32 ItemID, const char *ItemName, uint32 Quantity, uint32 Price);
	void RemoveBuyLine(uint32 CharID, uint32 BuySlot);
	void DeleteBuyLines(uint32 CharID);
	void UpdateBuyLine(uint32 CharID, uint32 BuySlot, uint32 Quantity);

	/*
	* General Character Related Stuff
	*/
	bool	SetServerFilters(char* name, ServerSideFilters_Struct *ssfs);
	uint32	GetServerFilters(char* name, ServerSideFilters_Struct *ssfs);
	bool	GetAccountInfoForLogin(uint32 account_id, int16* admin = 0, char* account_name = 0,
				uint32* lsaccountid = 0, uint8* gmspeed = 0, bool* revoked = 0, bool* gmhideme = 0);
	bool	GetAccountInfoForLogin_result(MYSQL_RES* result, int16* admin = 0, char* account_name = 0,
				uint32* lsaccountid = 0, uint8* gmspeed = 0, bool* revoked = 0, bool* gmhideme = false,
				uint32* account_creation = 0);
	bool	GetCharacterInfoForLogin_result(MYSQL_RES* result, uint32* character_id = 0, char* current_zone = 0,
				PlayerProfile_Struct* pp = 0, Inventory* inv = 0, ExtendedProfile_Struct *ext = 0, uint32* pplen = 0,
				uint32* guilddbid = 0, uint8* guildrank = 0, uint8 *class_= 0, uint8 *level = 0, bool *LFP = 0,
				bool *LFG = 0, uint8 *NumXTargets = 0, uint8* firstlogon = 0);
	bool	GetCharacterInfoForLogin(const char* name, uint32* character_id = 0, char* current_zone = 0,
				PlayerProfile_Struct* pp = 0, Inventory* inv = 0, ExtendedProfile_Struct *ext = 0, uint32* pplen = 0,
				uint32* guilddbid = 0, uint8* guildrank = 0, uint8 *class_ = 0, uint8 *level = 0, bool *LFP = 0,
				bool *LFG = 0, uint8 *NumXTargets = 0, uint8* firstlogon = 0);
	void SaveBuffs(Client *c);
	void LoadBuffs(Client *c);
	void LoadPetInfo(Client *c);
	void SavePetInfo(Client *c);
	void RemoveTempFactions(Client *c);

	/*
	* Character Inventory
	*/
	bool	NoRentExpired(const char* name);

	/*
	* Corpses
	*/
	bool	GetDecayTimes(npcDecayTimes_Struct* npcCorpseDecayTimes);
	uint32	CreatePlayerCorpse(uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, uchar* data, uint32 datasize, float x, float y, float z, float heading);
	bool	CreatePlayerCorpseBackup(uint32 dbid, uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, uchar* data, uint32 datasize, float x, float y, float z, float heading);
	uint32	UpdatePlayerCorpse(uint32 dbid, uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, uchar* data, uint32 datasize, float x, float y, float z, float heading, bool rezzed = false);
	void	MarkCorpseAsRezzed(uint32 dbid);
	bool	BuryPlayerCorpse(uint32 dbid);
	bool	BuryAllPlayerCorpses(uint32 charid);
	bool	DeletePlayerCorpse(uint32 dbid);
	uint32	GetPlayerBurriedCorpseCount(uint32 char_id);
	Corpse* SummonBurriedPlayerCorpse(uint32 char_id, uint32 dest_zoneid, uint16 dest_instanceid, float dest_x, float dest_y, float dest_z, float dest_heading);
	bool	SummonAllPlayerCorpses(uint32 char_id, uint32 dest_zoneid, uint16 dest_instanceid, float dest_x, float dest_y, float dest_z, float dest_heading);
	bool	SummonAllGraveyardCorpses(uint32 cur_zoneid, uint32 dest_zoneid, uint16 dest_instanceid, float dest_x, float dest_y, float dest_z, float dest_heading);
	Corpse*	LoadPlayerCorpse(uint32 player_corpse_id);
	bool	UnburyPlayerCorpse(uint32 dbid, uint32 new_zoneid, uint16 dest_instanceid, float new_x, float new_y, float new_z, float new_heading);
	bool	LoadPlayerCorpses(uint32 iZoneID, uint16 iInstanceID);
	uint32	GraveyardPlayerCorpse(uint32 dbid, uint32 zoneid, uint16 instanceid, float x, float y, float z, float heading);
	uint32	NewGraveyardRecord(uint32 graveyard_zoneid, float graveyard_x, float graveyard_y, float graveyard_z, float graveyard_heading);
	uint32	AddGraveyardIDToZone(uint32 zone_id, uint32 graveyard_id);
	bool	DeleteGraveyard(uint32 zone_id, uint32 graveyard_id);
	uint32	GetFirstCorpseID(uint32 char_id);
	uint32	GetPlayerCorpseCount(uint32 char_id);
	uint32	GetPlayerCorpseID(uint32 char_id, uint8 corpse);
	uint32	GetPlayerCorpseItemAt(uint32 corpse_id, uint16 slotid);
	uint32	GetPlayerCorpseTimeLeft(uint8 corpse, uint8 type);

	/*
	* Faction
	*/
	bool	GetNPCFactionList(uint32 npcfaction_id, int32* faction_id, int32* value, uint8* temp, int32* primary_faction = 0);
	bool	GetFactionData(FactionMods* fd, uint32 class_mod, uint32 race_mod, uint32 deity_mod, int32 faction_id); //rembrant, needed for factions Dec, 16 2001
	bool	GetFactionName(int32 faction_id, char* name, uint32 buflen); // rembrant, needed for factions Dec, 16 2001
	bool	GetFactionIdsForNPC(uint32 nfl_id, list<struct NPCFaction*> *faction_list, int32* primary_faction = 0); // neotokyo: improve faction handling
	bool	SetCharacterFactionLevel(uint32 char_id, int32 faction_id, int32 value, uint8 temp, faction_map &val_list); // rembrant, needed for factions Dec, 16 2001
	bool	LoadFactionData();
	bool	LoadFactionValues(uint32 char_id, faction_map & val_list);
	bool	LoadFactionValues_result(MYSQL_RES* result, faction_map & val_list);

	/*
	* AAs
	*/
	bool	LoadAAEffects();
	bool	LoadAAEffects2();
	bool	LoadSwarmSpells();
	SendAA_Struct*	GetAASkillVars(uint32 skill_id);
	uint8	GetTotalAALevels(uint32 skill_id);
	uint32	GetSizeAA();
	uint32	CountAAs();
	void	LoadAAs(SendAA_Struct **load);
	uint32 CountAAEffects();
	void FillAAEffects(SendAA_Struct* aa_struct);

	/*
	* Zone related
	*/
	bool	GetZoneCFG(uint32 zoneid, uint16 instance_id, NewZone_Struct *data, bool &can_bind, bool &can_combat, bool &can_levitate, bool &can_castoutdoor, bool &is_city, bool &is_hotzone, bool &allow_mercs, int &ruleset, char **map_filename);
	bool	SaveZoneCFG(uint32 zoneid, uint16 instance_id, NewZone_Struct* zd);
	bool	DumpZoneState();
	int8	LoadZoneState(const char* zonename, LinkedList<Spawn2*>& spawn2_list);
	bool	LoadStaticZonePoints(LinkedList<ZonePoint*>* zone_point_list,const char* zonename, uint32 version);
	bool	UpdateZoneSafeCoords(const char* zonename, float x, float y, float z);
	uint8	GetUseCFGSafeCoords();
	int		getZoneShutDownDelay(uint32 zoneID, uint32 version);

	/*
	* Spawns and Spawn Points
	*/
	bool	LoadSpawnGroups(const char* zone_name, uint16 version, SpawnGroupList* spawn_group_list);
	bool	LoadSpawnGroupsByID(int spawngroupid, SpawnGroupList* spawn_group_list);
	bool	PopulateZoneSpawnList(uint32 zoneid, LinkedList<Spawn2*> &spawn2_list, int16 version, uint32 repopdelay = 0);
	Spawn2*	LoadSpawn2(LinkedList<Spawn2*> &spawn2_list, uint32 spawn2id, uint32 timeleft);
	bool	CreateSpawn2(Client *c, uint32 spawngroup, const char* zone, float heading, float x, float y, float z, uint32 respawn, uint32 variance, uint16 condition, int16 cond_value);
	void	UpdateSpawn2Timeleft(uint32 id, uint16 instance_id,uint32 timeleft);
	uint32	GetSpawnTimeLeft(uint32 id, uint16 instance_id);
	void	UpdateSpawn2Status(uint32 id, uint8 new_status);

	/*
	* Grids/Paths
	*/
	uint32	GetFreeGrid(uint16 zoneid);
	void	DeleteGrid(Client *c, uint32 sg2, uint32 grid_num, bool grid_too,uint16 zoneid);
	void	DeleteWaypoint(Client *c, uint32 grid_num, uint32 wp_num,uint16 zoneid);
//	uint32	AddWP(Client *c, uint32 sg2, uint16 grid_num, uint8 wp_num, float xpos, float ypos, float zpos, uint32 pause, float xpos1, float ypos1, float zpos1, int type1, int type2,uint16 zoneid);
	void	AddWP(Client *c, uint32 gridid, uint32 wpnum, float xpos, float ypos, float zpos, uint32 pause, uint16 zoneid, float heading);
	uint32	AddWPForSpawn(Client *c, uint32 spawn2id, float xpos, float ypos, float zpos, uint32 pause, int type1, int type2, uint16 zoneid, float heading);
	void	ModifyGrid(Client *c, bool remove, uint32 id, uint8 type = 0, uint8 type2 = 0,uint16 zoneid = 0);
	void	ModifyWP(Client *c, uint32 grid_id, uint32 wp_num, float xpos, float ypos, float zpos, uint32 script=0,uint16 zoneid =0);
	uint8	GetGridType(uint32 grid,uint32 zoneid);
	uint8	GetGridType2(uint32 grid, uint16 zoneid);
	bool	GetWaypoints(uint32 grid, uint16 zoneid, uint32 num, wplist* wp);
	void	AssignGrid(Client *client, float x, float y, uint32 id);
	int		GetHighestGrid(uint32 zoneid);
	int		GetHighestWaypoint(uint32 zoneid, uint32 gridid);

	/*
	* NPCs
	*/
	const NPCType*			GetNPCType(uint32 id);
	uint32	NPCSpawnDB(uint8 command, const char* zone, uint32 zone_version, Client *c, NPC* spawn = 0, uint32 extra = 0); // 0 = Create 1 = Add; 2 = Update; 3 = Remove; 4 = Delete
	bool	SetSpecialAttkFlag(uint8 id, const char* flag);
	bool	GetPetEntry(const char *pet_type, PetRecord *into);
	bool	GetPoweredPetEntry(const char *pet_type, int16 petpower, PetRecord *into);
	bool	GetBasePetItems(int32 equipmentset, uint32 *items);
	void	AddLootTableToNPC(NPC* npc,uint32 loottable_id, ItemList* itemlist, uint32* copper, uint32* silver, uint32* gold, uint32* plat);
	void	AddLootDropToNPC(NPC* npc,uint32 lootdrop_id, ItemList* itemlist, uint8 droplimit, uint8 mindrop);
	uint32	GetMaxNPCSpellsID();
	DBnpcspells_Struct* GetNPCSpells(uint32 iDBSpellsID);

	/*
	* Mercs
	*/
	const	NPCType*	GetMercType(uint32 id, uint16 raceid, uint32 clientlevel);
	void	LoadMercEquipment(Merc *merc);
	void	SaveMercBuffs(Merc *merc);
	void	LoadMercBuffs(Merc *merc);
	bool	LoadMercInfo(Client *c);
	bool	LoadCurrentMerc(Client *c);
	bool	SaveMerc(Merc *merc);
	bool	DeleteMerc(uint32 merc_id);
	//void	LoadMercTypesForMercMerchant(NPC *merchant);
	//void	LoadMercsForMercMerchant(NPC *merchant);

	/*
	* Petitions
	*/
	void	UpdateBug(BugStruct* bug);
	void	UpdateBug(PetitionBug_Struct* bug);
	void	DeletePetitionFromDB(Petition* wpet);
	void	UpdatePetitionToDB(Petition* wpet);
	void	InsertPetitionToDB(Petition* wpet);
	void	RefreshPetitionsFromDB();


	/*
	* Merchants
	*/
	void	SaveMerchantTemp(uint32 npcid, uint32 slot, uint32 item, uint32 charges);
	void	DeleteMerchantTemp(uint32 npcid, uint32 slot);

	/*
	* Tradeskills
	*/
	bool	GetTradeRecipe(const ItemInst* container, uint8 c_type, uint32 some_id, uint32 char_id, DBTradeskillRecipe_Struct *spec);
	bool	GetTradeRecipe(uint32 recipe_id, uint8 c_type, uint32 some_id, uint32 char_id, DBTradeskillRecipe_Struct *spec);
	uint32	GetZoneForage(uint32 ZoneID, uint8 skill); /* for foraging - BoB */
	uint32	GetZoneFishing(uint32 ZoneID, uint8 skill, uint32 &npc_id, uint8 &npc_chance);
	void	UpdateRecipeMadecount(uint32 recipe_id, uint32 char_id, uint32 madecount);

	/*
	* Tribute
	*/
	bool	LoadTributes();

	/*
	* Doors
	*/
	bool	DoorIsOpen(uint8 door_id,const char* zone_name);
	void	SetDoorPlace(uint8 value,uint8 door_id,const char* zone_name);
	bool	LoadDoors(int32 iDoorCount, Door *into, const char *zone_name, int16 version);
	bool	CheckGuildDoor(uint8 doorid,uint16 guild_id, const char* zone);
	bool	SetGuildDoor(uint8 doorid,uint16 guild_id, const char* zone);
	uint32	GetGuildEQID(uint32 guilddbid);
	void	UpdateDoorGuildID(int doorid, int guild_id);
	int32	GetDoorsCount(uint32* oMaxID, const char *zone_name, int16 version);
	int32	GetDoorsCountPlusOne(const char *zone_name, int16 version);
	int32	GetDoorsDBCountPlusOne(const char *zone_name, int16 version);
	void InsertDoor(uint32 did, uint16 ddoorid, const char* ddoor_name, float dxpos, float dypos, float dzpos, float dheading, uint8 dopentype, uint16 dguildid, uint32 dlockpick, uint32 dkeyitem, uint8 ddoor_param, uint8 dinvert, int dincline, uint16 dsize);

	/*
	* Blocked Spells
	*/

	int32	GetBlockedSpellsCount(uint32 zoneid);
	bool	LoadBlockedSpells(int32 blockedSpellsCount, ZoneSpellsBlocked* into, uint32 zoneid);

	/*
	* Traps
	*/
	bool	LoadTraps(const char* zonename, int16 version);
	char*	GetTrapMessage(uint32 trap_id);

	/*
	* Time
	*/
	uint32	GetZoneTZ(uint32 zoneid, uint32 version);
	bool	SetZoneTZ(uint32 zoneid, uint32 version, uint32 tz);

	/*
	* Weather
	*/
	uint8	GetZoneWeather(uint32 zoneid, uint32 version);
	bool	SetZoneWeather(uint32 zoneid, uint32 version, uint8 w);
	/*
	* Group
	*/
	void RefreshGroupFromDB(Client *c);
	uint8 GroupCount(uint32 groupid);
	/*
	* Raid
	*/
	uint8 RaidGroupCount(uint32 raidid, uint32 groupid);

	/*
	* Instancing
	*/
	void ListAllInstances(Client* c, uint32 charid);

	/*
	* QGlobals
	*/
	void QGlobalPurge();

	/*
	* Alternate Currency
	*/
	void LoadAltCurrencyValues(uint32 char_id, std::map<uint32, uint32> &currency);
	void UpdateAltCurrencyValue(uint32 char_id, uint32 currency_id, uint32 value);

	/*
	* Misc stuff.
	* PLEASE DO NOT ADD TO THIS COLLECTION OF CRAP UNLESS YOUR METHOD
	* REALLY HAS NO BETTER SECTION
	*/
	bool	logevents(const char* accountname,uint32 accountid,uint8 status,const char* charname,const char* target, const char* descriptiontype, const char* description,int event_nid);
	void	GetEventLogs(const char* name,char* target,uint32 account_id=0,uint8 eventid=0,char* detail=0,char* timestamp=0, CharacterEventLog_Struct* cel=0);
	uint32	GetKarma(uint32 acct_id);
	void	UpdateKarma(uint32 acct_id, uint32 amount);

	/*
	* Things which really dont belong here...
	*/
	int16	CommandRequirement(const char* commandname);

protected:
	void ZDBInitVars();

	uint32				max_faction;
	Faction**			faction_array;
	uint32 npc_spells_maxid;
	DBnpcspells_Struct** npc_spells_cache;
	bool*				npc_spells_loadtried;
	uint8 door_isopen_array[255];
};

extern ZoneDatabase database;

#endif /*ZONEDB_H_*/

