
#include "shareddb.h"
#include "../common/Item.h"
#include "../common/EMuShareMem.h"
#include "../common/classes.h"
#include "../common/rulesys.h"
#include "../common/seperator.h"
#include "MiscFunctions.h"
#include "eq_packet_structs.h"
#include "guilds.h"
#include "extprofile.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;


extern LoadEMuShareMemDLL EMuShareMemDLL;

//hackish mechanism to support callbacks from sharedmem
SharedDatabase *SharedDatabase::s_usedb = NULL;

SharedDatabase::SharedDatabase()
: Database()
{
	SDBInitVars();
	s_usedb = this;
}

SharedDatabase::SharedDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
: Database(host, user, passwd, database, port)
{
	SDBInitVars();
	s_usedb = this;
}

void SharedDatabase::SDBInitVars() {
	max_item = 0;
	max_npc_type = 0;
	
	loottable_max = 0;
	lootdrop_max = 0;
	max_door_type = 0;
	npcfactionlist_max = 0;
}

SharedDatabase::~SharedDatabase() {
}

bool SharedDatabase::SetHideMe(uint32 account_id, uint8 hideme)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	
	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE account SET hideme = %i where id = %i", hideme, account_id), errbuf)) {
		cerr << "Error in SetGMSpeed query '" << query << "' " << errbuf << endl;
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
		
		cerr << "Error in GetGMSpeed query '" << query << "' " << errbuf << endl;
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
		cerr << "Error in SetGMSpeed query '" << query << "' " << errbuf << endl;
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

bool SharedDatabase::SaveCursor(uint32 char_id, list<ItemInst*>::const_iterator &start, list<ItemInst*>::const_iterator &end)
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
		cout << "Clearing cursor failed: " << errbuf << endl;
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
	_CP(Database_SaveInventory);
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

			uint32 len_query =  MakeAnyLenString(&query, 
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
		
		cerr << "Error in GetSharedPlatinum query '" << query << "' " << errbuf << endl;
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
		cerr << "Error in SetSharedPlatinum query '" << query << "' " << errbuf << endl;
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
						SaveInventory(id,NULL,slot_id);
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
	_CP(Database_GetInventory);
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
	_CP(Database_GetInventory_name);
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


int32 SharedDatabase::GetItemsCount(uint32* oMaxID) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    MYSQL_RES *result;
    MYSQL_ROW row;
	int32 ret = -1;
	
	char query[] = "SELECT MAX(id),count(*) FROM items";
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		row = mysql_fetch_row(result);
		if (row != NULL && row[1] != 0) {
			ret = atoi(row[1]);
			if (oMaxID) {
				if (row[0])
					*oMaxID = atoi(row[0]);
				else
					*oMaxID = 0;
			}
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in GetItemsCount query '" << query << "' " << errbuf << endl;
	}
	
	return ret;
}


int32 SharedDatabase::GetNPCTypesCount(uint32* oMaxID) {

	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT MAX(id), count(*) FROM npc_types");
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row != NULL && row[1] != 0) {
			int32 ret = atoi(row[1]);
			if (oMaxID) {
				if (row[0])
					*oMaxID = atoi(row[0]);
				else
					*oMaxID = 0;
			}
			mysql_free_result(result);
			return ret;
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in GetNPCTypesCount query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);

		return -1;
	}
	
	return -1;
}


bool SharedDatabase::extDBLoadItems(int32 iItemCount, uint32 iMaxItemID) {
	return s_usedb->DBLoadItems(iItemCount, iMaxItemID);
}

bool SharedDatabase::LoadItems() {
	if (!EMuShareMemDLL.Load())
		return false;
	int32 tmp = 0;
	tmp = GetItemsCount(&max_item);
	if (tmp == -1) {
		cout << "Error: SharedDatabase::LoadItems() (sharemem): GetItemsCount() returned -1" << endl;
		return false;
	}
	bool ret = EMuShareMemDLL.Items.DLLLoadItems(&extDBLoadItems, sizeof(Item_Struct), &tmp, &max_item);
	return ret;
}

// Load all database items into cache
bool SharedDatabase::DBLoadItems(int32 iItemCount, uint32 iMaxItemID) {
	_CP(Database_DBLoadItems);
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	bool ret = false;
	
	LogFile->write(EQEMuLog::Status, "Loading items from database: count=%i", iItemCount);
	
	// Make sure enough memory was alloc'd in cache
	int32 item_count = GetItemsCount(&max_item);
	if (item_count != iItemCount) {
		LogFile->write(EQEMuLog::Error, "Insufficient shared memory to load items (actual=%i, allocated=%i)", item_count, iItemCount);
		return ret;
	}
	else if (max_item != iMaxItemID) {
		LogFile->write(EQEMuLog::Error, "Insufficient shared memory to load items (max item=%i, allocated=%i).  Increase MMF_EQMAX_ITEMS define", max_item, iMaxItemID);
		return ret;
	}
	
	bool disableNoRent = false;
	char ndbuffer[4];
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
	
	// Retrieve all items from database
	char query[] = "select source,"
#define F(x) "`"#x"`,"
#include "item_fieldlist.h"
#undef F
		"updated"
		" from items order by id";
	
	Item_Struct item;
	if (RunQuery(query, sizeof(query), errbuf, &result)) {
                while((row = mysql_fetch_row(result))) {
#if EQDEBUG >= 6
				LogFile->write(EQEMuLog::Status, "Loading %s:%i", row[ItemField::name], row[ItemField::id]);
#endif				
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
			//item.Unk012 = (int32)atoul(row[ItemField::UNK012]);
			//item.Unk013 = (uint32)atoul(row[ItemField::UNK013]);
			item.BenefitFlag = (uint32)atoul(row[ItemField::benefitflag]);
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
			//item.Unk123 = (uint32)atoul(row[ItemField::UNK123]);
			//item.Unk124 = (uint32)atoul(row[ItemField::UNK124]);
			item.Attuneable = (atoi(row[ItemField::attuneable])==0) ? false : true;
			item.NoPet = (atoi(row[ItemField::nopet])==0) ? false : true;
			//item.Unk127 = (uint32)atoul(row[ItemField::UNK127]);
			item.PointType = (uint32)atoul(row[ItemField::pointtype]);
			item.PotionBelt = (atoi(row[ItemField::potionbelt])==0) ? false : true;
			item.PotionBeltSlots = (atoi(row[ItemField::potionbeltslots])==0) ? false : true;
			item.StackSize = (uint16)atoi(row[ItemField::stacksize]);
            item.NoTransfer = disableNoTransfer ? false : (atoi(row[ItemField::notransfer])==0) ? false : true;
			item.Stackable = (atoi(row[ItemField::stackable])==0) ? false : true;
			//item.Unk134 = (uint32)atoul(row[ItemField::UNK134]);
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

			if (!EMuShareMemDLL.Items.cbAddItem(item.ID, &item)) {
				LogFile->write(EQEMuLog::Error, "Database::DBLoadItems: Failure reported from EMuShareMemDLL.Items.cbAddItem(%i)", item.ID);
				break;
			}
		}
		
		mysql_free_result(result);
		ret = true;
	}
	else {
		LogFile->write(EQEMuLog::Error, "DBLoadItems query '%s', %s", query, errbuf);
		LogFile->write(EQEMuLog::Error, "If you got boat loads of errors, make sure you sourced all sql updates!\n");
	}
	return ret;
}


const Item_Struct* SharedDatabase::GetItem(uint32 id) {
	return EMuShareMemDLL.Items.GetItem(id);
}

const Item_Struct* SharedDatabase::IterateItems(uint32* NextIndex) {
	return EMuShareMemDLL.Items.IterateItems(NextIndex);
}


string SharedDatabase::GetBook(const char *txtfile)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	char txtfile2[20];
	string txtout;
    strcpy(txtfile2,txtfile);
	if (!RunQuery(query, MakeAnyLenString(&query, "SELECT txtfile FROM books where name='%s'", txtfile2), errbuf, &result)) {
		cerr << "Error in GetBook query '" << query << "' " << errbuf << endl;
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


bool SharedDatabase::extDBLoadNPCFactionLists(int32 iNPCFactionListCount, uint32 iMaxNPCFactionListID) {
	return s_usedb->DBLoadNPCFactionLists(iNPCFactionListCount, iMaxNPCFactionListID);
}

const NPCFactionList* SharedDatabase::GetNPCFactionEntry(uint32 id) {
	return EMuShareMemDLL.NPCFactionList.GetNPCFactionList(id);
}

bool SharedDatabase::LoadNPCFactionLists() {
	if (!EMuShareMemDLL.Load())
		return false;
	int32 tmp = -1;
	uint32 tmp_npcfactionlist_max;
	tmp = GetNPCFactionListsCount(&tmp_npcfactionlist_max);
	if (tmp < 0) {
		cout << "Error: SharedDatabase::LoadNPCFactionLists-ShareMem: GetNPCFactionListsCount() returned < 0" << endl;
		return false;
	}
	npcfactionlist_max = tmp_npcfactionlist_max;
	bool ret = EMuShareMemDLL.NPCFactionList.DLLLoadNPCFactionLists(&extDBLoadNPCFactionLists, sizeof(NPCFactionList), &tmp, &npcfactionlist_max, MAX_NPC_FACTIONS);
	return ret;
}

bool SharedDatabase::DBLoadNPCFactionLists(int32 iNPCFactionListCount, uint32 iMaxNPCFactionListID) {
	_CP(Database_DBLoadNPCFactionLists);
	LogFile->write(EQEMuLog::Status, "Loading NPC Faction Lists from database...");
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT MAX(id), Count(*) FROM npc_faction");
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row && row[0]) {
			if ((uint32)atoi(row[0]) > iMaxNPCFactionListID) {
				cout << "Error: Insufficient shared memory to load NPC Faction Lists." << endl;
				cout << "Max(id): " << atoi(row[0]) << ", iMaxNPCFactionListID: " << iMaxNPCFactionListID << endl;
				cout << "Fix this by increasing the MMF_MAX_NPCFactionList_ID define statement" << endl;
				mysql_free_result(result);
				return false;
			}
			if (atoi(row[1]) != iNPCFactionListCount) {
				cout << "Error: number of NPCFactionLists in memshare doesnt match database." << endl;
				cout << "Count(*): " << atoi(row[1]) << ", iNPCFactionListCount: " << iNPCFactionListCount << endl;
				mysql_free_result(result);
				return false;
			}
			npcfactionlist_max = atoi(row[0]);
			mysql_free_result(result);
			NPCFactionList tmpnfl;
			if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, primaryfaction, ignore_primary_assist from npc_faction"), errbuf, &result)) {
				safe_delete_array(query);
				while((row = mysql_fetch_row(result))) {
					memset(&tmpnfl, 0, sizeof(NPCFactionList));
					tmpnfl.id = atoi(row[0]);
					tmpnfl.primaryfaction = atoi(row[1]);
					//if we have ignore_primary_assist set to non-zero then we will not assist our own faction
					//else we will assist (this is the default)
					tmpnfl.assistprimaryfaction = (atoi(row[2]) == 0) ? true : false;
					if (!EMuShareMemDLL.NPCFactionList.cbAddNPCFactionList(tmpnfl.id, &tmpnfl)) {
						mysql_free_result(result);
						cout << "Error: SharedDatabase::DBLoadNPCFactionLists: !EMuShareMemDLL.NPCFactionList.cbAddNPCFactionList" << endl;
						return false;
					}

					Sleep(0);
				}
				mysql_free_result(result);
			}
			else {
				cerr << "Error in DBLoadNPCFactionLists query2 '" << query << "' " << errbuf << endl;
				safe_delete_array(query);
				return false;
			}
			if (RunQuery(query, MakeAnyLenString(&query, "SELECT npc_faction_id, faction_id, value, npc_value, temp FROM npc_faction_entries order by npc_faction_id"), errbuf, &result)) {
				safe_delete_array(query);
				int8 i = 0;
				uint32 curflid = 0;
				uint32 tmpflid = 0;
				uint32 tmpfactionid[MAX_NPC_FACTIONS];
				int32 tmpfactionvalue[MAX_NPC_FACTIONS];
				int8 tmpfactionnpcvalue[MAX_NPC_FACTIONS];
				uint8 tmpfactiontemp[MAX_NPC_FACTIONS];

				memset(tmpfactionid, 0, sizeof(tmpfactionid));
				memset(tmpfactionvalue, 0, sizeof(tmpfactionvalue));
				memset(tmpfactionnpcvalue, 0, sizeof(tmpfactionnpcvalue));
				memset(tmpfactiontemp, 0, sizeof(tmpfactiontemp));
				
				while((row = mysql_fetch_row(result))) {
					tmpflid = atoi(row[0]);
					if (curflid != tmpflid && curflid != 0) {
						if (!EMuShareMemDLL.NPCFactionList.cbSetFaction(curflid, tmpfactionid, tmpfactionvalue, tmpfactionnpcvalue, tmpfactiontemp)) {
							mysql_free_result(result);
							cout << "Error: SharedDatabase::DBLoadNPCFactionLists: !EMuShareMemDLL.NPCFactionList.cbSetFaction" << endl;
							return false;
						}
						memset(tmpfactionid, 0, sizeof(tmpfactionid));
						memset(tmpfactionvalue, 0, sizeof(tmpfactionvalue));
						memset(tmpfactionnpcvalue, 0, sizeof(tmpfactionnpcvalue));
						memset(tmpfactiontemp, 0, sizeof(tmpfactiontemp));
						i = 0;
					}
					curflid = tmpflid;
					tmpfactionid[i] = atoi(row[1]);
					tmpfactionvalue[i] = atoi(row[2]);
					tmpfactionnpcvalue[i] = atoi(row[3]);
					tmpfactiontemp[i] = atoi(row[4]);
					i++;
					if (i >= MAX_NPC_FACTIONS) {
						cerr << "Error in DBLoadNPCFactionLists: More than MAX_NPC_FACTIONS factions returned, flid=" << tmpflid << endl;
						break;
					}
					Sleep(0);
				}
				if (tmpflid) {
					EMuShareMemDLL.NPCFactionList.cbSetFaction(curflid, tmpfactionid, tmpfactionvalue, tmpfactionnpcvalue, tmpfactiontemp);
				}

				mysql_free_result(result);
			}
			else {
				cerr << "Error in DBLoadNPCFactionLists query3 '" << query << "' " << errbuf << endl;
				safe_delete_array(query);
				return false;
			}
		}
		else {
			mysql_free_result(result);
			//return false;
		}
	}
	else {
		cerr << "Error in DBLoadNPCFactionLists query1 '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}
	return true;
}

// Get the player profile and inventory for the given account "account_id" and
// character name "name".  Return true if the character was found, otherwise false.
// False will also be returned if there is a database error.
bool SharedDatabase::GetPlayerProfile(uint32 account_id, char* name, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext, char* current_zone, uint32 *current_instance) {
	_CP(Database_GetPlayerProfile);
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
	_CP(Database_SetPlayerProfile);
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
	const Item_Struct* item = NULL;
	ItemInst* inst = NULL;
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
	ItemInst* inst = NULL;
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
	ItemInst* inst = NULL;
	if (item) {
		// if maxcharges is -1 that means it is an unlimited use item. 
		// set it to 1 charge so that it is usable on creation
		if (charges == 0 && item->MaxCharges == -1)
			charges = 1;

		if(item->CharmFileID != 0 || (item->LoreGroup >= 1000 && item->LoreGroup != -1)) {
			inst = new EvoItemInst(item, charges);
			((EvoItemInst*)inst)->Initialize(this);
		}
		else 
			inst = new ItemInst(item, charges);		
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

int32 SharedDatabase::GetNPCFactionListsCount(uint32* oMaxID) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT MAX(id), count(*) FROM npc_faction");
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row != NULL && row[1] != 0) {
			int32 ret = atoi(row[1]);
			if (oMaxID) {
				if (row[0])
					*oMaxID = atoi(row[0]);
				else
					*oMaxID = 0;
			}
			mysql_free_result(result);
			return ret;
		}
	}
	else {
		cerr << "Error in GetNPCFactionListsCount query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return -1;
	}
	
	return -1;
}

bool SharedDatabase::GetCommandSettings(map<string,uint8> &commands) {
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
		cerr << "Error in GetCommands query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}
	
	return false;
}

bool SharedDatabase::extDBLoadSkillCaps() {
	return s_usedb->DBLoadSkillCaps();
}

bool SharedDatabase::LoadSkillCaps() {
	if (!EMuShareMemDLL.Load())
		return false;
	
	uint8 class_count = PLAYER_CLASS_COUNT;
	uint8 skill_count = HIGHEST_SKILL+1;
	uint8 level_count = HARD_LEVEL_CAP+1;

	return EMuShareMemDLL.SkillCaps.LoadSkillCaps(&extDBLoadSkillCaps,
			 sizeof(uint16), class_count, skill_count, level_count);
}

bool SharedDatabase::DBLoadSkillCaps() {
	LogFile->write(EQEMuLog::Status, "Loading skill caps from database...");
	
	uint8 class_count = PLAYER_CLASS_COUNT;
	uint8 skill_count = HIGHEST_SKILL+1;
	uint8 level_count = HARD_LEVEL_CAP+1;
	
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, 
		"SELECT skillID,class,level,cap FROM skill_caps ORDER BY skillID,class,level"), 
		errbuf, &result)) {
		safe_delete_array(query);
		
		while ((row = mysql_fetch_row(result))) {
			uint8 skillID = atoi(row[0]);
			uint8 class_ = atoi(row[1])-1;	//classes are base 1... 
			uint8 level = atoi(row[2]);
			uint16 cap = atoi(row[3]);
			if(skillID >= skill_count || class_ >= class_count || level >= level_count)
				continue;
			EMuShareMemDLL.SkillCaps.SetSkillCap(class_, skillID, level, cap);
		}
		mysql_free_result(result);
	}
	else {
		cerr << "Error in DBLoadSkillCaps (memshare) #2 query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	return true;
}

uint16 SharedDatabase::GetSkillCap(uint8 Class_, SkillType Skill, uint8 Level) {
	if(Class_ == 0)
		return(0);
	int SkillMaxLevel = RuleI(Character, SkillCapMaxLevel);
	if (SkillMaxLevel < 1) {
		SkillMaxLevel = RuleI(Character, MaxLevel);
	}
	if(Level > SkillMaxLevel){
		return EMuShareMemDLL.SkillCaps.GetSkillCap(Class_-1, Skill, SkillMaxLevel);
	}
	else{
		return EMuShareMemDLL.SkillCaps.GetSkillCap(Class_-1, Skill, Level);
	}
}

uint8 SharedDatabase::GetTrainLevel(uint8 Class_, SkillType Skill, uint8 Level) {
	if(Class_ == 0)
		return(0);

	uint8 ret = 0;
	int SkillMaxLevel = RuleI(Character, SkillCapMaxLevel);
	if (SkillMaxLevel < 1) {
		SkillMaxLevel = RuleI(Character, MaxLevel);
	}
	if(Level > SkillMaxLevel) {
		ret = EMuShareMemDLL.SkillCaps.GetTrainLevel(Class_-1, Skill, SkillMaxLevel);
	}
	else
	{
		ret = EMuShareMemDLL.SkillCaps.GetTrainLevel(Class_-1, Skill, Level);
	}
	if(ret > GetSkillCap(Class_, Skill, Level))
		ret = GetSkillCap(Class_, Skill, Level);

	return ret;
}

void SharedDatabase::DBLoadDamageShieldTypes(SPDat_Spell_Struct* sp, int32 iMaxSpellID) {

	const char *DSQuery = "SELECT `spellid`, `type` from `damageshieldtypes` WHERE `spellid` >0 "
	                         "AND `spellid` <= %i";

	const char *ERR_MYSQLERROR = "Error in DBLoadDamageShieldTypes: %s %s";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(RunQuery(query,MakeAnyLenString(&query,DSQuery,iMaxSpellID),errbuf,&result)) {

		while((row = mysql_fetch_row(result))) {

			int SpellID = atoi(row[0]);
			if((SpellID > 0) && (SpellID <= iMaxSpellID))  {
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
	return NULL;	// nothing here for now... database and/or sharemem pulls later
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
		cerr << "Error in GetPlayerInspectMessage query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}
}

void SharedDatabase::SetPlayerInspectMessage(char* playername, const InspectMessage_Struct* message) {

	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	
	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE character_ SET inspectmessage='%s' WHERE name='%s'", message->text, playername), errbuf)) {
		cerr << "Error in SetPlayerInspectMessage query '" << query << "' " << errbuf << endl;
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
		cerr << "Error in GetBotInspectMessage query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}
}

void SharedDatabase::SetBotInspectMessage(uint32 botid, const InspectMessage_Struct* message) {

	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	
	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE bots SET BotInspectMessage='%s' WHERE BotID=%i", message->text, botid), errbuf)) {
		cerr << "Error in SetBotInspectMessage query '" << query << "' " << errbuf << endl;
	}
	
	safe_delete_array(query);
}

int SharedDatabase::GetMaxSpellID() {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = NULL;
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
            strn0cpy(sp[tempid].name, row[1], sizeof(sp[tempid].name));
            strn0cpy(sp[tempid].player_1, row[2], sizeof(sp[tempid].player_1));
            strn0cpy(sp[tempid].teleport_zone, row[3], sizeof(sp[tempid].teleport_zone));
            strn0cpy(sp[tempid].you_cast,  row[4], sizeof(sp[tempid].you_cast));
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
                sp[tempid].base[y]=atoi(row[20+y]);    // effect_base_value
            for(y=0; y < EFFECT_COUNT; y++)
                sp[tempid].base2[y]=atoi(row[32+y]);    // effect_limit_value
            for(y=0; y< EFFECT_COUNT;y++)
                sp[tempid].max[y]=atoi(row[44+y]);
            
            sp[tempid].icon=atoi(row[56]);
            sp[tempid].memicon=atoi(row[57]);
            
            for(y=0; y< 4;y++)
                sp[tempid].components[y]=atoi(row[58+y]);
            
            for(y=0; y< 4;y++)
                sp[tempid].component_counts[y]=atoi(row[62+y]);
            
            for(y=0; y< 4;y++)
                sp[tempid].NoexpendReagent[y]=atoi(row[66+y]);
            
            for(y=0; y< EFFECT_COUNT;y++)
                sp[tempid].formula[y]=atoi(row[70+y]);
            
            sp[tempid].LightType=atoi(row[82]);
            sp[tempid].goodEffect=atoi(row[83]);
            sp[tempid].Activated=atoi(row[84]);
            sp[tempid].resisttype=atoi(row[85]);
            
            for(y=0; y< EFFECT_COUNT;y++)
                sp[tempid].effectid[y]=atoi(row[86+y]);
            
            sp[tempid].targettype = (SpellTargetType) atoi(row[98]);
            sp[tempid].basediff=atoi(row[99]);
            int tmp_skill = atoi(row[100]);;
            if(tmp_skill < 0 || tmp_skill > HIGHEST_SKILL)
                sp[tempid].skill = BEGGING;    /* not much better we can do. */
            else
                sp[tempid].skill = (SkillType) tmp_skill;
            sp[tempid].zonetype=atoi(row[101]);
            sp[tempid].EnvironmentType=atoi(row[102]);
            sp[tempid].TimeOfDay=atoi(row[103]);
            
            for(y=0; y < PLAYER_CLASS_COUNT;y++)
                sp[tempid].classes[y]=atoi(row[104+y]);
            
            sp[tempid].CastingAnim=atoi(row[120]);
            sp[tempid].TargetAnim=atoi(row[121]);
            sp[tempid].TravelType=atoi(row[122]);
            sp[tempid].SpellAffectIndex=atoi(row[123]);
            sp[tempid].disallow_sit=atoi(row[124]);
            sp[tempid].spacing125=atoi(row[125]);

            for (y = 0; y < 16; y++)
                sp[tempid].deities[y]=atoi(row[126+y]);

            for (y = 0; y < 2; y++)
                sp[tempid].spacing142[y]=atoi(row[142+y]);

            sp[tempid].new_icon=atoi(row[144]);
            sp[tempid].spellanim=atoi(row[145]);
            sp[tempid].uninterruptable=atoi(row[146]);
            sp[tempid].ResistDiff=atoi(row[147]);
            sp[tempid].dot_stacking_exempt=atoi(row[148]);
            sp[tempid].deletable=atoi(row[149]);
            sp[tempid].RecourseLink = atoi(row[150]);

            for(y = 0; y < 3;y++)
                sp[tempid].spacing151[y]=atoi(row[151+y]);

            sp[tempid].short_buff_box = atoi(row[154]);
            sp[tempid].descnum = atoi(row[155]);
            sp[tempid].typedescnum = atoi(row[156]);
            sp[tempid].effectdescnum = atoi(row[157]);
            
            for(y = 0; y < 4;y++)
                sp[tempid].spacing158[y]=atoi(row[158+y]);

            sp[tempid].bonushate=atoi(row[162]);

            for(y = 0; y < 3;y++)
                sp[tempid].spacing163[y]=atoi(row[163+y]);

            sp[tempid].EndurCost=atoi(row[166]);
            sp[tempid].EndurTimerIndex=atoi(row[167]);
            sp[tempid].IsDisciplineBuff=atoi(row[168]);

            for(y = 0; y < 4; y++)
                sp[tempid].spacing169[y]=atoi(row[169+y]);

            sp[tempid].HateAdded=atoi(row[173]);
            sp[tempid].EndurUpkeep=atoi(row[174]);

            sp[tempid].spacing175=atoi(row[175]);
            sp[tempid].numhits = atoi(row[176]);

            sp[tempid].pvpresistbase=atoi(row[177]);
            sp[tempid].pvpresistcalc=atoi(row[178]);
            sp[tempid].pvpresistcap=atoi(row[179]);
            sp[tempid].spell_category=atoi(row[180]);

            for(y = 0; y < 4;y++)
                sp[tempid].spacing181[y]=atoi(row[181+y]);

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
            sp[tempid].field209=atoi(row[209]);
            sp[tempid].CastRestriction = atoi(row[211]);
            sp[tempid].AllowRest = atoi(row[212]) != 0;
            sp[tempid].DamageShieldType = 0;
        }
        mysql_free_result(result);

        DBLoadDamageShieldTypes(sp, max_spells);
    } else {
		_log(SPELLS__LOAD_ERR, "Error in LoadSpells query '%s' %s", query, errbuf);
		safe_delete_array(query);
	}
}