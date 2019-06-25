#ifndef ZONEDB_H_
#define ZONEDB_H_

#include <unordered_set>

#include "../common/shareddb.h"
#include "../common/eq_packet_structs.h"
#include "position.h"
#include "../common/faction.h"
#include "../common/eqemu_logsys.h"
#include "aa_ability.h"
#include "event_codes.h"

#ifdef BOTS
#include "bot_database.h"
#endif

class Client;
class Corpse;
class Merc;
class NPC;
class Petition;
class Spawn2;
class SpawnGroupList;
class Trap;
struct CharacterEventLog_Struct;
struct Door;
struct ExtendedProfile_Struct;
struct NPCType;
struct PlayerCorpse_Struct;
struct ZonePoint;
struct npcDecayTimes_Struct;

namespace EQEmu
{
	class ItemInstance;
}

template <class TYPE> class LinkedList;

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
	uint8	minlevel;
	uint8	maxlevel;
	uint32	type;
	int16	manacost;
	int16	priority;
	int32	recast_delay;
	int16	resist_adjust;
	int8	min_hp;
	int8	max_hp;
};
#pragma pack()

#pragma pack(1)
struct DBnpcspellseffects_entries_Struct {
	int16	spelleffectid;
	uint8	minlevel;
	uint8	maxlevel;
	int32	base;
	int32	limit;
	int32	max;
};
#pragma pack()

struct DBnpcspells_Struct {
	uint32	parent_list;
	uint16	attack_proc;
	uint8	proc_chance;
	uint16	range_proc;
	int16	rproc_chance;
	uint16	defensive_proc;
	int16	dproc_chance;
	uint32	fail_recast;
	uint32	engaged_no_sp_recast_min;
	uint32	engaged_no_sp_recast_max;
	uint8	engaged_beneficial_self_chance;
	uint8	engaged_beneficial_other_chance;
	uint8	engaged_detrimental_chance;
	uint32  pursue_no_sp_recast_min;
	uint32  pursue_no_sp_recast_max;
	uint8   pursue_detrimental_chance;
	uint32  idle_no_sp_recast_min;
	uint32  idle_no_sp_recast_max;
	uint8	idle_beneficial_chance;
	std::vector<DBnpcspells_entries_Struct> entries;
};

struct DBnpcspellseffects_Struct {
	uint32	parent_list;
	uint32	numentries;
	DBnpcspellseffects_entries_Struct entries[0];
};

struct DBTradeskillRecipe_Struct {
	EQEmu::skills::SkillType tradeskill;
	int16 skill_needed;
	uint16 trivial;
	bool nofail;
	bool replace_container;
	std::vector< std::pair<uint32,uint8> > onsuccess;
	std::vector< std::pair<uint32,uint8> > onfail;
	std::vector< std::pair<uint32,uint8> > salvage;
	std::string name;
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

struct AuraRecord {
	uint32 npc_type;
	char name[64]; // name shown in UI if shown and spawn name
	int spell_id;
	int distance;
	int aura_type;
	int spawn_type;
	int movement;
	int duration; // seconds some live for 90 mins (normal) others for 2 mins (traps)
	int icon; // -1 will use the buffs NEW_ICON
	int cast_time; // seconds some auras recast on a timer, most seem to be every 12 seconds
};

// Actual pet info for a client.
struct PetInfo {
	uint16	SpellID;
	int16	petpower;
	uint32	HP;
	uint32	Mana;
	float	size;
	SpellBuff_Struct	Buffs[PET_BUFF_COUNT];
	uint32	Items[EQEmu::invslot::EQUIPMENT_COUNT];
	char	Name[64];
};

struct ZoneSpellsBlocked {
	uint32 spellid;
	int8 type;
	glm::vec3 m_Location;
	glm::vec3 m_Difference;
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
	float	MercSize;
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

class ZoneDatabase : public SharedDatabase {
	typedef std::list<ServerLootItem_Struct*> ItemList;
public:
	ZoneDatabase();
	ZoneDatabase(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	virtual ~ZoneDatabase();

	/* Objects and World Containers  */
	void	LoadWorldContainer(uint32 parentid, EQEmu::ItemInstance* container);
	void	SaveWorldContainer(uint32 zone_id, uint32 parent_id, const EQEmu::ItemInstance* container);
	void	DeleteWorldContainer(uint32 parent_id,uint32 zone_id);
	uint32	AddObject(uint32 type, uint32 icon, const Object_Struct& object, const EQEmu::ItemInstance* inst);
	void	UpdateObject(uint32 id, uint32 type, uint32 icon, const Object_Struct& object, const EQEmu::ItemInstance* inst);
	void	DeleteObject(uint32 id);
	Ground_Spawns*	LoadGroundSpawns(uint32 zone_id, int16 version, Ground_Spawns* gs);

	/* Traders  */
	void	SaveTraderItem(uint32 char_id,uint32 itemid,uint32 uniqueid, int32 charges,uint32 itemcost,uint8 slot);
	void	UpdateTraderItemCharges(int char_id, uint32 ItemInstID, int32 charges);
	void	UpdateTraderItemPrice(int CharID, uint32 ItemID, uint32 Charges, uint32 NewPrice);
	void	DeleteTraderItem(uint32 char_id);
	void	DeleteTraderItem(uint32 char_id,uint16 slot_id);

	EQEmu::ItemInstance* LoadSingleTraderItem(uint32 char_id, int uniqueid);
	Trader_Struct* LoadTraderItem(uint32 char_id);
	TraderCharges_Struct* LoadTraderItemWithCharges(uint32 char_id);

	/* Buyer/Barter  */
	void AddBuyLine(uint32 CharID, uint32 BuySlot, uint32 ItemID, const char *ItemName, uint32 Quantity, uint32 Price);
	void RemoveBuyLine(uint32 CharID, uint32 BuySlot);
	void DeleteBuyLines(uint32 CharID);
	void UpdateBuyLine(uint32 CharID, uint32 BuySlot, uint32 Quantity);

	/**
	 ************************************************
	 * Character
	 ************************************************
	 */

	bool	SetServerFilters(char* name, ServerSideFilters_Struct *ssfs);
	uint32	GetServerFilters(char* name, ServerSideFilters_Struct *ssfs);

	void SaveBuffs(Client *c);
	void LoadBuffs(Client *c);
	void SaveAuras(Client *c);
	void LoadAuras(Client *c);
	void LoadPetInfo(Client *c);
	void SavePetInfo(Client *c);
	void RemoveTempFactions(Client *c);
	void UpdateItemRecastTimestamps(uint32 char_id, uint32 recast_type, uint32 timestamp);

	bool DeleteCharacterAAs(uint32 character_id);
	bool DeleteCharacterBandolier(uint32 character_id, uint32 band_id);
	bool DeleteCharacterDisc(uint32 character_id, uint32 slot_id);
	bool DeleteCharacterDye(uint32 character_id);
	bool DeleteCharacterLeadershipAAs(uint32 character_id);
	bool DeleteCharacterMemorizedSpell(uint32 character_id, uint32 spell_id, uint32 slot_id);
	bool DeleteCharacterSpell(uint32 character_id, uint32 spell_id, uint32 slot_id);

	bool LoadCharacterBandolier(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterBindPoint(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterData(uint32 character_id, PlayerProfile_Struct* pp, ExtendedProfile_Struct* m_epp);
	bool LoadCharacterDisciplines(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterFactionValues(uint32 character_id, faction_map & val_list);
	bool LoadCharacterLanguages(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterLeadershipAA(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterMaterialColor(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterMemmedSpells(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterPotions(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterSkills(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterSpellBook(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterTribute(uint32 character_id, PlayerProfile_Struct* pp);

	bool SaveCharacterAA(uint32 character_id, uint32 aa_id, uint32 current_level, uint32 charges);
	bool SaveCharacterBandolier(uint32 character_id, uint8 bandolier_id, uint8 bandolier_slot, uint32 item_id, uint32 icon, const char* bandolier_name);
	bool SaveCharacterBindPoint(uint32 character_id, const BindStruct &bind, uint32 bind_num);
	bool SaveCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp);
	bool SaveCharacterData(uint32 character_id, uint32 account_id, PlayerProfile_Struct* pp, ExtendedProfile_Struct* m_epp);
	bool SaveCharacterDisc(uint32 character_id, uint32 slot_id, uint32 disc_id);
	bool SaveCharacterLanguage(uint32 character_id, uint32 lang_id, uint32 value);
	bool SaveCharacterLeadershipAA(uint32 character_id, PlayerProfile_Struct* pp);
	bool SaveCharacterMaterialColor(uint32 character_id, uint32 slot_id, uint32 color);
	bool SaveCharacterMemorizedSpell(uint32 character_id, uint32 spell_id, uint32 slot_id);
	bool SaveCharacterPotionBelt(uint32 character_id, uint8 potion_id, uint32 item_id, uint32 icon);
	bool SaveCharacterSkill(uint32 character_id, uint32 skill_id, uint32 value);
	bool SaveCharacterSpell(uint32 character_id, uint32 spell_id, uint32 slot_id);
	bool SaveCharacterTribute(uint32 character_id, PlayerProfile_Struct* pp);

	/* Character Inventory  */
	bool	NoRentExpired(const char* name);
	bool	SaveCharacterInvSnapshot(uint32 character_id);
	int		CountCharacterInvSnapshots(uint32 character_id);
	void	ClearCharacterInvSnapshots(uint32 character_id, bool from_now = false);
	void	ListCharacterInvSnapshots(uint32 character_id, std::list<std::pair<uint32, int>> &is_list);
	bool	ValidateCharacterInvSnapshotTimestamp(uint32 character_id, uint32 timestamp);
	void	ParseCharacterInvSnapshot(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &parse_list);
	void	DivergeCharacterInvSnapshotFromInventory(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list);
	void	DivergeCharacterInventoryFromInvSnapshot(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list);
	bool	RestoreCharacterInvSnapshot(uint32 character_id, uint32 timestamp);
	
	/* Corpses  */
	bool		DeleteItemOffCharacterCorpse(uint32 db_id, uint32 equip_slot, uint32 item_id);
	uint32		GetCharacterCorpseItemCount(uint32 corpse_id);
	bool		LoadCharacterCorpseData(uint32 corpse_id, PlayerCorpse_Struct* pcs);
	Corpse*		LoadCharacterCorpse(uint32 player_corpse_id);
	Corpse*		SummonBuriedCharacterCorpses(uint32 char_id, uint32 dest_zoneid, uint16 dest_instanceid, const glm::vec4& position);
	void		MarkCorpseAsRezzed(uint32 dbid);
	bool		GetDecayTimes(npcDecayTimes_Struct* npcCorpseDecayTimes);
	bool		BuryCharacterCorpse(uint32 dbid);
	bool		BuryAllCharacterCorpses(uint32 charid);
	bool		DeleteCharacterCorpse(uint32 dbid);
	bool		SummonAllCharacterCorpses(uint32 char_id, uint32 dest_zoneid, uint16 dest_instanceid, const glm::vec4& position);
	bool		SummonAllGraveyardCorpses(uint32 cur_zoneid, uint32 dest_zoneid, uint16 dest_instanceid, const glm::vec4& position);
	bool		UnburyCharacterCorpse(uint32 dbid, uint32 new_zoneid, uint16 dest_instanceid, const glm::vec4& position);
	bool		LoadCharacterCorpses(uint32 iZoneID, uint16 iInstanceID);
	bool		DeleteGraveyard(uint32 zone_id, uint32 graveyard_id);
	uint32		GetCharacterCorpseDecayTimer(uint32 corpse_db_id);
	uint32		GetCharacterBuriedCorpseCount(uint32 char_id);
	uint32		SendCharacterCorpseToGraveyard(uint32 dbid, uint32 zoneid, uint16 instanceid, const glm::vec4& position);
	uint32		CreateGraveyardRecord(uint32 graveyard_zoneid, const glm::vec4& position);
	uint32		AddGraveyardIDToZone(uint32 zone_id, uint32 graveyard_id);
	uint32		SaveCharacterCorpse(uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, PlayerCorpse_Struct* dbpc, const glm::vec4& position);
	uint32		UpdateCharacterCorpse(uint32 dbid, uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, PlayerCorpse_Struct* dbpc, const glm::vec4& position, bool rezzed = false);
	uint32		GetFirstCorpseID(uint32 char_id);
	uint32		GetCharacterCorpseCount(uint32 char_id);
	uint32		GetCharacterCorpseID(uint32 char_id, uint8 corpse);
	uint32		GetCharacterCorpseItemAt(uint32 corpse_id, uint16 slotid);
	uint32		GetPlayerCorpseTimeLeft(uint8 corpse, uint8 type);

	/* Faction   */
	bool		GetNPCFactionList(uint32 npcfaction_id, int32* faction_id, int32* value, uint8* temp, int32* primary_faction = 0);
	bool		GetFactionData(FactionMods* fd, uint32 class_mod, uint32 race_mod, uint32 deity_mod, int32 faction_id); //needed for factions Dec, 16 2001
	bool		GetFactionName(int32 faction_id, char* name, uint32 buflen); // needed for factions Dec, 16 2001
	bool		GetFactionIdsForNPC(uint32 nfl_id, std::list<struct NPCFaction*> *faction_list, int32* primary_faction = 0); // improve faction handling
	bool		SetCharacterFactionLevel(uint32 char_id, int32 faction_id, int32 value, uint8 temp, faction_map &val_list); // needed for factions Dec, 16 2001
	bool		LoadFactionData();

	/* AAs New */
	bool	LoadAlternateAdvancementAbilities(std::unordered_map<int, std::unique_ptr<AA::Ability>> &abilities,
											  std::unordered_map<int, std::unique_ptr<AA::Rank>> &ranks);
	bool	LoadAlternateAdvancement(Client *c);

	/* Zone related   */
	bool		GetZoneCFG(
		uint32 zoneid, 
		uint16 instance_id, 
		NewZone_Struct *data, 
		bool &can_bind, 
		bool &can_combat, 
		bool &can_levitate, 
		bool &can_castoutdoor, 
		bool &is_city, 
		bool &is_hotzone, 
		bool &allow_mercs, 
		double &max_movement_update_range, 
		uint8 &zone_type, 
		int &ruleset, 
		char **map_filename);
	bool		SaveZoneCFG(uint32 zoneid, uint16 instance_id, NewZone_Struct* zd);
	bool		LoadStaticZonePoints(LinkedList<ZonePoint*>* zone_point_list,const char* zonename, uint32 version);
	bool		UpdateZoneSafeCoords(const char* zonename, const glm::vec3& location);
	uint8		GetUseCFGSafeCoords();
	int			getZoneShutDownDelay(uint32 zoneID, uint32 version);

	/* Spawns and Spawn Points  */
	bool		LoadSpawnGroups(const char* zone_name, uint16 version, SpawnGroupList* spawn_group_list);
	bool		LoadSpawnGroupsByID(int spawngroupid, SpawnGroupList* spawn_group_list);
	bool		PopulateZoneSpawnList(uint32 zoneid, LinkedList<Spawn2*> &spawn2_list, int16 version, uint32 repopdelay = 0);
	bool		PopulateZoneSpawnListClose(uint32 zoneid, LinkedList<Spawn2*> &spawn2_list, int16 version, const glm::vec4& client_position, uint32 repop_distance);
	Spawn2*		LoadSpawn2(LinkedList<Spawn2*> &spawn2_list, uint32 spawn2id, uint32 timeleft);
	bool		CreateSpawn2(Client *c, uint32 spawngroup, const char* zone, const glm::vec4& position, uint32 respawn, uint32 variance, uint16 condition, int16 cond_value);
	void		UpdateRespawnTime(uint32 id, uint16 instance_id,uint32 timeleft);
	uint32		GetSpawnTimeLeft(uint32 id, uint16 instance_id);
	void		UpdateSpawn2Status(uint32 id, uint8 new_status);

	/* Grids/Paths  */
	uint32		GetFreeGrid(uint16 zoneid);
	void		DeleteGrid(Client *c, uint32 sg2, uint32 grid_num, bool grid_too, uint16 zoneid);
	void		DeleteWaypoint(Client *c, uint32 grid_num, uint32 wp_num, uint16 zoneid);
	void		AddWP(Client *c, uint32 gridid, uint32 wpnum, const glm::vec4& position, uint32 pause, uint16 zoneid);
	uint32		AddWPForSpawn(Client *c, uint32 spawn2id, const glm::vec4& position, uint32 pause, int type1, int type2, uint16 zoneid);
	void		ModifyGrid(Client *c, bool remove, uint32 id, uint8 type = 0, uint8 type2 = 0, uint16 zoneid = 0);
	void		ModifyWP(Client *c, uint32 grid_id, uint32 wp_num, const glm::vec3& location, uint32 script = 0, uint16 zoneid = 0);
	uint8		GetGridType(uint32 grid, uint32 zoneid);
	uint8		GetGridType2(uint32 grid, uint16 zoneid);
	bool		GetWaypoints(uint32 grid, uint16 zoneid, uint32 num, wplist* wp);
	void        AssignGrid(Client *client, int grid, int spawn2id);
	int			GetHighestGrid(uint32 zoneid);
	int			GetHighestWaypoint(uint32 zoneid, uint32 gridid);

	/* NPCs  */

	uint32		NPCSpawnDB(uint8 command, const char* zone, uint32 zone_version, Client *c, NPC* spawn = 0, uint32 extra = 0); // 0 = Create 1 = Add; 2 = Update; 3 = Remove; 4 = Delete
	uint32		CreateNewNPCCommand(const char* zone, uint32 zone_version, Client *client, NPC* spawn, uint32 extra);
	uint32		AddNewNPCSpawnGroupCommand(const char* zone, uint32 zone_version, Client *client, NPC* spawn, uint32 respawnTime);
	uint32		DeleteSpawnLeaveInNPCTypeTable(const char* zone, Client *client, NPC* spawn);
	uint32		DeleteSpawnRemoveFromNPCTypeTable(const char* zone, uint32 zone_version, Client *client, NPC* spawn);
	uint32		AddSpawnFromSpawnGroup(const char* zone, uint32 zone_version, Client *client, NPC* spawn, uint32 spawnGroupID);
	uint32		AddNPCTypes(const char* zone, uint32 zone_version, Client *client, NPC* spawn, uint32 spawnGroupID);
	uint32		UpdateNPCTypeAppearance(Client *client, NPC* spawn);
	bool		SetSpecialAttkFlag(uint8 id, const char* flag);
	bool		GetPetEntry(const char *pet_type, PetRecord *into);
	bool		GetPoweredPetEntry(const char *pet_type, int16 petpower, PetRecord *into);
	bool		GetBasePetItems(int32 equipmentset, uint32 *items);
	void		AddLootTableToNPC(NPC* npc, uint32 loottable_id, ItemList* itemlist, uint32* copper, uint32* silver, uint32* gold, uint32* plat);
	void		AddLootDropToNPC(NPC* npc, uint32 lootdrop_id, ItemList* itemlist, uint8 droplimit, uint8 mindrop);
	uint32		GetMaxNPCSpellsID();
	uint32		GetMaxNPCSpellsEffectsID();
	bool GetAuraEntry(uint16 spell_id, AuraRecord &record);
	void LoadGlobalLoot();

	DBnpcspells_Struct*				GetNPCSpells(uint32 iDBSpellsID);
	DBnpcspellseffects_Struct*		GetNPCSpellsEffects(uint32 iDBSpellsEffectsID);
	void ClearNPCSpells() { npc_spells_cache.clear(); npc_spells_loadtried.clear(); }
	const NPCType* LoadNPCTypesData(uint32 id, bool bulk_load = false);

	/* Mercs   */
	const	NPCType*	GetMercType(uint32 id, uint16 raceid, uint32 clientlevel);
	void	LoadMercEquipment(Merc *merc);
	void	SaveMercBuffs(Merc *merc);
	void	LoadMercBuffs(Merc *merc);
	bool	LoadMercInfo(Client *c);
	bool	LoadCurrentMerc(Client *c);
	bool	SaveMerc(Merc *merc);
	bool	DeleteMerc(uint32 merc_id);

	/* Petitions   */
	void	RegisterBug(BugReport_Struct* bug_report); // old method
	void	RegisterBug(Client* client, BugReport_Struct* bug_report); // new method
	//void	UpdateBug(PetitionBug_Struct* bug);
	void	DeletePetitionFromDB(Petition* wpet);
	void	UpdatePetitionToDB(Petition* wpet);
	void	InsertPetitionToDB(Petition* wpet);
	void	RefreshPetitionsFromDB();

	/* Merchants  */
	void	SaveMerchantTemp(uint32 npcid, uint32 slot, uint32 item, uint32 charges);
	void	DeleteMerchantTemp(uint32 npcid, uint32 slot);

	/* Tradeskills  */
	bool	GetTradeRecipe(const EQEmu::ItemInstance* container, uint8 c_type, uint32 some_id, uint32 char_id, DBTradeskillRecipe_Struct *spec);
	bool	GetTradeRecipe(uint32 recipe_id, uint8 c_type, uint32 some_id, uint32 char_id, DBTradeskillRecipe_Struct *spec);
	uint32	GetZoneForage(uint32 ZoneID, uint8 skill); /* for foraging */
	uint32	GetZoneFishing(uint32 ZoneID, uint8 skill, uint32 &npc_id, uint8 &npc_chance);
	void	UpdateRecipeMadecount(uint32 recipe_id, uint32 char_id, uint32 madecount);
	bool	EnableRecipe(uint32 recipe_id);
	bool	DisableRecipe(uint32 recipe_id);

	/* Tribute  */
	bool	LoadTributes();

	/* Doors   */
	bool	DoorIsOpen(uint8 door_id,const char* zone_name);
	void	SetDoorPlace(uint8 value,uint8 door_id,const char* zone_name);
	bool	LoadDoors(int32 door_count, Door *into, const char *zone_name, int16 version);
	bool	CheckGuildDoor(uint8 doorid,uint16 guild_id, const char* zone);
	bool	SetGuildDoor(uint8 doorid,uint16 guild_id, const char* zone);
	uint32	GetGuildEQID(uint32 guilddbid);
	void	UpdateDoorGuildID(int doorid, int guild_id);
	int32	GetDoorsCount(uint32* oMaxID, const char *zone_name, int16 version);
	int32	GetDoorsCountPlusOne(const char *zone_name, int16 version);
	int32	GetDoorsDBCountPlusOne(const char *zone_name, int16 version);
	void	InsertDoor(uint32 did, uint16 ddoorid, const char* ddoor_name, const glm::vec4& position, uint8 dopentype, uint16 dguildid, uint32 dlockpick, uint32 dkeyitem, uint8 ddoor_param, uint8 dinvert, int dincline, uint16 dsize, bool ddisabletimer = false);

	/* Blocked Spells   */
	int32	GetBlockedSpellsCount(uint32 zoneid);
	bool	LoadBlockedSpells(int32 blockedSpellsCount, ZoneSpellsBlocked* into, uint32 zoneid);

	/* Traps   */
	bool	LoadTraps(const char* zonename, int16 version);
	bool	SetTrapData(Trap* trap, bool repopnow = false);

	/* Time   */
	uint32	GetZoneTZ(uint32 zoneid, uint32 version);
	bool	SetZoneTZ(uint32 zoneid, uint32 version, uint32 tz);

	/* Group   */
	void RefreshGroupFromDB(Client *c);
	uint8 GroupCount(uint32 groupid);

	/* Raid   */
	uint8 RaidGroupCount(uint32 raidid, uint32 groupid);

	/* Instancing   */
	void ListAllInstances(Client* c, uint32 charid);

	/* QGlobals   */
	void QGlobalPurge();

	/* Alternate Currency   */
	void LoadAltCurrencyValues(uint32 char_id, std::map<uint32, uint32> &currency);
	void UpdateAltCurrencyValue(uint32 char_id, uint32 currency_id, uint32 value);

	/* Saylinks   */
	uint32 LoadSaylinkID(const char* saylink_text, bool auto_insert = true);
	uint32 SaveSaylinkID(const char* saylink_text);

	/*
		* Misc stuff.
		* PLEASE DO NOT ADD TO THIS COLLECTION OF CRAP UNLESS YOUR METHOD
		* REALLY HAS NO BETTER SECTION
	*/
	bool	logevents(const char* accountname,uint32 accountid,uint8 status,const char* charname,const char* target, const char* descriptiontype, const char* description,int event_nid);
	void	GetEventLogs(const char* name,char* target,uint32 account_id=0,uint8 eventid=0,char* detail=0,char* timestamp=0, CharacterEventLog_Struct* cel=0);
	uint32	GetKarma(uint32 acct_id);
	void	UpdateKarma(uint32 acct_id, uint32 amount);

	/* Things which really dont belong here... */
	int16	CommandRequirement(const char* commandname);

#ifdef BOTS
	// bot database add-on to eliminate the need for a second database connection
	BotDatabase botdb;
#endif

protected:
	void ZDBInitVars();

	uint32				max_faction;
	Faction**			faction_array;
	uint32 npc_spellseffects_maxid;
	std::unordered_map<uint32, DBnpcspells_Struct> npc_spells_cache;
	std::unordered_set<uint32> npc_spells_loadtried;
	DBnpcspellseffects_Struct** npc_spellseffects_cache;
	bool*				npc_spellseffects_loadtried;
	uint8 door_isopen_array[255];
};

extern ZoneDatabase database;

#endif /*ZONEDB_H_*/

