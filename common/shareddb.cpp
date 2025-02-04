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

#include <iostream>
#include <cstring>
#include <fmt/format.h>

#if defined(_MSC_VER) && _MSC_VER >= 1800
	#include <algorithm>
#endif

#include "classes.h"
#include "eq_packet_structs.h"
#include "faction.h"
#include "features.h"
#include "ipc_mutex.h"
#include "inventory_profile.h"
#include "memory_mapped_file.h"
#include "mysql.h"
#include "rulesys.h"
#include "shareddb.h"
#include "strings.h"
#include "eqemu_config.h"
#include "data_verification.h"
#include "evolving_items.h"
#include "repositories/criteria/content_filter_criteria.h"
#include "repositories/account_repository.h"
#include "repositories/faction_association_repository.h"
#include "repositories/starting_items_repository.h"
#include "path_manager.h"
#include "../zone/client.h"
#include "repositories/loottable_repository.h"
#include "repositories/character_item_recast_repository.h"
#include "repositories/character_corpses_repository.h"
#include "repositories/skill_caps_repository.h"
#include "repositories/inventory_repository.h"
#include "repositories/books_repository.h"
#include "repositories/sharedbank_repository.h"

namespace ItemField
{
	enum {
		source = 0,
#define F(x) x,
#include "item_fieldlist.h"
#undef F
		updated,
		minstatus,
		comment,
	};
}

SharedDatabase::SharedDatabase()
: Database()
{
}

SharedDatabase::SharedDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
: Database(host, user, passwd, database, port)
{
}

SharedDatabase::~SharedDatabase() = default;

bool SharedDatabase::SetHideMe(uint32 account_id, uint8 hideme)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		a.hideme = hideme ? 1 : 0;
		AccountRepository::UpdateOne(*this, a);
	}

	return a.id > 0;
}

uint8 SharedDatabase::GetGMSpeed(uint32 account_id)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		return a.gmspeed;
	}

	return 0;
}

bool SharedDatabase::SetGMSpeed(uint32 account_id, uint8 gmspeed)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		a.gmspeed = gmspeed ? 1 : 0;
		AccountRepository::UpdateOne(*this, a);
	}

	return a.id > 0;
}

bool SharedDatabase::SetGMInvul(uint32 account_id, bool gminvul)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		a.invulnerable = gminvul ? 1 : 0;
		AccountRepository::UpdateOne(*this, a);
	}

	return a.id > 0;
}

bool SharedDatabase::SetGMFlymode(uint32 account_id, uint8 flymode)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		a.flymode = flymode;
		AccountRepository::UpdateOne(*this, a);
	}

	return a.id > 0;
}

uint32 SharedDatabase::GetTotalTimeEntitledOnAccount(uint32 AccountID) {
	uint32 EntitledTime = 0;
	const std::string query = StringFormat("SELECT `time_played` FROM `character_data` WHERE `account_id` = %u", AccountID);
	auto results = QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) {
		EntitledTime += Strings::ToUnsignedInt(row[0]);
	}
	return EntitledTime;
}

void SharedDatabase::SetMailKey(int CharID, int IPAddress, int MailKey)
{
	char mail_key[17];

	if (RuleB(Chat, EnableMailKeyIPVerification) == true) {
		sprintf(mail_key, "%08X%08X", IPAddress, MailKey);
	}
	else {
		sprintf(mail_key, "%08X", MailKey);
	}

	const std::string query = StringFormat(
		"UPDATE character_data SET mailkey = '%s' WHERE id = '%i'",
		mail_key, CharID
	);

	const auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogError("SharedDatabase::SetMailKey({}, {}) : {}", CharID, mail_key, results.ErrorMessage().c_str());
	}
}

SharedDatabase::MailKeys SharedDatabase::GetMailKey(int character_id)
{
	const std::string query   = StringFormat("SELECT `mailkey` FROM `character_data` WHERE `id`='%i' LIMIT 1", character_id);
	auto              results = QueryDatabase(query);
	if (!results.Success()) {
		return MailKeys{};
	}

	if (!results.RowCount()) {
		Log(Logs::General,
			Logs::ClientLogin,
			"Error: Mailkey for character id [%i] does not exist or could not be found",
			character_id
		);
		return MailKeys{};
	}

	auto &row = results.begin();
	if (row != results.end()) {
		std::string mail_key = row[0];

		return MailKeys{
			.mail_key = mail_key.substr(8),
			.mail_key_full = mail_key
		};
	}

	return MailKeys{};
}

bool SharedDatabase::SaveCursor(
	uint32 char_id,
	std::list<EQ::ItemInstance*>::const_iterator& start,
	std::list<EQ::ItemInstance*>::const_iterator& end
)
{
	const int deleted = InventoryRepository::DeleteWhere(
		*this,
		fmt::format(
			"`character_id` = {} AND (`slot_id` = {} OR `slot_id` BETWEEN {} AND {})",
			char_id,
			EQ::invslot::slotCursor,
			EQ::invbag::CURSOR_BAG_BEGIN,
			EQ::invbag::CURSOR_BAG_END
		)
	);

	int16 i = EQ::invbag::CURSOR_BAG_BEGIN;
	for (auto& it = start; it != end; ++it, i++) {
		// shouldn't be anything in the queue that indexes this high
		if (i > EQ::invbag::CURSOR_BAG_END) {
			break;
		}

		const EQ::ItemInstance* inst = *it;
		const int16 use_slot = i == EQ::invbag::CURSOR_BAG_BEGIN ? EQ::invslot::slotCursor : i;
		if (!SaveInventory(char_id, inst, use_slot)) {
			return false;
		}
	}

	return true;
}

bool SharedDatabase::VerifyInventory(uint32 account_id, int16 slot_id, const EQ::ItemInstance* inst)
{
	if (!inst || !inst->GetItem()) {
		return false;
	}

	const auto& l = SharedbankRepository::GetWhere(
		*this,
		fmt::format(
			"`account_id` = {} AND `slot_id` = {} LIMIT 1",
			account_id,
			slot_id
		)
	);

	if (l.empty()) {
		return false;
	}

	const auto& e = l.front();

	uint16 expect_charges = inst->GetCharges() >= 0 ? inst->GetCharges() : std::numeric_limits<int16>::max();

	return e.item_id == inst->GetID() && e.charges == expect_charges;
}

bool SharedDatabase::SaveInventory(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id)
{
	// Don't save any Tribute slots
	if (
		EQ::ValueWithin(slot_id, EQ::invslot::GUILD_TRIBUTE_BEGIN, EQ::invslot::GUILD_TRIBUTE_END) ||
		EQ::ValueWithin(slot_id, EQ::invslot::TRIBUTE_BEGIN, EQ::invslot::TRIBUTE_END)
	) {
		return true;
	}

	if (
		EQ::ValueWithin(slot_id, EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END) ||
		EQ::ValueWithin(slot_id, EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END)
	) {
		if (!inst) {
			return DeleteSharedBankSlot(char_id, slot_id);
		} else {
			// Needed to clear out bag slots that 'REPLACE' in UpdateSharedBankSlot does not overwrite..otherwise, duplication occurs
			// (This requires that parent then child items be sent..which should be how they are currently passed)
			if (EQ::InventoryProfile::SupportsContainers(slot_id)) {
				DeleteSharedBankSlot(char_id, slot_id);
			}

			return UpdateSharedBankSlot(char_id, inst, slot_id);
		}
	} else if (!inst) { // All other inventory
		return DeleteInventorySlot(char_id, slot_id);
	}

	// Needed to clear out bag slots that 'REPLACE' in UpdateInventorySlot does not overwrite..otherwise, duplication occurs
	// (This requires that parent then child items be sent..which should be how they are currently passed)
	if (EQ::InventoryProfile::SupportsContainers(slot_id)) {
		DeleteInventorySlot(char_id, slot_id);
	}

	return UpdateInventorySlot(char_id, inst, slot_id);
}

bool SharedDatabase::UpdateInventorySlot(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id)
{
	if (!inst || !inst->GetItem()) {
		return false;
	}

	std::vector<uint32> augment_ids = inst->GetAugmentIDs();

	uint16 charges = inst->GetCharges() >= 0 ? inst->GetCharges() : std::numeric_limits<int16>::max();

	auto e = InventoryRepository::NewEntity();

	e.character_id        = char_id;
	e.slot_id             = slot_id;
	e.item_id             = inst->GetID();
	e.charges             = charges;
	e.color               = inst->GetColor();
	e.augment_one         = augment_ids[0];
	e.augment_two         = augment_ids[1];
	e.augment_three       = augment_ids[2];
	e.augment_four        = augment_ids[3];
	e.augment_five        = augment_ids[4];
	e.augment_six         = augment_ids[5];
	e.instnodrop          = inst->IsAttuned() ? 1 : 0;
	e.custom_data         = inst->GetCustomDataString();
	e.ornament_icon       = inst->GetOrnamentationIcon();
	e.ornament_idfile     = inst->GetOrnamentationIDFile();
	e.ornament_hero_model = inst->GetOrnamentHeroModel();
	e.guid                = inst->GetSerialNumber();

	const int replaced = InventoryRepository::ReplaceOne(*this, e);

	// Save bag contents, if slot supports bag contents
	if (inst->IsClassBag() && EQ::InventoryProfile::SupportsContainers(slot_id)) {
		// Limiting to bag slot count will get rid of 'hidden' duplicated items and 'Invalid Slot ID'
		// messages through attrition (and the modded code in SaveInventory)
		for (
			uint8 i = EQ::invbag::SLOT_BEGIN;
			i < inst->GetItem()->BagSlots && i <= EQ::invbag::SLOT_END;
			i++
		) {
			const EQ::ItemInstance* bag_inst = inst->GetItem(i);
			SaveInventory(char_id, bag_inst, EQ::InventoryProfile::CalcSlotId(slot_id, i));
		}
	}

	return replaced;
}

bool SharedDatabase::UpdateSharedBankSlot(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id)
{
	if (!inst || !inst->GetItem()) {
		return false;
	}

	std::vector<uint32> augment_ids = inst->GetAugmentIDs();

	uint16 charges = inst->GetCharges() >= 0 ? inst->GetCharges() : std::numeric_limits<int16>::max();

	const uint32 account_id = GetAccountIDByChar(char_id);

	auto e = SharedbankRepository::NewEntity();

	e.account_id          = account_id;
	e.slot_id             = slot_id;
	e.item_id             = inst->GetID();
	e.charges             = charges;
	e.color               = inst->GetColor();
	e.augment_one         = augment_ids[0];
	e.augment_two         = augment_ids[1];
	e.augment_three       = augment_ids[2];
	e.augment_four        = augment_ids[3];
	e.augment_five        = augment_ids[4];
	e.augment_six         = augment_ids[5];
	e.custom_data         = inst->GetCustomDataString();
	e.ornament_icon       = inst->GetOrnamentationIcon();
	e.ornament_idfile     = inst->GetOrnamentationIDFile();
	e.ornament_hero_model = inst->GetOrnamentHeroModel();
	e.guid                = inst->GetSerialNumber();

	const int replaced = SharedbankRepository::ReplaceOne(*this, e);

	// Save bag contents, if slot supports bag contents
	if (inst->IsClassBag() && EQ::InventoryProfile::SupportsContainers(slot_id)) {
		// Limiting to bag slot count will get rid of 'hidden' duplicated items and 'Invalid Slot ID'
		// messages through attrition (and the modded code in SaveInventory)
		for (
			uint8 i = EQ::invbag::SLOT_BEGIN;
			i < inst->GetItem()->BagSlots && i <= EQ::invbag::SLOT_END;
			i++
		) {
			const EQ::ItemInstance* bag_inst = inst->GetItem(i);
			SaveInventory(char_id, bag_inst, EQ::InventoryProfile::CalcSlotId(slot_id, i));
		}
	}

	return replaced;
}

bool SharedDatabase::DeleteInventorySlot(uint32 char_id, int16 slot_id)
{
	const int deleted = InventoryRepository::DeleteWhere(
		*this,
		fmt::format(
			"`character_id` = {} AND `slot_id` = {}",
			char_id,
			slot_id
		)
	);

	if (!deleted) {
		return false;
	}

	// Delete bag slots, if need be
	if (!EQ::InventoryProfile::SupportsContainers(slot_id)) {
		return true;
	}

	const int16 base_slot_id = EQ::InventoryProfile::CalcSlotId(slot_id, EQ::invbag::SLOT_BEGIN);

	return InventoryRepository::DeleteWhere(
		*this,
		fmt::format(
			"`character_id` = {} AND `slot_id` BETWEEN {} AND {}",
			char_id,
			base_slot_id,
			base_slot_id + (EQ::invbag::SLOT_COUNT - 1)
		)
	);
}

bool SharedDatabase::DeleteSharedBankSlot(uint32 char_id, int16 slot_id)
{
	const uint32 account_id = GetAccountIDByChar(char_id);

	const int deleted = SharedbankRepository::DeleteWhere(
		*this,
		fmt::format(
			"`account_id` = {} AND `slot_id` = {}",
			account_id,
			slot_id
		)
	);

	if (!deleted) {
		return false;
	}

	if (!EQ::InventoryProfile::SupportsContainers(slot_id)) {
		return true;
	}

	const int16 base_slot_id = EQ::InventoryProfile::CalcSlotId(slot_id, EQ::invbag::SLOT_BEGIN);

	return SharedbankRepository::DeleteWhere(
		*this,
		fmt::format(
			"`account_id` = {} AND `slot_id` BETWEEN {} AND {}",
			account_id,
			base_slot_id,
			base_slot_id + (EQ::invbag::SLOT_COUNT - 1)
		)
	);
}


int32 SharedDatabase::GetSharedPlatinum(uint32 account_id)
{
	const auto query   = fmt::format("SELECT sharedplat FROM account WHERE id = {}", account_id);
	auto       results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto row = results.begin();
	return Strings::ToInt(row[0]);
}

bool SharedDatabase::SetSharedPlatinum(uint32 account_id, int32 amount_to_add) {
	const std::string query = StringFormat("UPDATE account SET sharedplat = sharedplat + %i WHERE id = %i", amount_to_add, account_id);
	const auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}

bool SharedDatabase::SetStartingItems(
	PlayerProfile_Struct *pp,
	EQ::InventoryProfile *inv,
	uint32 si_race,
	uint32 si_class,
	uint32 si_deity,
	uint32 si_current_zone,
	char *si_name,
	int admin_level
)
{
	const EQ::ItemData *item_data;

	const auto& l = StartingItemsRepository::GetWhere(
		*this,
		fmt::format(
			"TRUE {}",
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return false;
	}

	std::vector<StartingItemsRepository::StartingItems> v;

	for (const auto &e : l) {
		const auto &classes = Strings::Split(e.class_list, "|");
		const auto &deities = Strings::Split(e.deity_list, "|");
		const auto &races   = Strings::Split(e.race_list, "|");
		const auto &zones   = Strings::Split(e.zone_id_list, "|");

		const std::string &all = std::to_string(0);

		if (classes[0] != all) {
			if (!Strings::Contains(classes, std::to_string(si_class))) {
				continue;
			}
		}

		if (deities[0] != all) {
			if (!Strings::Contains(deities, std::to_string(si_deity))) {
				continue;
			}
		}

		if (races[0] != all) {
			if (!Strings::Contains(races, std::to_string(si_race))) {
				continue;
			}
		}

		if (zones[0] != all) {
			if (!Strings::Contains(zones, std::to_string(si_current_zone))) {
				continue;
			}
		}

		v.emplace_back(e);
	}

	for (const auto &e : v) {
		const uint32 item_id      = e.item_id;
		const uint8  item_charges = e.item_charges;
		int32        slot         = e.inventory_slot;

		item_data = GetItem(item_id);

		if (!item_data) {
			continue;
		}

		const auto* inst = CreateItem(
			item_data,
			item_charges,
			e.augment_one,
			e.augment_two,
			e.augment_three,
			e.augment_four,
			e.augment_five,
			e.augment_six
		);

		if (slot < EQ::invslot::slotCharm) {
			slot = inv->FindFreeSlot(false, false);
		}

		inv->PutItem(slot, *inst);
		safe_delete(inst);
	}

	return true;
}


// Retrieve shared bank inventory based on either account or character
bool SharedDatabase::GetSharedBank(uint32 id, EQ::InventoryProfile *inv, bool is_charid)
{
	const uint32 account_id = is_charid ? GetAccountIDByChar(id) : id;

	if (!account_id) {
		return false;
	}

	const auto& l = SharedbankRepository::GetWhere(
		*this,
		fmt::format(
			"`account_id` = {}",
			account_id
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		uint32 augment_ids[EQ::invaug::SOCKET_COUNT] = {
			e.augment_one,
			e.augment_two,
			e.augment_three,
			e.augment_four,
			e.augment_five,
			e.augment_six
		};

		const EQ::ItemData* item = GetItem(e.item_id);

		if (!item) {
			LogError(
				"Warning: {} [{}] has an invalid item_id [{}] in slot_id [{}]",
				is_charid ? "character_id" : "account_id",
				id,
				e.item_id,
				e.slot_id
			);
			continue;
		}

		EQ::ItemInstance* inst = CreateBaseItem(item, e.charges);
		if (!inst) {
			continue;
		}

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (augment_ids[i]) {
					inst->PutAugment(this, i, augment_ids[i]);
				}
			}
		}

		if (!e.custom_data.empty()) {
			inst->SetCustomDataString(e.custom_data);
		}

		const int16 put_slot_id = inv->PutItem(e.slot_id, *inst);
		safe_delete(inst);

		if (put_slot_id != INVALID_INDEX) {
			continue;
		}

		LogError(
			"Warning: Invalid slot_id for item in shared bank inventory for {} [{}] item_id [{}] slot_id [{}]",
			is_charid ? "character_id" : "account_id",
			id,
			e.item_id,
			e.slot_id
		);

		if (is_charid) {
			SaveInventory(id, nullptr, e.slot_id);
		}
	}

	return true;
}
// Overloaded: Retrieve character inventory based on character id (zone entry)
//bool SharedDatabase::GetInventory(uint32 char_id, EQ::InventoryProfile *inv)
bool SharedDatabase::GetInventory(Client *c)
{
	if (!c) {
		return false;
	}

	uint32                char_id = c->CharacterID();
	EQ::InventoryProfile &inv     = c->GetInv();

	// Retrieve character inventory
	auto results   = InventoryRepository::GetWhere(*this, fmt::format("`character_id` = '{}' ORDER BY `slot_id`", char_id));
	auto e_results = CharacterEvolvingItemsRepository::GetWhere(
		*this, fmt::format("`character_id` = '{}' AND `deleted_at` IS NULL", char_id)
	);

	if (results.empty()) {
		LogError("Error loading inventory for char_id {} from the database.", char_id);
		return false;
	}

	for (auto const& row: results) {
		if (row.guid != 0) {
			EQ::ItemInstance::AddGUIDToMap(row.guid);
		}
	}

	const auto timestamps  = GetItemRecastTimestamps(char_id);
	auto       cv_conflict = false;
	const auto pmask       = inv.GetLookup()->PossessionsBitmask;
	const auto bank_size   = inv.GetLookup()->InventoryTypeSize.Bank;

	std::vector<InventoryRepository::Inventory> queue{ };
	for (auto& row: results) {
		const int16  slot_id             = row.slot_id;
		const uint32 item_id             = row.item_id;
		const uint16 charges             = row.charges;
		const uint32 color               = row.color;
		const bool   instnodrop          = row.instnodrop;
		const uint32 ornament_icon       = row.ornament_icon;
		const uint32 ornament_idfile     = row.ornament_idfile;
		const uint32 ornament_hero_model = row.ornament_hero_model;

		uint32 augment_ids[EQ::invaug::SOCKET_COUNT] = {
			row.augment_one,
			row.augment_two,
			row.augment_three,
			row.augment_four,
			row.augment_five,
			row.augment_six
		};

		if (EQ::ValueWithin(slot_id, EQ::invslot::POSSESSIONS_BEGIN, EQ::invslot::POSSESSIONS_END)) {
			// Titanium thru UF check
			if (((static_cast<uint64>(1) << slot_id) & pmask) == 0) {
				cv_conflict = true;
				continue;
			}
		} else if (EQ::ValueWithin(slot_id, EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END)) {
			// Titanium thru UF check
			const auto parent_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
			if (((static_cast<uint64>(1) << parent_slot) & pmask) == 0) {
				cv_conflict = true;
				continue;
			}
		} else if (EQ::ValueWithin(slot_id, EQ::invslot::BANK_BEGIN, EQ::invslot::BANK_END)) {
			// Titanium check
			if ((slot_id - EQ::invslot::BANK_BEGIN) >= bank_size) {
				cv_conflict = true;
				continue;
			}
		} else if (EQ::ValueWithin(slot_id, EQ::invbag::BANK_BAGS_BEGIN, EQ::invbag::BANK_BAGS_END)) {
			// Titanium check
			const auto parent_index = ((slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
			if (parent_index >= bank_size) {
				cv_conflict = true;
				continue;
			}
		}

		auto* item = GetItem(item_id);
		if (!item) {
			LogError(
				"Warning: charid [{}] has an invalid item_id [{}] in inventory slot [{}]",
				char_id,
				item_id,
				slot_id
			);
			continue;
		}

		auto* inst = CreateBaseItem(item, charges);
		if (!inst) {
			continue;
		}

		if (!row.custom_data.empty()) {
			inst->SetCustomDataString(row.custom_data);
		}

		inst->SetOrnamentIcon(ornament_icon);
		inst->SetOrnamentationIDFile(ornament_idfile);
		inst->SetOrnamentHeroModel(item->HerosForgeModel);

		if (
			instnodrop ||
			(
				inst->GetItem()->Attuneable &&
				EQ::ValueWithin(slot_id, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)
			)
		) {
			inst->SetAttuned(true);
		}

		if (color > 0) {
			inst->SetColor(color);
		}

		if (charges == std::numeric_limits<int16>::max()) {
			inst->SetCharges(-1);
		} else if (charges == 0 && inst->IsStackable()) {
			// Stackable items need a minimum charge of 1 remain moveable.
			inst->SetCharges(1);
		} else {
			inst->SetCharges(charges);
		}

		if (item->RecastDelay) {
			if (item->RecastType != RECAST_TYPE_UNLINKED_ITEM && timestamps.count(item->RecastType)) {
				inst->SetRecastTimestamp(timestamps.at(item->RecastType));
			} else if (item->RecastType == RECAST_TYPE_UNLINKED_ITEM && timestamps.count(item->ID)) {
				inst->SetRecastTimestamp(timestamps.at(item->ID));
			} else {
				inst->SetRecastTimestamp(0);
			}
		}

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (augment_ids[i]) {
					inst->PutAugment(this, i, augment_ids[i]);
				}
			}
		}

		if (item->EvolvingItem) {
			if (slot_id >= EQ::invslot::EQUIPMENT_BEGIN && slot_id <= EQ::invslot::EQUIPMENT_END) {
				inst->SetEvolveEquipped(true);
			}

			auto t = std::ranges::find_if(
				e_results.cbegin(),
				e_results.cend(),
				[&](const CharacterEvolvingItemsRepository::CharacterEvolvingItems &x) {
					return x.item_id == item_id;
				}
			);

			if (t == std::end(e_results)) {
				auto e = CharacterEvolvingItemsRepository::NewEntity();

				e.character_id  = char_id;
				e.item_id       = item_id;
				e.equipped      = inst->GetEvolveEquipped();
				e.final_item_id = evolving_items_manager.GetFinalItemID(*inst);

				auto r = CharacterEvolvingItemsRepository::InsertOne(*this, e);
				e.id = r.id;
				e_results.push_back(e);

				inst->SetEvolveUniqueID(e.id);
				inst->SetEvolveCharID(e.character_id);
				inst->SetEvolveItemID(e.item_id);
				inst->SetEvolveActivated(e.activated);
				inst->SetEvolveEquipped(e.equipped);
				inst->SetEvolveCurrentAmount(e.current_amount);
				inst->CalculateEvolveProgression();
				inst->SetEvolveFinalItemID(e.final_item_id);
			}
			else {
				inst->SetEvolveUniqueID(t->id);
				inst->SetEvolveCharID(t->character_id);
				inst->SetEvolveItemID(t->item_id);
				inst->SetEvolveActivated(t->activated);
				inst->SetEvolveEquipped(t->equipped);
				inst->SetEvolveCurrentAmount(t->current_amount);
				inst->CalculateEvolveProgression();
				inst->SetEvolveFinalItemID(t->final_item_id);
			}
		}

		int16 put_slot_id;
		// this had  || slot_id == EQ::invslot::slotCursor ??s
		if (EQ::ValueWithin(slot_id, EQ::invbag::CURSOR_BAG_BEGIN, EQ::invbag::CURSOR_BAG_END)) {
			put_slot_id = inv.PushCursor(*inst);
		} else {
			put_slot_id = inv.PutItem(slot_id, *inst);
		}

		row.guid = inst->GetSerialNumber();
		queue.push_back(row);

		safe_delete(inst);

		// Save ptr to item in inventory
		if (put_slot_id == INVALID_INDEX) {
			LogError(
				"Warning: Invalid slot_id for item in inventory for character_id [{}] item_id [{}] slot_id [{}]",
				char_id,
				item_id,
				slot_id
			);
		}
	}

	if (cv_conflict) {
		const std::string& char_name = GetCharName(char_id);
		LogError(
			"ClientVersion/Expansion conflict during inventory load at zone entry for [{}] (charid: [{}], inver: [{}], gmi: [{}])",
			char_name,
			char_id,
			EQ::versions::MobVersionName(inv.InventoryVersion()),
			(inv.GMInventory() ? "true" : "false")
		);
	}

	if (!queue.empty()) {
		InventoryRepository::ReplaceMany(*this, queue);
	}

	EQ::ItemInstance::ClearGUIDMap();

	// Retrieve shared inventory
	return GetSharedBank(char_id, &inv, true);
}

std::map<uint32, uint32> SharedDatabase::GetItemRecastTimestamps(uint32 char_id)
{
	std::map<uint32, uint32> timers;

	const auto& l = CharacterItemRecastRepository::GetWhere(
		*this,
		fmt::format(
			"`id` = {}",
			char_id
		)
	);

	if (l.empty()) {
		return timers;
	}

	for (const auto& e : l) {
		timers[e.recast_type] = e.timestamp;
	}

	return timers;
}

uint32 SharedDatabase::GetItemRecastTimestamp(uint32 char_id, uint32 recast_type)
{
	const auto& l = CharacterItemRecastRepository::GetWhere(
		*this,
		fmt::format(
			"`id` = {} AND `recast_type` = {}",
			char_id,
			recast_type
		)
	);

	return l.empty() ? 0 : l[0].timestamp;
}

void SharedDatabase::ClearOldRecastTimestamps(uint32 char_id)
{
	CharacterItemRecastRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `timestamp` < UNIX_TIMESTAMP()",
			char_id
		)
	);
}

void SharedDatabase::GetItemsCount(int32 &item_count, uint32 &max_id)
{
	item_count = -1;
	max_id = 0;

	const std::string query = "SELECT MAX(id), count(*) FROM items";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	if (results.RowCount() == 0)
		return;

	auto& row = results.begin();

	if (row[0])
		max_id = Strings::ToUnsignedInt(row[0]);

	if (row[1])
		item_count = Strings::ToUnsignedInt(row[1]);
}

bool SharedDatabase::LoadItems(const std::string &prefix) {
	items_mmf.reset(nullptr);

	try {
		const auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("items");
		mutex.Lock();
		std::string file_name = fmt::format("{}/{}{}", path.GetSharedMemoryPath(), prefix, std::string("items"));
		items_mmf = std::make_unique<EQ::MemoryMappedFile>(file_name);
		items_hash = std::make_unique<EQ::FixedMemoryHashSet<EQ::ItemData>>(static_cast<uint8*>(items_mmf->Get()), items_mmf->Size());
		mutex.Unlock();

		LogInfo("Loaded [{}] items via shared memory", Strings::Commify(m_shared_items_count));
	} catch(std::exception& ex) {
		LogError("Error Loading Items: {}", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadItems(void *data, uint32 size, int32 items, uint32 max_item_id)
{
	EQ::FixedMemoryHashSet<EQ::ItemData> hash(static_cast<uint8 *>(data), size, items, max_item_id);

	std::string variable_buffer;

	bool disable_attuneable = RuleB(Items, DisableAttuneable);
	bool disable_bard_focus_effects = RuleB(Items, DisableBardFocusEffects);
	bool disable_lore = RuleB(Items, DisableLore);
	bool disable_no_drop = RuleB(Items, DisableNoDrop);
	bool disable_no_pet = RuleB(Items, DisableNoPet);
	bool disable_no_rent = RuleB(Items, DisableNoRent);
	bool disable_no_transfer = RuleB(Items, DisableNoTransfer);
	bool disable_potion_belt = RuleB(Items, DisablePotionBelt);
	bool disable_spell_focus_effects = RuleB(Items, DisableSpellFocusEffects);

	// Old Variable Code
	if (GetVariable("disablelore", variable_buffer)) {
		if (variable_buffer == "1") {
			disable_lore = true;
		}
	}

	if (GetVariable("disablenodrop", variable_buffer)) {
		if (variable_buffer == "1") {
			disable_no_drop = true;
		}
	}

	if (GetVariable("disablenorent", variable_buffer)) {
		if (variable_buffer == "1") {
			disable_no_rent = true;
		}
	}


	if (GetVariable("disablenotransfer", variable_buffer)) {
		if (variable_buffer == "1") {
			disable_no_transfer = true;
		}
	}

	EQ::ItemData item;

	const std::string query = "SELECT source,"
#define F(x) "`"#x"`,"
#include "item_fieldlist.h"
#undef F
		"updated, minstatus, comment FROM items ORDER BY id";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto& row = results.begin(); row != results.end(); ++row) {
		memset(&item, 0, sizeof(EQ::ItemData));

		// Unique Identifier
		item.ID = Strings::ToUnsignedInt(row[ItemField::id]);

		// Minimum Status
		item.MinStatus = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::minstatus]));

		// Name, Lore, and Comment
		strn0cpy(item.Name, row[ItemField::name], sizeof(item.Name));
		strn0cpy(item.Lore, row[ItemField::lore], sizeof(item.Lore));
		strn0cpy(item.Comment, row[ItemField::comment], sizeof(item.Comment));

		// Flags
		item.ArtifactFlag = Strings::ToBool(row[ItemField::artifactflag]);
		item.Attuneable = !disable_attuneable && Strings::ToBool(row[ItemField::attuneable]);
		item.BenefitFlag = Strings::ToBool(row[ItemField::benefitflag]);
		item.FVNoDrop = Strings::ToBool(row[ItemField::fvnodrop]);
		item.Magic = Strings::ToBool(row[ItemField::magic]);
		item.NoDrop = disable_no_drop ? static_cast<uint8>(255) : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::nodrop]));
		item.NoPet = !disable_no_pet && Strings::ToBool(row[ItemField::nopet]);
		item.NoRent = disable_no_rent ? static_cast<uint8>(255) : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::norent]));
		item.NoTransfer = !disable_no_transfer && Strings::ToBool(row[ItemField::notransfer]);
		item.PendingLoreFlag = Strings::ToBool(row[ItemField::pendingloreflag]);
		item.QuestItemFlag = Strings::ToBool(row[ItemField::questitemflag]);
		item.Stackable = Strings::ToBool(row[ItemField::stackable]);
		item.Tradeskills = Strings::ToBool(row[ItemField::tradeskills]);
		item.SummonedFlag = Strings::ToBool(row[ItemField::summonedflag]);

		// Lore
		item.LoreGroup = disable_lore ? 0 : Strings::ToInt(row[ItemField::loregroup]);
		item.LoreFlag = !disable_lore && item.LoreGroup != 0;

		// Type
		item.AugType = Strings::ToUnsignedInt(row[ItemField::augtype]);
		item.ItemType = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::itemtype]));
		item.SubType = Strings::ToInt(row[ItemField::subtype]);

		// Miscellaneous
		item.ExpendableArrow = static_cast<uint16>(Strings::ToUnsignedInt(row[ItemField::expendablearrow]));
		item.Light = static_cast<int8>(Strings::ToInt(row[ItemField::light]));
		item.MaxCharges = static_cast<int16>(Strings::ToInt(row[ItemField::maxcharges]));
		item.Size = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::size]));
		item.StackSize = static_cast<int16>(Strings::ToInt(row[ItemField::stacksize]));
		item.Weight = Strings::ToInt(row[ItemField::weight]);

		// Potion Belt
		item.PotionBelt = !disable_potion_belt && Strings::ToBool(row[ItemField::potionbelt]);
		item.PotionBeltSlots = disable_potion_belt ? 0 : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::potionbeltslots]));

		// Merchant
		item.Favor = Strings::ToUnsignedInt(row[ItemField::favor]);
		item.GuildFavor = Strings::ToUnsignedInt(row[ItemField::guildfavor]);
		item.Price = Strings::ToUnsignedInt(row[ItemField::price]);
		item.SellRate = Strings::ToFloat(row[ItemField::sellrate]);

		// Display
		item.Color = Strings::ToUnsignedInt(row[ItemField::color]);
		item.EliteMaterial = Strings::ToUnsignedInt(row[ItemField::elitematerial]);
		item.HerosForgeModel = Strings::ToUnsignedInt(row[ItemField::herosforgemodel]);
		item.Icon = Strings::ToUnsignedInt(row[ItemField::icon]);
		strn0cpy(item.IDFile, row[ItemField::idfile], sizeof(item.IDFile));
		item.Material = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::material]));

		// Resists
		item.CR = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::cr]), -128, 127));
		item.DR = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::dr]), -128, 127));
		item.FR = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::fr]), -128, 127));
		item.MR = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::mr]), -128, 127));
		item.PR = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::pr]), -128, 127));
		item.SVCorruption = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::svcorruption]), -128, 127));

		// Heroic Resists
		item.HeroicCR = Strings::ToInt(row[ItemField::heroic_cr]);
		item.HeroicDR = Strings::ToInt(row[ItemField::heroic_dr]);
		item.HeroicFR = Strings::ToInt(row[ItemField::heroic_fr]);
		item.HeroicMR = Strings::ToInt(row[ItemField::heroic_mr]);
		item.HeroicPR = Strings::ToInt(row[ItemField::heroic_pr]);
		item.HeroicSVCorrup = Strings::ToInt(row[ItemField::heroic_svcorrup]);

		// Stats
		item.AAgi = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::aagi]), -128, 127));
		item.ACha = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::acha]), -128, 127));
		item.ADex = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::adex]), -128, 127));
		item.AInt = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::aint]), -128, 127));
		item.ASta = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::asta]), -128, 127));
		item.AStr = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::astr]), -128, 127));
		item.AWis = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::awis]), -128, 127));

		// Heroic Stats
		item.HeroicAgi = Strings::ToInt(row[ItemField::heroic_agi]);
		item.HeroicCha = Strings::ToInt(row[ItemField::heroic_cha]);
		item.HeroicDex = Strings::ToInt(row[ItemField::heroic_dex]);
		item.HeroicInt = Strings::ToInt(row[ItemField::heroic_int]);
		item.HeroicSta = Strings::ToInt(row[ItemField::heroic_sta]);
		item.HeroicStr = Strings::ToInt(row[ItemField::heroic_str]);
		item.HeroicWis = Strings::ToInt(row[ItemField::heroic_wis]);

		// Health, Mana, and Endurance
		item.HP = Strings::ToInt(row[ItemField::hp]);
		item.Regen = Strings::ToInt(row[ItemField::regen]);
		item.Mana = Strings::ToInt(row[ItemField::mana]);
		item.ManaRegen = Strings::ToInt(row[ItemField::manaregen]);
		item.Endur = Strings::ToInt(row[ItemField::endur]);
		item.EnduranceRegen = Strings::ToInt(row[ItemField::enduranceregen]);

		// Bane Damage
		item.BaneDmgAmt = Strings::ToInt(row[ItemField::banedmgamt]);
		item.BaneDmgBody = Strings::ToUnsignedInt(row[ItemField::banedmgbody]);
		item.BaneDmgRace = Strings::ToUnsignedInt(row[ItemField::banedmgrace]);
		item.BaneDmgRaceAmt = Strings::ToUnsignedInt(row[ItemField::banedmgraceamt]);

		// Elemental Damage
		item.ElemDmgType = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::elemdmgtype]));
		item.ElemDmgAmt = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::elemdmgamt]));

		// Combat
		item.BackstabDmg = Strings::ToUnsignedInt(row[ItemField::backstabdmg]);
		item.Damage = Strings::ToUnsignedInt(row[ItemField::damage]);
		item.Delay = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::delay]));
		item.Range = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::range]));

		// Combat Stats
		item.AC = Strings::ToInt(row[ItemField::ac]);
		item.Accuracy = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::accuracy]), -128, 127));
		item.Attack = Strings::ToInt(row[ItemField::attack]);
		item.Avoidance = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::avoidance]), -128, 127));
		item.Clairvoyance = Strings::ToUnsignedInt(row[ItemField::clairvoyance]);
		item.CombatEffects = Strings::IsNumber(row[ItemField::combateffects]) ? static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::combateffects]), -128, 127)) : 0;
		item.DamageShield = Strings::ToInt(row[ItemField::damageshield]);
		item.DotShielding = Strings::ToInt(row[ItemField::dotshielding]);
		item.DSMitigation = Strings::ToUnsignedInt(row[ItemField::dsmitigation]);
		item.Haste = Strings::ToInt(row[ItemField::haste]);
		item.HealAmt = Strings::ToInt(row[ItemField::healamt]);
		item.Purity = Strings::ToUnsignedInt(row[ItemField::purity]);
		item.Shielding = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::shielding]), -128, 127));
		item.SpellDmg = Strings::ToInt(row[ItemField::spelldmg]);
		item.SpellShield = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::spellshield]), -128, 127));
		item.StrikeThrough = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::strikethrough]), -128, 127));
		item.StunResist = static_cast<int8>(EQ::Clamp(Strings::ToInt(row[ItemField::stunresist]), -128, 127));

		// Restrictions
		item.AugRestrict = Strings::ToUnsignedInt(row[ItemField::augrestrict]);
		item.Classes = Strings::ToUnsignedInt(row[ItemField::classes]);
		item.Deity = Strings::ToUnsignedInt(row[ItemField::deity]);
		item.ItemClass = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::itemclass]));
		item.Races = Strings::ToUnsignedInt(row[ItemField::races]);
		item.RecLevel = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::reclevel]));
		item.RecSkill = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::recskill]));
		item.ReqLevel = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::reqlevel]));
		item.Slots = Strings::ToUnsignedInt(row[ItemField::slots]);

		// Skill Modifier
		item.SkillModValue = Strings::ToInt(row[ItemField::skillmodvalue]);
		item.SkillModMax = Strings::ToInt(row[ItemField::skillmodmax]);
		item.SkillModType = Strings::ToUnsignedInt(row[ItemField::skillmodtype]);

		// Extra Damage Skill
		item.ExtraDmgSkill = Strings::ToInt(row[ItemField::extradmgskill]);
		item.ExtraDmgAmt = Strings::ToInt(row[ItemField::extradmgamt]);

		// Bard
		item.BardType = Strings::ToUnsignedInt(row[ItemField::bardtype]);
		item.BardValue = Strings::ToInt(row[ItemField::bardvalue]);

		// Faction
		item.FactionAmt1 = Strings::ToInt(row[ItemField::factionamt1]);
		item.FactionMod1 = Strings::ToInt(row[ItemField::factionmod1]);
		item.FactionAmt2 = Strings::ToInt(row[ItemField::factionamt2]);
		item.FactionMod2 = Strings::ToInt(row[ItemField::factionmod2]);
		item.FactionAmt3 = Strings::ToInt(row[ItemField::factionamt3]);
		item.FactionMod3 = Strings::ToInt(row[ItemField::factionmod3]);
		item.FactionAmt4 = Strings::ToInt(row[ItemField::factionamt4]);
		item.FactionMod4 = Strings::ToInt(row[ItemField::factionmod4]);

		// Augment
		item.AugDistiller = Strings::ToUnsignedInt(row[ItemField::augdistiller]);
		item.AugSlotType[0] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot1type]));
		item.AugSlotVisible[0] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot1visible]));
		item.AugSlotType[1] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot2type]));
		item.AugSlotVisible[1] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot2visible]));
		item.AugSlotType[2] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot3type]));
		item.AugSlotVisible[2] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot3visible]));
		item.AugSlotType[3] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot4type]));
		item.AugSlotVisible[3] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot4visible]));
		item.AugSlotType[4] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot5type]));
		item.AugSlotVisible[4] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot5visible]));
		item.AugSlotType[5] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot6type]));
		item.AugSlotVisible[5] = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::augslot6visible]));

		// Augment Unknowns
		for (uint8 i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
			item.AugSlotUnk2[i] = 0;
		}

		// LDoN
		item.LDoNTheme = Strings::ToUnsignedInt(row[ItemField::ldontheme]);
		item.LDoNPrice = Strings::ToUnsignedInt(row[ItemField::ldonprice]);
		item.LDoNSellBackRate = Strings::ToUnsignedInt(row[ItemField::ldonsellbackrate]);
		item.LDoNSold = Strings::ToUnsignedInt(row[ItemField::ldonsold]);
		item.PointType = Strings::ToUnsignedInt(row[ItemField::pointtype]);

		// Bag
		item.BagSize = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::bagsize]));
		item.BagSlots = static_cast<uint8>(EQ::Clamp(Strings::ToInt(row[ItemField::bagslots]), 0, static_cast<int>(EQ::invbag::SLOT_COUNT)));
		item.BagType = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::bagtype]));
		item.BagWR = static_cast<uint8>(EQ::Clamp(Strings::ToInt(row[ItemField::bagwr]), 0, 100));

		// Bard Effect
		item.Bard.Effect = disable_bard_focus_effects ? 0 : Strings::ToInt(row[ItemField::bardeffect]);
		item.Bard.Type = disable_bard_focus_effects ? 0 : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::bardtype]));
		item.Bard.Level = disable_bard_focus_effects ? 0 : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::bardlevel]));
		item.Bard.Level2 = disable_bard_focus_effects ? 0 : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::bardlevel2]));

		// Book
		item.Book = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::book]));
		item.BookType = Strings::ToUnsignedInt(row[ItemField::booktype]);

		// Click Effect
		item.CastTime = Strings::ToUnsignedInt(row[ItemField::casttime]);
		item.CastTime_ = Strings::ToInt(row[ItemField::casttime_]);
		item.Click.Effect = Strings::ToInt(row[ItemField::clickeffect]);
		item.Click.Type = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::clicktype]));
		item.Click.Level = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::clicklevel]));
		item.Click.Level2 = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::clicklevel2]));
		strn0cpy(item.ClickName, row[ItemField::clickname], sizeof(item.ClickName));
		item.RecastDelay = Strings::ToUnsignedInt(row[ItemField::recastdelay]);
		item.RecastType = Strings::ToInt(row[ItemField::recasttype]);

		// Focus Effect
		item.Focus.Effect = disable_spell_focus_effects ? 0 : Strings::ToInt(row[ItemField::focuseffect]);
		item.Focus.Type = disable_spell_focus_effects ? 0 : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::focustype]));
		item.Focus.Level = disable_spell_focus_effects ? 0 : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::focuslevel]));
		item.Focus.Level2 = disable_spell_focus_effects ? 0 : static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::focuslevel2]));
		strn0cpy(item.FocusName, disable_spell_focus_effects ? "" : row[ItemField::focusname], sizeof(item.FocusName));

		// Proc Effect
		item.Proc.Effect = Strings::ToInt(row[ItemField::proceffect]);
		item.Proc.Type = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::proctype]));
		item.Proc.Level = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::proclevel]));
		item.Proc.Level2 = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::proclevel2]));
		strn0cpy(item.ProcName, row[ItemField::procname], sizeof(item.ProcName));
		item.ProcRate = Strings::ToInt(row[ItemField::procrate]);

		// Scroll Effect
		item.Scroll.Effect = Strings::ToInt(row[ItemField::scrolleffect]);
		item.Scroll.Type = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::scrolltype]));
		item.Scroll.Level = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::scrolllevel]));
		item.Scroll.Level2 = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::scrolllevel2]));
		strn0cpy(item.ScrollName, row[ItemField::scrollname], sizeof(item.ScrollName));

		// Worn Effect
		item.Worn.Effect = Strings::ToInt(row[ItemField::worneffect]);
		item.Worn.Type = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::worntype]));
		item.Worn.Level = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::wornlevel]));
		item.Worn.Level2 = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::wornlevel2]));
		strn0cpy(item.WornName, row[ItemField::wornname], sizeof(item.WornName));

		// Evolving Item
		item.EvolvingID = Strings::ToUnsignedInt(row[ItemField::evoid]);
		item.EvolvingItem = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::evoitem]));
		item.EvolvingLevel = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::evolvinglevel]));
		item.EvolvingMax = static_cast<uint8>(Strings::ToUnsignedInt(row[ItemField::evomax]));

		// Scripting
		item.CharmFileID = Strings::IsNumber(row[ItemField::charmfileid]) ? Strings::ToUnsignedInt(row[ItemField::charmfileid]) : 0;
		strn0cpy(item.CharmFile, row[ItemField::charmfile], sizeof(item.CharmFile));
		strn0cpy(item.Filename, row[ItemField::filename], sizeof(item.Filename));
		item.ScriptFileID = Strings::ToUnsignedInt(row[ItemField::scriptfileid]);

		try {
			hash.insert(item.ID, item);
		} catch (std::exception &ex) {
			LogError("Database::LoadItems: {}", ex.what());
			break;
		}
	}
}

const EQ::ItemData *SharedDatabase::GetItem(uint32 id) const
{
	if (id == 0) {
		return nullptr;
	}

	if (!items_hash || id > items_hash->max_key()) {
		return nullptr;
	}

	if (items_hash->exists(id)) {
		return &(items_hash->at(id));
	}

	return nullptr;
}

const EQ::ItemData* SharedDatabase::IterateItems(uint32* id) const
{
	if(!items_hash || !id) {
		return nullptr;
	}

	for(;;) {
		if(*id > items_hash->max_key()) {
			break;
		}

		if(items_hash->exists(*id)) {
			return &(items_hash->at((*id)++));
		} else {
			++(*id);
		}
	}

	return nullptr;
}

Book_Struct SharedDatabase::GetBook(const std::string& text_file)
{
	const auto& l = BooksRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(text_file)
		)
	);

	Book_Struct b;

	if (l.empty()) {
		return b;
	}

	const auto& e = l.front();

	b.language = e.language;
	b.text     = e.txtfile;

	return b;
}

// Create appropriate EQ::ItemInstance class
EQ::ItemInstance* SharedDatabase::CreateItem(
	uint32 item_id,
	int16 charges,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6,
	bool attuned,
	const std::string& custom_data,
	uint32 ornamenticon,
	uint32 ornamentidfile,
	uint32 ornament_hero_model
) {
	EQ::ItemInstance* inst = nullptr;

	const EQ::ItemData* item = GetItem(item_id);
	if (item) {
		inst = CreateBaseItem(item, charges);

		if (!inst) {
			LogError("Error: valid item data returned a null reference for EQ::ItemInstance creation in SharedDatabase::CreateItem()");
			LogError("Item Data = ID: {}, Name: {}, Charges: {}", item->ID, item->Name, charges);
			return nullptr;
		}

		inst->PutAugment(this, 0, aug1);
		inst->PutAugment(this, 1, aug2);
		inst->PutAugment(this, 2, aug3);
		inst->PutAugment(this, 3, aug4);
		inst->PutAugment(this, 4, aug5);
		inst->PutAugment(this, 5, aug6);
		inst->SetAttuned(attuned);
		inst->SetCustomDataString(custom_data);
		inst->SetOrnamentIcon(ornamenticon);
		inst->SetOrnamentationIDFile(ornamentidfile);
		inst->SetOrnamentHeroModel(ornament_hero_model);
	}

	return inst;
}


// Create appropriate EQ::ItemInstance class
EQ::ItemInstance* SharedDatabase::CreateItem(
	const EQ::ItemData* item,
	int16 charges,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6,
	bool attuned,
	const std::string& custom_data,
	uint32 ornamenticon,
	uint32 ornamentidfile,
	uint32 ornament_hero_model
) {
	EQ::ItemInstance* inst = nullptr;
	if (item) {
		inst = CreateBaseItem(item, charges);

		if (!inst) {
			LogError("Error: valid item data returned a null reference for EQ::ItemInstance creation in SharedDatabase::CreateItem()");
			LogError("Item Data = ID: {}, Name: {}, Charges: {}", item->ID, item->Name, charges);
			return nullptr;
		}

		inst->PutAugment(this, 0, aug1);
		inst->PutAugment(this, 1, aug2);
		inst->PutAugment(this, 2, aug3);
		inst->PutAugment(this, 3, aug4);
		inst->PutAugment(this, 4, aug5);
		inst->PutAugment(this, 5, aug6);
		inst->SetAttuned(attuned);
		inst->SetCustomDataString(custom_data);
		inst->SetOrnamentIcon(ornamenticon);
		inst->SetOrnamentationIDFile(ornamentidfile);
		inst->SetOrnamentHeroModel(ornament_hero_model);
	}

	return inst;
}

EQ::ItemInstance* SharedDatabase::CreateBaseItem(const EQ::ItemData* item, int16 charges) {
	EQ::ItemInstance* inst = nullptr;
	if (item) {
		// if maxcharges is -1 that means it is an unlimited use item.
		// set it to 1 charge so that it is usable on creation
		if (charges == 0 && item->MaxCharges == -1) {
			charges = 1;
		}

		// Stackable items need a minimum charge of 1 to remain moveable.
		if (charges <= 0 && item->Stackable) {
			charges = 1;
		}

		inst = new EQ::ItemInstance(item, charges);

		if (!inst) {
			LogError("Error: valid item data returned a null reference for EQ::ItemInstance creation in SharedDatabase::CreateBaseItem()");
			LogError("Item Data = ID: {}, Name: {}, Charges: {}", item->ID, item->Name, charges);
			return nullptr;
		}

		if (item->CharmFileID != 0 || item->LoreGroup >= 1000) {
			inst->Initialize(this);
		}
	}

	return inst;
}

int SharedDatabase::DeleteStalePlayerCorpses() {
	return (
		RuleB(Zone, EnableShadowrest) ?
		CharacterCorpsesRepository::BuryDecayedCorpses(*this) :
		CharacterCorpsesRepository::DeleteWhere(
			*this,
			fmt::format(
				"(UNIX_TIMESTAMP() - UNIX_TIMESTAMP(time_of_death)) > {} AND time_of_death != 0",
				RuleI(Character, CorpseDecayTime) / 1000
			)
		)
	);
}

bool SharedDatabase::GetCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &command_settings)
{
	command_settings.clear();

	const std::string& query = "SELECT `command`, `access`, `aliases` FROM `command_settings`";
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return false;
	}

	for (auto row : results) {
		command_settings[row[0]].first = Strings::ToUnsignedInt(row[1]);
		if (row[2][0] == 0) {
			continue;
		}

		std::vector<std::string> aliases = Strings::Split(row[2], '|');
		for (const auto& e : aliases) {
			if (e.empty()) {
				continue;
			}

			command_settings[row[0]].second.push_back(e);
		}
	}

	return true;
}

bool SharedDatabase::UpdateInjectedCommandSettings(const std::vector<std::pair<std::string, uint8>> &injected)
{
	if (injected.size()) {
		const std::string query = fmt::format(
			"REPLACE INTO `command_settings`(`command`, `access`) VALUES {}",
			Strings::ImplodePair(
				",",
				std::pair<char, char>('(', ')'),
				join_pair(",", std::pair<char, char>('\'', '\''), injected)
			)
		);

		auto results = QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		LogInfo(
			"[{}] New Command{} Added",
			injected.size(),
			injected.size() != 1 ? "s" : ""
		);
	}

	return true;
}

bool SharedDatabase::UpdateOrphanedCommandSettings(const std::vector<std::string> &orphaned)
{
	if (orphaned.size()) {
		std::string query = fmt::format(
			"DELETE FROM `command_settings` WHERE `command` IN ({})",
			Strings::ImplodePair(",", std::pair<char, char>('\'', '\''), orphaned)
		);

		auto results = QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		query = fmt::format(
			"DELETE FROM `command_subsettings` WHERE `parent_command` IN ({})",
			Strings::ImplodePair(",", std::pair<char, char>('\'', '\''), orphaned)
		);

		auto results_two = QueryDatabase(query);
		if (!results_two.Success()) {
			return false;
		}

		LogInfo(
			"{} Orphaned Command{} Deleted | {} Orphaned Subcommand{} Deleted",
			orphaned.size(),
			orphaned.size() != 1 ? "s" : "",
			results_two.RowsAffected(),
			results_two.RowsAffected() != 1 ? "s" : ""
		);
	}

	return true;
}

bool SharedDatabase::GetCommandSubSettings(std::vector<CommandSubsettingsRepository::CommandSubsettings> &command_subsettings)
{
	command_subsettings.clear();

	const auto& l = CommandSubsettingsRepository::GetAll(*this);

	if (l.empty()) {
		return false;
	}

	command_subsettings.reserve(l.size());

	for (const auto& e : l) {
		command_subsettings.emplace_back(e);
	}

	return true;
}

void SharedDatabase::LoadDamageShieldTypes(SPDat_Spell_Struct* sp, int32 iMaxSpellID) {
	const std::string query = StringFormat("SELECT `spellid`, `type` FROM `damageshieldtypes` WHERE `spellid` > 0 "
	                                       "AND `spellid` <= %i", iMaxSpellID);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    for(auto& row = results.begin(); row != results.end(); ++row) {
	    const int spellID = Strings::ToInt(row[0]);
        if((spellID > 0) && (spellID <= iMaxSpellID))
            sp[spellID].damage_shield_type = Strings::ToUnsignedInt(row[1]);
    }

}

const EvolveInfo* SharedDatabase::GetEvolveInfo(uint32 loregroup) {
	return nullptr;	// nothing here for now... database and/or sharemem pulls later
}

int SharedDatabase::GetMaxSpellID() {
	const std::string query = "SELECT MAX(id) FROM spells_new";
	auto results = QueryDatabase(query);
    if (!results.Success()) {
        return -1;
    }

    auto& row = results.begin();

	return Strings::ToInt(row[0]);
}

bool SharedDatabase::LoadSpells(const std::string &prefix, int32 *records, const SPDat_Spell_Struct **sp) {
	spells_mmf.reset(nullptr);

	try {
		const auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("spells");
		mutex.Lock();

		std::string file_name = fmt::format("{}/{}{}", path.GetSharedMemoryPath(), prefix, std::string("spells"));
		spells_mmf = std::make_unique<EQ::MemoryMappedFile>(file_name);
		LogInfo("Loading [{}]", file_name);
		*records = *static_cast<uint32*>(spells_mmf->Get());
		*sp = reinterpret_cast<const SPDat_Spell_Struct*>(static_cast<char*>(spells_mmf->Get()) + 4);
		mutex.Unlock();

		LogInfo("Loaded [{}] spells via shared memory", Strings::Commify(m_shared_spells_count));
	}
	catch(std::exception& ex) {
		LogError("Error Loading Spells: {}", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadSpells(void *data, int max_spells) {
	*static_cast<uint32*>(data) = max_spells;
	SPDat_Spell_Struct *sp = reinterpret_cast<SPDat_Spell_Struct*>(static_cast<char*>(data) + sizeof(uint32));

	const std::string query = "SELECT * FROM spells_new ORDER BY id ASC";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    if(results.ColumnCount() <= SPELL_LOAD_FIELD_COUNT) {
		LogSpells("Fatal error loading spells: Spell field count < SPELL_LOAD_FIELD_COUNT([{}])", SPELL_LOAD_FIELD_COUNT);
		return;
    }

	int counter = 0;

    for (auto& row = results.begin(); row != results.end(); ++row) {
	    const int tempid = Strings::ToInt(row[0]);
        if(tempid >= max_spells) {
			LogSpells("Non fatal error: spell.id >= max_spells, ignoring");
			continue;
		}

        ++counter;
        sp[tempid].id = tempid;
        strn0cpy(sp[tempid].name, row[1], sizeof(sp[tempid].name));
        strn0cpy(sp[tempid].player_1, row[2], sizeof(sp[tempid].player_1));
		strn0cpy(sp[tempid].teleport_zone, row[3], sizeof(sp[tempid].teleport_zone));
		strn0cpy(sp[tempid].you_cast, row[4], sizeof(sp[tempid].you_cast));
		strn0cpy(sp[tempid].other_casts, row[5], sizeof(sp[tempid].other_casts));
		strn0cpy(sp[tempid].cast_on_you, row[6], sizeof(sp[tempid].cast_on_you));
		strn0cpy(sp[tempid].cast_on_other, row[7], sizeof(sp[tempid].cast_on_other));
		strn0cpy(sp[tempid].spell_fades, row[8], sizeof(sp[tempid].spell_fades));

		sp[tempid].range = Strings::ToFloat(row[9]);
		sp[tempid].aoe_range = Strings::ToFloat(row[10]);
		sp[tempid].push_back = Strings::ToFloat(row[11]);
		sp[tempid].push_up = Strings::ToFloat(row[12]);
		sp[tempid].cast_time=Strings::ToUnsignedInt(row[13]);
		sp[tempid].recovery_time=Strings::ToUnsignedInt(row[14]);
		sp[tempid].recast_time=Strings::ToUnsignedInt(row[15]);
		sp[tempid].buff_duration_formula=Strings::ToUnsignedInt(row[16]);
		sp[tempid].buff_duration=Strings::ToUnsignedInt(row[17]);
		sp[tempid].aoe_duration=Strings::ToUnsignedInt(row[18]);
		sp[tempid].mana=Strings::ToInt(row[19]);

		int y=0;
		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].base_value[y]=Strings::ToInt(row[20+y]); // effect_base_value

		for(y=0; y < EFFECT_COUNT; y++)
			sp[tempid].limit_value[y]=Strings::ToInt(row[32+y]); // effect_limit_value

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].max_value[y]=Strings::ToInt(row[44+y]);

		for(y=0; y< 4;y++)
			sp[tempid].component[y]=Strings::ToInt(row[58+y]);

		for(y=0; y< 4;y++)
			sp[tempid].component_count[y]=Strings::ToInt(row[62+y]);

		for(y=0; y< 4;y++)
			sp[tempid].no_expend_reagent[y]=Strings::ToInt(row[66+y]);

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].formula[y]=Strings::ToUnsignedInt(row[70+y]);

		sp[tempid].good_effect=Strings::ToInt(row[83]);
		sp[tempid].activated=Strings::ToInt(row[84]);
		sp[tempid].resist_type=Strings::ToInt(row[85]);

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].effect_id[y]=Strings::ToInt(row[86+y]);

		sp[tempid].target_type = static_cast<SpellTargetType>(Strings::ToInt(row[98]));
		sp[tempid].base_difficulty=Strings::ToInt(row[99]);

		int tmp_skill = Strings::ToInt(row[100]);

		if (tmp_skill < 0 || tmp_skill > EQ::skills::HIGHEST_SKILL)
			sp[tempid].skill = EQ::skills::SkillBegging; /* not much better we can do. */ // can probably be changed to client-based 'SkillNone' once activated
        else
			sp[tempid].skill = static_cast<EQ::skills::SkillType>(tmp_skill);

		sp[tempid].zone_type=Strings::ToInt(row[101]);
		sp[tempid].environment_type=Strings::ToInt(row[102]);
		sp[tempid].time_of_day=Strings::ToInt(row[103]);

		for(y=0; y < Class::PLAYER_CLASS_COUNT;y++)
			sp[tempid].classes[y]=Strings::ToInt(row[104+y]);

		sp[tempid].casting_animation=Strings::ToInt(row[120]);
		sp[tempid].spell_affect_index=Strings::ToInt(row[123]);
		sp[tempid].disallow_sit=Strings::ToInt(row[124]);
		sp[tempid].deity_agnostic=Strings::ToInt(row[125]);

		for (y = 0; y < 16; y++)
			sp[tempid].deities[y]=Strings::ToInt(row[126+y]);

		sp[tempid].new_icon=Strings::ToInt(row[144]);
		sp[tempid].uninterruptable=Strings::ToBool(row[146]);
		sp[tempid].resist_difficulty=Strings::ToInt(row[147]);
		sp[tempid].unstackable_dot = Strings::ToBool(row[148]);
		sp[tempid].recourse_link = Strings::ToUnsignedInt(row[150]);
		sp[tempid].no_partial_resist = Strings::ToBool(row[151]);

		sp[tempid].short_buff_box = Strings::ToInt(row[154]);
		sp[tempid].description_id = Strings::ToInt(row[155]);
		sp[tempid].type_description_id = Strings::ToInt(row[156]);
		sp[tempid].effect_description_id = Strings::ToInt(row[157]);

		sp[tempid].npc_no_los = Strings::ToBool(row[159]);
		sp[tempid].feedbackable = Strings::ToBool(row[160]);
		sp[tempid].reflectable = Strings::ToBool(row[161]);
		sp[tempid].bonus_hate=Strings::ToInt(row[162]);

		sp[tempid].ldon_trap = Strings::ToBool(row[165]);
		sp[tempid].endurance_cost= Strings::ToInt(row[166]);
		sp[tempid].timer_id= Strings::ToInt(row[167]);
		sp[tempid].is_discipline = Strings::ToBool(row[168]);
		sp[tempid].hate_added= Strings::ToInt(row[173]);
		sp[tempid].endurance_upkeep=Strings::ToInt(row[174]);
		sp[tempid].hit_number_type = Strings::ToInt(row[175]);
		sp[tempid].hit_number = Strings::ToInt(row[176]);
		sp[tempid].pvp_resist_base= Strings::ToInt(row[177]);
		sp[tempid].pvp_resist_per_level= Strings::ToInt(row[178]);
		sp[tempid].pvp_resist_cap= Strings::ToInt(row[179]);
		sp[tempid].spell_category= Strings::ToInt(row[180]);
		sp[tempid].pvp_duration = Strings::ToInt(row[181]);
		sp[tempid].pvp_duration_cap = Strings::ToInt(row[182]);
		sp[tempid].pcnpc_only_flag= Strings::ToInt(row[183]);
		sp[tempid].cast_not_standing = Strings::ToInt(row[184]) != 0;
		sp[tempid].can_mgb= Strings::ToBool(row[185]);
		sp[tempid].dispel_flag = Strings::ToInt(row[186]);
		sp[tempid].min_resist = Strings::ToInt(row[189]);
		sp[tempid].max_resist = Strings::ToInt(row[190]);
		sp[tempid].viral_targets = Strings::ToInt(row[191]);
		sp[tempid].viral_timer = Strings::ToInt(row[192]);
		sp[tempid].nimbus_effect = Strings::ToInt(row[193]);
		sp[tempid].directional_start = Strings::ToFloat(row[194]);
		sp[tempid].directional_end = Strings::ToFloat(row[195]);
		sp[tempid].sneak = Strings::ToBool(row[196]);
		sp[tempid].not_focusable = Strings::ToBool(row[197]);
		sp[tempid].no_detrimental_spell_aggro = Strings::ToBool(row[198]);
		sp[tempid].suspendable = Strings::ToBool(row[200]);
		sp[tempid].viral_range = Strings::ToInt(row[201]);
		sp[tempid].song_cap = Strings::ToInt(row[202]);
		sp[tempid].no_block = Strings::ToInt(row[205]);
		sp[tempid].spell_group=Strings::ToInt(row[207]);
		sp[tempid].rank = Strings::ToInt(row[208]);
		sp[tempid].no_resist=Strings::ToInt(row[209]);
		sp[tempid].cast_restriction = Strings::ToInt(row[211]);
		sp[tempid].allow_rest = Strings::ToBool(row[212]);
		sp[tempid].can_cast_in_combat = Strings::ToBool(row[213]);
		sp[tempid].can_cast_out_of_combat = Strings::ToBool(row[214]);
		sp[tempid].override_crit_chance = Strings::ToInt(row[217]);
		sp[tempid].aoe_max_targets = Strings::ToInt(row[218]);
		sp[tempid].no_heal_damage_item_mod = Strings::ToInt(row[219]);
		sp[tempid].caster_requirement_id = Strings::ToInt(row[220]);
		sp[tempid].spell_class = Strings::ToInt(row[221]);
		sp[tempid].spell_subclass = Strings::ToInt(row[222]);
		sp[tempid].persist_death = Strings::ToBool(row[224]);
		sp[tempid].min_distance = Strings::ToFloat(row[227]);
		sp[tempid].min_distance_mod = Strings::ToFloat(row[228]);
		sp[tempid].max_distance = Strings::ToFloat(row[229]);
		sp[tempid].max_distance_mod = Strings::ToFloat(row[230]);
		sp[tempid].min_range = Strings::ToFloat(row[231]);
		sp[tempid].no_remove = Strings::ToBool(row[232]);
		sp[tempid].damage_shield_type = 0;
	}

	LoadDamageShieldTypes(sp, max_spells);
}

void SharedDatabase::LoadCharacterInspectMessage(uint32 character_id, InspectMessage_Struct* message) {
	const std::string query = StringFormat("SELECT `inspect_message` FROM `character_inspect_messages` WHERE `id` = %u LIMIT 1", character_id);
	auto results = QueryDatabase(query);
	memset(message, '\0', sizeof(InspectMessage_Struct));
	for (auto& row = results.begin(); row != results.end(); ++row) {
		memcpy(message, row[0], sizeof(InspectMessage_Struct));
	}
}

void SharedDatabase::SaveCharacterInspectMessage(uint32 character_id, const InspectMessage_Struct* message) {
	const std::string query = StringFormat("REPLACE INTO `character_inspect_messages` (id, inspect_message) VALUES (%u, '%s')", character_id, Strings::Escape(message->text).c_str());
	auto results = QueryDatabase(query);
}

uint32 SharedDatabase::GetSpellsCount()
{
	auto results = QueryDatabase("SELECT count(*) FROM spells_new");
	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto& row = results.begin();

	if (row[0]) {
		return Strings::ToUnsignedInt(row[0]);
	}

	return 0;
}

uint32 SharedDatabase::GetItemsCount()
{
	auto results = QueryDatabase("SELECT count(*) FROM items");
	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto& row = results.begin();

	if (row[0]) {
		return Strings::ToUnsignedInt(row[0]);
	}

	return 0;
}

void SharedDatabase::SetSharedItemsCount(uint32 shared_items_count)
{
	SharedDatabase::m_shared_items_count = shared_items_count;
}

void SharedDatabase::SetSharedSpellsCount(uint32 shared_spells_count)
{
	SharedDatabase::m_shared_spells_count = shared_spells_count;
}
