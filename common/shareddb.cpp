#include <iostream>
#include <cstring>
#include <cstdlib>

#include "shareddb.h"
#include "mysql.h"
#include "Item.h"
#include "classes.h"
#include "rulesys.h"
#include "seperator.h"
#include "StringUtil.h"
#include "eq_packet_structs.h"
#include "guilds.h"
#include "extprofile.h"
#include "memory_mapped_file.h"
#include "ipc_mutex.h"
#include "eqemu_exception.h"
#include "loottable.h"
#include "faction.h"
#include "features.h"

SharedDatabase::SharedDatabase()
: Database(), skill_caps_mmf(nullptr), items_mmf(nullptr), items_hash(nullptr), faction_mmf(nullptr), faction_hash(nullptr),
	loot_table_mmf(nullptr), loot_table_hash(nullptr), loot_drop_mmf(nullptr), loot_drop_hash(nullptr), base_data_mmf(nullptr)
{
}

SharedDatabase::SharedDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
: Database(host, user, passwd, database, port), skill_caps_mmf(nullptr), items_mmf(nullptr), items_hash(nullptr),
	faction_mmf(nullptr), faction_hash(nullptr), loot_table_mmf(nullptr), loot_table_hash(nullptr), loot_drop_mmf(nullptr),
	loot_drop_hash(nullptr), base_data_mmf(nullptr)
{
}

SharedDatabase::~SharedDatabase() {
	safe_delete(skill_caps_mmf);
	safe_delete(items_mmf);
	safe_delete(items_hash);
	safe_delete(faction_mmf);
	safe_delete(faction_hash);
	safe_delete(loot_table_mmf);
	safe_delete(loot_drop_mmf);
	safe_delete(loot_table_hash);
	safe_delete(loot_drop_hash);
	safe_delete(base_data_mmf);
}

bool SharedDatabase::SetHideMe(uint32 account_id, uint8 hideme)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE account SET hideme = %i where id = %i", hideme, account_id), errbuf)) {
		std::cerr << "Error in SetGMSpeed query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return true;

}

uint8 SharedDatabase::GetGMSpeed(uint32 account_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT gmspeed FROM account where id='%i'", account_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			uint8 gmspeed = atoi(row[0]);
			mysql_free_result(result);
			return gmspeed;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
		mysql_free_result(result);
	}
	else
	{

		std::cerr << "Error in GetGMSpeed query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	return 0;


}

bool SharedDatabase::SetGMSpeed(uint32 account_id, uint8 gmspeed)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE account SET gmspeed = %i where id = %i", gmspeed, account_id), errbuf)) {
		std::cerr << "Error in SetGMSpeed query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return true;

}

uint32 SharedDatabase::GetTotalTimeEntitledOnAccount(uint32 AccountID) {

	uint32 EntitledTime = 0;

	const char *EntitledQuery = "select sum(ascii(substring(profile, 237, 1)) + (ascii(substring(profile, 238, 1)) * 256) +"
				"(ascii(substring(profile, 239, 1)) * 65536) + (ascii(substring(profile, 240, 1)) * 16777216))"
				"from character_ where account_id = %i";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, EntitledQuery, AccountID), errbuf, &result)) {

		if (mysql_num_rows(result) == 1) {

			row = mysql_fetch_row(result);

			EntitledTime = atoi(row[0]);
		}

		mysql_free_result(result);
	}

	safe_delete_array(query);

	return EntitledTime;
}

bool SharedDatabase::SaveCursor(uint32 char_id, std::list<ItemInst*>::const_iterator &start, std::list<ItemInst*>::const_iterator &end)
{
iter_queue it;
int i;
bool ret=true;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	// Delete cursor items
	if ((ret = RunQuery(query, MakeAnyLenString(&query, "DELETE FROM inventory WHERE charid=%i AND ( (slotid >=8000 and slotid<=8999) or slotid=30 or (slotid>=331 and slotid<=340))", char_id), errbuf))) {
		for(it=start,i=8000;it!=end;it++,i++) {
			ItemInst *inst=*it;
			if (!(ret=SaveInventory(char_id,inst,(i==8000) ? 30 : i)))
				break;
		}
	} else {
		std::cout << "Clearing cursor failed: " << errbuf << std::endl;
	}
	safe_delete_array(query);

	return ret;
}

bool SharedDatabase::VerifyInventory(uint32 account_id, int16 slot_id, const ItemInst* inst)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	// Delete cursor items
	if (!RunQuery(query, MakeAnyLenString(&query,
		"SELECT itemid,charges FROM sharedbank "
		"WHERE acctid=%d AND slotid=%d",
		account_id, slot_id), errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error runing inventory verification query '%s': %s", query, errbuf);
		safe_delete_array(query);
		//returning true is less harmful in the face of a query error
		return(true);
	}
	safe_delete_array(query);

	row = mysql_fetch_row(result);
	bool found = false;
	if(row) {
		uint32 id = atoi(row[0]);
		uint16 charges = atoi(row[1]);

		uint16 expect_charges = 0;
		if(inst->GetCharges() >= 0)
			expect_charges = inst->GetCharges();
		else
			expect_charges = 0x7FFF;

		if(id == inst->GetItem()->ID && charges == expect_charges)
			found = true;
	}
	mysql_free_result(result);
	return(found);
}

bool SharedDatabase::SaveInventory(uint32 char_id, const ItemInst* inst, int16 slot_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	bool ret = false;
	uint32 augslot[5] = { 0, 0, 0, 0, 0 };

	//never save tribute slots:
	if(slot_id >= 400 && slot_id <= 404)
		return(true);

	if (inst && inst->IsType(ItemClassCommon)) {
		for(int i=0;i<5;i++) {
			ItemInst *auginst=inst->GetItem(i);
			augslot[i]=(auginst && auginst->GetItem()) ? auginst->GetItem()->ID : 0;
		}
	}

	if (slot_id>=2500 && slot_id<=2600) { // Shared bank inventory
		if (!inst) {
			// Delete item
			uint32 account_id = GetAccountIDByChar(char_id);
			uint32 len_query = MakeAnyLenString(&query, "DELETE FROM sharedbank WHERE acctid=%i AND slotid=%i",
				account_id, slot_id);

			ret = RunQuery(query, len_query, errbuf);

			// Delete bag slots, if need be
			if (ret && Inventory::SupportsContainers(slot_id)) {
				safe_delete_array(query);
				int16 base_slot_id = Inventory::CalcSlotId(slot_id, 0);
				ret = RunQuery(query, MakeAnyLenString(&query, "DELETE FROM sharedbank WHERE acctid=%i AND slotid>=%i AND slotid<%i",
					account_id, base_slot_id, (base_slot_id+10)), errbuf);
			}

			// @merth: need to delete augments here
		}
		else {
			// Update/Insert item
			uint32 account_id = GetAccountIDByChar(char_id);
			uint16 charges = 0;
			if(inst->GetCharges() >= 0)
				charges = inst->GetCharges();
			else
				charges = 0x7FFF;

			uint32 len_query = MakeAnyLenString(&query,
				"REPLACE INTO sharedbank "
				"	(acctid,slotid,itemid,charges,custom_data,"
				"	augslot1,augslot2,augslot3,augslot4,augslot5)"
				" VALUES(%lu,%lu,%lu,%lu,'%s',"
				"	%lu,%lu,%lu,%lu,%lu)",
				(unsigned long)account_id, (unsigned long)slot_id, (unsigned long)inst->GetItem()->ID, (unsigned long)charges,
				inst->GetCustomDataString().c_str(),
				(unsigned long)augslot[0],(unsigned long)augslot[1],(unsigned long)augslot[2],(unsigned long)augslot[3],(unsigned long)augslot[4]);


			ret = RunQuery(query, len_query, errbuf);
		}
	}
	else { // All other inventory
		if (!inst) {
			// Delete item
			ret = RunQuery(query, MakeAnyLenString(&query, "DELETE FROM inventory WHERE charid=%i AND slotid=%i",
				char_id, slot_id), errbuf);

			// Delete bag slots, if need be
			if (ret && Inventory::SupportsContainers(slot_id)) {
				safe_delete_array(query);
				int16 base_slot_id = Inventory::CalcSlotId(slot_id, 0);
				ret = RunQuery(query, MakeAnyLenString(&query, "DELETE FROM inventory WHERE charid=%i AND slotid>=%i AND slotid<%i",
					char_id, base_slot_id, (base_slot_id+10)), errbuf);
			}

			// @merth: need to delete augments here
		}
		else {
			uint16 charges = 0;
			if(inst->GetCharges() >= 0)
				charges = inst->GetCharges();
			else
				charges = 0x7FFF;
			// Update/Insert item
			uint32 len_query = MakeAnyLenString(&query,
				"REPLACE INTO inventory "
				"	(charid,slotid,itemid,charges,instnodrop,custom_data,color,"
				"	augslot1,augslot2,augslot3,augslot4,augslot5)"
				" VALUES(%lu,%lu,%lu,%lu,%lu,'%s',%lu,"
				"	%lu,%lu,%lu,%lu,%lu)",
				(unsigned long)char_id, (unsigned long)slot_id, (unsigned long)inst->GetItem()->ID, (unsigned long)charges,
				(unsigned long)(inst->IsInstNoDrop() ? 1:0),inst->GetCustomDataString().c_str(),(unsigned long)inst->GetColor(),
				(unsigned long)augslot[0],(unsigned long)augslot[1],(unsigned long)augslot[2],(unsigned long)augslot[3],(unsigned long)augslot[4] );

			ret = RunQuery(query, len_query, errbuf);
		}
	}

	if (!ret)
		LogFile->write(EQEMuLog::Error, "SaveInventory query '%s': %s", query, errbuf);
	safe_delete_array(query);

	// Save bag contents, if slot supports bag contents
	if (inst && inst->IsType(ItemClassContainer) && Inventory::SupportsContainers(slot_id)) {
		for (uint8 idx=0; idx<10; idx++) {
			const ItemInst* baginst = inst->GetItem(idx);
			SaveInventory(char_id, baginst, Inventory::CalcSlotId(slot_id, idx));
		}
	}

	// @merth: need to save augments here

	return ret;
}

int32 SharedDatabase::GetSharedPlatinum(uint32 account_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT sharedplat FROM account WHERE id='%i'", account_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			uint32 shared_platinum = atoi(row[0]);
			mysql_free_result(result);
			return shared_platinum;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error in GetSharedPlatinum query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	return 0;
}

bool SharedDatabase::SetSharedPlatinum(uint32 account_id, int32 amount_to_add)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE account SET sharedplat = sharedplat + %i WHERE id = %i", amount_to_add, account_id), errbuf)) {
		std::cerr << "Error in SetSharedPlatinum query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return true;
}

bool SharedDatabase::SetStartingItems(PlayerProfile_Struct* pp, Inventory* inv, uint32 si_race, uint32 si_class, uint32 si_deity, uint32 si_current_zone, char* si_name, int admin_level)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	const Item_Struct* myitem;

	RunQuery
	(
		query,
		MakeAnyLenString
		(
			&query,
			"SELECT itemid, item_charges, slot FROM starting_items "
			"WHERE (race = %i or race = 0) AND (class = %i or class = 0) AND "
			"(deityid = %i or deityid=0) AND (zoneid = %i or zoneid = 0) AND "
			"gm <= %i ORDER BY id",
			si_race, si_class, si_deity, si_current_zone, admin_level
		),
		errbuf,
		&result
	);
	safe_delete_array(query);

	while((row = mysql_fetch_row(result))) {
		int itemid = atoi(row[0]);
		int charges = atoi(row[1]);
		int slot = atoi(row[2]);
		myitem = GetItem(itemid);
		if(!myitem)
			continue;
		ItemInst* myinst = CreateBaseItem(myitem, charges);
		if(slot < 0)
			slot = inv->FindFreeSlot(0,0);
		inv->PutItem(slot, *myinst);
		safe_delete(myinst);
	}

	if(result) mysql_free_result(result);

	return true;
}


// Retrieve shared bank inventory based on either account or character
bool SharedDatabase::GetSharedBank(uint32 id, Inventory* inv, bool is_charid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 len_query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	bool ret = false;

	if (is_charid) {
		len_query = MakeAnyLenString(&query,
			"SELECT sb.slotid,sb.itemid,sb.charges,sb.augslot1,sb.augslot2,sb.augslot3,sb.augslot4,sb.augslot5,sb.custom_data from sharedbank sb "
			"INNER JOIN character_ ch ON ch.account_id=sb.acctid "
			"WHERE ch.id=%i", id);
	}
	else {
		len_query = MakeAnyLenString(&query,
			"SELECT slotid,itemid,charges,augslot1,augslot2,augslot3,augslot4,augslot5,custom_data from sharedbank WHERE acctid=%i", id);
	}

	if (RunQuery(query, len_query, errbuf, &result)) {
		while ((row = mysql_fetch_row(result))) {
			int16 slot_id	= (int16)atoi(row[0]);
			uint32 item_id	= (uint32)atoi(row[1]);
			int8 charges	= (int8)atoi(row[2]);
			uint32 aug[5];
			aug[0]	= (uint32)atoi(row[3]);
			aug[1]	= (uint32)atoi(row[4]);
			aug[2]	= (uint32)atoi(row[5]);
			aug[3]	= (uint32)atoi(row[6]);
			aug[4]	= (uint32)atoi(row[7]);
			const Item_Struct* item = GetItem(item_id);

			if (item) {
				int16 put_slot_id = SLOT_INVALID;

				ItemInst* inst = CreateBaseItem(item, charges);
				if (item->ItemClass == ItemClassCommon) {
					for(int i=0;i<5;i++) {
						if (aug[i]) {
							inst->PutAugment(this, i, aug[i]);
						}
					}
				}
				if(row[8]) {
					std::string data_str(row[8]);
					std::string id;
					std::string value;
					bool use_id = true;

					for(int i = 0; i < data_str.length(); ++i) {
						if(data_str[i] == '^') {
							if(!use_id) {
								inst->SetCustomData(id, value);
								id.clear();
								value.clear();
							}
							use_id = !use_id;
						}
						else {
							char v = data_str[i];
							if(use_id) {
								id.push_back(v);
							} else {
								value.push_back(v);
							}
						}
					}
				}

				put_slot_id = inv->PutItem(slot_id, *inst);
				safe_delete(inst);

				// Save ptr to item in inventory
				if (put_slot_id == SLOT_INVALID) {
					LogFile->write(EQEMuLog::Error,
						"Warning: Invalid slot_id for item in shared bank inventory: %s=%i, item_id=%i, slot_id=%i",
						((is_charid==true) ? "charid" : "acctid"), id, item_id, slot_id);

					if(is_charid)
						SaveInventory(id,nullptr,slot_id);
				}
			}
			else {
				LogFile->write(EQEMuLog::Error,
					"Warning: %s %i has an invalid item_id %i in inventory slot %i",
					((is_charid==true) ? "charid" : "acctid"), id, item_id, slot_id);
			}
		}

		mysql_free_result(result);
		ret = true;
	}
	else {
		LogFile->write(EQEMuLog::Error, "Database::GetSharedBank(uint32 account_id): %s", errbuf);
	}

	safe_delete_array(query);
	return ret;
}


// Overloaded: Retrieve character inventory based on character id
bool SharedDatabase::GetInventory(uint32 char_id, Inventory* inv) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
	bool ret = false;

	// Retrieve character inventory
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT slotid,itemid,charges,color,augslot1,augslot2,augslot3,augslot4,augslot5,"
		"instnodrop,custom_data FROM inventory WHERE charid=%i ORDER BY slotid", char_id), errbuf, &result)) {

		while ((row = mysql_fetch_row(result))) {
			int16 slot_id	= atoi(row[0]);
			uint32 item_id	= atoi(row[1]);
			uint16 charges	= atoi(row[2]);
			uint32 color		= atoul(row[3]);
			uint32 aug[5];
			aug[0]	= (uint32)atoul(row[4]);
			aug[1]	= (uint32)atoul(row[5]);
			aug[2]	= (uint32)atoul(row[6]);
			aug[3]	= (uint32)atoul(row[7]);
			aug[4]	= (uint32)atoul(row[8]);
			bool instnodrop	= (row[9] && (uint16)atoi(row[9])) ? true : false;

			const Item_Struct* item = GetItem(item_id);

			if (item) {
				int16 put_slot_id = SLOT_INVALID;

				ItemInst* inst = CreateBaseItem(item, charges);

				if(row[10]) {
					std::string data_str(row[10]);
					std::string id;
					std::string value;
					bool use_id = true;

					for(int i = 0; i < data_str.length(); ++i) {
						if(data_str[i] == '^') {
							if(!use_id) {
								inst->SetCustomData(id, value);
								id.clear();
								value.clear();
							}
							use_id = !use_id;
						}
						else {
							char v = data_str[i];
							if(use_id) {
								id.push_back(v);
							} else {
								value.push_back(v);
							}
						}
					}
				}

				if (instnodrop || (slot_id >= 0 && slot_id <= 21 && inst->GetItem()->Attuneable))
						inst->SetInstNoDrop(true);
				if (color > 0)
					inst->SetColor(color);
				if(charges==0x7FFF)
					inst->SetCharges(-1);
				else
					inst->SetCharges(charges);

				if (item->ItemClass == ItemClassCommon) {
					for(int i=0;i<5;i++) {
						if (aug[i]) {
							inst->PutAugment(this, i, aug[i]);
						}
					}
				}

				if (slot_id>=8000 && slot_id <= 8999)
					put_slot_id = inv->PushCursor(*inst);
				else
					put_slot_id = inv->PutItem(slot_id, *inst);
				safe_delete(inst);

				// Save ptr to item in inventory
				if (put_slot_id == SLOT_INVALID) {
					LogFile->write(EQEMuLog::Error,
						"Warning: Invalid slot_id for item in inventory: charid=%i, item_id=%i, slot_id=%i",
						char_id, item_id, slot_id);
				}
			}
			else {
				LogFile->write(EQEMuLog::Error,
					"Warning: charid %i has an invalid item_id %i in inventory slot %i",
					char_id, item_id, slot_id);
			}
		}
		mysql_free_result(result);

		// Retrieve shared inventory
		ret = GetSharedBank(char_id, inv, true);
	}
	else {
		LogFile->write(EQEMuLog::Error, "GetInventory query '%s' %s", query, errbuf);
		LogFile->write(EQEMuLog::Error, "If you got an error related to the 'instnodrop' field, run the following SQL Queries:\nalter table inventory add instnodrop tinyint(1) unsigned default 0 not null;\n");
	}

	safe_delete_array(query);
	return ret;
}

// Overloaded: Retrieve character inventory based on account_id and character name
bool SharedDatabase::GetInventory(uint32 account_id, char* name, Inventory* inv) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
	bool ret = false;

	// Retrieve character inventory
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT slotid,itemid,charges,color,augslot1,augslot2,augslot3,augslot4,augslot5,"
		"instnodrop,custom_data FROM inventory INNER JOIN character_ ch ON ch.id=charid WHERE ch.name='%s' AND ch.account_id=%i ORDER BY slotid",
		name, account_id), errbuf, &result))
	{
		while ((row = mysql_fetch_row(result))) {
			int16 slot_id	= atoi(row[0]);
			uint32 item_id	= atoi(row[1]);
			int8 charges	= atoi(row[2]);
			uint32 color		= atoul(row[3]);
			uint32 aug[5];
			aug[0]	= (uint32)atoi(row[4]);
			aug[1]	= (uint32)atoi(row[5]);
			aug[2]	= (uint32)atoi(row[6]);
			aug[3]	= (uint32)atoi(row[7]);
			aug[4]	= (uint32)atoi(row[8]);
			bool instnodrop	= (row[9] && (uint16)atoi(row[9])) ? true : false;
			const Item_Struct* item = GetItem(item_id);
			int16 put_slot_id = SLOT_INVALID;
			if(!item)
				continue;

			ItemInst* inst = CreateBaseItem(item, charges);
			inst->SetInstNoDrop(instnodrop);

			if(row[10]) {
				std::string data_str(row[10]);
				std::string id;
				std::string value;
				bool use_id = true;

				for(int i = 0; i < data_str.length(); ++i) {
					if(data_str[i] == '^') {
						if(!use_id) {
							inst->SetCustomData(id, value);
							id.clear();
							value.clear();
						}
						use_id = !use_id;
					}
					else {
						char v = data_str[i];
						if(use_id) {
							id.push_back(v);
						} else {
							value.push_back(v);
						}
					}
				}
			}

			if (color > 0)
				inst->SetColor(color);
			inst->SetCharges(charges);

			if (item->ItemClass == ItemClassCommon) {
				for(int i=0;i<5;i++) {
					if (aug[i]) {
						inst->PutAugment(this, i, aug[i]);
					}
				}
			}
			if (slot_id>=8000 && slot_id <= 8999)
				put_slot_id = inv->PushCursor(*inst);
			else
				put_slot_id = inv->PutItem(slot_id, *inst);
			safe_delete(inst);

			// Save ptr to item in inventory
			if (put_slot_id == SLOT_INVALID) {
				LogFile->write(EQEMuLog::Error,
					"Warning: Invalid slot_id for item in inventory: name=%s, acctid=%i, item_id=%i, slot_id=%i",
					name, account_id, item_id, slot_id);
			}
		}
		mysql_free_result(result);

		// Retrieve shared inventory
		ret = GetSharedBank(account_id, inv, false);
	}
	else {
		LogFile->write(EQEMuLog::Error, "GetInventory query '%s' %s", query, errbuf);
		LogFile->write(EQEMuLog::Error, "If you got an error related to the 'instnodrop' field, run the following SQL Queries:\nalter table inventory add instnodrop tinyint(1) unsigned default 0 not null;\n");
	}

	safe_delete_array(query);
	return ret;
}


void SharedDatabase::GetItemsCount(int32 &item_count, uint32 &max_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	item_count = -1;
	max_id = 0;

	char query[] = "SELECT MAX(id), count(*) FROM items";
	if (RunQuery(query, static_cast<uint32>(strlen(query)), errbuf, &result)) {
		row = mysql_fetch_row(result);
		if (row != nullptr && row[1] != 0) {
			item_count = atoi(row[1]);
			if(row[0])
				max_id = atoi(row[0]);
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in GetItemsCount '%s': '%s'", query, errbuf);
	}
}

bool SharedDatabase::LoadItems() {
	if(items_mmf) {
		return true;
	}

	try {
		EQEmu::IPCMutex mutex("items");
		mutex.Lock();
		items_mmf = new EQEmu::MemoryMappedFile("shared/items");

		int32 items = -1;
		uint32 max_item = 0;
		GetItemsCount(items, max_item);
		if(items == -1) {
			EQ_EXCEPT("SharedDatabase", "Database returned no result");
		}
		uint32 size = static_cast<uint32>(EQEmu::FixedMemoryHashSet<Item_Struct>::estimated_size(items, max_item));
		if(items_mmf->Size() != size) {
			EQ_EXCEPT("SharedDatabase", "Couldn't load items because items_mmf->Size() != size");
		}

		items_hash = new EQEmu::FixedMemoryHashSet<Item_Struct>(reinterpret_cast<uint8*>(items_mmf->Get()), size);
		mutex.Unlock();
	} catch(std::exception& ex) {
		LogFile->write(EQEMuLog::Error, "Error Loading Items: %s", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadItems(void *data, uint32 size, int32 items, uint32 max_item_id) {
	EQEmu::FixedMemoryHashSet<Item_Struct> hash(reinterpret_cast<uint8*>(data), size, items, max_item_id);
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;

	char ndbuffer[4];
	bool disableNoRent = false;
	if(GetVariable("disablenorent", ndbuffer, 4)) {
		if(ndbuffer[0] == '1' && ndbuffer[1] == '\0') {
			disableNoRent = true;
		}
	}
	bool disableNoDrop = false;
	if(GetVariable("disablenodrop", ndbuffer, 4)) {
		if(ndbuffer[0] == '1' && ndbuffer[1] == '\0') {
			disableNoDrop = true;
		}
	}
	bool disableLoreGroup = false;
	if(GetVariable("disablelore", ndbuffer, 4)) {
		if(ndbuffer[0] == '1' && ndbuffer[1] == '\0') {
			disableLoreGroup = true;
		}
	}
	bool disableNoTransfer = false;
	if(GetVariable("disablenotransfer", ndbuffer, 4)) {
		if(ndbuffer[0] == '1' && ndbuffer[1] == '\0') {
			disableNoTransfer = true;
		}
	}

	char query[] = "select source,"
#define F(x) "`"#x"`,"
#include "item_fieldlist.h"
#undef F
		"updated"
		" from items order by id";
	Item_Struct item;
	if(RunQuery(query, sizeof(query), errbuf, &result)) {
		while((row = mysql_fetch_row(result))) {
			memset(&item, 0, sizeof(Item_Struct));

			item.ItemClass = (uint8)atoi(row[ItemField::itemclass]);
			strcpy(item.Name,row[ItemField::name]);
			strcpy(item.Lore,row[ItemField::lore]);
			strcpy(item.IDFile,row[ItemField::idfile]);
			item.ID = (uint32)atoul(row[ItemField::id]);
			item.Weight = (uint8)atoi(row[ItemField::weight]);
			item.NoRent = disableNoRent ? (uint8)atoi("255") : (uint8)atoi(row[ItemField::norent]);
			item.NoDrop = disableNoDrop ? (uint8)atoi("255") : (uint8)atoi(row[ItemField::nodrop]);
			item.Size = (uint8)atoi(row[ItemField::size]);
			item.Slots = (uint32)atoul(row[ItemField::slots]);
			item.Price = (uint32)atoul(row[ItemField::price]);
			item.Icon = (uint32)atoul(row[ItemField::icon]);
			item.BenefitFlag = (atoul(row[ItemField::benefitflag]) != 0);
			item.Tradeskills = (atoi(row[ItemField::tradeskills])==0) ? false : true;
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
			//item.Unk033 = (int32)atoul(row[ItemField::UNK033]);
			item.SkillModType = (uint32)atoul(row[ItemField::skillmodtype]);
			item.BaneDmgRace = (uint32)atoul(row[ItemField::banedmgrace]);
			item.BaneDmgAmt = (int8)atoi(row[ItemField::banedmgamt]);
			item.BaneDmgBody = (uint32)atoul(row[ItemField::banedmgbody]);
			item.Magic = (atoi(row[ItemField::magic])==0) ? false : true;
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
			//item.Unk054 = (uint32)atoul(row[ItemField::UNK054]);
			item.MaxCharges = (int16)atoi(row[ItemField::maxcharges]);
			item.ItemType = (uint8)atoi(row[ItemField::itemtype]);
			item.Material = (uint8)atoi(row[ItemField::material]);
			item.SellRate = (float)atof(row[ItemField::sellrate]);
			//item.Unk059 = (uint32)atoul(row[ItemField::UNK059]);
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
			strcpy(item.CharmFile,row[ItemField::charmfile]);
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
			item.LDoNTheme = (uint32)atoul(row[ItemField::ldontheme]);
			item.LDoNPrice = (uint32)atoul(row[ItemField::ldonprice]);
			item.LDoNSold = (uint32)atoul(row[ItemField::ldonsold]);
			item.BagType = (uint8)atoi(row[ItemField::bagtype]);
			item.BagSlots = (uint8)atoi(row[ItemField::bagslots]);
			item.BagSize = (uint8)atoi(row[ItemField::bagsize]);
			item.BagWR = (uint8)atoi(row[ItemField::bagwr]);
			item.Book = (uint8)atoi(row[ItemField::book]);
			item.BookType = (uint32)atoul(row[ItemField::booktype]);
			strcpy(item.Filename,row[ItemField::filename]);
			item.BaneDmgRaceAmt = (uint32)atoul(row[ItemField::banedmgraceamt]);
			item.AugRestrict = (uint32)atoul(row[ItemField::augrestrict]);
			item.LoreGroup = disableLoreGroup ? (uint8)atoi("0") : atoi(row[ItemField::loregroup]);
			item.LoreFlag = item.LoreGroup!=0;
			item.PendingLoreFlag = (atoi(row[ItemField::pendingloreflag])==0) ? false : true;
			item.ArtifactFlag = (atoi(row[ItemField::artifactflag])==0) ? false : true;
			item.SummonedFlag = (atoi(row[ItemField::summonedflag])==0) ? false : true;
			item.Favor = (uint32)atoul(row[ItemField::favor]);
			item.FVNoDrop = (atoi(row[ItemField::fvnodrop])==0) ? false : true;
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
			item.Attuneable = (atoi(row[ItemField::attuneable])==0) ? false : true;
			item.NoPet = (atoi(row[ItemField::nopet])==0) ? false : true;
			item.PointType = (uint32)atoul(row[ItemField::pointtype]);
			item.PotionBelt = (atoi(row[ItemField::potionbelt])==0) ? false : true;
			item.PotionBeltSlots = (atoi(row[ItemField::potionbeltslots])==0) ? false : true;
			item.StackSize = (uint16)atoi(row[ItemField::stacksize]);
			item.NoTransfer = disableNoTransfer ? false : (atoi(row[ItemField::notransfer])==0) ? false : true;
			item.Stackable = (atoi(row[ItemField::stackable])==0) ? false : true;
			item.Click.Effect = (uint32)atoul(row[ItemField::clickeffect]);
			item.Click.Type = (uint8)atoul(row[ItemField::clicktype]);
			item.Click.Level = (uint8)atoul(row[ItemField::clicklevel]);
			item.Click.Level2 = (uint8)atoul(row[ItemField::clicklevel2]);
			strcpy(item.CharmFile,row[ItemField::charmfile]);
			item.Proc.Effect = (uint16)atoul(row[ItemField::proceffect]);
			item.Proc.Type = (uint8)atoul(row[ItemField::proctype]);
			item.Proc.Level = (uint8)atoul(row[ItemField::proclevel]);
			item.Proc.Level2 = (uint8)atoul(row[ItemField::proclevel2]);
			item.Worn.Effect = (uint16)atoul(row[ItemField::worneffect]);
			item.Worn.Type = (uint8)atoul(row[ItemField::worntype]);
			item.Worn.Level = (uint8)atoul(row[ItemField::wornlevel]);
			item.Worn.Level2 = (uint8)atoul(row[ItemField::wornlevel2]);
			item.Focus.Effect = (uint16)atoul(row[ItemField::focuseffect]);
			item.Focus.Type = (uint8)atoul(row[ItemField::focustype]);
			item.Focus.Level = (uint8)atoul(row[ItemField::focuslevel]);
			item.Focus.Level2 = (uint8)atoul(row[ItemField::focuslevel2]);
			item.Scroll.Effect = (uint16)atoul(row[ItemField::scrolleffect]);
			item.Scroll.Type = (uint8)atoul(row[ItemField::scrolltype]);
			item.Scroll.Level = (uint8)atoul(row[ItemField::scrolllevel]);
			item.Scroll.Level2 = (uint8)atoul(row[ItemField::scrolllevel2]);
			item.Bard.Effect = (uint16)atoul(row[ItemField::bardeffect]);
			item.Bard.Type = (uint8)atoul(row[ItemField::bardtype]);
			item.Bard.Level = (uint8)atoul(row[ItemField::bardlevel]);
			item.Bard.Level2 = (uint8)atoul(row[ItemField::bardlevel2]);
			item.QuestItemFlag = (atoi(row[ItemField::questitemflag])==0) ? false : true;
			item.SVCorruption = (int32)atoi(row[ItemField::svcorruption]);
			item.Purity = (uint32)atoul(row[ItemField::purity]);
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
			strcpy(item.ClickName,row[ItemField::clickname]);
			strcpy(item.ProcName,row[ItemField::procname]);
			strcpy(item.WornName,row[ItemField::wornname]);
			strcpy(item.FocusName,row[ItemField::focusname]);
			strcpy(item.ScrollName,row[ItemField::scrollname]);

			try {
				hash.insert(item.ID, item);
			} catch(std::exception &ex) {
				LogFile->write(EQEMuLog::Error, "Database::LoadItems: %s", ex.what());
				break;
			}
		}

		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "LoadItems '%s', %s", query, errbuf);
	}
}

const Item_Struct* SharedDatabase::GetItem(uint32 id) {
	if(!items_hash || id > items_hash->max_key()) {
		return nullptr;
	}

	if(items_hash->exists(id)) {
		return &(items_hash->at(id));
	}

	return nullptr;
}

const Item_Struct* SharedDatabase::IterateItems(uint32* id) {
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

std::string SharedDatabase::GetBook(const char *txtfile)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char txtfile2[20];
	std::string txtout;
	strcpy(txtfile2,txtfile);
	if (!RunQuery(query, MakeAnyLenString(&query, "SELECT txtfile FROM books where name='%s'", txtfile2), errbuf, &result)) {
		std::cerr << "Error in GetBook query '" << query << "' " << errbuf << std::endl;
		if (query != 0)
			safe_delete_array(query);
		txtout.assign(" ",1);
		return txtout;
	}
	else {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 0) {
			mysql_free_result(result);
			LogFile->write(EQEMuLog::Error, "No book to send, (%s)", txtfile);
			txtout.assign(" ",1);
			return txtout;
		}
		else {
			row = mysql_fetch_row(result);
			txtout.assign(row[0],strlen(row[0]));
			mysql_free_result(result);
			return txtout;
		}
	}
}

void SharedDatabase::GetFactionListInfo(uint32 &list_count, uint32 &max_lists) {
	list_count = 0;
	max_lists = 0;
	const char *query = "SELECT COUNT(*), MAX(id) FROM npc_faction";
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(RunQuery(query, strlen(query), errbuf, &result)) {
		if(row = mysql_fetch_row(result)) {
			list_count = static_cast<uint32>(atoul(row[0]));
			max_lists = static_cast<uint32>(atoul(row[1]));
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error getting npc faction info from database: %s, %s", query, errbuf);
	}
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
	EQEmu::FixedMemoryHashSet<NPCFactionList> hash(reinterpret_cast<uint8*>(data), size, list_count, max_lists);
	const char *query = "SELECT npc_faction.id, npc_faction.primaryfaction, npc_faction.ignore_primary_assist, "
		"npc_faction_entries.faction_id, npc_faction_entries.value, npc_faction_entries.npc_value, npc_faction_entries.temp "
		"FROM npc_faction LEFT JOIN npc_faction_entries ON npc_faction.id = npc_faction_entries.npc_faction_id ORDER BY "
		"npc_faction.id;";

	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	NPCFactionList faction;

	if(RunQuery(query, strlen(query), errbuf, &result)) {
		uint32 current_id = 0;
		uint32 current_entry = 0;
		while(row = mysql_fetch_row(result)) {
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

			if(!row[3]) {
				continue;
			}

			if(current_entry >= MAX_NPC_FACTIONS) {
				continue;
			}

			faction.factionid[current_entry] = static_cast<uint32>(atoul(row[3]));
			faction.factionvalue[current_entry] = static_cast<int32>(atoi(row[4]));
			faction.factionnpcvalue[current_entry] = static_cast<int8>(atoi(row[5]));
			faction.factiontemp[current_entry] = static_cast<uint8>(atoi(row[6]));
			++current_entry;
		}

		if(current_id != 0) {
			hash.insert(current_id, faction);
		}

		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error getting npc faction info from database: %s, %s", query, errbuf);
}
}

bool SharedDatabase::LoadNPCFactionLists() {
	if(faction_hash) {
		return true;
	}

	try {
		EQEmu::IPCMutex mutex("faction");
		mutex.Lock();
		faction_mmf = new EQEmu::MemoryMappedFile("shared/faction");

		uint32 list_count = 0;
		uint32 max_lists = 0;
		GetFactionListInfo(list_count, max_lists);
		if(list_count == 0) {
			EQ_EXCEPT("SharedDatabase", "Database returned no result");
		}
		uint32 size = static_cast<uint32>(EQEmu::FixedMemoryHashSet<NPCFactionList>::estimated_size(
			list_count, max_lists));

		if(faction_mmf->Size() != size) {
			EQ_EXCEPT("SharedDatabase", "Couldn't load npc factions because faction_mmf->Size() != size");
		}

		faction_hash = new EQEmu::FixedMemoryHashSet<NPCFactionList>(reinterpret_cast<uint8*>(faction_mmf->Get()), size);
		mutex.Unlock();
	} catch(std::exception& ex) {
		LogFile->write(EQEMuLog::Error, "Error Loading npc factions: %s", ex.what());
		return false;
	}

	return true;
}

// Get the player profile and inventory for the given account "account_id" and
// character name "name". Return true if the character was found, otherwise false.
// False will also be returned if there is a database error.
bool SharedDatabase::GetPlayerProfile(uint32 account_id, char* name, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext, char* current_zone, uint32 *current_instance) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES* result;
	MYSQL_ROW row;
	bool ret = false;

	unsigned long* lengths;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT profile,zonename,x,y,z,extprofile,instanceid FROM character_ WHERE account_id=%i AND name='%s'", account_id, name), errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			lengths = mysql_fetch_lengths(result);
			if (lengths[0] == sizeof(PlayerProfile_Struct)) {
				memcpy(pp, row[0], sizeof(PlayerProfile_Struct));

				if (current_zone)
					strcpy(current_zone, row[1]);
				pp->zone_id = GetZoneID(row[1]);
				pp->x = atof(row[2]);
				pp->y = atof(row[3]);
				pp->z = atof(row[4]);
				pp->zoneInstance = atoi(row[6]);
				if (pp->x == -1 && pp->y == -1 && pp->z == -1)
					GetSafePoints(pp->zone_id, GetInstanceVersion(pp->zoneInstance), &pp->x, &pp->y, &pp->z);

				if(current_instance)
					*current_instance = pp->zoneInstance;

				if(ext) {
					//SetExtendedProfile handles any conversion
					SetExtendedProfile(ext, row[5], lengths[5]);
				}

				// Retrieve character inventory
				ret = GetInventory(account_id, name, inv);
			}
			else {
				LogFile->write(EQEMuLog::Error, "Player profile length mismatch in GetPlayerProfile. Found: %i, Expected: %i",
					lengths[0], sizeof(PlayerProfile_Struct));
			}
		}

		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "GetPlayerProfile query '%s' %s", query, errbuf);
	}

	safe_delete_array(query);
	return ret;
}

bool SharedDatabase::SetPlayerProfile(uint32 account_id, uint32 charid, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext, uint32 current_zone, uint32 current_instance, uint8 MaxXTargets) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 affected_rows = 0;
	bool ret = false;

	if (RunQuery(query, SetPlayerProfile_MQ(&query, account_id, charid, pp, inv, ext, current_zone, current_instance, MaxXTargets), errbuf, 0, &affected_rows)) {
		ret = (affected_rows != 0);
	}

	if (!ret) {
		LogFile->write(EQEMuLog::Error, "SetPlayerProfile query '%s' %s", query, errbuf);
	}

	safe_delete_array(query);
	return ret;
}

// Generate SQL for updating player profile
uint32 SharedDatabase::SetPlayerProfile_MQ(char** query, uint32 account_id, uint32 charid, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext, uint32 current_zone, uint32 current_instance, uint8 MaxXTargets) {
	*query = new char[396 + sizeof(PlayerProfile_Struct)*2 + sizeof(ExtendedProfile_Struct)*2 + 4];
	char* end = *query;
	if (!current_zone)
		current_zone = pp->zone_id;

	if (!current_instance)
		current_instance = pp->zoneInstance;

	if(strlen(pp->name) == 0) // Sanity check in case pp never loaded
		return false;

	end += sprintf(end, "UPDATE character_ SET timelaston=unix_timestamp(now()),name=\'%s\', zonename=\'%s\', zoneid=%u, instanceid=%u, x = %f, y = %f, z = %f, profile=\'", pp->name, GetZoneName(current_zone), current_zone, current_instance, pp->x, pp->y, pp->z);
	end += DoEscapeString(end, (char*)pp, sizeof(PlayerProfile_Struct));
	end += sprintf(end,"\', extprofile=\'");
	end += DoEscapeString(end, (char*)ext, sizeof(ExtendedProfile_Struct));
	end += sprintf(end,"\',class=%d,level=%d,xtargets=%u WHERE id=%u", pp->class_, pp->level, MaxXTargets, charid);

	return (uint32) (end - (*query));
}



// Create appropriate ItemInst class
ItemInst* SharedDatabase::CreateItem(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5)
{
	const Item_Struct* item = nullptr;
	ItemInst* inst = nullptr;
	item = GetItem(item_id);
	if (item) {
		inst = CreateBaseItem(item, charges);
		inst->PutAugment(this, 0, aug1);
		inst->PutAugment(this, 1, aug2);
		inst->PutAugment(this, 2, aug3);
		inst->PutAugment(this, 3, aug4);
		inst->PutAugment(this, 4, aug5);
	}

	return inst;
}


// Create appropriate ItemInst class
ItemInst* SharedDatabase::CreateItem(const Item_Struct* item, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5)
{
	ItemInst* inst = nullptr;
	if (item) {
		inst = CreateBaseItem(item, charges);
		inst->PutAugment(this, 0, aug1);
		inst->PutAugment(this, 1, aug2);
		inst->PutAugment(this, 2, aug3);
		inst->PutAugment(this, 3, aug4);
		inst->PutAugment(this, 4, aug5);
	}

	return inst;
}

ItemInst* SharedDatabase::CreateBaseItem(const Item_Struct* item, int16 charges) {
	ItemInst* inst = nullptr;
	if (item) {
		// if maxcharges is -1 that means it is an unlimited use item.
		// set it to 1 charge so that it is usable on creation
		if (charges == 0 && item->MaxCharges == -1)
			charges = 1;

		inst = new ItemInst(item, charges);

		if(item->CharmFileID != 0 || (item->LoreGroup >= 1000 && item->LoreGroup != -1)) {
			inst->Initialize(this);
		}
	}
	return inst;
}

int32 SharedDatabase::DeleteStalePlayerCorpses() {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;

	if(RuleB(Zone, EnableShadowrest))
	{
		if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE player_corpses SET IsBurried = 1 WHERE IsBurried=0 and "
			"(UNIX_TIMESTAMP() - UNIX_TIMESTAMP(timeofdeath)) > %d and not timeofdeath=0",
			(RuleI(Character, CorpseDecayTimeMS) / 1000)), errbuf, 0, &affected_rows))
		{
			safe_delete_array(query);
			return -1;
		}
	}
	else
	{
		if (!RunQuery(query, MakeAnyLenString(&query, "Delete from player_corpses where (UNIX_TIMESTAMP() - "
			"UNIX_TIMESTAMP(timeofdeath)) > %d and not timeofdeath=0", (RuleI(Character, CorpseDecayTimeMS) / 1000)),
			errbuf, 0, &affected_rows))
		{
			safe_delete_array(query);
			return -1;
		}
	}

	safe_delete_array(query);
	return affected_rows;
}

int32 SharedDatabase::DeleteStalePlayerBackups() {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;

	// 1209600 seconds = 2 weeks
	if (!RunQuery(query, MakeAnyLenString(&query, "Delete from player_corpses_backup where (UNIX_TIMESTAMP() - UNIX_TIMESTAMP(timeofdeath)) > 1209600"), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		return -1;
	}
	safe_delete_array(query);

	return affected_rows;
}

bool SharedDatabase::GetCommandSettings(std::map<std::string,uint8> &commands) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT command,access from commands");
	commands.clear();
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		while((row = mysql_fetch_row(result))) {
			commands[row[0]]=atoi(row[1]);
		}
		mysql_free_result(result);
		return true;
	} else {
		std::cerr << "Error in GetCommands query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	return false;
}

bool SharedDatabase::LoadSkillCaps() {
	if(skill_caps_mmf)
		return true;

	uint32 class_count = PLAYER_CLASS_COUNT;
	uint32 skill_count = HIGHEST_SKILL + 1;
	uint32 level_count = HARD_LEVEL_CAP + 1;
	uint32 size = (class_count * skill_count * level_count * sizeof(uint16));

	try {
		EQEmu::IPCMutex mutex("skill_caps");
		mutex.Lock();
		skill_caps_mmf = new EQEmu::MemoryMappedFile("shared/skill_caps");
		if(skill_caps_mmf->Size() != size) {
			EQ_EXCEPT("SharedDatabase", "Unable to load skill caps: skill_caps_mmf->Size() != size");
		}

		mutex.Unlock();
	} catch(std::exception &ex) {
		LogFile->write(EQEMuLog::Error, "Error loading skill caps: %s", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadSkillCaps(void *data) {
	uint32 class_count = PLAYER_CLASS_COUNT;
	uint32 skill_count = HIGHEST_SKILL + 1;
	uint32 level_count = HARD_LEVEL_CAP + 1;
	uint16 *skill_caps_table = reinterpret_cast<uint16*>(data);

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(RunQuery(query, MakeAnyLenString(&query,
		"SELECT skillID, class, level, cap FROM skill_caps ORDER BY skillID, class, level"),
		errbuf, &result)) {
		safe_delete_array(query);

		while((row = mysql_fetch_row(result))) {
			uint8 skillID = atoi(row[0]);
			uint8 class_ = atoi(row[1]) - 1;
			uint8 level = atoi(row[2]);
			uint16 cap = atoi(row[3]);
			if(skillID >= skill_count || class_ >= class_count || level >= level_count)
				continue;

			uint32 index = (((class_ * skill_count) + skillID) * level_count) + level;
			skill_caps_table[index] = cap;
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error loading skill caps from database: %s", errbuf);
		safe_delete_array(query);
	}
}

uint16 SharedDatabase::GetSkillCap(uint8 Class_, SkillUseTypes Skill, uint8 Level) {
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
	uint32 skill_count = HIGHEST_SKILL + 1;
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

uint8 SharedDatabase::GetTrainLevel(uint8 Class_, SkillUseTypes Skill, uint8 Level) {
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
	uint32 skill_count = HIGHEST_SKILL + 1;
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

	const char *DSQuery = "SELECT `spellid`, `type` from `damageshieldtypes` WHERE `spellid` > 0 "
							"AND `spellid` <= %i";

	const char *ERR_MYSQLERROR = "Error in LoadDamageShieldTypes: %s %s";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(RunQuery(query,MakeAnyLenString(&query,DSQuery,iMaxSpellID),errbuf,&result)) {

		while((row = mysql_fetch_row(result))) {

			int SpellID = atoi(row[0]);
			if((SpellID > 0) && (SpellID <= iMaxSpellID)) {
				sp[SpellID].DamageShieldType = atoi(row[1]);
			}
		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, query, errbuf);
		safe_delete_array(query);
	}
}

const EvolveInfo* SharedDatabase::GetEvolveInfo(uint32 loregroup) {
	return nullptr;	// nothing here for now... database and/or sharemem pulls later
}

int SharedDatabase::GetMaxSpellID() {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = nullptr;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 ret = 0;
	if(RunQuery(query, MakeAnyLenString(&query, "SELECT MAX(id) FROM spells_new"),
		errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		ret = atoi(row[0]);
		mysql_free_result(result);
	} else {
		_log(SPELLS__LOAD_ERR, "Error in GetMaxSpellID query '%s' %s", query, errbuf);
		safe_delete_array(query);
		ret = -1;
	}
	return ret;
}

void SharedDatabase::LoadSpells(void *data, int max_spells) {
	SPDat_Spell_Struct *sp = reinterpret_cast<SPDat_Spell_Struct*>(data);
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(RunQuery(query, MakeAnyLenString(&query,
		"SELECT * FROM spells_new ORDER BY id ASC"),
		errbuf, &result)) {
		safe_delete_array(query);

		int tempid = 0;
		int counter = 0;
		while (row = mysql_fetch_row(result)) {
			tempid = atoi(row[0]);
			if(tempid >= max_spells) {
				_log(SPELLS__LOAD_ERR, "Non fatal error: spell.id >= max_spells, ignoring.");
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
			if(tmp_skill < 0 || tmp_skill > HIGHEST_SKILL)
				sp[tempid].skill = SkillBegging; /* not much better we can do. */ // can probably be changed to client-based 'SkillNone' once activated
			else
				sp[tempid].skill = (SkillUseTypes) tmp_skill;
			sp[tempid].zonetype=atoi(row[101]);
			sp[tempid].EnvironmentType=atoi(row[102]);
			sp[tempid].TimeOfDay=atoi(row[103]);

			for(y=0; y < PLAYER_CLASS_COUNT;y++)
				sp[tempid].classes[y]=atoi(row[104+y]);

			sp[tempid].CastingAnim=atoi(row[120]);
			sp[tempid].SpellAffectIndex=atoi(row[123]);
			sp[tempid].disallow_sit=atoi(row[124]);

			for (y = 0; y < 16; y++)
				sp[tempid].deities[y]=atoi(row[126+y]);

			sp[tempid].uninterruptable=atoi(row[146]);
			sp[tempid].ResistDiff=atoi(row[147]);
			sp[tempid].RecourseLink = atoi(row[150]);

			sp[tempid].short_buff_box = atoi(row[154]);
			sp[tempid].descnum = atoi(row[155]);
			sp[tempid].effectdescnum = atoi(row[157]);

			sp[tempid].bonushate=atoi(row[162]);

			sp[tempid].EndurCost=atoi(row[166]);
			sp[tempid].EndurTimerIndex=atoi(row[167]);
			sp[tempid].HateAdded=atoi(row[173]);
			sp[tempid].EndurUpkeep=atoi(row[174]);
			sp[tempid].numhits = atoi(row[176]);
			sp[tempid].pvpresistbase=atoi(row[177]);
			sp[tempid].pvpresistcalc=atoi(row[178]);
			sp[tempid].pvpresistcap=atoi(row[179]);
			sp[tempid].spell_category=atoi(row[180]);
			sp[tempid].can_mgb=atoi(row[185]);
			sp[tempid].dispel_flag = atoi(row[186]);
			sp[tempid].MinResist = atoi(row[189]);
			sp[tempid].MaxResist = atoi(row[190]);
			sp[tempid].viral_targets = atoi(row[191]);
			sp[tempid].viral_timer = atoi(row[192]);
			sp[tempid].NimbusEffect = atoi(row[193]);
			sp[tempid].directional_start = (float)atoi(row[194]);
			sp[tempid].directional_end = (float)atoi(row[195]);
			sp[tempid].spellgroup=atoi(row[207]);
			sp[tempid].powerful_flag=atoi(row[209]);
			sp[tempid].CastRestriction = atoi(row[211]);
			sp[tempid].AllowRest = atoi(row[212]) != 0;
			sp[tempid].DamageShieldType = 0;
		}
		mysql_free_result(result);

		LoadDamageShieldTypes(sp, max_spells);
	} else {
		_log(SPELLS__LOAD_ERR, "Error in LoadSpells query '%s' %s", query, errbuf);
		safe_delete_array(query);
	}
}

int SharedDatabase::GetMaxBaseDataLevel() {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = "SELECT MAX(level) FROM base_data";
	MYSQL_RES *result;
	MYSQL_ROW row;
	int32 ret = 0;
	if(RunQuery(query, strlen(query), errbuf, &result)) {
		row = mysql_fetch_row(result);
		if(row) {
			ret = atoi(row[0]);
			mysql_free_result(result);
		} else {
			ret = -1;
			mysql_free_result(result);
		}
	} else {
		LogFile->write(EQEMuLog::Error, "Error in GetMaxBaseDataLevel query '%s' %s", query, errbuf);
		ret = -1;
	}
	return ret;
}

bool SharedDatabase::LoadBaseData() {
	if(base_data_mmf) {
		return true;
	}

	try {
		EQEmu::IPCMutex mutex("base_data");
		mutex.Lock();
		base_data_mmf = new EQEmu::MemoryMappedFile("shared/base_data");
	
		int size = 16 * (GetMaxBaseDataLevel() + 1) * sizeof(BaseDataStruct);
		if(size == 0) {
			EQ_EXCEPT("SharedDatabase", "Base Data size is zero");
		}

		if(base_data_mmf->Size() != size) {
			EQ_EXCEPT("SharedDatabase", "Couldn't load base data because base_data_mmf->Size() != size");
		}

		mutex.Unlock();
	} catch(std::exception& ex) {
		LogFile->write(EQEMuLog::Error, "Error Loading Base Data: %s", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadBaseData(void *data, int max_level) {
	char *base_ptr = reinterpret_cast<char*>(data);
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = "SELECT * FROM base_data ORDER BY level, class ASC";
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	if(RunQuery(query, strlen(query), errbuf, &result)) {
	
		int lvl = 0;
		int cl = 0;
		while (row = mysql_fetch_row(result)) {
			lvl = atoi(row[0]);
			cl = atoi(row[1]);
			if(lvl <= 0) {
				LogFile->write(EQEMuLog::Error, "Non fatal error: base_data.level <= 0, ignoring.");
				continue;
			}

			if(lvl >= max_level) {
				LogFile->write(EQEMuLog::Error, "Non fatal error: base_data.level >= max_level, ignoring.");
				continue;
			}

			if(cl <= 0) {
				LogFile->write(EQEMuLog::Error, "Non fatal error: base_data.cl <= 0, ignoring.");
				continue;
			}

			if(cl > 16) {
				LogFile->write(EQEMuLog::Error, "Non fatal error: base_data.class > 16, ignoring.");
				continue;
			}

			BaseDataStruct *bd = reinterpret_cast<BaseDataStruct*>(base_ptr + (((16 * (lvl - 1)) + (cl - 1)) * sizeof(BaseDataStruct)));
			bd->base_hp = atof(row[2]);
			bd->base_mana = atof(row[3]);
			bd->base_end = atof(row[4]);
			bd->unk1 = atof(row[5]);
			bd->unk2 = atof(row[6]);
			bd->hp_factor = atof(row[7]);
			bd->mana_factor = atof(row[8]);
			bd->endurance_factor = atof(row[9]);
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in LoadBaseData query '%s' %s", query, errbuf);
		safe_delete_array(query);
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
	const char *query = "SELECT COUNT(*), MAX(id), (SELECT COUNT(*) FROM loottable_entries) FROM loottable";
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(RunQuery(query, strlen(query), errbuf, &result)) {
		if(row = mysql_fetch_row(result)) {
			loot_table_count = static_cast<uint32>(atoul(row[0]));
			max_loot_table = static_cast<uint32>(atoul(row[1]));
			loot_table_entries = static_cast<uint32>(atoul(row[2]));
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error getting loot table info from database: %s, %s", query, errbuf);
	}
}

void SharedDatabase::GetLootDropInfo(uint32 &loot_drop_count, uint32 &max_loot_drop, uint32 &loot_drop_entries) {
	loot_drop_count = 0;
	max_loot_drop = 0;
	loot_drop_entries = 0;
	const char *query = "SELECT COUNT(*), MAX(id), (SELECT COUNT(*) FROM lootdrop_entries) FROM lootdrop";
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(RunQuery(query, strlen(query), errbuf, &result)) {
		if(row = mysql_fetch_row(result)) {
			loot_drop_count = static_cast<uint32>(atoul(row[0]));
			max_loot_drop = static_cast<uint32>(atoul(row[1]));
			loot_drop_entries = static_cast<uint32>(atoul(row[2]));
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error getting loot table info from database: %s, %s", query, errbuf);
	}
}

void SharedDatabase::LoadLootTables(void *data, uint32 size) {
	EQEmu::FixedMemoryVariableHashSet<LootTable_Struct> hash(reinterpret_cast<uint8*>(data), size);
	const char *query = "SELECT loottable.id, loottable.mincash, loottable.maxcash, loottable.avgcoin,"
		" loottable_entries.lootdrop_id, loottable_entries.multiplier, loottable_entries.droplimit, "
		"loottable_entries.mindrop, loottable_entries.probability FROM loottable LEFT JOIN loottable_entries"
		" ON loottable.id = loottable_entries.loottable_id ORDER BY id";
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint8 loot_table[sizeof(LootTable_Struct) + (sizeof(LootTableEntries_Struct) * 128)];
	LootTable_Struct *lt = reinterpret_cast<LootTable_Struct*>(loot_table);

	if(RunQuery(query, strlen(query), errbuf, &result)) {
		uint32 current_id = 0;
		uint32 current_entry = 0;
		while(row = mysql_fetch_row(result)) {
			uint32 id = static_cast<uint32>(atoul(row[0]));
			if(id != current_id) {
				if(current_id != 0) {
					hash.insert(current_id, loot_table, (sizeof(LootTable_Struct) +
						(sizeof(LootTableEntries_Struct) * lt->NumEntries)));
				}

				memset(loot_table, 0, sizeof(LootTable_Struct) + (sizeof(LootTableEntries_Struct) * 128));
				current_entry = 0;
				current_id = id;
				lt->mincash = static_cast<uint32>(atoul(row[1]));
				lt->maxcash = static_cast<uint32>(atoul(row[2]));
				lt->avgcoin = static_cast<uint32>(atoul(row[3]));
			}

			if(current_entry > 128) {
				continue;
			}

			if(!row[4]) {
				continue;
			}

			lt->Entries[current_entry].lootdrop_id = static_cast<uint32>(atoul(row[4]));
			lt->Entries[current_entry].multiplier = static_cast<uint8>(atoi(row[5]));
			lt->Entries[current_entry].droplimit = static_cast<uint8>(atoi(row[6]));
			lt->Entries[current_entry].mindrop = static_cast<uint8>(atoi(row[7]));
			lt->Entries[current_entry].probability = static_cast<float>(atof(row[8]));

			++(lt->NumEntries);
			++current_entry;
		}
		if(current_id != 0) {
			hash.insert(current_id, loot_table, (sizeof(LootTable_Struct) +
				(sizeof(LootTableEntries_Struct) * lt->NumEntries)));
		}

		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error getting loot table info from database: %s, %s", query, errbuf);
	}
}

void SharedDatabase::LoadLootDrops(void *data, uint32 size) {
	EQEmu::FixedMemoryVariableHashSet<LootDrop_Struct> hash(reinterpret_cast<uint8*>(data), size);
	const char *query = "SELECT lootdrop.id, lootdrop_entries.item_id, lootdrop_entries.item_charges, "
		"lootdrop_entries.equip_item, lootdrop_entries.chance, lootdrop_entries.minlevel, "
		"lootdrop_entries.maxlevel, lootdrop_entries.multiplier FROM lootdrop JOIN lootdrop_entries "
		"ON lootdrop.id = lootdrop_entries.lootdrop_id ORDER BY lootdrop_id";
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;

	uint8 loot_drop[sizeof(LootDrop_Struct) + (sizeof(LootDropEntries_Struct) * 1260)];
	LootDrop_Struct *ld = reinterpret_cast<LootDrop_Struct*>(loot_drop);
	if(RunQuery(query, strlen(query), errbuf, &result)) {
		uint32 current_id = 0;
		uint32 current_entry = 0;
		while(row = mysql_fetch_row(result)) {
			uint32 id = static_cast<uint32>(atoul(row[0]));
			if(id != current_id) {
				if(current_id != 0) {
					hash.insert(current_id, loot_drop, (sizeof(LootDrop_Struct) +
						(sizeof(LootDropEntries_Struct) * ld->NumEntries)));
				}

				memset(loot_drop, 0, sizeof(LootDrop_Struct) + (sizeof(LootDropEntries_Struct) * 1260));
				current_entry = 0;
				current_id = id;
			}

			if(current_entry > 1260) {
				continue;
			}

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
		if(current_id != 0) {
			hash.insert(current_id, loot_drop, (sizeof(LootDrop_Struct) +
				(sizeof(LootDropEntries_Struct) * ld->NumEntries)));
		}

		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error getting loot drop info from database: %s, %s", query, errbuf);
	}
}

bool SharedDatabase::LoadLoot() {
	if(loot_table_mmf || loot_drop_mmf)
		return true;

	try {
		EQEmu::IPCMutex mutex("loot");
		mutex.Lock();
		loot_table_mmf = new EQEmu::MemoryMappedFile("shared/loot_table");
		loot_table_hash = new EQEmu::FixedMemoryVariableHashSet<LootTable_Struct>(
			reinterpret_cast<uint8*>(loot_table_mmf->Get()),
			loot_table_mmf->Size());
		loot_drop_mmf = new EQEmu::MemoryMappedFile("shared/loot_drop");
		loot_drop_hash = new EQEmu::FixedMemoryVariableHashSet<LootDrop_Struct>(
			reinterpret_cast<uint8*>(loot_drop_mmf->Get()),
			loot_drop_mmf->Size());
		mutex.Unlock();
	} catch(std::exception &ex) {
		LogFile->write(EQEMuLog::Error, "Error loading loot: %s", ex.what());
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
		LogFile->write(EQEMuLog::Error, "Could not get loot table: %s", ex.what());
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
		LogFile->write(EQEMuLog::Error, "Could not get loot drop: %s", ex.what());
	}
	return nullptr;
}

void SharedDatabase::GetPlayerInspectMessage(char* playername, InspectMessage_Struct* message) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT inspectmessage FROM character_ WHERE name='%s'", playername), errbuf, &result)) {
		safe_delete_array(query);

		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			memcpy(message, row[0], sizeof(InspectMessage_Struct));
		}

		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetPlayerInspectMessage query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
	}
}

void SharedDatabase::SetPlayerInspectMessage(char* playername, const InspectMessage_Struct* message) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE character_ SET inspectmessage='%s' WHERE name='%s'", message->text, playername), errbuf)) {
		std::cerr << "Error in SetPlayerInspectMessage query '" << query << "' " << errbuf << std::endl;
	}

	safe_delete_array(query);
}

void SharedDatabase::GetBotInspectMessage(uint32 botid, InspectMessage_Struct* message) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT BotInspectMessage FROM bots WHERE BotID=%i", botid), errbuf, &result)) {
		safe_delete_array(query);

		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			memcpy(message, row[0], sizeof(InspectMessage_Struct));
		}

		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetBotInspectMessage query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
	}
}

void SharedDatabase::SetBotInspectMessage(uint32 botid, const InspectMessage_Struct* message) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE bots SET BotInspectMessage='%s' WHERE BotID=%i", message->text, botid), errbuf)) {
		std::cerr << "Error in SetBotInspectMessage query '" << query << "' " << errbuf << std::endl;
	}

	safe_delete_array(query);
}
