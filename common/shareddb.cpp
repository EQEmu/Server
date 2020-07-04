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
#include "eqemu_exception.h"
#include "faction.h"
#include "features.h"
#include "ipc_mutex.h"
#include "inventory_profile.h"
#include "loottable.h"
#include "memory_mapped_file.h"
#include "mysql.h"
#include "rulesys.h"
#include "shareddb.h"
#include "string_util.h"
#include "eqemu_config.h"

namespace ItemField
{
	enum {
		source = 0,
#define F(x) x,
#include "item_fieldlist.h"
#undef F
		updated
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

SharedDatabase::~SharedDatabase() {
}

bool SharedDatabase::SetHideMe(uint32 account_id, uint8 hideme)
{
	std::string query = StringFormat("UPDATE account SET hideme = %i WHERE id = %i", hideme, account_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}

uint8 SharedDatabase::GetGMSpeed(uint32 account_id)
{
	std::string query = StringFormat("SELECT gmspeed FROM account WHERE id = '%i'", account_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

    if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();

	return atoi(row[0]);
}

bool SharedDatabase::SetGMSpeed(uint32 account_id, uint8 gmspeed)
{
	std::string query = StringFormat("UPDATE account SET gmspeed = %i WHERE id = %i", gmspeed, account_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}

uint32 SharedDatabase::GetTotalTimeEntitledOnAccount(uint32 AccountID) {
	uint32 EntitledTime = 0;
	std::string query = StringFormat("SELECT `time_played` FROM `character_data` WHERE `account_id` = %u", AccountID);
	auto results = QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		EntitledTime += atoi(row[0]);
	}
	return EntitledTime;
}

void SharedDatabase::SetMailKey(int CharID, int IPAddress, int MailKey)
{
	char MailKeyString[17];

	if (RuleB(Chat, EnableMailKeyIPVerification) == true)
		sprintf(MailKeyString, "%08X%08X", IPAddress, MailKey);
	else
		sprintf(MailKeyString, "%08X", MailKey);

	std::string query = StringFormat("UPDATE character_data SET mailkey = '%s' WHERE id = '%i'",
		MailKeyString, CharID);
	auto results = QueryDatabase(query);
	if (!results.Success())
		LogError("SharedDatabase::SetMailKey({}, {}) : {}", CharID, MailKeyString, results.ErrorMessage().c_str());

}

std::string SharedDatabase::GetMailKey(int CharID, bool key_only)
{
	std::string query = StringFormat("SELECT `mailkey` FROM `character_data` WHERE `id`='%i' LIMIT 1", CharID);
	auto results = QueryDatabase(query);

	if (!results.Success()) {

		Log(Logs::Detail, Logs::MySQLError, "Error retrieving mailkey from database: %s", results.ErrorMessage().c_str());
		return std::string();
	}

	if (!results.RowCount()) {

		Log(Logs::General, Logs::ClientLogin, "Error: Mailkey for character id [%i] does not exist or could not be found", CharID);
		return std::string();
	}

	auto row = results.begin();
	if (row != results.end()) {

		std::string mail_key = row[0];

		if (mail_key.length() > 8 && key_only) {
			return mail_key.substr(8);
		}
		else {
			return mail_key;
		}
	}
	else {

		Log(Logs::General, Logs::MySQLError, "Internal MySQL error in SharedDatabase::GetMailKey(int, bool)");
		return std::string();
	}
}

bool SharedDatabase::SaveCursor(uint32 char_id, std::list<EQ::ItemInstance*>::const_iterator &start, std::list<EQ::ItemInstance*>::const_iterator &end)
{
	// Delete cursor items
	std::string query = StringFormat("DELETE FROM inventory WHERE charid = %i "
                                    "AND ((slotid >= 8000 AND slotid <= 8999) "
                                    "OR slotid = %i OR (slotid >= %i AND slotid <= %i) )",
									char_id, EQ::invslot::slotCursor,
									EQ::invbag::CURSOR_BAG_BEGIN, EQ::invbag::CURSOR_BAG_END);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        std::cout << "Clearing cursor failed: " << results.ErrorMessage() << std::endl;
        return false;
    }

    int i = 8000;
    for(auto it = start; it != end; ++it, i++) {
		if (i > 8999) { break; } // shouldn't be anything in the queue that indexes this high
        EQ::ItemInstance *inst = *it;
		int16 use_slot = (i == 8000) ? EQ::invslot::slotCursor : i;
		if (!SaveInventory(char_id, inst, use_slot)) {
			return false;
		}
    }

	return true;
}

bool SharedDatabase::VerifyInventory(uint32 account_id, int16 slot_id, const EQ::ItemInstance* inst)
{
	// Delete cursor items
	std::string query = StringFormat("SELECT itemid, charges FROM sharedbank "
                                    "WHERE acctid = %d AND slotid = %d",
                                    account_id, slot_id);
    auto results = QueryDatabase(query);
	if (!results.Success()) {
		//returning true is less harmful in the face of a query error
		return true;
	}

	if (results.RowCount() == 0)
        return false;

	auto row = results.begin();

    uint32 id = atoi(row[0]);
    uint16 charges = atoi(row[1]);

    uint16 expect_charges = 0;

    if(inst->GetCharges() >= 0)
        expect_charges = inst->GetCharges();
    else
        expect_charges = 0x7FFF;

    if(id != inst->GetItem()->ID || charges != expect_charges)
        return false;

	return true;
}

bool SharedDatabase::SaveInventory(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id) {

	//never save tribute slots:
	if (slot_id >= EQ::invslot::TRIBUTE_BEGIN && slot_id <= EQ::invslot::TRIBUTE_END)
		return true;
	if (slot_id >= EQ::invslot::GUILD_TRIBUTE_BEGIN && slot_id <= EQ::invslot::GUILD_TRIBUTE_END)
		return true;

	if (slot_id >= EQ::invslot::SHARED_BANK_BEGIN && slot_id <= EQ::invbag::SHARED_BANK_BAGS_END) {
        // Shared bank inventory
		if (!inst) {
			return DeleteSharedBankSlot(char_id, slot_id);
		}
		else {
			// Needed to clear out bag slots that 'REPLACE' in UpdateSharedBankSlot does not overwrite..otherwise, duplication occurs
			// (This requires that parent then child items be sent..which should be how they are currently passed)
			if (EQ::InventoryProfile::SupportsContainers(slot_id))
				DeleteSharedBankSlot(char_id, slot_id);
			return UpdateSharedBankSlot(char_id, inst, slot_id);
		}
	}
	else if (!inst) { // All other inventory
		return DeleteInventorySlot(char_id, slot_id);
	}

	// Needed to clear out bag slots that 'REPLACE' in UpdateInventorySlot does not overwrite..otherwise, duplication occurs
	// (This requires that parent then child items be sent..which should be how they are currently passed)
	if (EQ::InventoryProfile::SupportsContainers(slot_id))
		DeleteInventorySlot(char_id, slot_id);
    return UpdateInventorySlot(char_id, inst, slot_id);
}

bool SharedDatabase::UpdateInventorySlot(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id) {
	// need to check 'inst' argument for valid pointer

	uint32 augslot[EQ::invaug::SOCKET_COUNT] = { 0, 0, 0, 0, 0, 0 };
	if (inst->IsClassCommon()) {
		for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
			EQ::ItemInstance *auginst = inst->GetItem(i);
			augslot[i] = (auginst && auginst->GetItem()) ? auginst->GetItem()->ID : 0;
		}
	}

    uint16 charges = 0;
	if(inst->GetCharges() >= 0)
		charges = inst->GetCharges();
	else
		charges = 0x7FFF;

	// Update/Insert item
    std::string query = StringFormat("REPLACE INTO inventory "
                                    "(charid, slotid, itemid, charges, instnodrop, custom_data, color, "
                                    "augslot1, augslot2, augslot3, augslot4, augslot5, augslot6, ornamenticon, ornamentidfile, ornament_hero_model) "
                                    "VALUES( %lu, %lu, %lu, %lu, %lu, '%s', %lu, "
                                    "%lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu)",
                                    (unsigned long)char_id, (unsigned long)slot_id, (unsigned long)inst->GetItem()->ID,
                                    (unsigned long)charges, (unsigned long)(inst->IsAttuned()? 1: 0),
                                    inst->GetCustomDataString().c_str(), (unsigned long)inst->GetColor(),
                                    (unsigned long)augslot[0], (unsigned long)augslot[1], (unsigned long)augslot[2],
									(unsigned long)augslot[3], (unsigned long)augslot[4], (unsigned long)augslot[5], (unsigned long)inst->GetOrnamentationIcon(),
									(unsigned long)inst->GetOrnamentationIDFile(), (unsigned long)inst->GetOrnamentHeroModel());
	auto results = QueryDatabase(query);

    // Save bag contents, if slot supports bag contents
	if (inst->IsClassBag() && EQ::InventoryProfile::SupportsContainers(slot_id))
		// Limiting to bag slot count will get rid of 'hidden' duplicated items and 'Invalid Slot ID'
		// messages through attrition (and the modded code in SaveInventory)
		for (uint8 idx = EQ::invbag::SLOT_BEGIN; idx < inst->GetItem()->BagSlots && idx <= EQ::invbag::SLOT_END; idx++) {
			const EQ::ItemInstance* baginst = inst->GetItem(idx);
			SaveInventory(char_id, baginst, EQ::InventoryProfile::CalcSlotId(slot_id, idx));
		}

    if (!results.Success()) {
        return false;
    }

	return true;
}

bool SharedDatabase::UpdateSharedBankSlot(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id) {
	// need to check 'inst' argument for valid pointer

	uint32 augslot[EQ::invaug::SOCKET_COUNT] = { 0, 0, 0, 0, 0, 0 };
	if (inst->IsClassCommon()) {
		for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
			EQ::ItemInstance *auginst = inst->GetItem(i);
			augslot[i] = (auginst && auginst->GetItem()) ? auginst->GetItem()->ID : 0;
		}
	}

// Update/Insert item
    uint32 account_id = GetAccountIDByChar(char_id);
    uint16 charges = 0;
    if(inst->GetCharges() >= 0)
        charges = inst->GetCharges();
    else
        charges = 0x7FFF;

    std::string query = StringFormat("REPLACE INTO sharedbank "
                                    "(acctid, slotid, itemid, charges, custom_data, "
                                    "augslot1, augslot2, augslot3, augslot4, augslot5, augslot6) "
                                    "VALUES( %lu, %lu, %lu, %lu, '%s', "
                                    "%lu, %lu, %lu, %lu, %lu, %lu)",
                                    (unsigned long)account_id, (unsigned long)slot_id, (unsigned long)inst->GetItem()->ID,
                                    (unsigned long)charges, inst->GetCustomDataString().c_str(), (unsigned long)augslot[0],
									(unsigned long)augslot[1], (unsigned long)augslot[2], (unsigned long)augslot[3], (unsigned long)augslot[4],
									(unsigned long)augslot[5]);
    auto results = QueryDatabase(query);

    // Save bag contents, if slot supports bag contents
	if (inst->IsClassBag() && EQ::InventoryProfile::SupportsContainers(slot_id)) {
		// Limiting to bag slot count will get rid of 'hidden' duplicated items and 'Invalid Slot ID'
		// messages through attrition (and the modded code in SaveInventory)
		for (uint8 idx = EQ::invbag::SLOT_BEGIN; idx < inst->GetItem()->BagSlots && idx <= EQ::invbag::SLOT_END; idx++) {
			const EQ::ItemInstance* baginst = inst->GetItem(idx);
			SaveInventory(char_id, baginst, EQ::InventoryProfile::CalcSlotId(slot_id, idx));
		}
	}

    if (!results.Success()) {
        return false;
    }

	return true;
}

bool SharedDatabase::DeleteInventorySlot(uint32 char_id, int16 slot_id) {

	// Delete item
	std::string query = StringFormat("DELETE FROM inventory WHERE charid = %i AND slotid = %i", char_id, slot_id);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return false;
    }

    // Delete bag slots, if need be
	if (!EQ::InventoryProfile::SupportsContainers(slot_id))
        return true;

	int16 base_slot_id = EQ::InventoryProfile::CalcSlotId(slot_id, EQ::invbag::SLOT_BEGIN);
    query = StringFormat("DELETE FROM inventory WHERE charid = %i AND slotid >= %i AND slotid < %i",
                        char_id, base_slot_id, (base_slot_id+10));
    results = QueryDatabase(query);
    if (!results.Success()) {
        return false;
    }

    // @merth: need to delete augments here
    return true;
}

bool SharedDatabase::DeleteSharedBankSlot(uint32 char_id, int16 slot_id) {

    // Delete item
	uint32 account_id = GetAccountIDByChar(char_id);
	std::string query = StringFormat("DELETE FROM sharedbank WHERE acctid=%i AND slotid=%i", account_id, slot_id);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return false;
    }

	// Delete bag slots, if need be
	if (!EQ::InventoryProfile::SupportsContainers(slot_id))
        return true;

	int16 base_slot_id = EQ::InventoryProfile::CalcSlotId(slot_id, EQ::invbag::SLOT_BEGIN);
    query = StringFormat("DELETE FROM sharedbank WHERE acctid = %i "
                        "AND slotid >= %i AND slotid < %i",
                        account_id, base_slot_id, (base_slot_id+10));
    results = QueryDatabase(query);
    if (!results.Success()) {
        return false;
    }

    // @merth: need to delete augments here
    return true;
}


int32 SharedDatabase::GetSharedPlatinum(uint32 account_id)
{
	std::string query = StringFormat("SELECT sharedplat FROM account WHERE id = '%i'", account_id);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

    if (results.RowCount() != 1)
        return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

bool SharedDatabase::SetSharedPlatinum(uint32 account_id, int32 amount_to_add) {
	std::string query = StringFormat("UPDATE account SET sharedplat = sharedplat + %i WHERE id = %i", amount_to_add, account_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}

bool SharedDatabase::SetStartingItems(PlayerProfile_Struct* pp, EQ::InventoryProfile* inv, uint32 si_race, uint32 si_class, uint32 si_deity, uint32 si_current_zone, char* si_name, int admin_level) {

	const EQ::ItemData* myitem;

    std::string query = StringFormat("SELECT itemid, item_charges, slot FROM starting_items "
                                    "WHERE (race = %i or race = 0) AND (class = %i or class = 0) AND "
                                    "(deityid = %i or deityid = 0) AND (zoneid = %i or zoneid = 0) AND "
                                    "gm <= %i ORDER BY id",
                                    si_race, si_class, si_deity, si_current_zone, admin_level);
    auto results = QueryDatabase(query);
    if (!results.Success())
        return false;


	for (auto row = results.begin(); row != results.end(); ++row) {
		int32 itemid = atoi(row[0]);
		int32 charges = atoi(row[1]);
		int32 slot = atoi(row[2]);
		myitem = GetItem(itemid);

		if(!myitem)
			continue;

		EQ::ItemInstance* myinst = CreateBaseItem(myitem, charges);

		if(slot < 0)
			slot = inv->FindFreeSlot(0, 0);

		inv->PutItem(slot, *myinst);
		safe_delete(myinst);
	}

	return true;
}


// Retrieve shared bank inventory based on either account or character
bool SharedDatabase::GetSharedBank(uint32 id, EQ::InventoryProfile *inv, bool is_charid)
{
	std::string query;

	if (is_charid)
		query = StringFormat("SELECT sb.slotid, sb.itemid, sb.charges, "
				     "sb.augslot1, sb.augslot2, sb.augslot3, "
				     "sb.augslot4, sb.augslot5, sb.augslot6, sb.custom_data "
				     "FROM sharedbank sb INNER JOIN character_data ch "
				     "ON ch.account_id=sb.acctid WHERE ch.id = %i ORDER BY sb.slotid",
				     id);
	else
		query = StringFormat("SELECT slotid, itemid, charges, "
				     "augslot1, augslot2, augslot3, "
				     "augslot4, augslot5, augslot6, custom_data "
				     "FROM sharedbank WHERE acctid=%i ORDER BY slotid",
				     id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int16 slot_id = (int16)atoi(row[0]);
		uint32 item_id = (uint32)atoi(row[1]);
		int8 charges = (int8)atoi(row[2]);

		uint32 aug[EQ::invaug::SOCKET_COUNT];
		aug[0] = (uint32)atoi(row[3]);
		aug[1] = (uint32)atoi(row[4]);
		aug[2] = (uint32)atoi(row[5]);
		aug[3] = (uint32)atoi(row[6]);
		aug[4] = (uint32)atoi(row[7]);
		aug[5] = (uint32)atoi(row[8]);

		const EQ::ItemData *item = GetItem(item_id);

		if (!item) {
			LogError("Warning: [{}] [{}] has an invalid item_id [{}] in inventory slot [{}]",
				((is_charid == true) ? "charid" : "acctid"), id, item_id, slot_id);
			continue;
		}

		int16 put_slot_id = INVALID_INDEX;

		EQ::ItemInstance *inst = CreateBaseItem(item, charges);
		if (inst && item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (aug[i])
					inst->PutAugment(this, i, aug[i]);
			}
		}

		if (inst && row[9]) {
			std::string data_str(row[9]);
			std::string idAsString;
			std::string value;
			bool use_id = true;

			for (int i = 0; i < data_str.length(); ++i) {
				if (data_str[i] == '^') {
					if (!use_id) {
						inst->SetCustomData(idAsString, value);
						idAsString.clear();
						value.clear();
					}
					use_id = !use_id;
					continue;
				}

				char v = data_str[i];
				if (use_id)
					idAsString.push_back(v);
				else
					value.push_back(v);
			}
		}

		// theoretically inst can be nullptr ... this would be very bad ...
		put_slot_id = inv->PutItem(slot_id, *inst);
		safe_delete(inst);

		// Save ptr to item in inventory
		if (put_slot_id != INVALID_INDEX)
			continue;

		LogError("Warning: Invalid slot_id for item in shared bank inventory: [{}]=[{}], item_id=[{}], slot_id=[{}]",
			((is_charid == true) ? "charid" : "acctid"), id, item_id, slot_id);

		if (is_charid)
			SaveInventory(id, nullptr, slot_id);
	}

	return true;
}

// Overloaded: Retrieve character inventory based on character id (zone entry)
bool SharedDatabase::GetInventory(uint32 char_id, EQ::InventoryProfile *inv)
{
	if (!char_id || !inv)
		return false;
	
	// Retrieve character inventory
	std::string query =
	    StringFormat("SELECT slotid, itemid, charges, color, augslot1, augslot2, augslot3, augslot4, augslot5, "
			 "augslot6, instnodrop, custom_data, ornamenticon, ornamentidfile, ornament_hero_model FROM "
			 "inventory WHERE charid = %i ORDER BY slotid",
			 char_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogError("If you got an error related to the 'instnodrop' field, run the "
						    "following SQL Queries:\nalter table inventory add instnodrop "
						    "tinyint(1) unsigned default 0 not null;\n");
		return false;
	}

	auto timestamps = GetItemRecastTimestamps(char_id);

	auto cv_conflict = false;
	auto pmask = inv->GetLookup()->PossessionsBitmask;
	auto bank_size = inv->GetLookup()->InventoryTypeSize.Bank;

	for (auto row = results.begin(); row != results.end(); ++row) {
		int16 slot_id = atoi(row[0]);

		if (slot_id <= EQ::invslot::POSSESSIONS_END && slot_id >= EQ::invslot::POSSESSIONS_BEGIN) { // Titanium thru UF check
			if ((((uint64)1 << slot_id) & pmask) == 0) {
				cv_conflict = true;
				continue;
			}
		}
		else if (slot_id <= EQ::invbag::GENERAL_BAGS_END && slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN) { // Titanium thru UF check
			auto parent_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
			if ((((uint64)1 << parent_slot) & pmask) == 0) {
				cv_conflict = true;
				continue;
			}
		}
		else if (slot_id <= EQ::invslot::BANK_END && slot_id >= EQ::invslot::BANK_BEGIN) { // Titanium check
			if ((slot_id - EQ::invslot::BANK_BEGIN) >= bank_size) {
				cv_conflict = true;
				continue;
			}
		}
		else if (slot_id <= EQ::invbag::BANK_BAGS_END && slot_id >= EQ::invbag::BANK_BAGS_BEGIN) { // Titanium check
			auto parent_index = ((slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
			if (parent_index < EQ::invslot::SLOT_BEGIN || parent_index >= bank_size) {
				cv_conflict = true;
				continue;
			}
		}

		uint32 item_id = atoi(row[1]);
		uint16 charges = atoi(row[2]);
		uint32 color = atoul(row[3]);

		uint32 aug[EQ::invaug::SOCKET_COUNT];

		aug[0] = (uint32)atoul(row[4]);
		aug[1] = (uint32)atoul(row[5]);
		aug[2] = (uint32)atoul(row[6]);
		aug[3] = (uint32)atoul(row[7]);
		aug[4] = (uint32)atoul(row[8]);
		aug[5] = (uint32)atoul(row[9]);

		bool instnodrop = (row[10] && (uint16)atoi(row[10])) ? true : false;

		uint32 ornament_icon = (uint32)atoul(row[12]);
		uint32 ornament_idfile = (uint32)atoul(row[13]);
		uint32 ornament_hero_model = (uint32)atoul(row[14]);

		const EQ::ItemData *item = GetItem(item_id);

		if (!item) {
			LogError("Warning: charid [{}] has an invalid item_id [{}] in inventory slot [{}]", char_id, item_id,
				slot_id);
			continue;
		}

		int16 put_slot_id = INVALID_INDEX;

		EQ::ItemInstance *inst = CreateBaseItem(item, charges);

		if (inst == nullptr)
			continue;

		if (row[11]) {
			std::string data_str(row[11]);
			std::string idAsString;
			std::string value;
			bool use_id = true;

			for (int i = 0; i < data_str.length(); ++i) {
				if (data_str[i] == '^') {
					if (!use_id) {
						inst->SetCustomData(idAsString, value);
						idAsString.clear();
						value.clear();
					}

					use_id = !use_id;
					continue;
				}

				char v = data_str[i];
				if (use_id)
					idAsString.push_back(v);
				else
					value.push_back(v);
			}
		}

		inst->SetOrnamentIcon(ornament_icon);
		inst->SetOrnamentationIDFile(ornament_idfile);
		inst->SetOrnamentHeroModel(item->HerosForgeModel);

		if (instnodrop || (inst->GetItem()->Attuneable && slot_id >= EQ::invslot::EQUIPMENT_BEGIN && slot_id <= EQ::invslot::EQUIPMENT_END))
			inst->SetAttuned(true);

		if (color > 0)
			inst->SetColor(color);

		if (charges == 0x7FFF)
			inst->SetCharges(-1);
		else if (charges == 0 && inst->IsStackable()) // Stackable items need a minimum charge of 1 remain moveable.
			inst->SetCharges(1);
		else
			inst->SetCharges(charges);

		if (item->RecastDelay) {
			if (timestamps.count(item->RecastType))
				inst->SetRecastTimestamp(timestamps.at(item->RecastType));
			else
				inst->SetRecastTimestamp(0);
		}

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (aug[i])
					inst->PutAugment(this, i, aug[i]);
			}
		}

		if (slot_id >= 8000 && slot_id <= 8999) {
			put_slot_id = inv->PushCursor(*inst);
		} else if (slot_id >= 3111 && slot_id <= 3179) {
			// Admins: please report any occurrences of this error
			LogError("Warning: Defunct location for item in inventory: charid={}, item_id={}, slot_id={} .. pushing to cursor...",
				char_id, item_id, slot_id);
			put_slot_id = inv->PushCursor(*inst);
		} else {
			put_slot_id = inv->PutItem(slot_id, *inst);
		}

		safe_delete(inst);

		// Save ptr to item in inventory
		if (put_slot_id == INVALID_INDEX) {
			LogError("Warning: Invalid slot_id for item in inventory: charid=[{}], item_id=[{}], slot_id=[{}]",
				char_id, item_id, slot_id);
		}
	}
	
	if (cv_conflict) {
		char char_name[64] = "";
		GetCharName(char_id, char_name);
		LogError("ClientVersion/Expansion conflict during inventory load at zone entry for [{}] (charid: [{}], inver: [{}], gmi: [{}])",
			char_name,
			char_id,
			EQ::versions::MobVersionName(inv->InventoryVersion()),
			(inv->GMInventory() ? "true" : "false")
		);
	}

	// Retrieve shared inventory
	return GetSharedBank(char_id, inv, true);
}

// Overloaded: Retrieve character inventory based on account_id and character name (char select)
bool SharedDatabase::GetInventory(uint32 account_id, char *name, EQ::InventoryProfile *inv) // deprecated
{
	// Retrieve character inventory
	std::string query =
	    StringFormat("SELECT slotid, itemid, charges, color, augslot1, "
			 "augslot2, augslot3, augslot4, augslot5, augslot6, instnodrop, custom_data, ornamenticon, "
			 "ornamentidfile, ornament_hero_model "
			 "FROM inventory INNER JOIN character_data ch "
			 "ON ch.id = charid WHERE ch.name = '%s' AND ch.account_id = %i ORDER BY slotid",
			 name, account_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogError("If you got an error related to the 'instnodrop' field, run the "
						    "following SQL Queries:\nalter table inventory add instnodrop "
						    "tinyint(1) unsigned default 0 not null;\n");
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int16 slot_id = atoi(row[0]);
		uint32 item_id = atoi(row[1]);
		int8 charges = atoi(row[2]);
		uint32 color = atoul(row[3]);

		uint32 aug[EQ::invaug::SOCKET_COUNT];
		aug[0] = (uint32)atoi(row[4]);
		aug[1] = (uint32)atoi(row[5]);
		aug[2] = (uint32)atoi(row[6]);
		aug[3] = (uint32)atoi(row[7]);
		aug[4] = (uint32)atoi(row[8]);
		aug[5] = (uint32)atoi(row[9]);

		bool instnodrop = (row[10] && (uint16)atoi(row[10])) ? true : false;
		uint32 ornament_icon = (uint32)atoul(row[12]);
		uint32 ornament_idfile = (uint32)atoul(row[13]);
		uint32 ornament_hero_model = (uint32)atoul(row[14]);

		const EQ::ItemData *item = GetItem(item_id);
		int16 put_slot_id = INVALID_INDEX;
		if (!item)
			continue;

		EQ::ItemInstance *inst = CreateBaseItem(item, charges);

		if (inst == nullptr)
			continue;

		inst->SetAttuned(instnodrop);

		if (row[11]) {
			std::string data_str(row[11]);
			std::string idAsString;
			std::string value;
			bool use_id = true;

			for (int i = 0; i < data_str.length(); ++i) {
				if (data_str[i] == '^') {
					if (!use_id) {
						inst->SetCustomData(idAsString, value);
						idAsString.clear();
						value.clear();
					}

					use_id = !use_id;
					continue;
				}

				char v = data_str[i];
				if (use_id)
					idAsString.push_back(v);
				else
					value.push_back(v);
			}
		}

		inst->SetOrnamentIcon(ornament_icon);
		inst->SetOrnamentationIDFile(ornament_idfile);
		inst->SetOrnamentHeroModel(item->HerosForgeModel);

		if (color > 0)
			inst->SetColor(color);

		inst->SetCharges(charges);

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (aug[i])
					inst->PutAugment(this, i, aug[i]);
			}
		}

		if (slot_id >= 8000 && slot_id <= 8999)
			put_slot_id = inv->PushCursor(*inst);
		else
			put_slot_id = inv->PutItem(slot_id, *inst);

		safe_delete(inst);

		// Save ptr to item in inventory
		if (put_slot_id == INVALID_INDEX)
			LogError("Warning: Invalid slot_id for item in inventory: name={}, acctid={}, item_id={}, slot_id={}",
				name, account_id, item_id, slot_id);
	}

	// Retrieve shared inventory
	return GetSharedBank(account_id, inv, false);
}

std::map<uint32, uint32> SharedDatabase::GetItemRecastTimestamps(uint32 char_id)
{
	std::map<uint32, uint32> timers;
	std::string query = StringFormat("SELECT recast_type,timestamp FROM character_item_recast WHERE id=%u", char_id);
	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() == 0)
		return timers;

	for (auto row = results.begin(); row != results.end(); ++row)
		timers[atoul(row[0])] = atoul(row[1]);
	return timers; // RVO or move assigned
}

uint32 SharedDatabase::GetItemRecastTimestamp(uint32 char_id, uint32 recast_type)
{
	std::string query = StringFormat("SELECT timestamp FROM character_item_recast WHERE id=%u AND recast_type=%u",
					 char_id, recast_type);
	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return static_cast<uint32>(atoul(row[0]));
}

void SharedDatabase::ClearOldRecastTimestamps(uint32 char_id)
{
	// This actually isn't strictly live-like. Live your recast timestamps are forever
	std::string query =
	    StringFormat("DELETE FROM character_item_recast WHERE id = %u and timestamp < UNIX_TIMESTAMP()", char_id);
	QueryDatabase(query);
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

	auto row = results.begin();

	if (row[0])
		max_id = atoi(row[0]);

	if (row[1])
		item_count = atoi(row[1]);
}

bool SharedDatabase::LoadItems(const std::string &prefix) {
	items_mmf.reset(nullptr);

	try {
		auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("items");
		mutex.Lock();
		std::string file_name = Config->SharedMemDir + prefix + std::string("items");
		LogInfo("[Shared Memory] Attempting to load file [{}]", file_name);
		items_mmf = std::unique_ptr<EQ::MemoryMappedFile>(new EQ::MemoryMappedFile(file_name));
		items_hash = std::unique_ptr<EQ::FixedMemoryHashSet<EQ::ItemData>>(new EQ::FixedMemoryHashSet<EQ::ItemData>(reinterpret_cast<uint8*>(items_mmf->Get()), items_mmf->Size()));
		mutex.Unlock();
	} catch(std::exception& ex) {
		LogError("Error Loading Items: {}", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadItems(void *data, uint32 size, int32 items, uint32 max_item_id)
{
	EQ::FixedMemoryHashSet<EQ::ItemData> hash(reinterpret_cast<uint8 *>(data), size, items, max_item_id);

	std::string ndbuffer;
	bool disableNoRent = false;
	if (GetVariable("disablenorent", ndbuffer)) {
		if (ndbuffer[0] == '1' && ndbuffer[1] == '\0') {
			disableNoRent = true;
		}
	}
	bool disableNoDrop = false;
	if (GetVariable("disablenodrop", ndbuffer)) {
		if (ndbuffer[0] == '1' && ndbuffer[1] == '\0') {
			disableNoDrop = true;
		}
	}
	bool disableLoreGroup = false;
	if (GetVariable("disablelore", ndbuffer)) {
		if (ndbuffer[0] == '1' && ndbuffer[1] == '\0') {
			disableLoreGroup = true;
		}
	}
	bool disableNoTransfer = false;
	if (GetVariable("disablenotransfer", ndbuffer)) {
		if (ndbuffer[0] == '1' && ndbuffer[1] == '\0') {
			disableNoTransfer = true;
		}
	}

	EQ::ItemData item;

	const std::string query = "SELECT source,"
#define F(x) "`"#x"`,"
#include "item_fieldlist.h"
#undef F
		"updated FROM items ORDER BY id";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		memset(&item, 0, sizeof(EQ::ItemData));

		item.ItemClass = (uint8)atoi(row[ItemField::itemclass]);
		strcpy(item.Name, row[ItemField::name]);
		strcpy(item.Lore, row[ItemField::lore]);
		strcpy(item.IDFile, row[ItemField::idfile]);

		item.ID = (uint32)atoul(row[ItemField::id]);
		item.Weight = (int32)atoi(row[ItemField::weight]);
		item.NoRent = disableNoRent ? (uint8)atoi("255") : (uint8)atoi(row[ItemField::norent]);
		item.NoDrop = disableNoDrop ? (uint8)atoi("255") : (uint8)atoi(row[ItemField::nodrop]);
		item.Size = (uint8)atoi(row[ItemField::size]);
		item.Slots = (uint32)atoul(row[ItemField::slots]);
		item.Price = (uint32)atoul(row[ItemField::price]);
		item.Icon = (uint32)atoul(row[ItemField::icon]);
		item.BenefitFlag = (atoul(row[ItemField::benefitflag]) != 0);
		item.Tradeskills = (atoi(row[ItemField::tradeskills]) == 0) ? false : true;
		item.CR = (int8)atoi(row[ItemField::cr]);
		item.DR = (int8)atoi(row[ItemField::dr]);
		item.PR = (int8)atoi(row[ItemField::pr]);
		item.MR = (int8)atoi(row[ItemField::mr]);
		item.FR = (int8)atoi(row[ItemField::fr]);
		item.AStr = (int8)atoi(row[ItemField::astr]);
		item.ASta = (int8)atoi(row[ItemField::asta]);
		item.AAgi = (int8)atoi(row[ItemField::aagi]);
		item.ADex = (int8)atoi(row[ItemField::adex]);
		item.ACha = (int8)atoi(row[ItemField::acha]);
		item.AInt = (int8)atoi(row[ItemField::aint]);
		item.AWis = (int8)atoi(row[ItemField::awis]);
		item.HP = (int32)atoul(row[ItemField::hp]);
		item.Mana = (int32)atoul(row[ItemField::mana]);
		item.AC = (int32)atoul(row[ItemField::ac]);
		item.Deity = (uint32)atoul(row[ItemField::deity]);
		item.SkillModValue = (int32)atoul(row[ItemField::skillmodvalue]);
		item.SkillModMax = (int32)atoul(row[ItemField::skillmodmax]);
		item.SkillModType = (uint32)atoul(row[ItemField::skillmodtype]);
		item.BaneDmgRace = (uint32)atoul(row[ItemField::banedmgrace]);
		item.BaneDmgAmt = (int32)atoul(row[ItemField::banedmgamt]);
		item.BaneDmgBody = (uint32)atoul(row[ItemField::banedmgbody]);
		item.Magic = (atoi(row[ItemField::magic]) == 0) ? false : true;
		item.CastTime_ = (int32)atoul(row[ItemField::casttime_]);
		item.ReqLevel = (uint8)atoi(row[ItemField::reqlevel]);
		item.BardType = (uint32)atoul(row[ItemField::bardtype]);
		item.BardValue = (int32)atoul(row[ItemField::bardvalue]);
		item.Light = (int8)atoi(row[ItemField::light]);
		item.Delay = (uint8)atoi(row[ItemField::delay]);
		item.RecLevel = (uint8)atoi(row[ItemField::reclevel]);
		item.RecSkill = (uint8)atoi(row[ItemField::recskill]);
		item.ElemDmgType = (uint8)atoi(row[ItemField::elemdmgtype]);
		item.ElemDmgAmt = (uint8)atoi(row[ItemField::elemdmgamt]);
		item.Range = (uint8)atoi(row[ItemField::range]);
		item.Damage = (uint32)atoi(row[ItemField::damage]);
		item.Color = (uint32)atoul(row[ItemField::color]);
		item.Classes = (uint32)atoul(row[ItemField::classes]);
		item.Races = (uint32)atoul(row[ItemField::races]);

		item.MaxCharges = (int16)atoi(row[ItemField::maxcharges]);
		item.ItemType = (uint8)atoi(row[ItemField::itemtype]);
		item.SubType = atoi(row[ItemField::subtype]);
		item.Material = (uint8)atoi(row[ItemField::material]);
		item.HerosForgeModel = (uint32)atoi(row[ItemField::herosforgemodel]);
		item.SellRate = (float)atof(row[ItemField::sellrate]);
		item.CastTime = (uint32)atoul(row[ItemField::casttime]);
		item.EliteMaterial = (uint32)atoul(row[ItemField::elitematerial]);
		item.ProcRate = (int32)atoi(row[ItemField::procrate]);
		item.CombatEffects = (int8)atoi(row[ItemField::combateffects]);
		item.Shielding = (int8)atoi(row[ItemField::shielding]);
		item.StunResist = (int8)atoi(row[ItemField::stunresist]);
		item.StrikeThrough = (int8)atoi(row[ItemField::strikethrough]);
		item.ExtraDmgSkill = (uint32)atoul(row[ItemField::extradmgskill]);
		item.ExtraDmgAmt = (uint32)atoul(row[ItemField::extradmgamt]);
		item.SpellShield = (int8)atoi(row[ItemField::spellshield]);
		item.Avoidance = (int8)atoi(row[ItemField::avoidance]);
		item.Accuracy = (int8)atoi(row[ItemField::accuracy]);
		item.CharmFileID = (uint32)atoul(row[ItemField::charmfileid]);
		item.FactionMod1 = (int32)atoul(row[ItemField::factionmod1]);
		item.FactionMod2 = (int32)atoul(row[ItemField::factionmod2]);
		item.FactionMod3 = (int32)atoul(row[ItemField::factionmod3]);
		item.FactionMod4 = (int32)atoul(row[ItemField::factionmod4]);
		item.FactionAmt1 = (int32)atoul(row[ItemField::factionamt1]);
		item.FactionAmt2 = (int32)atoul(row[ItemField::factionamt2]);
		item.FactionAmt3 = (int32)atoul(row[ItemField::factionamt3]);
		item.FactionAmt4 = (int32)atoul(row[ItemField::factionamt4]);

		strcpy(item.CharmFile, row[ItemField::charmfile]);

		item.AugType = (uint32)atoul(row[ItemField::augtype]);
		item.AugSlotType[0] = (uint8)atoi(row[ItemField::augslot1type]);
		item.AugSlotVisible[0] = (uint8)atoi(row[ItemField::augslot1visible]);
		item.AugSlotUnk2[0] = 0;
		item.AugSlotType[1] = (uint8)atoi(row[ItemField::augslot2type]);
		item.AugSlotVisible[1] = (uint8)atoi(row[ItemField::augslot2visible]);
		item.AugSlotUnk2[1] = 0;
		item.AugSlotType[2] = (uint8)atoi(row[ItemField::augslot3type]);
		item.AugSlotVisible[2] = (uint8)atoi(row[ItemField::augslot3visible]);
		item.AugSlotUnk2[2] = 0;
		item.AugSlotType[3] = (uint8)atoi(row[ItemField::augslot4type]);
		item.AugSlotVisible[3] = (uint8)atoi(row[ItemField::augslot4visible]);
		item.AugSlotUnk2[3] = 0;
		item.AugSlotType[4] = (uint8)atoi(row[ItemField::augslot5type]);
		item.AugSlotVisible[4] = (uint8)atoi(row[ItemField::augslot5visible]);
		item.AugSlotUnk2[4] = 0;
		item.AugSlotType[5] = (uint8)atoi(row[ItemField::augslot6type]);
		item.AugSlotVisible[5] = (uint8)atoi(row[ItemField::augslot6visible]);
		item.AugSlotUnk2[5] = 0;

		item.LDoNTheme = (uint32)atoul(row[ItemField::ldontheme]);
		item.LDoNPrice = (uint32)atoul(row[ItemField::ldonprice]);
		item.LDoNSold = (uint32)atoul(row[ItemField::ldonsold]);
		item.BagType = (uint8)atoi(row[ItemField::bagtype]);
		item.BagSlots = (uint8)std::min(atoi(row[ItemField::bagslots]), 10); // FIXME: remove when big bags supported
		item.BagSize = (uint8)atoi(row[ItemField::bagsize]);
		item.BagWR = (uint8)atoi(row[ItemField::bagwr]);
		item.Book = (uint8)atoi(row[ItemField::book]);
		item.BookType = (uint32)atoul(row[ItemField::booktype]);

		strcpy(item.Filename, row[ItemField::filename]);

		item.BaneDmgRaceAmt = (uint32)atoul(row[ItemField::banedmgraceamt]);
		item.AugRestrict = (uint32)atoul(row[ItemField::augrestrict]);
		item.LoreGroup = disableLoreGroup ? (uint8)atoi("0") : atoi(row[ItemField::loregroup]);
		item.LoreFlag = item.LoreGroup != 0;
		item.PendingLoreFlag = (atoi(row[ItemField::pendingloreflag]) == 0) ? false : true;
		item.ArtifactFlag = (atoi(row[ItemField::artifactflag]) == 0) ? false : true;
		item.SummonedFlag = (atoi(row[ItemField::summonedflag]) == 0) ? false : true;
		item.Favor = (uint32)atoul(row[ItemField::favor]);
		item.FVNoDrop = (atoi(row[ItemField::fvnodrop]) == 0) ? false : true;
		item.Endur = (uint32)atoul(row[ItemField::endur]);
		item.DotShielding = (uint32)atoul(row[ItemField::dotshielding]);
		item.Attack = (uint32)atoul(row[ItemField::attack]);
		item.Regen = (uint32)atoul(row[ItemField::regen]);
		item.ManaRegen = (uint32)atoul(row[ItemField::manaregen]);
		item.EnduranceRegen = (uint32)atoul(row[ItemField::enduranceregen]);
		item.Haste = (uint32)atoul(row[ItemField::haste]);
		item.DamageShield = (uint32)atoul(row[ItemField::damageshield]);
		item.RecastDelay = (uint32)atoul(row[ItemField::recastdelay]);
		item.RecastType = (uint32)atoul(row[ItemField::recasttype]);
		item.GuildFavor = (uint32)atoul(row[ItemField::guildfavor]);
		item.AugDistiller = (uint32)atoul(row[ItemField::augdistiller]);
		item.Attuneable = (atoi(row[ItemField::attuneable]) == 0) ? false : true;
		item.NoPet = (atoi(row[ItemField::nopet]) == 0) ? false : true;
		item.PointType = (uint32)atoul(row[ItemField::pointtype]);
		item.PotionBelt = (atoi(row[ItemField::potionbelt]) == 0) ? false : true;
		item.PotionBeltSlots = (atoi(row[ItemField::potionbeltslots]) == 0) ? false : true;
		item.StackSize = (uint16)atoi(row[ItemField::stacksize]);
		item.NoTransfer = disableNoTransfer ? false : (atoi(row[ItemField::notransfer]) == 0) ? false : true;
		item.Stackable = (atoi(row[ItemField::stackable]) == 0) ? false : true;
		item.Click.Effect = (uint32)atoul(row[ItemField::clickeffect]);
		item.Click.Type = (uint8)atoul(row[ItemField::clicktype]);
		item.Click.Level = (uint8)atoul(row[ItemField::clicklevel]);
		item.Click.Level2 = (uint8)atoul(row[ItemField::clicklevel2]);

		strcpy(item.CharmFile, row[ItemField::charmfile]);

		item.Proc.Effect = (int32)atoul(row[ItemField::proceffect]);
		item.Proc.Type = (uint8)atoul(row[ItemField::proctype]);
		item.Proc.Level = (uint8)atoul(row[ItemField::proclevel]);
		item.Proc.Level2 = (uint8)atoul(row[ItemField::proclevel2]);
		item.Worn.Effect = (int32)atoul(row[ItemField::worneffect]);
		item.Worn.Type = (uint8)atoul(row[ItemField::worntype]);
		item.Worn.Level = (uint8)atoul(row[ItemField::wornlevel]);
		item.Worn.Level2 = (uint8)atoul(row[ItemField::wornlevel2]);
		item.Focus.Effect = (int32)atoul(row[ItemField::focuseffect]);
		item.Focus.Type = (uint8)atoul(row[ItemField::focustype]);
		item.Focus.Level = (uint8)atoul(row[ItemField::focuslevel]);
		item.Focus.Level2 = (uint8)atoul(row[ItemField::focuslevel2]);
		item.Scroll.Effect = (int32)atoul(row[ItemField::scrolleffect]);
		item.Scroll.Type = (uint8)atoul(row[ItemField::scrolltype]);
		item.Scroll.Level = (uint8)atoul(row[ItemField::scrolllevel]);
		item.Scroll.Level2 = (uint8)atoul(row[ItemField::scrolllevel2]);
		item.Bard.Effect = (int32)atoul(row[ItemField::bardeffect]);
		item.Bard.Type = (uint8)atoul(row[ItemField::bardtype]);
		item.Bard.Level = (uint8)atoul(row[ItemField::bardlevel]);
		item.Bard.Level2 = (uint8)atoul(row[ItemField::bardlevel2]);
		item.QuestItemFlag = (atoi(row[ItemField::questitemflag]) == 0) ? false : true;
		item.SVCorruption = (int32)atoi(row[ItemField::svcorruption]);
		item.Purity = (uint32)atoul(row[ItemField::purity]);
		item.EvolvingItem = (uint8)atoul(row[ItemField::evoitem]);
		item.EvolvingID = (uint8)atoul(row[ItemField::evoid]);
		item.EvolvingLevel = (uint8)atoul(row[ItemField::evolvinglevel]);
		item.EvolvingMax = (uint8)atoul(row[ItemField::evomax]);
		item.BackstabDmg = (uint32)atoul(row[ItemField::backstabdmg]);
		item.DSMitigation = (uint32)atoul(row[ItemField::dsmitigation]);
		item.HeroicStr = (int32)atoi(row[ItemField::heroic_str]);
		item.HeroicInt = (int32)atoi(row[ItemField::heroic_int]);
		item.HeroicWis = (int32)atoi(row[ItemField::heroic_wis]);
		item.HeroicAgi = (int32)atoi(row[ItemField::heroic_agi]);
		item.HeroicDex = (int32)atoi(row[ItemField::heroic_dex]);
		item.HeroicSta = (int32)atoi(row[ItemField::heroic_sta]);
		item.HeroicCha = (int32)atoi(row[ItemField::heroic_cha]);
		item.HeroicMR = (int32)atoi(row[ItemField::heroic_mr]);
		item.HeroicFR = (int32)atoi(row[ItemField::heroic_fr]);
		item.HeroicCR = (int32)atoi(row[ItemField::heroic_cr]);
		item.HeroicDR = (int32)atoi(row[ItemField::heroic_dr]);
		item.HeroicPR = (int32)atoi(row[ItemField::heroic_pr]);
		item.HeroicSVCorrup = (int32)atoi(row[ItemField::heroic_svcorrup]);
		item.HealAmt = (int32)atoi(row[ItemField::healamt]);
		item.SpellDmg = (int32)atoi(row[ItemField::spelldmg]);
		item.LDoNSellBackRate = (uint32)atoul(row[ItemField::ldonsellbackrate]);
		item.ScriptFileID = (uint32)atoul(row[ItemField::scriptfileid]);
		item.ExpendableArrow = (uint16)atoul(row[ItemField::expendablearrow]);
		item.Clairvoyance = (uint32)atoul(row[ItemField::clairvoyance]);

		strcpy(item.ClickName, row[ItemField::clickname]);
		strcpy(item.ProcName, row[ItemField::procname]);
		strcpy(item.WornName, row[ItemField::wornname]);
		strcpy(item.FocusName, row[ItemField::focusname]);
		strcpy(item.ScrollName, row[ItemField::scrollname]);

		try {
			hash.insert(item.ID, item);
		} catch (std::exception &ex) {
			LogError("Database::LoadItems: {}", ex.what());
			break;
		}
	}
}

const EQ::ItemData *SharedDatabase::GetItem(uint32 id)
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

const EQ::ItemData* SharedDatabase::IterateItems(uint32* id) {
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

std::string SharedDatabase::GetBook(const char *txtfile, int16 *language)
{
	char txtfile2[20];
	std::string txtout;
	strcpy(txtfile2, txtfile);

	std::string query = StringFormat("SELECT txtfile, language FROM books WHERE name = '%s'", txtfile2);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		txtout.assign(" ",1);
		return txtout;
	}

    if (results.RowCount() == 0) {
        LogError("No book to send, ({})", txtfile);
        txtout.assign(" ",1);
        return txtout;
    }

    auto row = results.begin();
    txtout.assign(row[0],strlen(row[0]));
    *language = static_cast<int16>(atoi(row[1]));

    return txtout;
}

void SharedDatabase::GetFactionListInfo(uint32 &list_count, uint32 &max_lists) {
	list_count = 0;
	max_lists = 0;

	const std::string query = "SELECT COUNT(*), MAX(id) FROM npc_faction";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        return;
	}

	if (results.RowCount() == 0)
        return;

    auto row = results.begin();

    list_count = static_cast<uint32>(atoul(row[0]));
    max_lists = static_cast<uint32>(atoul(row[1] ? row[1] : "0"));
}

const NPCFactionList* SharedDatabase::GetNPCFactionEntry(uint32 id) {
	if(!faction_hash) {
		return nullptr;
	}

	if(faction_hash->exists(id)) {
		return &(faction_hash->at(id));
	}

	return nullptr;
}

void SharedDatabase::LoadNPCFactionLists(void *data, uint32 size, uint32 list_count, uint32 max_lists) {
	EQ::FixedMemoryHashSet<NPCFactionList> hash(reinterpret_cast<uint8*>(data), size, list_count, max_lists);
	NPCFactionList faction;

	const std::string query = "SELECT npc_faction.id, npc_faction.primaryfaction, npc_faction.ignore_primary_assist, "
                            "npc_faction_entries.faction_id, npc_faction_entries.value, npc_faction_entries.npc_value, "
                            "npc_faction_entries.temp FROM npc_faction LEFT JOIN npc_faction_entries "
                            "ON npc_faction.id = npc_faction_entries.npc_faction_id ORDER BY npc_faction.id;";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return;
    }

    uint32 current_id = 0;
    uint32 current_entry = 0;

    for(auto row = results.begin(); row != results.end(); ++row) {
        uint32 id = static_cast<uint32>(atoul(row[0]));
        if(id != current_id) {
            if(current_id != 0) {
                hash.insert(current_id, faction);
            }

            memset(&faction, 0, sizeof(faction));
            current_entry = 0;
            current_id = id;
            faction.id = id;
            faction.primaryfaction = static_cast<uint32>(atoul(row[1]));
            faction.assistprimaryfaction = (atoi(row[2]) == 0);
        }

        if(!row[3])
            continue;

        if(current_entry >= MAX_NPC_FACTIONS)
				continue;

        faction.factionid[current_entry] = static_cast<uint32>(atoul(row[3]));
        faction.factionvalue[current_entry] = static_cast<int32>(atoi(row[4]));
        faction.factionnpcvalue[current_entry] = static_cast<int8>(atoi(row[5]));
        faction.factiontemp[current_entry] = static_cast<uint8>(atoi(row[6]));
        ++current_entry;
    }

    if(current_id != 0)
        hash.insert(current_id, faction);

}

bool SharedDatabase::LoadNPCFactionLists(const std::string &prefix) {
	faction_mmf.reset(nullptr);
	faction_hash.reset(nullptr);

	try {
		auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("faction");
		mutex.Lock();
		std::string file_name = Config->SharedMemDir + prefix + std::string("faction");
		LogInfo("[Shared Memory] Attempting to load file [{}]", file_name);
		faction_mmf = std::unique_ptr<EQ::MemoryMappedFile>(new EQ::MemoryMappedFile(file_name));
		faction_hash = std::unique_ptr<EQ::FixedMemoryHashSet<NPCFactionList>>(new EQ::FixedMemoryHashSet<NPCFactionList>(reinterpret_cast<uint8*>(faction_mmf->Get()), faction_mmf->Size()));
		mutex.Unlock();
	} catch(std::exception& ex) {
		LogError("Error Loading npc factions: {}", ex.what());
		return false;
	}

	return true;
}

// Create appropriate EQ::ItemInstance class
EQ::ItemInstance* SharedDatabase::CreateItem(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6, uint8 attuned)
{
	const EQ::ItemData* item = nullptr;
	EQ::ItemInstance* inst = nullptr;

	item = GetItem(item_id);
	if (item) {
		inst = CreateBaseItem(item, charges);

		if (inst == nullptr) {
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
	}

	return inst;
}


// Create appropriate EQ::ItemInstance class
EQ::ItemInstance* SharedDatabase::CreateItem(const EQ::ItemData* item, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6, uint8 attuned)
{
	EQ::ItemInstance* inst = nullptr;
	if (item) {
		inst = CreateBaseItem(item, charges);

		if (inst == nullptr) {
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
	}

	return inst;
}

EQ::ItemInstance* SharedDatabase::CreateBaseItem(const EQ::ItemData* item, int16 charges) {
	EQ::ItemInstance* inst = nullptr;
	if (item) {
		// if maxcharges is -1 that means it is an unlimited use item.
		// set it to 1 charge so that it is usable on creation
		if (charges == 0 && item->MaxCharges == -1)
			charges = 1;
		// Stackable items need a minimum charge of 1 to remain moveable.
		if(charges <= 0 && item->Stackable)
			charges = 1;

		inst = new EQ::ItemInstance(item, charges);

		if (inst == nullptr) {
			LogError("Error: valid item data returned a null reference for EQ::ItemInstance creation in SharedDatabase::CreateBaseItem()");
			LogError("Item Data = ID: {}, Name: {}, Charges: {}", item->ID, item->Name, charges);
			return nullptr;
		}

		if(item->CharmFileID != 0 || (item->LoreGroup >= 1000 && item->LoreGroup != -1)) {
			inst->Initialize(this);
		}
	}
	return inst;
}

int32 SharedDatabase::DeleteStalePlayerCorpses() {
	if(RuleB(Zone, EnableShadowrest)) {
        std::string query = StringFormat(
			"UPDATE `character_corpses` SET `is_buried` = 1 WHERE `is_buried` = 0 AND "
            "(UNIX_TIMESTAMP() - UNIX_TIMESTAMP(time_of_death)) > %d AND NOT time_of_death = 0",
             (RuleI(Character, CorpseDecayTimeMS) / 1000));
        auto results = QueryDatabase(query);
		if (!results.Success())
			return -1;

		return results.RowsAffected();
	}

    std::string query = StringFormat(
		"DELETE FROM `character_corpses` WHERE (UNIX_TIMESTAMP() - UNIX_TIMESTAMP(time_of_death)) > %d "
		"AND NOT time_of_death = 0", (RuleI(Character, CorpseDecayTimeMS) / 1000));
    auto results = QueryDatabase(query);
    if (!results.Success())
        return -1;

    return results.RowsAffected();
}

bool SharedDatabase::GetCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &command_settings)
{
	command_settings.clear();

	std::string query = "SELECT `command`, `access`, `aliases` FROM `command_settings`";
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;
    
	for (auto row = results.begin(); row != results.end(); ++row) {
		command_settings[row[0]].first = atoi(row[1]);
		if (row[2][0] == 0)
			continue;

		std::vector<std::string> aliases = SplitString(row[2], '|');
		for (auto iter = aliases.begin(); iter != aliases.end(); ++iter) {
			if (iter->empty())
				continue;
			command_settings[row[0]].second.push_back(*iter);
		}
	}

    return true;
}

bool SharedDatabase::UpdateInjectedCommandSettings(const std::vector<std::pair<std::string, uint8>> &injected)
{
	if (injected.size()) {

		std::string query = fmt::format(
			"REPLACE INTO `command_settings`(`command`, `access`) VALUES {}",
			implode(
				",",
				std::pair<char, char>('(', ')'),
				join_pair(",", std::pair<char, char>('\'', '\''), injected)
			)
		);

		if (!QueryDatabase(query).Success()) {
			return false;
		}

		LogInfo(
			"[{0}] New Command(s) Added",
			injected.size()
		);
	}

	return true;
}

bool SharedDatabase::UpdateOrphanedCommandSettings(const std::vector<std::string> &orphaned)
{
	if (orphaned.size()) {

		std::string query = fmt::format(
			"DELETE FROM `command_settings` WHERE `command` IN ({})",
			implode(",", std::pair<char, char>('\'', '\''), orphaned)
		);

		if (!QueryDatabase(query).Success()) {
			return false;
		}

		LogInfo(
			"{} Orphaned Command{} Deleted",
			orphaned.size(),
			(orphaned.size() == 1 ? "" : "s")
		);
	}

	return true;
}

bool SharedDatabase::LoadSkillCaps(const std::string &prefix) {
	skill_caps_mmf.reset(nullptr);

	uint32 class_count = PLAYER_CLASS_COUNT;
	uint32 skill_count = EQ::skills::HIGHEST_SKILL + 1;
	uint32 level_count = HARD_LEVEL_CAP + 1;
	uint32 size = (class_count * skill_count * level_count * sizeof(uint16));

	try {
		auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("skill_caps");
		mutex.Lock();
		std::string file_name = Config->SharedMemDir + prefix + std::string("skill_caps");
		LogInfo("[Shared Memory] Attempting to load file [{}]", file_name);
		skill_caps_mmf = std::unique_ptr<EQ::MemoryMappedFile>(new EQ::MemoryMappedFile(file_name));
		mutex.Unlock();
	} catch(std::exception &ex) {
		LogError("Error loading skill caps: {}", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadSkillCaps(void *data) {
	uint32 class_count = PLAYER_CLASS_COUNT;
	uint32 skill_count = EQ::skills::HIGHEST_SKILL + 1;
	uint32 level_count = HARD_LEVEL_CAP + 1;
	uint16 *skill_caps_table = reinterpret_cast<uint16*>(data);

	const std::string query = "SELECT skillID, class, level, cap FROM skill_caps ORDER BY skillID, class, level";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        LogError("Error loading skill caps from database: {}", results.ErrorMessage().c_str());
        return;
	}

    for(auto row = results.begin(); row != results.end(); ++row) {
        uint8 skillID = atoi(row[0]);
        uint8 class_ = atoi(row[1]) - 1;
        uint8 level = atoi(row[2]);
        uint16 cap = atoi(row[3]);

        if(skillID >= skill_count || class_ >= class_count || level >= level_count)
            continue;

        uint32 index = (((class_ * skill_count) + skillID) * level_count) + level;
        skill_caps_table[index] = cap;
    }
}

uint16 SharedDatabase::GetSkillCap(uint8 Class_, EQ::skills::SkillType Skill, uint8 Level) {
	if(!skill_caps_mmf) {
		return 0;
	}

	if(Class_ == 0)
		return 0;

	int SkillMaxLevel = RuleI(Character, SkillCapMaxLevel);
	if(SkillMaxLevel < 1) {
		SkillMaxLevel = RuleI(Character, MaxLevel);
	}

	uint32 class_count = PLAYER_CLASS_COUNT;
	uint32 skill_count = EQ::skills::HIGHEST_SKILL + 1;
	uint32 level_count = HARD_LEVEL_CAP + 1;
	if(Class_ > class_count || static_cast<uint32>(Skill) > skill_count || Level > level_count) {
		return 0;
	}

	if(Level > static_cast<uint8>(SkillMaxLevel)){
		Level = static_cast<uint8>(SkillMaxLevel);
	}

	uint32 index = ((((Class_ - 1) * skill_count) + Skill) * level_count) + Level;
	uint16 *skill_caps_table = reinterpret_cast<uint16*>(skill_caps_mmf->Get());
	return skill_caps_table[index];
}

uint8 SharedDatabase::GetTrainLevel(uint8 Class_, EQ::skills::SkillType Skill, uint8 Level) {
	if(!skill_caps_mmf) {
		return 0;
	}

	if(Class_ == 0)
		return 0;

	int SkillMaxLevel = RuleI(Character, SkillCapMaxLevel);
	if (SkillMaxLevel < 1) {
		SkillMaxLevel = RuleI(Character, MaxLevel);
	}

	uint32 class_count = PLAYER_CLASS_COUNT;
	uint32 skill_count = EQ::skills::HIGHEST_SKILL + 1;
	uint32 level_count = HARD_LEVEL_CAP + 1;
	if(Class_ > class_count || static_cast<uint32>(Skill) > skill_count || Level > level_count) {
		return 0;
	}

	uint8 ret = 0;
	if(Level > static_cast<uint8>(SkillMaxLevel)) {
		uint32 index = ((((Class_ - 1) * skill_count) + Skill) * level_count);
		uint16 *skill_caps_table = reinterpret_cast<uint16*>(skill_caps_mmf->Get());
		for(uint8 x = 0; x < Level; x++){
			if(skill_caps_table[index + x]){
				ret = x;
				break;
			}
		}
	}
	else
	{
		uint32 index = ((((Class_ - 1) * skill_count) + Skill) * level_count);
		uint16 *skill_caps_table = reinterpret_cast<uint16*>(skill_caps_mmf->Get());
		for(int x = 0; x < SkillMaxLevel; x++){
			if(skill_caps_table[index + x]){
				ret = x;
				break;
			}
		}
	}

	if(ret > GetSkillCap(Class_, Skill, Level))
		ret = static_cast<uint8>(GetSkillCap(Class_, Skill, Level));

	return ret;
}

void SharedDatabase::LoadDamageShieldTypes(SPDat_Spell_Struct* sp, int32 iMaxSpellID) {

	std::string query = StringFormat("SELECT `spellid`, `type` FROM `damageshieldtypes` WHERE `spellid` > 0 "
                                    "AND `spellid` <= %i", iMaxSpellID);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    for(auto row = results.begin(); row != results.end(); ++row) {
        int spellID = atoi(row[0]);
        if((spellID > 0) && (spellID <= iMaxSpellID))
            sp[spellID].DamageShieldType = atoi(row[1]);
    }

}

const EvolveInfo* SharedDatabase::GetEvolveInfo(uint32 loregroup) {
	return nullptr;	// nothing here for now... database and/or sharemem pulls later
}

int SharedDatabase::GetMaxSpellID() {
	std::string query = "SELECT MAX(id) FROM spells_new";
	auto results = QueryDatabase(query);
    if (!results.Success()) {
        return -1;
    }

    auto row = results.begin();

	return atoi(row[0]);
}

bool SharedDatabase::LoadSpells(const std::string &prefix, int32 *records, const SPDat_Spell_Struct **sp) {
	spells_mmf.reset(nullptr);

	try {
		auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("spells");
		mutex.Lock();
	
		std::string file_name = Config->SharedMemDir + prefix + std::string("spells");
		spells_mmf = std::unique_ptr<EQ::MemoryMappedFile>(new EQ::MemoryMappedFile(file_name));
		LogInfo("[Shared Memory] Attempting to load file [{}]", file_name);
		*records = *reinterpret_cast<uint32*>(spells_mmf->Get());
		*sp = reinterpret_cast<const SPDat_Spell_Struct*>((char*)spells_mmf->Get() + 4);
		mutex.Unlock();
	}
	catch(std::exception& ex) {
		LogError("Error Loading Spells: {}", ex.what());
		return false;
	}
	return true;
}

void SharedDatabase::LoadSpells(void *data, int max_spells) {
	*(uint32*)data = max_spells;
	SPDat_Spell_Struct *sp = reinterpret_cast<SPDat_Spell_Struct*>((char*)data + sizeof(uint32));

	const std::string query = "SELECT * FROM spells_new ORDER BY id ASC";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    if(results.ColumnCount() <= SPELL_LOAD_FIELD_COUNT) {
		LogSpells("Fatal error loading spells: Spell field count < SPELL_LOAD_FIELD_COUNT([{}])", SPELL_LOAD_FIELD_COUNT);
		return;
    }

    int tempid = 0;
    int counter = 0;

    for (auto row = results.begin(); row != results.end(); ++row) {
        tempid = atoi(row[0]);
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

		sp[tempid].range=static_cast<float>(atof(row[9]));
		sp[tempid].aoerange=static_cast<float>(atof(row[10]));
		sp[tempid].pushback=static_cast<float>(atof(row[11]));
		sp[tempid].pushup=static_cast<float>(atof(row[12]));
		sp[tempid].cast_time=atoi(row[13]);
		sp[tempid].recovery_time=atoi(row[14]);
		sp[tempid].recast_time=atoi(row[15]);
		sp[tempid].buffdurationformula=atoi(row[16]);
		sp[tempid].buffduration=atoi(row[17]);
		sp[tempid].AEDuration=atoi(row[18]);
		sp[tempid].mana=atoi(row[19]);

		int y=0;
		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].base[y]=atoi(row[20+y]); // effect_base_value

		for(y=0; y < EFFECT_COUNT; y++)
			sp[tempid].base2[y]=atoi(row[32+y]); // effect_limit_value

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].max[y]=atoi(row[44+y]);

		for(y=0; y< 4;y++)
			sp[tempid].components[y]=atoi(row[58+y]);

		for(y=0; y< 4;y++)
			sp[tempid].component_counts[y]=atoi(row[62+y]);

		for(y=0; y< 4;y++)
			sp[tempid].NoexpendReagent[y]=atoi(row[66+y]);

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].formula[y]=atoi(row[70+y]);

		sp[tempid].goodEffect=atoi(row[83]);
		sp[tempid].Activated=atoi(row[84]);
		sp[tempid].resisttype=atoi(row[85]);

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].effectid[y]=atoi(row[86+y]);

		sp[tempid].targettype = (SpellTargetType) atoi(row[98]);
		sp[tempid].basediff=atoi(row[99]);

		int tmp_skill = atoi(row[100]);;

		if (tmp_skill < 0 || tmp_skill > EQ::skills::HIGHEST_SKILL)
			sp[tempid].skill = EQ::skills::SkillBegging; /* not much better we can do. */ // can probably be changed to client-based 'SkillNone' once activated
        else
			sp[tempid].skill = (EQ::skills::SkillType) tmp_skill;

		sp[tempid].zonetype=atoi(row[101]);
		sp[tempid].EnvironmentType=atoi(row[102]);
		sp[tempid].TimeOfDay=atoi(row[103]);

		for(y=0; y < PLAYER_CLASS_COUNT;y++)
			sp[tempid].classes[y]=atoi(row[104+y]);

		sp[tempid].CastingAnim=atoi(row[120]);
		sp[tempid].SpellAffectIndex=atoi(row[123]);
		sp[tempid].disallow_sit=atoi(row[124]);
		sp[tempid].diety_agnostic=atoi(row[125]);

		for (y = 0; y < 16; y++)
			sp[tempid].deities[y]=atoi(row[126+y]);

		sp[tempid].new_icon=atoi(row[144]);
		sp[tempid].uninterruptable=atoi(row[146]) != 0;
		sp[tempid].ResistDiff=atoi(row[147]);
		sp[tempid].dot_stacking_exempt = atoi(row[148]) != 0;
		sp[tempid].RecourseLink = atoi(row[150]);
		sp[tempid].no_partial_resist = atoi(row[151]) != 0;

		sp[tempid].short_buff_box = atoi(row[154]);
		sp[tempid].descnum = atoi(row[155]);
		sp[tempid].typedescnum = atoi(row[156]);
		sp[tempid].effectdescnum = atoi(row[157]);

		sp[tempid].npc_no_los = atoi(row[159]) != 0;
		sp[tempid].reflectable = atoi(row[161]) != 0;
		sp[tempid].bonushate=atoi(row[162]);

		sp[tempid].ldon_trap = atoi(row[165]) != 0;
		sp[tempid].EndurCost=atoi(row[166]);
		sp[tempid].EndurTimerIndex=atoi(row[167]);
		sp[tempid].IsDisciplineBuff = atoi(row[168]) != 0;
		sp[tempid].HateAdded=atoi(row[173]);
		sp[tempid].EndurUpkeep=atoi(row[174]);
		sp[tempid].numhitstype = atoi(row[175]);
		sp[tempid].numhits = atoi(row[176]);
		sp[tempid].pvpresistbase=atoi(row[177]);
		sp[tempid].pvpresistcalc=atoi(row[178]);
		sp[tempid].pvpresistcap=atoi(row[179]);
		sp[tempid].spell_category=atoi(row[180]);
		sp[tempid].pcnpc_only_flag=atoi(row[183]);
		sp[tempid].cast_not_standing = atoi(row[184]) != 0;
		sp[tempid].can_mgb=atoi(row[185]);
		sp[tempid].dispel_flag = atoi(row[186]);
		sp[tempid].MinResist = atoi(row[189]);
		sp[tempid].MaxResist = atoi(row[190]);
		sp[tempid].viral_targets = atoi(row[191]);
		sp[tempid].viral_timer = atoi(row[192]);
		sp[tempid].NimbusEffect = atoi(row[193]);
		sp[tempid].directional_start = static_cast<float>(atoi(row[194]));
		sp[tempid].directional_end = static_cast<float>(atoi(row[195]));
		sp[tempid].sneak = atoi(row[196]) != 0;
		sp[tempid].not_focusable = atoi(row[197]) != 0;
		sp[tempid].no_detrimental_spell_aggro = atoi(row[198]) != 0;
		sp[tempid].suspendable = atoi(row[200]) != 0;
		sp[tempid].viral_range = atoi(row[201]);
		sp[tempid].songcap = atoi(row[202]);
		sp[tempid].no_block = atoi(row[205]);
		sp[tempid].spellgroup=atoi(row[207]);
		sp[tempid].rank = atoi(row[208]);
		sp[tempid].no_resist=atoi(row[209]);
		sp[tempid].CastRestriction = atoi(row[211]);
		sp[tempid].AllowRest = atoi(row[212]) != 0;
		sp[tempid].InCombat = atoi(row[213]) != 0;
		sp[tempid].OutofCombat = atoi(row[214]) != 0;
		sp[tempid].override_crit_chance = atoi(row[217]);
		sp[tempid].aemaxtargets = atoi(row[218]);
		sp[tempid].no_heal_damage_item_mod = atoi(row[219]);
		sp[tempid].persistdeath = atoi(row[224]) != 0;
		sp[tempid].min_dist = atof(row[227]);
		sp[tempid].min_dist_mod = atof(row[228]);
		sp[tempid].max_dist = atof(row[229]);
		sp[tempid].max_dist_mod = atof(row[230]);
		sp[tempid].min_range = static_cast<float>(atoi(row[231]));
		sp[tempid].no_remove = atoi(row[232]) != 0;
		sp[tempid].DamageShieldType = 0;
    }

    LoadDamageShieldTypes(sp, max_spells);
}

int SharedDatabase::GetMaxBaseDataLevel() {
	const std::string query = "SELECT MAX(level) FROM base_data";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return -1;
	}

	if (results.RowCount() == 0)
        return -1;

    auto row = results.begin();

	return atoi(row[0]);
}

bool SharedDatabase::LoadBaseData(const std::string &prefix) {
	base_data_mmf.reset(nullptr);

	try {
		auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("base_data");
		mutex.Lock();

		std::string file_name = Config->SharedMemDir + prefix + std::string("base_data");
		base_data_mmf = std::unique_ptr<EQ::MemoryMappedFile>(new EQ::MemoryMappedFile(file_name));
		mutex.Unlock();
	} catch(std::exception& ex) {
		LogError("Error Loading Base Data: {}", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadBaseData(void *data, int max_level) {
	char *base_ptr = reinterpret_cast<char*>(data);

	const std::string query = "SELECT * FROM base_data ORDER BY level, class ASC";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        return;
	}

    int lvl = 0;
    int cl = 0;

    for (auto row = results.begin(); row != results.end(); ++row) {
        lvl = atoi(row[0]);
        cl = atoi(row[1]);

        if(lvl <= 0) {
            LogError("Non fatal error: base_data.level <= 0, ignoring.");
            continue;
        }

        if(lvl >= max_level) {
            LogError("Non fatal error: base_data.level >= max_level, ignoring.");
            continue;
        }

        if(cl <= 0) {
            LogError("Non fatal error: base_data.cl <= 0, ignoring.");
            continue;
        }

        if(cl > 16) {
            LogError("Non fatal error: base_data.class > 16, ignoring.");
            continue;
        }

        BaseDataStruct *bd = reinterpret_cast<BaseDataStruct*>(base_ptr + (((16 * (lvl - 1)) + (cl - 1)) * sizeof(BaseDataStruct)));
		bd->base_hp = atof(row[2]);
		bd->base_mana = atof(row[3]);
		bd->base_end = atof(row[4]);
		bd->hp_regen = atof(row[5]);
		bd->end_regen = atof(row[6]);
		bd->hp_factor = atof(row[7]);
		bd->mana_factor = atof(row[8]);
		bd->endurance_factor = atof(row[9]);
    }
}

const BaseDataStruct* SharedDatabase::GetBaseData(int lvl, int cl) {
	if(!base_data_mmf) {
		return nullptr;
	}

	if(lvl <= 0) {
		return nullptr;
	}

	if(cl <= 0) {
		return nullptr;
	}

	if(cl > 16) {
		return nullptr;
	}

	char *base_ptr = reinterpret_cast<char*>(base_data_mmf->Get());

	uint32 offset = ((16 * (lvl - 1)) + (cl - 1)) * sizeof(BaseDataStruct);

	if(offset >= base_data_mmf->Size()) {
		return nullptr;
	}

	BaseDataStruct *bd = reinterpret_cast<BaseDataStruct*>(base_ptr + offset);
	return bd;
}

void SharedDatabase::GetLootTableInfo(uint32 &loot_table_count, uint32 &max_loot_table, uint32 &loot_table_entries) {
	loot_table_count = 0;
	max_loot_table = 0;
	loot_table_entries = 0;
	const std::string query = "SELECT COUNT(*), MAX(id), (SELECT COUNT(*) FROM loottable_entries) FROM loottable";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

	if (results.RowCount() == 0)
        return;

	auto row = results.begin();

    loot_table_count = static_cast<uint32>(atoul(row[0]));
	max_loot_table = static_cast<uint32>(atoul(row[1] ? row[1] : "0"));
	loot_table_entries = static_cast<uint32>(atoul(row[2]));
}

void SharedDatabase::GetLootDropInfo(uint32 &loot_drop_count, uint32 &max_loot_drop, uint32 &loot_drop_entries) {
	loot_drop_count = 0;
	max_loot_drop = 0;
	loot_drop_entries = 0;

	const std::string query = "SELECT COUNT(*), MAX(id), (SELECT COUNT(*) FROM lootdrop_entries) FROM lootdrop";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

	if (results.RowCount() == 0)
        return;

    auto row =results.begin();

    loot_drop_count = static_cast<uint32>(atoul(row[0]));
	max_loot_drop = static_cast<uint32>(atoul(row[1] ? row[1] : "0"));
	loot_drop_entries = static_cast<uint32>(atoul(row[2]));
}

void SharedDatabase::LoadLootTables(void *data, uint32 size) {
	EQ::FixedMemoryVariableHashSet<LootTable_Struct> hash(reinterpret_cast<uint8*>(data), size);

	uint8 loot_table[sizeof(LootTable_Struct) + (sizeof(LootTableEntries_Struct) * 128)];
	LootTable_Struct *lt = reinterpret_cast<LootTable_Struct*>(loot_table);

	const std::string query = "SELECT loottable.id, loottable.mincash, loottable.maxcash, loottable.avgcoin, "
                            "loottable_entries.lootdrop_id, loottable_entries.multiplier, loottable_entries.droplimit, "
                            "loottable_entries.mindrop, loottable_entries.probability FROM loottable LEFT JOIN loottable_entries "
                            "ON loottable.id = loottable_entries.loottable_id ORDER BY id";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    uint32 current_id = 0;
    uint32 current_entry = 0;

    for (auto row = results.begin(); row != results.end(); ++row) {
        uint32 id = static_cast<uint32>(atoul(row[0]));
        if(id != current_id) {
            if(current_id != 0)
                hash.insert(current_id, loot_table, (sizeof(LootTable_Struct) + (sizeof(LootTableEntries_Struct) * lt->NumEntries)));

            memset(loot_table, 0, sizeof(LootTable_Struct) + (sizeof(LootTableEntries_Struct) * 128));
            current_entry = 0;
            current_id = id;
            lt->mincash = static_cast<uint32>(atoul(row[1]));
            lt->maxcash = static_cast<uint32>(atoul(row[2]));
            lt->avgcoin = static_cast<uint32>(atoul(row[3]));
        }

        if(current_entry > 128)
            continue;

        if(!row[4])
            continue;

        lt->Entries[current_entry].lootdrop_id = static_cast<uint32>(atoul(row[4]));
        lt->Entries[current_entry].multiplier = static_cast<uint8>(atoi(row[5]));
        lt->Entries[current_entry].droplimit = static_cast<uint8>(atoi(row[6]));
        lt->Entries[current_entry].mindrop = static_cast<uint8>(atoi(row[7]));
        lt->Entries[current_entry].probability = static_cast<float>(atof(row[8]));

        ++(lt->NumEntries);
        ++current_entry;
    }

    if(current_id != 0)
        hash.insert(current_id, loot_table, (sizeof(LootTable_Struct) + (sizeof(LootTableEntries_Struct) * lt->NumEntries)));

}

void SharedDatabase::LoadLootDrops(void *data, uint32 size) {

	EQ::FixedMemoryVariableHashSet<LootDrop_Struct> hash(reinterpret_cast<uint8*>(data), size);
	uint8 loot_drop[sizeof(LootDrop_Struct) + (sizeof(LootDropEntries_Struct) * 1260)];
	LootDrop_Struct *ld = reinterpret_cast<LootDrop_Struct*>(loot_drop);

	const std::string query = "SELECT lootdrop.id, lootdrop_entries.item_id, lootdrop_entries.item_charges, "
                            "lootdrop_entries.equip_item, lootdrop_entries.chance, lootdrop_entries.minlevel, "
                            "lootdrop_entries.maxlevel, lootdrop_entries.multiplier FROM lootdrop JOIN lootdrop_entries "
                            "ON lootdrop.id = lootdrop_entries.lootdrop_id ORDER BY lootdrop_id";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return;
    }

    uint32 current_id = 0;
    uint32 current_entry = 0;

    for (auto row = results.begin(); row != results.end(); ++row) {
        uint32 id = static_cast<uint32>(atoul(row[0]));
        if(id != current_id) {
            if(current_id != 0)
                hash.insert(current_id, loot_drop, (sizeof(LootDrop_Struct) +(sizeof(LootDropEntries_Struct) * ld->NumEntries)));

            memset(loot_drop, 0, sizeof(LootDrop_Struct) + (sizeof(LootDropEntries_Struct) * 1260));
			current_entry = 0;
			current_id = id;
        }

		if(current_entry >= 1260)
            continue;

        ld->Entries[current_entry].item_id = static_cast<uint32>(atoul(row[1]));
        ld->Entries[current_entry].item_charges = static_cast<int8>(atoi(row[2]));
        ld->Entries[current_entry].equip_item = static_cast<uint8>(atoi(row[3]));
        ld->Entries[current_entry].chance = static_cast<float>(atof(row[4]));
        ld->Entries[current_entry].minlevel = static_cast<uint8>(atoi(row[5]));
        ld->Entries[current_entry].maxlevel = static_cast<uint8>(atoi(row[6]));
        ld->Entries[current_entry].multiplier = static_cast<uint8>(atoi(row[7]));

        ++(ld->NumEntries);
        ++current_entry;
    }

    if(current_id != 0)
        hash.insert(current_id, loot_drop, (sizeof(LootDrop_Struct) + (sizeof(LootDropEntries_Struct) * ld->NumEntries)));

}

bool SharedDatabase::LoadLoot(const std::string &prefix) {
	loot_table_mmf.reset(nullptr);
	loot_drop_mmf.reset(nullptr);

	try {
		auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("loot");
		mutex.Lock();
		std::string file_name_lt = Config->SharedMemDir + prefix + std::string("loot_table");
		loot_table_mmf = std::unique_ptr<EQ::MemoryMappedFile>(new EQ::MemoryMappedFile(file_name_lt));
		loot_table_hash = std::unique_ptr<EQ::FixedMemoryVariableHashSet<LootTable_Struct>>(new EQ::FixedMemoryVariableHashSet<LootTable_Struct>(
			reinterpret_cast<uint8*>(loot_table_mmf->Get()),
			loot_table_mmf->Size()));
		std::string file_name_ld = Config->SharedMemDir + prefix + std::string("loot_drop");
		loot_drop_mmf = std::unique_ptr<EQ::MemoryMappedFile>(new EQ::MemoryMappedFile(file_name_ld));
		loot_drop_hash = std::unique_ptr<EQ::FixedMemoryVariableHashSet<LootDrop_Struct>>(new EQ::FixedMemoryVariableHashSet<LootDrop_Struct>(
			reinterpret_cast<uint8*>(loot_drop_mmf->Get()),
			loot_drop_mmf->Size()));
		mutex.Unlock();
	} catch(std::exception &ex) {
		LogError("Error loading loot: {}", ex.what());
		return false;
	}

	return true;
}

const LootTable_Struct* SharedDatabase::GetLootTable(uint32 loottable_id) {
	if(!loot_table_hash)
		return nullptr;

	try {
		if(loot_table_hash->exists(loottable_id)) {
			return &loot_table_hash->at(loottable_id);
		}
	} catch(std::exception &ex) {
		LogError("Could not get loot table: {}", ex.what());
	}
	return nullptr;
}

const LootDrop_Struct* SharedDatabase::GetLootDrop(uint32 lootdrop_id) {
	if(!loot_drop_hash)
		return nullptr;

	try {
		if(loot_drop_hash->exists(lootdrop_id)) {
			return &loot_drop_hash->at(lootdrop_id);
		}
	} catch(std::exception &ex) {
		LogError("Could not get loot drop: {}", ex.what());
	}
	return nullptr;
}

void SharedDatabase::LoadCharacterInspectMessage(uint32 character_id, InspectMessage_Struct* message) {
	std::string query = StringFormat("SELECT `inspect_message` FROM `character_inspect_messages` WHERE `id` = %u LIMIT 1", character_id);
	auto results = QueryDatabase(query);
	auto row = results.begin();
	memset(message, '\0', sizeof(InspectMessage_Struct));
	for (auto row = results.begin(); row != results.end(); ++row) {
		memcpy(message, row[0], sizeof(InspectMessage_Struct));
	}
}

void SharedDatabase::SaveCharacterInspectMessage(uint32 character_id, const InspectMessage_Struct* message) {
	std::string query = StringFormat("REPLACE INTO `character_inspect_messages` (id, inspect_message) VALUES (%u, '%s')", character_id, EscapeString(message->text).c_str());
	auto results = QueryDatabase(query);
}
