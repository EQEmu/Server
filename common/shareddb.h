/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef SHAREDDB_H_
#define SHAREDDB_H_

#include "database.h"
#include "skills.h"
#include "spdat.h"
#include "fixed_memory_hash_set.h"
#include "fixed_memory_variable_hash_set.h"
#include "say_link.h"
#include "repositories/command_subsettings_repository.h"
#include "repositories/items_evolving_details_repository.h"
#include "../common/repositories/character_evolving_items_repository.h"

#include <list>
#include <map>
#include <memory>
#include <functional>
#include <string>
#include <array>
#include <unordered_map>
#include <vector>

class EvolveInfo;
struct InspectMessage_Struct;
struct PlayerProfile_Struct;
struct SPDat_Spell_Struct;
struct NPCFactionList;
struct FactionAssociations;


namespace EQ {

	struct ItemData;
	class ItemInstance;
	class InventoryProfile;
	class MemoryMappedFile;
}

struct Book_Struct
{
	uint8       language;
	std::string text;
};

/*
    This object is inherited by world and zone's DB object,
    and is mainly here to facilitate shared memory, and other
    things which only world and zone need.
*/

 class md5
{
	public:
		static std::string digest(std::string str);
	protected:
		md5() = delete;
		~md5() = delete;
	private:
		constexpr static std::array<uint32_t, 64> make_k_array();
		static std::vector<char> padder(std::string str);
		static void init();
	private:
		static const std::array<uint32_t, 64> k_array;
		static const std::array<uint32_t, 64> s_array;
		inline static std::array<uint64_t, 16> m_array;
		inline static uint32_t a0 = 0x67452301;
		inline static uint32_t b0 = 0xefcdab89;
		inline static uint32_t c0 = 0x98badcfe;
		inline static uint32_t d0 = 0x10325476;
};

class SharedDatabase : public Database {
public:
	SharedDatabase();
	SharedDatabase(const char *host, const char *user, const char *passwd, const char *database, uint32 port);
	virtual ~SharedDatabase();

	/**
	 * Character
	 */
	bool SetGMSpeed(uint32 account_id, uint8 gmspeed);
	uint8 GetGMSpeed(uint32 account_id);
	bool SetHideMe(uint32 account_id, uint8 hideme);
	int DeleteStalePlayerCorpses();
	void LoadCharacterInspectMessage(uint32 character_id, InspectMessage_Struct *message);
	void SaveCharacterInspectMessage(uint32 character_id, const InspectMessage_Struct *message);
	bool GetCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &command_settings);
	bool UpdateInjectedCommandSettings(const std::vector<std::pair<std::string, uint8>> &injected);
	bool UpdateOrphanedCommandSettings(const std::vector<std::string> &orphaned);
	bool GetCommandSubSettings(std::vector<CommandSubsettingsRepository::CommandSubsettings> &command_subsettings);
	uint32 GetTotalTimeEntitledOnAccount(uint32 AccountID);
	bool SetGMInvul(uint32 account_id, bool gminvul);
	bool SetGMFlymode(uint32 account_id, uint8 flymode);
	void SetMailKey(int CharID, int IPAddress, int MailKey);
	struct MailKeys {
		std::string mail_key;
		std::string mail_key_full;
	};
	MailKeys GetMailKey(int character_id);
	bool SaveCursor(
		uint32 char_id,
		std::list<EQ::ItemInstance *>::const_iterator &start,
		std::list<EQ::ItemInstance *>::const_iterator &end
	);
	bool SaveInventory(uint32 char_id, const EQ::ItemInstance *inst, int16 slot_id);
	bool DeleteSharedBankSlot(uint32 char_id, int16 slot_id);
	bool DeleteInventorySlot(uint32 char_id, int16 slot_id);
	bool UpdateInventorySlot(uint32 char_id, const EQ::ItemInstance *inst, int16 slot_id);
	bool UpdateSharedBankSlot(uint32 char_id, const EQ::ItemInstance *inst, int16 slot_id);
	bool VerifyInventory(uint32 account_id, int16 slot_id, const EQ::ItemInstance *inst);
	bool GetSharedBank(uint32 id, EQ::InventoryProfile *inv, bool is_charid);
	int32 GetSharedPlatinum(uint32 account_id);
	bool SetSharedPlatinum(uint32 account_id, int32 amount_to_add);
	bool GetInventory(Client* c);
	bool GetInventory(uint32 account_id, char *name, EQ::InventoryProfile *inv); // deprecated
	std::map<uint32, uint32> GetItemRecastTimestamps(uint32 char_id);
	uint32 GetItemRecastTimestamp(uint32 char_id, uint32 recast_type);
	void ClearOldRecastTimestamps(uint32 char_id);
	bool SetStartingItems(
		PlayerProfile_Struct *pp,
		EQ::InventoryProfile *inv,
		uint32 si_race,
		uint32 si_class,
		uint32 si_deity,
		uint32 si_current_zone,
		char *si_name,
		int admin
	);

	Book_Struct GetBook(const std::string& text_file);

	/**
	 * items
	 */
	EQ::ItemInstance *CreateItem(
		uint32 item_id,
		int16 charges = 0,
		uint32 aug1 = 0,
		uint32 aug2 = 0,
		uint32 aug3 = 0,
		uint32 aug4 = 0,
		uint32 aug5 = 0,
		uint32 aug6 = 0,
		bool attuned = false,
		const std::string& custom_data = "",
		uint32 ornamenticon = 0,
		uint32 ornamentidfile = 0,
		uint32 ornament_hero_model = 0
	);
	EQ::ItemInstance *CreateItem(
		const EQ::ItemData *item,
		int16 charges = 0,
		uint32 aug1 = 0,
		uint32 aug2 = 0,
		uint32 aug3 = 0,
		uint32 aug4 = 0,
		uint32 aug5 = 0,
		uint32 aug6 = 0,
		bool attuned = false,
		const std::string &custom_data = "",
		uint32 ornamenticon = 0,
		uint32 ornamentidfile = 0,
		uint32 ornament_hero_model = 0
	);
	EQ::ItemInstance *CreateBaseItem(const EQ::ItemData *item, int16 charges = 0);

	void GetItemsCount(int32 &item_count, uint32 &max_id);
	void LoadItems(void *data, uint32 size, int32 items, uint32 max_item_id);
	bool LoadItems(const std::string &prefix);
	const EQ::ItemData *IterateItems(uint32 *id) const;
	EQ::ItemData *GetItem(uint32 id) const;
	const EvolveInfo *GetEvolveInfo(uint32 loregroup);
	uint32 GetSharedItemsCount() { return m_shared_items_count; }
	uint32 GetItemsCount();

	/**
	 * spells
	 */
	int GetMaxSpellID();
	bool LoadSpells(const std::string &prefix, int32 *records, const SPDat_Spell_Struct **sp);
	void LoadSpells(void *data, int max_spells);
	void LoadDamageShieldTypes(SPDat_Spell_Struct *sp, int32 iMaxSpellID);
	uint32 GetSharedSpellsCount() { return m_shared_spells_count; }
	uint32 GetSpellsCount();

	std::string CreateItemLink(uint32 item_id) const
	{
		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemData);
		const EQ::ItemData *item = GetItem(item_id);
		linker.SetItemData(item);
		return linker.GenerateLink();
	}

protected:
	std::unique_ptr<EQ::MemoryMappedFile>                        skill_caps_mmf;
	std::unique_ptr<EQ::MemoryMappedFile>                        items_mmf;
	std::unique_ptr<EQ::FixedMemoryHashSet<EQ::ItemData>>        items_hash;
	std::unique_ptr<EQ::MemoryMappedFile>                        faction_mmf;
	std::unique_ptr<EQ::FixedMemoryHashSet<NPCFactionList>>      faction_hash;
	std::unique_ptr<EQ::MemoryMappedFile>                        faction_associations_mmf;
	std::unique_ptr<EQ::FixedMemoryHashSet<FactionAssociations>> faction_associations_hash;
	std::unique_ptr<EQ::MemoryMappedFile>                        spells_mmf;
private:
	std::unordered_map<std::string, int> generated_item_cache;
public:
	void SetSharedItemsCount(uint32 shared_items_count);
	void SetSharedSpellsCount(uint32 shared_spells_count);
protected:
	uint32 m_shared_items_count = 0;
	uint32 m_shared_spells_count = 0;
};

#endif /*SHAREDDB_H_*/
