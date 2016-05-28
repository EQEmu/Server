/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#ifndef SHAREDDB_H_
#define SHAREDDB_H_

#define MAX_ITEM_ID				200000

#include "database.h"
#include "skills.h"
#include "spdat.h"
#include "base_data.h"
#include "fixed_memory_hash_set.h"
#include "fixed_memory_variable_hash_set.h"

#include <list>
#include <map>
#include <memory>

class EvolveInfo;
class Inventory;
class ItemInst;
struct BaseDataStruct;
struct InspectMessage_Struct;
struct PlayerProfile_Struct;
struct SPDat_Spell_Struct;
struct NPCFactionList;
struct LootTable_Struct;
struct LootDrop_Struct;

namespace EQEmu
{
	struct ItemBase;
	class MemoryMappedFile;
}

/*
    This object is inherited by world and zone's DB object,
    and is mainly here to facilitate shared memory, and other
    things which only world and zone need.
*/
class SharedDatabase : public Database
{
	public:
		SharedDatabase();
		SharedDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port);
		virtual ~SharedDatabase();

		/*
		    General Character Related Stuff
		*/
		bool	SetGMSpeed(uint32 account_id, uint8 gmspeed);
		uint8	GetGMSpeed(uint32 account_id);
		bool	SetHideMe(uint32 account_id, uint8 hideme);
		int32	DeleteStalePlayerCorpses();
		void	LoadCharacterInspectMessage(uint32 character_id, InspectMessage_Struct* message);
		void	SaveCharacterInspectMessage(uint32 character_id, const InspectMessage_Struct* message);
		bool	GetCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &command_settings);
		uint32	GetTotalTimeEntitledOnAccount(uint32 AccountID);

		/*
		    Character Inventory
		*/
		bool	SaveCursor(uint32 char_id, std::list<ItemInst*>::const_iterator &start, std::list<ItemInst*>::const_iterator &end);
		bool	SaveInventory(uint32 char_id, const ItemInst* inst, int16 slot_id);
		bool    DeleteSharedBankSlot(uint32 char_id, int16 slot_id);
		bool    DeleteInventorySlot(uint32 char_id, int16 slot_id);
		bool    UpdateInventorySlot(uint32 char_id, const ItemInst* inst, int16 slot_id);
		bool    UpdateSharedBankSlot(uint32 char_id, const ItemInst* inst, int16 slot_id);
		bool	VerifyInventory(uint32 account_id, int16 slot_id, const ItemInst* inst);
		bool	GetSharedBank(uint32 id, Inventory* inv, bool is_charid);
		int32	GetSharedPlatinum(uint32 account_id);
		bool	SetSharedPlatinum(uint32 account_id, int32 amount_to_add);
		bool	GetInventory(uint32 char_id, Inventory* inv);
		bool	GetInventory(uint32 account_id, char* name, Inventory* inv);
		std::map<uint32, uint32> GetItemRecastTimestamps(uint32 char_id);
		uint32	GetItemRecastTimestamp(uint32 char_id, uint32 recast_type);
		void	ClearOldRecastTimestamps(uint32 char_id);
		bool	SetStartingItems(PlayerProfile_Struct* pp, Inventory* inv, uint32 si_race, uint32 si_class, uint32 si_deity, uint32 si_current_zone, char* si_name, int admin);


		std::string	GetBook(const char *txtfile);

		/*
		    Item Methods
		*/
		ItemInst* CreateItem(uint32 item_id, int16 charges = 0, uint32 aug1 = 0, uint32 aug2 = 0, uint32 aug3 = 0, uint32 aug4 = 0, uint32 aug5 = 0, uint32 aug6 = 0, uint8 attuned = 0);
		ItemInst* CreateItem(const EQEmu::ItemBase* item, int16 charges = 0, uint32 aug1 = 0, uint32 aug2 = 0, uint32 aug3 = 0, uint32 aug4 = 0, uint32 aug5 = 0, uint32 aug6 = 0, uint8 attuned = 0);
		ItemInst* CreateBaseItem(const EQEmu::ItemBase* item, int16 charges = 0);

		/*
		    Shared Memory crap
		*/

		//items
		void GetItemsCount(int32 &item_count, uint32 &max_id);
		void LoadItems(void *data, uint32 size, int32 items, uint32 max_item_id);
		bool LoadItems(const std::string &prefix);
		const EQEmu::ItemBase* IterateItems(uint32* id);
		const EQEmu::ItemBase* GetItem(uint32 id);
		const EvolveInfo* GetEvolveInfo(uint32 loregroup);

		//faction lists
		void GetFactionListInfo(uint32 &list_count, uint32 &max_lists);
		const NPCFactionList* GetNPCFactionEntry(uint32 id);
		void LoadNPCFactionLists(void *data, uint32 size, uint32 list_count, uint32 max_lists);
		bool LoadNPCFactionLists(const std::string &prefix);

		//loot
		void GetLootTableInfo(uint32 &loot_table_count, uint32 &max_loot_table, uint32 &loot_table_entries);
		void GetLootDropInfo(uint32 &loot_drop_count, uint32 &max_loot_drop, uint32 &loot_drop_entries);
		void LoadLootTables(void *data, uint32 size);
		void LoadLootDrops(void *data, uint32 size);
		bool LoadLoot(const std::string &prefix);
		const LootTable_Struct* GetLootTable(uint32 loottable_id);
		const LootDrop_Struct* GetLootDrop(uint32 lootdrop_id);

		void LoadSkillCaps(void *data);
		bool LoadSkillCaps(const std::string &prefix);
		uint16 GetSkillCap(uint8 Class_, EQEmu::skills::SkillType Skill, uint8 Level);
		uint8 GetTrainLevel(uint8 Class_, EQEmu::skills::SkillType Skill, uint8 Level);

		int GetMaxSpellID();
		bool LoadSpells(const std::string &prefix, int32 *records, const SPDat_Spell_Struct **sp);
		void LoadSpells(void *data, int max_spells);
		void LoadDamageShieldTypes(SPDat_Spell_Struct* sp, int32 iMaxSpellID);

		int GetMaxBaseDataLevel();
		bool LoadBaseData(const std::string &prefix);
		void LoadBaseData(void *data, int max_level);
		const BaseDataStruct* GetBaseData(int lvl, int cl);

	protected:

		std::unique_ptr<EQEmu::MemoryMappedFile> skill_caps_mmf;
		std::unique_ptr<EQEmu::MemoryMappedFile> items_mmf;
		std::unique_ptr<EQEmu::FixedMemoryHashSet<EQEmu::ItemBase>> items_hash;
		std::unique_ptr<EQEmu::MemoryMappedFile> faction_mmf;
		std::unique_ptr<EQEmu::FixedMemoryHashSet<NPCFactionList>> faction_hash;
		std::unique_ptr<EQEmu::MemoryMappedFile> loot_table_mmf;
		std::unique_ptr<EQEmu::FixedMemoryVariableHashSet<LootTable_Struct>> loot_table_hash;
		std::unique_ptr<EQEmu::MemoryMappedFile> loot_drop_mmf;
		std::unique_ptr<EQEmu::FixedMemoryVariableHashSet<LootDrop_Struct>> loot_drop_hash;
		std::unique_ptr<EQEmu::MemoryMappedFile> base_data_mmf;
		std::unique_ptr<EQEmu::MemoryMappedFile> spells_mmf;
};

#endif /*SHAREDDB_H_*/
