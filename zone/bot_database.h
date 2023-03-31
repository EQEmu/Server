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


#ifndef BOT_DATABASE_H
#define BOT_DATABASE_H


#include <list>
#include <map>
#include <vector>


class Bot;
class Client;
struct BotsAvailableList;
struct InspectMessage_Struct;

namespace EQ
{
	class ItemInstance;
	class InventoryProfile;
}


class BotDatabase
{
public:
	bool LoadBotCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &bot_command_settings);
	bool UpdateInjectedBotCommandSettings(const std::vector<std::pair<std::string, uint8>> &injected);
	bool UpdateOrphanedBotCommandSettings(const std::vector<std::string> &orphaned);
	bool LoadBotSpellCastingChances();


	/* Bot functions   */
	bool QueryNameAvailablity(const std::string& bot_name, bool& available_flag);
	bool QueryBotCount(const uint32 owner_id, int class_id, uint32& bot_count, uint32& bot_class_count);
	bool LoadBotsList(const uint32 owner_id, std::list<BotsAvailableList>& bots_list, bool ByAccount = false);

	bool LoadOwnerID(const std::string& bot_name, uint32& owner_id);
	bool LoadOwnerID(const uint32 bot_id, uint32& owner_id);
	uint32 GetOwnerID(const uint32 bot_id);
	bool LoadBotID(const uint32 owner_id, const std::string& bot_name, uint32& bot_id);
	bool LoadBotID(const uint32 owner_id, const std::string& bot_name, uint32& bot_id, uint8& bot_class_id);

	bool LoadBot(const uint32 bot_id, Bot*& loaded_bot);
	bool SaveNewBot(Bot* bot_inst, uint32& bot_id);
	bool SaveBot(Bot* bot_inst);
	bool DeleteBot(const uint32 bot_id);

	bool LoadBuffs(Bot* bot_inst);
	bool SaveBuffs(Bot* bot_inst);
	bool DeleteBuffs(const uint32 bot_id);

	bool LoadStance(const uint32 bot_id, int& bot_stance);
	bool LoadStance(Bot* bot_inst, bool& stance_flag);
	bool SaveStance(const uint32 bot_id, const int bot_stance);
	bool SaveStance(Bot* bot_inst);
	bool DeleteStance(const uint32 bot_id);

	bool LoadTimers(Bot* bot_inst);
	bool SaveTimers(Bot* bot_inst);
	bool DeleteTimers(const uint32 bot_id);


	/* Bot inventory functions   */
	bool QueryInventoryCount(const uint32 bot_id, uint32& item_count);

	bool LoadItems(const uint32 bot_id, EQ::InventoryProfile &inventory_inst);
	bool SaveItems(Bot* bot_inst);
	bool DeleteItems(const uint32 bot_id);

	bool LoadItemSlots(const uint32 bot_id, std::map<uint16, uint32>& m);
	bool LoadItemBySlot(Bot* bot_inst);
	bool LoadItemBySlot(const uint32 bot_id, const uint32 slot_id, uint32& item_id);
	bool SaveItemBySlot(Bot* bot_inst, const uint32 slot_id, const EQ::ItemInstance* item_inst);
	bool DeleteItemBySlot(const uint32 bot_id, const uint32 slot_id);

	bool LoadEquipmentColor(const uint32 bot_id, const uint8 material_slot_id, uint32& rgb);
	bool SaveEquipmentColor(const uint32 bot_id, const int16 slot_id, const uint32 rgb);

	bool SaveExpansionBitmask(const uint32 bot_id, const int expansion_bitmask);
	bool SaveEnforceSpellSetting(const uint32 bot_id, const bool enforce_spell_setting);


	/* Bot pet functions   */
	bool LoadPetIndex(const uint32 bot_id, uint32& pet_index);
	bool LoadPetSpellID(const uint32 bot_id, uint32& pet_spell_id);

	bool LoadPetStats(const uint32 bot_id, std::string& pet_name, uint32& pet_mana, uint32& pet_hp, uint32& pet_spell_id);
	bool SavePetStats(const uint32 bot_id, const std::string& pet_name, const uint32 pet_mana, const uint32 pet_hp, const uint32 pet_spell_id);
	bool DeletePetStats(const uint32 bot_id);

	bool LoadPetBuffs(const uint32 bot_id, SpellBuff_Struct* pet_buffs);
	bool SavePetBuffs(const uint32 bot_id, const SpellBuff_Struct* pet_buffs, bool delete_flag = false);
	bool DeletePetBuffs(const uint32 bot_id);

	bool LoadPetItems(const uint32 bot_id, uint32* pet_items);
	bool SavePetItems(const uint32 bot_id, const uint32* pet_items, bool delete_flag = false);
	bool DeletePetItems(const uint32 bot_id);


	/* Bot command functions   */
	bool LoadInspectMessage(const uint32 bot_id, InspectMessage_Struct& inspect_message);
	bool SaveInspectMessage(const uint32 bot_id, const InspectMessage_Struct& inspect_message);
	bool DeleteInspectMessage(const uint32 bot_id);

	bool SaveAllInspectMessages(const uint32 owner_id, const InspectMessage_Struct& inspect_message);
	bool DeleteAllInspectMessages(const uint32 owner_id);

	bool SaveAllArmorColorBySlot(const uint32 owner_id, const int16 slot_id, const uint32 rgb_value);
	bool SaveAllArmorColors(const uint32 owner_id, const uint32 rgb_value);

	bool SaveHelmAppearance(const uint32 owner_id, const uint32 bot_id, const bool show_flag = true);
	bool SaveAllHelmAppearances(const uint32 owner_id, const bool show_flag = true);

	bool ToggleHelmAppearance(const uint32 owner_id, const uint32 bot_id);
	bool ToggleAllHelmAppearances(const uint32 owner_id);

	bool SaveFollowDistance(const uint32 owner_id, const uint32 bot_id, const uint32 follow_distance);
	bool SaveAllFollowDistances(const uint32 owner_id, const uint32 follow_distance);

	bool CreateCloneBot(const uint32 owner_id, const uint32 bot_id, const std::string& clone_name, uint32& clone_id);
	bool CreateCloneBotInventory(const uint32 owner_id, const uint32 bot_id, const uint32 clone_id);

	bool SaveStopMeleeLevel(const uint32 owner_id, const uint32 bot_id, const uint8 sml_value);

	bool SaveBotArcherSetting(const uint32 bot_id, const bool bot_archer_setting);

	bool LoadOwnerOptions(Client *owner);
	bool SaveOwnerOption(const uint32 owner_id, size_t type, const bool flag);
	bool SaveOwnerOption(const uint32 owner_id, const std::pair<size_t, size_t> type, const std::pair<bool, bool> flag);

	bool SaveBotCasterRange(const uint32 owner_id, const uint32 bot_id, const uint32 bot_caster_range_value);

	/* Bot group functions   */
	bool LoadGroupedBotsByGroupID(const uint32 owner_id, const uint32 group_id, std::list<uint32>& group_list);

	/* Bot heal rotation functions   */
	bool LoadHealRotationIDByBotID(const uint32 bot_id, uint32& hr_index);

	bool LoadHealRotation(Bot* hr_member, std::list<uint32>& member_list, std::list<std::string>& target_list, bool& load_flag, bool& member_fail, bool& target_fail);
	bool LoadHealRotationMembers(const uint32 hr_index, std::list<uint32>& member_list);
	bool LoadHealRotationTargets(const uint32 hr_index, std::list<std::string>& target_list);
	bool SaveHealRotation(Bot* hr_member, bool& member_fail, bool& target_fail);
	bool DeleteHealRotation(const uint32 creator_id);

	bool DeleteAllHealRotations(const uint32 owner_id);

	/* Bot miscellaneous functions   */
	uint8 GetSpellCastingChance(uint8 spell_type_index, uint8 class_index, uint8 stance_index, uint8 conditional_index);
	std::string GetBotNameByID(const uint32 bot_id); 

	uint16 GetRaceClassBitmask(uint16 bot_race);

	class fail {
	public:
		/* fail::Bot functions   */
		static const char* LoadBotsList();
		static const char* LoadOwnerID();
		static const char* LoadBotID();
		static const char* LoadBot();
		static const char* SaveNewBot();
		static const char* SaveBot();
		static const char* DeleteBot();
		static const char* LoadBuffs();
		static const char* SaveBuffs();
		static const char* DeleteBuffs();
		static const char* DeleteStance();
		static const char* LoadTimers();
		static const char* SaveTimers();
		static const char* DeleteTimers();

		/* fail::Bot inventory functions   */
		static const char* QueryInventoryCount();
		static const char* LoadItems();
		static const char* SaveItems();
		static const char* DeleteItems();
		static const char* SaveItemBySlot();
		static const char* DeleteItemBySlot();
		static const char* LoadEquipmentColor();
		static const char* SaveEquipmentColor();

		/* fail::Bot pet functions   */
		static const char* LoadPetIndex();
		static const char* LoadPetSpellID();
		static const char* LoadPetStats();
		static const char* SavePetStats();
		static const char* DeletePetStats();
		static const char* LoadPetBuffs();
		static const char* SavePetBuffs();
		static const char* DeletePetBuffs();
		static const char* LoadPetItems();
		static const char* SavePetItems();
		static const char* DeletePetItems();

		/* fail::Bot command functions   */
		static const char* LoadInspectMessage();
		static const char* SaveInspectMessage();
		static const char* DeleteInspectMessage();
		static const char* SaveAllInspectMessages();
		static const char* DeleteAllInspectMessages();
		static const char* SaveAllArmorColorBySlot();
		static const char* SaveAllArmorColors();
		static const char* SaveHelmAppearance();
		static const char* SaveAllHelmAppearances();
		static const char* ToggleHelmAppearance();
		static const char* ToggleAllHelmAppearances();
		static const char* SaveFollowDistance();
		static const char* SaveAllFollowDistances();
		static const char* SaveStopMeleeLevel();
		static const char* SaveBotCasterRange();

		/* fail::Bot group functions   */
		static const char* LoadGroupedBotsByGroupID();

		/* fail::Bot heal rotation functions   */
		static const char* LoadHealRotationIDByBotID();
		static const char* LoadHealRotation();
		static const char* LoadHealRotationMembers();
		static const char* LoadHealRotationTargets();
		static const char* SaveHealRotation();
		static const char* DeleteHealRotation();
		static const char* DeleteAllHealRotations();

		/* fail::Bot miscellaneous functions   */
		static const char* GetBotNameByID();
	};

	private:
		std::string query;
};

#endif
