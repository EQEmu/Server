#ifndef SHAREDDB_H_
#define SHAREDDB_H_

#define MAX_ITEM_ID				200000

#include "database.h"
#include "skills.h"
#include "spdat.h"
#include "item.h"
#include "base_data.h"
#include "fixed_memory_hash_set.h"
#include "fixed_memory_variable_hash_set.h"

#include <list>


struct Item_Struct;
struct NPCFactionList;
struct Faction;
struct LootTable_Struct;
struct LootDrop_Struct;
namespace EQEmu {
	class MemoryMappedFile;
}

/*
 * This object is inherited by world and zone's DB object,
 * and is mainly here to facilitate shared memory, and other
 * things which only world and zone need.
 */
class SharedDatabase : public Database {
public:
	SharedDatabase();
	SharedDatabase(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	virtual ~SharedDatabase();

	/*
	* General Character Related Stuff
	*/
	bool	SetGMSpeed(uint32 account_id, uint8 gmspeed);
	uint8	GetGMSpeed(uint32 account_id);
	bool	SetHideMe(uint32 account_id, uint8 hideme);
	int32	DeleteStalePlayerCorpses();
	int32	DeleteStalePlayerBackups();
	void	LoadCharacterInspectMessage(uint32 character_id, InspectMessage_Struct* message);
	void	SaveCharacterInspectMessage(uint32 character_id, const InspectMessage_Struct* message);
	void	GetBotInspectMessage(uint32 botid, InspectMessage_Struct* message);
	void	SetBotInspectMessage(uint32 botid, const InspectMessage_Struct* message);
	bool	GetCommandSettings(std::map<std::string,uint8> &commands);
	uint32	GetTotalTimeEntitledOnAccount(uint32 AccountID);

	/*
	* Character Inventory
	*/
	bool	SaveCursor(uint32 char_id, std::list<ItemInst*>::const_iterator &start, std::list<ItemInst*>::const_iterator &end);
	bool	SaveInventory(uint32 char_id, const ItemInst* inst, int16 slot_id);
	bool	VerifyInventory(uint32 account_id, int16 slot_id, const ItemInst* inst);
	bool	GetSharedBank(uint32 id, Inventory* inv, bool is_charid);
	int32	GetSharedPlatinum(uint32 account_id);
	bool	SetSharedPlatinum(uint32 account_id, int32 amount_to_add);
	bool	GetInventory(uint32 char_id, Inventory* inv);
	bool	GetInventory(uint32 account_id, char* name, Inventory* inv);
	bool	SetStartingItems(PlayerProfile_Struct* pp, Inventory* inv, uint32 si_race, uint32 si_class, uint32 si_deity, uint32 si_current_zone, char* si_name, int admin);


	std::string	GetBook(const char *txtfile);

	/*
	* Item Methods
	*/
	ItemInst* CreateItem(uint32 item_id, int16 charges=0, uint32 aug1=0, uint32 aug2=0, uint32 aug3=0, uint32 aug4=0, uint32 aug5=0);
	ItemInst* CreateItem(const Item_Struct* item, int16 charges=0, uint32 aug1=0, uint32 aug2=0, uint32 aug3=0, uint32 aug4=0, uint32 aug5=0);
	ItemInst* CreateBaseItem(const Item_Struct* item, int16 charges=0);

	/*
	* Shared Memory crap
	*/

	//items
	void GetItemsCount(int32 &item_count, uint32 &max_id);
	void LoadItems(void *data, uint32 size, int32 items, uint32 max_item_id);
	bool LoadItems();
	const Item_Struct* IterateItems(uint32* id);
	const Item_Struct* GetItem(uint32 id);
	const EvolveInfo* GetEvolveInfo(uint32 loregroup);

	//faction lists
	void GetFactionListInfo(uint32 &list_count, uint32 &max_lists);
	const NPCFactionList* GetNPCFactionEntry(uint32 id);
	void LoadNPCFactionLists(void *data, uint32 size, uint32 list_count, uint32 max_lists);
	bool LoadNPCFactionLists();

	//loot
	void GetLootTableInfo(uint32 &loot_table_count, uint32 &max_loot_table, uint32 &loot_table_entries);
	void GetLootDropInfo(uint32 &loot_drop_count, uint32 &max_loot_drop, uint32 &loot_drop_entries);
	void LoadLootTables(void *data, uint32 size);
	void LoadLootDrops(void *data, uint32 size);
	bool LoadLoot();
	const LootTable_Struct* GetLootTable(uint32 loottable_id);
	const LootDrop_Struct* GetLootDrop(uint32 lootdrop_id);

	void LoadSkillCaps(void *data);
	bool LoadSkillCaps();
	uint16 GetSkillCap(uint8 Class_, SkillUseTypes Skill, uint8 Level);
	uint8 GetTrainLevel(uint8 Class_, SkillUseTypes Skill, uint8 Level);

	int GetMaxSpellID();
	void LoadSpells(void *data, int max_spells);
	void LoadDamageShieldTypes(SPDat_Spell_Struct* sp, int32 iMaxSpellID);

	int GetMaxBaseDataLevel();
	bool LoadBaseData();
	void LoadBaseData(void *data, int max_level);
	const BaseDataStruct* GetBaseData(int lvl, int cl);

protected:

	EQEmu::MemoryMappedFile *skill_caps_mmf;
	EQEmu::MemoryMappedFile *items_mmf;
	EQEmu::FixedMemoryHashSet<Item_Struct> *items_hash;
	EQEmu::MemoryMappedFile *faction_mmf;
	EQEmu::FixedMemoryHashSet<NPCFactionList> *faction_hash;
	EQEmu::MemoryMappedFile *loot_table_mmf;
	EQEmu::FixedMemoryVariableHashSet<LootTable_Struct> *loot_table_hash;
	EQEmu::MemoryMappedFile *loot_drop_mmf;
	EQEmu::FixedMemoryVariableHashSet<LootDrop_Struct> *loot_drop_hash;
	EQEmu::MemoryMappedFile *base_data_mmf;
};

#endif /*SHAREDDB_H_*/
