/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

#include <float.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WINDOWS
#define	snprintf	_snprintf
#define	vsnprintf	_vsnprintf
#else
#include <pthread.h>
#include "../common/unix.h"
#endif

#include "../common/global_define.h"
#include "../common/features.h"
#include "../common/rulesys.h"
#include "../common/seperator.h"
#include "../common/string_util.h"
#include "../common/eqemu_logsys.h"

#include "expedition.h"
#include "guild_mgr.h"
#include "map.h"
#include "npc.h"
#include "object.h"
#include "pathfinder_null.h"
#include "pathfinder_nav_mesh.h"
#include "pathfinder_waypoint.h"
#include "petitions.h"
#include "quest_parser_collection.h"
#include "spawn2.h"
#include "spawngroup.h"
#include "water_map.h"
#include "worldserver.h"
#include "zone.h"
#include "zone_config.h"
#include "mob_movement_manager.h"
#include "npc_scale_manager.h"
#include "../common/data_verification.h"
#include "zone_reload.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "../common/repositories/content_flags_repository.h"
#include "../common/repositories/zone_points_repository.h"
#include "../common/serverinfo.h"

#include <time.h>
#include <ctime>
#include <iostream>

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

extern bool staticzone;
extern PetitionList petition_list;
extern QuestParserCollection* parse;
extern uint32 numclients;
extern WorldServer worldserver;
extern Zone* zone;
extern NpcScaleManager* npc_scale_manager;

Mutex MZoneShutdown;

volatile bool is_zone_loaded = false;
Zone* zone = 0;

void UpdateWindowTitle(char* iNewTitle);

bool Zone::Bootup(uint32 iZoneID, uint32 iInstanceID, bool is_static) {
	const char* zonename = ZoneName(iZoneID);

	if (iZoneID == 0 || zonename == 0)
		return false;
	if (zone != 0 || is_zone_loaded) {
		std::cerr << "Error: Zone::Bootup call when zone already booted!" << std::endl;
		worldserver.SetZoneData(0);
		return false;
	}

	LogInfo("Booting [{}] ([{}]:[{}])", zonename, iZoneID, iInstanceID);

	numclients = 0;
	zone = new Zone(iZoneID, iInstanceID, zonename);

	//init the zone, loads all the data, etc
	if (!zone->Init(is_static)) {
		safe_delete(zone);
		std::cerr << "Zone->Init failed" << std::endl;
		worldserver.SetZoneData(0);
		return false;
	}

	std::string tmp;
	if (database.GetVariable("loglevel", tmp)) {
		int log_levels[4];
		int tmp_i = atoi(tmp.c_str());
		if (tmp_i>9){ //Server is using the new code
			for(int i=0;i<4;i++){
				if (((int)tmp[i]>=48) && ((int)tmp[i]<=57))
					log_levels[i]=(int)tmp[i]-48; //get the value to convert it to an int from the ascii value
				else
					log_levels[i]=0; //set to zero on a bogue char
			}
			zone->loglevelvar = log_levels[0];
			LogInfo("General logging level: [{}]", zone->loglevelvar);
			zone->merchantvar = log_levels[1];
			LogInfo("Merchant logging level: [{}]", zone->merchantvar);
			zone->tradevar = log_levels[2];
			LogInfo("Trade logging level: [{}]", zone->tradevar);
			zone->lootvar = log_levels[3];
			LogInfo("Loot logging level: [{}]", zone->lootvar);
		}
		else {
			zone->loglevelvar = uint8(tmp_i); //continue supporting only command logging (for now)
			zone->merchantvar = 0;
			zone->tradevar = 0;
			zone->lootvar = 0;
		}
	}

	is_zone_loaded = true;

	worldserver.SetZoneData(iZoneID, iInstanceID);
	if(iInstanceID != 0)
	{
		auto pack = new ServerPacket(ServerOP_AdventureZoneData, sizeof(uint16));
		*((uint16*)pack->pBuffer) = iInstanceID;
		worldserver.SendPacket(pack);
		delete pack;
	}

	LogInfo("---- Zone server [{}], listening on port:[{}] ----", zonename, ZoneConfig::get()->ZonePort);
	LogInfo("Zone Bootup: [{}] [{}] ([{}]: [{}])",
		(is_static) ? "Static" : "Dynamic", zonename, iZoneID, iInstanceID);
	parse->Init();
	UpdateWindowTitle(nullptr);

	// Dynamic zones need to Sync here.
	// Static zones sync when they connect in worldserver.cpp.
	// Static zones cannot sync here as request is ignored by worldserver.
	if (!is_static)
	{
		zone->GetTimeSync();
	}

	zone->RequestUCSServerStatus();

	/**
	 * Set Shutdown timer
	 */
	uint32 shutdown_timer = static_cast<uint32>(content_db.getZoneShutDownDelay(zone->GetZoneID(), zone->GetInstanceVersion()));
	zone->StartShutdownTimer(shutdown_timer);

	/*
	 * Set Logging
	 */
	LogSys.StartFileLogs(StringFormat("%s_version_%u_inst_id_%u_port_%u", zone->GetShortName(), zone->GetInstanceVersion(), zone->GetInstanceID(), ZoneConfig::get()->ZonePort));

	return true;
}

//this really loads the objects into entity_list
bool Zone::LoadZoneObjects()
{
	std::string query = StringFormat(
		"SELECT id, zoneid, xpos, ypos, zpos, heading, itemid, charges, objectname, type, icon, "
		"unknown08, unknown10, unknown20, unknown24, unknown76, size, tilt_x, tilt_y, display_name "
		"FROM object WHERE zoneid = %i AND (version = %u OR version = -1) %s",
		zoneid,
		instanceversion,
		ContentFilterCriteria::apply().c_str()
	);
	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error Loading Objects from DB: [{}]",
			results.ErrorMessage().c_str());
		return false;
	}

	LogInfo("Loading Objects from DB");
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (atoi(row[9]) == 0) {
			// Type == 0 - Static Object
			const char *shortname = ZoneName(atoi(row[1]), false); // zoneid -> zone_shortname

			if (!shortname)
				continue;

			// todo: clean up duplicate code with command_object
			auto d = DoorsRepository::NewEntity();

			d.zone = shortname;
			d.id = 1000000000 + atoi(row[0]); // Out of range of normal use for doors.id
			d.doorid = -1; // Client doesn't care if these are all the same door_id
			d.pos_x = atof(row[2]);		     // xpos
			d.pos_y = atof(row[3]);		     // ypos
			d.pos_z = atof(row[4]);		     // zpos
			d.heading = atof(row[5]);	    // heading

			d.name = row[8]; // objectname

			// Strip trailing "_ACTORDEF" if present. Client won't accept it for doors.
			int pos = d.name.size() - strlen("_ACTORDEF");
			if (pos > 0 && d.name.compare(pos, std::string::npos, "_ACTORDEF") == 0)
			{
				d.name.erase(pos);
			}

			d.dest_zone = "NONE";

			if ((d.size = atoi(row[11])) == 0) // unknown08 = optional size percentage
				d.size = 100;

			switch (d.opentype = atoi(row[12])) // unknown10 = optional request_nonsolid (0 or 1 or experimental number)
			{
			case 0:
				d.opentype = 31;
				break;
			case 1:
				d.opentype = 9;
				break;
			}

			d.incline = atoi(row[13]);	  // unknown20 = optional model incline value
			d.client_version_mask = 0xFFFFFFFF; // We should load the mask from the zone.

			auto door = new Doors(d);
			entity_list.AddDoor(door);
		}

		Object_Struct data = {0};
		uint32 id = 0;
		uint32 icon = 0;
		uint32 type = 0;
		uint32 itemid = 0;
		uint32 idx = 0;
		int16 charges = 0;

		id = (uint32)atoi(row[0]);
		data.zone_id = atoi(row[1]);
		data.x = atof(row[2]);
		data.y = atof(row[3]);
		data.z = atof(row[4]);
		data.heading = atof(row[5]);
		itemid = (uint32)atoi(row[6]);
		charges = (int16)atoi(row[7]);
		strcpy(data.object_name, row[8]);
		type = (uint8)atoi(row[9]);
		icon = (uint32)atoi(row[10]);
		data.object_type = type;
		data.linked_list_addr[0] = 0;
		data.linked_list_addr[1] = 0;

		data.solidtype = (uint32)atoi(row[12]);
		data.unknown020 = (uint32)atoi(row[13]);
		data.unknown024 = (uint32)atoi(row[14]);
		data.unknown076 = (uint32)atoi(row[15]);
		data.size = atof(row[16]);
		data.tilt_x = atof(row[17]);
		data.tilt_y = atof(row[18]);
		data.unknown084 = 0;


		glm::vec3 position;
		position.x = data.x;
		position.y = data.y;
		position.z = data.z;

		if (zone->HasMap()) {
			data.z = zone->zonemap->FindBestZ(position, nullptr);
		}

		EQ::ItemInstance *inst = nullptr;
		// FatherNitwit: this dosent seem to work...
		// tradeskill containers do not have an itemid of 0... at least what I am seeing
		if (itemid == 0) {
			// Generic tradeskill container
			inst = new EQ::ItemInstance(ItemInstWorldContainer);
		} else {
			// Groundspawn object
			inst = database.CreateItem(itemid);
		}

		// Father Nitwit's fix... not perfect...
		if (inst == nullptr && type != OT_DROPPEDITEM) {
			inst = new EQ::ItemInstance(ItemInstWorldContainer);
		}

		// Load child objects if container
		if (inst && inst->IsType(EQ::item::ItemClassBag)) {
			database.LoadWorldContainer(id, inst);
		}

		auto object = new Object(id, type, icon, data, inst);
		object->SetDisplayName(row[19]);
		entity_list.AddObject(object, false);
		if (type == OT_DROPPEDITEM && itemid != 0)
			entity_list.RemoveObject(object->GetID());

		safe_delete(inst);
	}

	return true;
}

//this also just loads into entity_list, not really into zone
bool Zone::LoadGroundSpawns() {
	Ground_Spawns groundspawn;

	memset(&groundspawn, 0, sizeof(groundspawn));
	int gsindex=0;
	LogInfo("Loading Ground Spawns from DB");
	content_db.LoadGroundSpawns(zoneid, GetInstanceVersion(), &groundspawn);
	uint32 ix=0;
	char* name = nullptr;
	uint32 gsnumber=0;
	for(gsindex=0;gsindex<50;gsindex++){
		if(groundspawn.spawn[gsindex].item>0 && groundspawn.spawn[gsindex].item<SAYLINK_ITEM_ID){
			EQ::ItemInstance* inst = nullptr;
			inst = database.CreateItem(groundspawn.spawn[gsindex].item);
			gsnumber=groundspawn.spawn[gsindex].max_allowed;
			ix=0;
			if(inst){
				name = groundspawn.spawn[gsindex].name;
				for(ix=0;ix<gsnumber;ix++){
					auto object = new Object(
					    inst, name, groundspawn.spawn[gsindex].max_x,
					    groundspawn.spawn[gsindex].min_x, groundspawn.spawn[gsindex].max_y,
					    groundspawn.spawn[gsindex].min_y, groundspawn.spawn[gsindex].max_z,
					    groundspawn.spawn[gsindex].heading,
					    groundspawn.spawn[gsindex].respawntimer); // new object with id of 10000+
					entity_list.AddObject(object, false);
				}
				safe_delete(inst);
			}
		}
	}
	return(true);
}

void Zone::DumpMerchantList(uint32 npcid) {
	std::list<TempMerchantList> tmp_merlist = tmpmerchanttable[npcid];
	std::list<TempMerchantList>::const_iterator tmp_itr;
	TempMerchantList ml;

	for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
		ml = *tmp_itr;

		LogInventory("slot[{}] Orig[{}] Item[{}] Charges[{}]", ml.slot, ml.origslot, ml.item, ml.charges);
	}
}

int Zone::SaveTempItem(uint32 merchantid, uint32 npcid, uint32 item, int32 charges, bool sold) {

	LogInventory("[{}] [{}] charges of [{}]", ((sold) ? "Sold" : "Bought"),
		charges, item);
	//DumpMerchantList(npcid);
	// Iterate past main items.
	// If the item being transacted is in this list, return 0;
	std::list<MerchantList> merlist = merchanttable[merchantid];
	std::list<MerchantList>::const_iterator itr;
	uint32 temp_slot_index = 1;
	for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
		MerchantList ml = *itr;
		if (ml.item == item) {
			return 0;
		}

		// Account for merchant lists with gaps in them.
		if (ml.slot >= temp_slot_index) {
			temp_slot_index = ml.slot + 1;
		}
	}

	LogInventory("Searching Temporary List.  Main list ended at [{}]", temp_slot_index-1);

	// Now search the temporary list.
	std::list<TempMerchantList> tmp_merlist = tmpmerchanttable[npcid];
	std::list<TempMerchantList>::const_iterator tmp_itr;
	TempMerchantList ml;
	uint32 first_empty_slot = 0; // Save 1st vacant slot while searching..
	bool found = false;

	for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
		ml = *tmp_itr;

		if (ml.item == item) {
			found = true;
			LogInventory("Item found in temp list at [{}] with [{}] charges", ml.origslot, ml.charges);
			break;
		}
	}

	if (found) {
		tmp_merlist.clear();
		std::list<TempMerchantList> oldtmp_merlist = tmpmerchanttable[npcid];
		for (tmp_itr = oldtmp_merlist.begin(); tmp_itr != oldtmp_merlist.end(); ++tmp_itr) {
			TempMerchantList ml2 = *tmp_itr;
			if(ml2.item != item)
				tmp_merlist.push_back(ml2);
			else {
				if (sold) {
					LogInventory("Total charges is [{}] + [{}] charges", ml.charges, charges);
					ml.charges = ml.charges + charges;
				}
				else {
					ml.charges = charges;
					LogInventory("new charges is [{}] charges", ml.charges);
				}

				if (!ml.origslot) {
					ml.origslot = ml.slot;
				}
				if (ml.charges > 0) {
					database.SaveMerchantTemp(npcid, ml.origslot, item, ml.charges);
					tmp_merlist.push_back(ml);
				} else {
					database.DeleteMerchantTemp(npcid, ml.origslot);
				}
			}
		}

		tmpmerchanttable[npcid] = tmp_merlist;
		//DumpMerchantList(npcid);
		return ml.slot;
	}
	else {
		if (charges < 0) { //sanity check only, shouldnt happen
			charges = 0x7FFF;
		}

		// Find an ununsed db slot #
		std::list<int> slots;
		TempMerchantList ml3;
		for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
			ml3 = *tmp_itr;
			slots.push_back(ml3.origslot);
		}
		slots.sort();
		std::list<int>::const_iterator slots_itr;
		uint32 first_empty_slot = 0;
		uint32 idx = temp_slot_index;
		for (slots_itr = slots.begin(); slots_itr != slots.end(); ++slots_itr) {
			if (!first_empty_slot && *slots_itr > idx) {
				LogInventory("Popped [{}]", *slots_itr);
				LogInventory("First Gap Found at [{}]", idx);
				break;
			}

			++idx;
		}

		first_empty_slot = idx;

		// Find an ununsed mslot
		slots.clear();
		for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
			ml3 = *tmp_itr;
			slots.push_back(ml3.slot);
		}
		slots.sort();
		uint32 first_empty_mslot=0;
		idx = temp_slot_index;
		for (slots_itr = slots.begin(); slots_itr != slots.end(); ++slots_itr) {
			if (!first_empty_mslot && *slots_itr > idx) {
				LogInventory("Popped [{}]", *slots_itr);
				LogInventory("First Gap Found at [{}]", idx);
				break;
			}

			++idx;
		}

		first_empty_mslot = idx;

		database.SaveMerchantTemp(npcid, first_empty_slot, item, charges);
		tmp_merlist = tmpmerchanttable[npcid];
		TempMerchantList ml2;
		ml2.charges = charges;
		LogInventory("Adding slot [{}] with [{}] charges.", first_empty_mslot, charges);
		ml2.item = item;
		ml2.npcid = npcid;
		ml2.slot = first_empty_mslot;
		ml2.origslot = first_empty_slot;
		tmp_merlist.push_back(ml2);
		tmpmerchanttable[npcid] = tmp_merlist;
		//DumpMerchantList(npcid);
		return ml2.slot;
	}
}

uint32 Zone::GetTempMerchantQuantity(uint32 NPCID, uint32 Slot) {

	std::list<TempMerchantList> TmpMerchantList = tmpmerchanttable[NPCID];
	std::list<TempMerchantList>::const_iterator Iterator;

	for (Iterator = TmpMerchantList.begin(); Iterator != TmpMerchantList.end(); ++Iterator)
		if ((*Iterator).slot == Slot) {
			LogInventory("Slot [{}] has [{}] charges.", Slot, (*Iterator).charges);
			return (*Iterator).charges;
		}

	return 0;
}

void Zone::LoadTempMerchantData()
{
	LogInfo("Loading Temporary Merchant Lists");

	auto results = content_db.QueryDatabase(
		fmt::format(
			SQL(
				SELECT
				DISTINCT npc_types.id
				FROM
					npc_types
				JOIN spawnentry ON spawnentry.npcID = npc_types.id
				JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID
				WHERE
				spawn2.zone = '{}'
				AND spawn2.version = {}
			),
			GetShortName(),
			GetInstanceVersion()
		)
	);

    if (!results.Success() || results.RowCount() == 0) {
        return;
	}

	std::vector<std::string> npc_ids;
	for (auto row = results.begin(); row != results.end(); ++row) {
		npc_ids.push_back(row[0]);
	}

	results = database.QueryDatabase(
		fmt::format(
			SQL(
				SELECT
				npcid,
				slot,
				charges,
				itemid
				FROM merchantlist_temp
				WHERE npcid IN ({})
			),
			implode(", ", npc_ids)
		)
	);

	std::map<uint32, std::list<TempMerchantList> >::iterator temp_merchant_table_entry;

	uint32 npc_id = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		TempMerchantList temp_merchant_list;
		temp_merchant_list.npcid = atoul(row[0]);
		if (npc_id != temp_merchant_list.npcid) {
			temp_merchant_table_entry = tmpmerchanttable.find(temp_merchant_list.npcid);
			if (temp_merchant_table_entry == tmpmerchanttable.end()) {
				std::list<TempMerchantList> empty;
				tmpmerchanttable[temp_merchant_list.npcid] = empty;
				temp_merchant_table_entry = tmpmerchanttable.find(temp_merchant_list.npcid);
			}
			npc_id = temp_merchant_list.npcid;
		}

		temp_merchant_list.slot     = atoul(row[1]);
		temp_merchant_list.charges  = atoul(row[2]);
		temp_merchant_list.item     = atoul(row[3]);
		temp_merchant_list.origslot = temp_merchant_list.slot;

		LogMerchants(
			"[LoadTempMerchantData] Loading merchant temp items npc_id [{}] slot [{}] charges [{}] item [{}] origslot [{}]",
			npc_id,
			temp_merchant_list.slot,
			temp_merchant_list.charges,
			temp_merchant_list.item,
			temp_merchant_list.origslot
		);

		temp_merchant_table_entry->second.push_back(temp_merchant_list);
	}
}

void Zone::LoadNewMerchantData(uint32 merchantid) {

	std::list<MerchantList> merchant_list;

	auto query = fmt::format(
		SQL(
			SELECT
				item,
				slot,
				faction_required,
				level_required,
				alt_currency_cost,
				classes_required,
				probability,
				bucket_name,
				bucket_value,
				bucket_comparison
			FROM merchantlist
			WHERE  merchantid = {} {}
			ORDER BY slot
		),
		merchantid,
		ContentFilterCriteria::apply()
	);

    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
        return;
	}

	for (auto row : results) {
		MerchantList ml;
		ml.id = merchantid;
		ml.item = std::stoul(row[0]);
		ml.slot = std::stoul(row[1]);
		ml.faction_required = static_cast<int16>(std::stoi(row[2]));
		ml.level_required = static_cast<uint8>(std::stoul(row[3]));
		ml.alt_currency_cost = static_cast<uint16>(std::stoul(row[4]));
		ml.classes_required = std::stoul(row[5]);
		ml.probability = static_cast<uint8>(std::stoul(row[6]));
		ml.bucket_name = row[7];
		ml.bucket_value = row[8];
		ml.bucket_comparison = static_cast<uint8>(std::stoul(row[9]));
		merchant_list.push_back(ml);
	}

	merchanttable[merchantid] = merchant_list;
}

void Zone::GetMerchantDataForZoneLoad() {
	LogInfo("Loading Merchant Lists");

	auto query = fmt::format(
		SQL (
			SELECT
			merchantid,
			slot,
			item,
			faction_required,
			level_required,
			alt_currency_cost,
			classes_required,
			probability,
			bucket_name,
			bucket_value,
			bucket_comparison
			from merchantlist where merchantid IN (
					select merchant_id from npc_types where id in (
						select npcID from spawnentry where spawngroupID IN (
							select spawngroupID from spawn2 where `zone` = '{}' and (`version` = {} OR `version` = -1)
					)
				)
			)
			{}
			ORDER BY
			merchantlist.slot
		),
		GetShortName(),
		GetInstanceVersion(),
		ContentFilterCriteria::apply()
	);

	auto results = content_db.QueryDatabase(query);

	std::map<uint32, std::list<MerchantList> >::iterator merchant_list;

	uint32 npc_id = 0;
	if (!results.Success() || !results.RowCount()) {
		LogDebug("No Merchant Data found for [{}]", GetShortName());
		return;
	}

	for (auto row : results) {
		MerchantList mle{};
		mle.id = std::stoul(row[0]);
		if (npc_id != mle.id) {
			merchant_list = merchanttable.find(mle.id);
			if (merchant_list == merchanttable.end()) {
				std::list<MerchantList> empty;
				merchanttable[mle.id] = empty;
				merchant_list = merchanttable.find(mle.id);
			}

			npc_id = mle.id;
		}

		bool found = false;
		for (const auto &m : merchant_list->second) {
			if (m.item == mle.id) {
				found = true;
				break;
			}
		}

		if (found) {
			continue;
		}

		mle.slot = std::stoul(row[1]);
		mle.item = std::stoul(row[2]);
		mle.faction_required = static_cast<int16>(std::stoi(row[3]));
		mle.level_required = static_cast<uint8>(std::stoul(row[4]));
		mle.alt_currency_cost = static_cast<uint16>(std::stoul(row[5]));
		mle.classes_required = std::stoul(row[6]);
		mle.probability = static_cast<uint8>(std::stoul(row[7]));
		mle.bucket_name = row[8];
		mle.bucket_value = row[9];
		mle.bucket_comparison = static_cast<uint8>(std::stoul(row[10]));

		merchant_list->second.push_back(mle);
	}
}

void Zone::LoadMercTemplates(){

	std::list<MercStanceInfo> merc_stances;
	merc_templates.clear();
    std::string query = "SELECT `class_id`, `proficiency_id`, `stance_id`, `isdefault` FROM "
                        "`merc_stance_entries` ORDER BY `class_id`, `proficiency_id`, `stance_id`";
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in ZoneDatabase::LoadMercTemplates()");
	}
	else {
		for (auto row = results.begin(); row != results.end(); ++row) {
			MercStanceInfo tempMercStanceInfo;

			tempMercStanceInfo.ClassID       = atoi(row[0]);
			tempMercStanceInfo.ProficiencyID = atoi(row[1]);
			tempMercStanceInfo.StanceID      = atoi(row[2]);
			tempMercStanceInfo.IsDefault     = atoi(row[3]);

			merc_stances.push_back(tempMercStanceInfo);
		}
	}

    query = "SELECT DISTINCT MTem.merc_template_id, MTyp.dbstring "
            "AS merc_type_id, MTem.dbstring "
            "AS merc_subtype_id, MTyp.race_id, MS.class_id, MTyp.proficiency_id, MS.tier_id, 0 "
            "AS CostFormula, MTem.clientversion, MTem.merc_npc_type_id "
            "FROM merc_types MTyp, merc_templates MTem, merc_subtypes MS "
            "WHERE MTem.merc_type_id = MTyp.merc_type_id AND MTem.merc_subtype_id = MS.merc_subtype_id "
			"ORDER BY MTyp.race_id, MS.class_id, MTyp.proficiency_id;";
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in ZoneDatabase::LoadMercTemplates()");
		return;
	}

    for (auto row = results.begin(); row != results.end(); ++row) {

        MercTemplate tempMercTemplate;

        tempMercTemplate.MercTemplateID = atoi(row[0]);
        tempMercTemplate.MercType = atoi(row[1]);
        tempMercTemplate.MercSubType = atoi(row[2]);
        tempMercTemplate.RaceID = atoi(row[3]);
        tempMercTemplate.ClassID = atoi(row[4]);
        tempMercTemplate.ProficiencyID = atoi(row[5]);
        tempMercTemplate.TierID = atoi(row[6]);
        tempMercTemplate.CostFormula = atoi(row[7]);
        tempMercTemplate.ClientVersion = atoi(row[8]);
        tempMercTemplate.MercNPCID = atoi(row[9]);

        for(int i = 0; i < MaxMercStanceID; i++)
            tempMercTemplate.Stances[i] = 0;

        int stanceIndex = 0;
        for (auto mercStanceListItr = merc_stances.begin(); mercStanceListItr != merc_stances.end(); ++mercStanceListItr) {
            if(mercStanceListItr->ClassID != tempMercTemplate.ClassID || mercStanceListItr->ProficiencyID != tempMercTemplate.ProficiencyID)
                continue;

            zone->merc_stance_list[tempMercTemplate.MercTemplateID].push_back((*mercStanceListItr));
            tempMercTemplate.Stances[stanceIndex] = mercStanceListItr->StanceID;
            ++stanceIndex;
        }

        merc_templates[tempMercTemplate.MercTemplateID] = tempMercTemplate;

    }

}

void Zone::LoadLevelEXPMods(){

	level_exp_mod.clear();
    const std::string query = "SELECT level, exp_mod, aa_exp_mod FROM level_exp_mods";
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
    LogError("Error in ZoneDatabase::LoadEXPLevelMods()");
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row) {
        uint32 index = atoi(row[0]);
		float exp_mod = atof(row[1]);
		float aa_exp_mod = atof(row[2]);
		level_exp_mod[index].ExpMod = exp_mod;
		level_exp_mod[index].AAExpMod = aa_exp_mod;
    }

}

void Zone::LoadMercSpells(){

	merc_spells_list.clear();
    const std::string query = "SELECT msl.class_id, msl.proficiency_id, msle.spell_id, msle.spell_type, "
                            "msle.stance_id, msle.minlevel, msle.maxlevel, msle.slot, msle.procChance "
                            "FROM merc_spell_lists msl, merc_spell_list_entries msle "
                            "WHERE msle.merc_spell_list_id = msl.merc_spell_list_id "
                            "ORDER BY msl.class_id, msl.proficiency_id, msle.spell_type, msle.minlevel, msle.slot;";
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in Zone::LoadMercSpells()");
		return;
	}

    for (auto row = results.begin(); row != results.end(); ++row) {
        uint32 classid;
        MercSpellEntry tempMercSpellEntry;

        classid = atoi(row[0]);
        tempMercSpellEntry.proficiencyid = atoi(row[1]);
        tempMercSpellEntry.spellid = atoi(row[2]);
        tempMercSpellEntry.type = atoi(row[3]);
        tempMercSpellEntry.stance = atoi(row[4]);
        tempMercSpellEntry.minlevel = atoi(row[5]);
        tempMercSpellEntry.maxlevel = atoi(row[6]);
        tempMercSpellEntry.slot = atoi(row[7]);
        tempMercSpellEntry.proc_chance = atoi(row[8]);

        merc_spells_list[classid].push_back(tempMercSpellEntry);
    }

	Log(Logs::General, Logs::Mercenaries, "Loaded %i merc spells.", merc_spells_list[1].size() + merc_spells_list[2].size() + merc_spells_list[9].size() + merc_spells_list[12].size());

}

bool Zone::IsLoaded() {
	return is_zone_loaded;
}

void Zone::Shutdown(bool quiet)
{
	if (!is_zone_loaded) {
		return;
	}

	entity_list.StopMobAI();

	std::map<uint32, NPCType *>::iterator itr;
	while (!zone->npctable.empty()) {
		itr = zone->npctable.begin();
		delete itr->second;
		itr->second = nullptr;
		zone->npctable.erase(itr);
	}

	while (!zone->merctable.empty()) {
		itr = zone->merctable.begin();
		delete itr->second;
		itr->second = nullptr;
		zone->merctable.erase(itr);
	}

	zone->adventure_entry_list_flavor.clear();

	std::map<uint32, LDoNTrapTemplate *>::iterator itr4;
	while (!zone->ldon_trap_list.empty()) {
		itr4 = zone->ldon_trap_list.begin();
		delete itr4->second;
		itr4->second = nullptr;
		zone->ldon_trap_list.erase(itr4);
	}
	zone->ldon_trap_entry_list.clear();

	LogInfo("Zone Shutdown: [{}] ([{}])", zone->GetShortName(), zone->GetZoneID());
	petition_list.ClearPetitions();
	zone->SetZoneHasCurrentTime(false);
	if (!quiet) {
		LogInfo("Zone Shutdown: Going to sleep");
	}

	is_zone_loaded = false;

	zone->ResetAuth();
	safe_delete(zone);
	entity_list.ClearAreas();
	parse->ReloadQuests(true);
	UpdateWindowTitle(nullptr);

	LogSys.CloseFileLogs();

	if (RuleB(Zone, KillProcessOnDynamicShutdown)) {
		LogInfo("[KillProcessOnDynamicShutdown] Shutting down");
		EQ::EventLoop::Get().Shutdown();
	}
}

void Zone::LoadZoneDoors()
{
	LogInfo("Loading doors for [{}] ", GetShortName());

	auto door_entries = content_db.LoadDoors(GetShortName(), GetInstanceVersion());
	if (door_entries.empty()) {
		LogInfo("No doors loaded");
		return;
	}

	for (const auto &entry : door_entries) {
		auto d = new Doors(entry);
		entity_list.AddDoor(d);
		LogDoorsDetail("Door added to entity list, db id: [{}], door_id: [{}]", entry.id, entry.doorid);
	}
}

Zone::Zone(uint32 in_zoneid, uint32 in_instanceid, const char* in_short_name)
:	initgrids_timer(10000),
	autoshutdown_timer((RuleI(Zone, AutoShutdownDelay))),
	clientauth_timer(AUTHENTICATION_TIMEOUT * 1000),
	spawn2_timer(1000),
	hot_reload_timer(1000),
	qglobal_purge_timer(30000),
	m_SafePoint(0.0f,0.0f,0.0f,0.0f),
	m_Graveyard(0.0f,0.0f,0.0f,0.0f)
{
	zoneid = in_zoneid;
	instanceid = in_instanceid;
	instanceversion = database.GetInstanceVersion(instanceid);
	pers_instance = false;
	zonemap = nullptr;
	watermap = nullptr;
	pathing = nullptr;
	qGlobals = nullptr;
	default_ruleset = 0;

	is_zone_time_localized = false;
	process_mobs_while_empty = false;

	loglevelvar = 0;
	merchantvar = 0;
	tradevar = 0;
	lootvar = 0;

	if(RuleB(TaskSystem, EnableTaskSystem)) {
		task_manager->LoadProximities(zoneid);
	}

	short_name = strcpy(new char[strlen(in_short_name)+1], in_short_name);
	strlwr(short_name);
	memset(file_name, 0, sizeof(file_name));
	long_name = 0;
	aggroedmobs =0;
	pgraveyard_id = 0;
	pgraveyard_zoneid = 0;
	pMaxClients = 0;
	pvpzone = false;
	if(database.GetServerType() == 1)
		pvpzone = true;
	content_db.GetZoneLongName(short_name, &long_name, file_name, &m_SafePoint.x, &m_SafePoint.y, &m_SafePoint.z, &pgraveyard_id, &pMaxClients);
	if(graveyard_id() > 0)
	{
		LogDebug("Graveyard ID is [{}]", graveyard_id());
		bool GraveYardLoaded = content_db.GetZoneGraveyard(graveyard_id(), &pgraveyard_zoneid, &m_Graveyard.x, &m_Graveyard.y, &m_Graveyard.z, &m_Graveyard.w);

		if (GraveYardLoaded) {
			LogDebug("Loaded a graveyard for zone [{}]: graveyard zoneid is [{}] at [{}]", short_name, graveyard_zoneid(), to_string(m_Graveyard).c_str());
		}
		else {
			LogError("Unable to load the graveyard id [{}] for zone [{}]", graveyard_id(), short_name);
		}
	}
	if (long_name == 0) {
		long_name = strcpy(new char[18], "Long zone missing");
	}
	autoshutdown_timer.Start(AUTHENTICATION_TIMEOUT * 1000, false);
	Weather_Timer = new Timer(60000);
	Weather_Timer->Start();
	LogDebug("The next weather check for zone: [{}] will be in [{}] seconds", short_name, Weather_Timer->GetRemainingTime()/1000);
	zone_weather              = 0;
	weather_intensity         = 0;
	blocked_spells            = nullptr;
	zone_total_blocked_spells = 0;
	zone_has_current_time     = false;

	Instance_Shutdown_Timer = nullptr;
	bool is_perma = false;
	if(instanceid > 0)
	{
		uint32 rem = database.GetTimeRemainingInstance(instanceid, is_perma);

		if(!is_perma)
		{
			if(rem < 150) //give some leeway to people who are zoning in 2.5 minutes to finish zoning in and get ported out
				rem = 150;
			Instance_Timer = new Timer(rem * 1000);
		}
		else
		{
			pers_instance = true;
			Instance_Timer = nullptr;
		}
	}
	else
	{
		Instance_Timer = nullptr;
	}
	adv_data = nullptr;
	map_name = nullptr;
	Instance_Warning_timer = nullptr;
	did_adventure_actions = false;
	database.QGlobalPurge();

	if(zoneid == RuleI(World, GuildBankZoneID))
		GuildBanks = new GuildBankManager;
	else
		GuildBanks = nullptr;

	m_ucss_available = false;
	m_last_ucss_update = 0;

	mMovementManager = &MobMovementManager::Get();

	SetNpcPositionUpdateDistance(0);
	SetQuestHotReloadQueued(false);
}

Zone::~Zone() {
	spawn2_list.Clear();
	safe_delete(zonemap);
	safe_delete(watermap);
	safe_delete(pathing);
	if (worldserver.Connected()) {
		worldserver.SetZoneData(0);
	}
	safe_delete_array(short_name);
	safe_delete_array(long_name);
	safe_delete(Weather_Timer);
	NPCEmoteList.Clear();
	zone_point_list.Clear();
	entity_list.Clear();
	ClearBlockedSpells();

	safe_delete(Instance_Timer);
	safe_delete(Instance_Shutdown_Timer);
	safe_delete(Instance_Warning_timer);
	safe_delete(qGlobals);
	safe_delete_array(adv_data);
	safe_delete_array(map_name);
	safe_delete(GuildBanks);
}

//Modified for timezones.
bool Zone::Init(bool is_static) {
	SetStaticZone(is_static);

	//load the zone config file.
	if (!LoadZoneCFG(GetShortName(), GetInstanceVersion())) { // try loading the zone name...
		LoadZoneCFG(
			GetFileName(),
			GetInstanceVersion()
		);
	} // if that fails, try the file name, then load defaults

	if (RuleManager::Instance()->GetActiveRulesetID() != default_ruleset) {
		std::string r_name = RuleManager::Instance()->GetRulesetName(&database, default_ruleset);
		if (r_name.size() > 0) {
			RuleManager::Instance()->LoadRules(&database, r_name.c_str(), false);
		}
	}

	zonemap  = Map::LoadMapFile(map_name);
	watermap = WaterMap::LoadWaterMapfile(map_name);
	pathing  = IPathfinder::Load(map_name);

	LogInfo("Loading spawn conditions");
	if(!spawn_conditions.LoadSpawnConditions(short_name, instanceid)) {
		LogError("Loading spawn conditions failed, continuing without them");
	}

	LogInfo("Loading static zone points");
	if (!content_db.LoadStaticZonePoints(&zone_point_list, short_name, GetInstanceVersion())) {
		LogError("Loading static zone points failed");
		return false;
	}

	LogInfo("Loading spawn groups");
	if (!content_db.LoadSpawnGroups(short_name, GetInstanceVersion(), &spawn_group_list)) {
		LogError("Loading spawn groups failed");
		return false;
	}

	LogInfo("Loading spawn2 points");
	if (!content_db.PopulateZoneSpawnList(zoneid, spawn2_list, GetInstanceVersion()))
	{
		LogError("Loading spawn2 points failed");
		return false;
	}

	LogInfo("Loading player corpses");
	if (!database.LoadCharacterCorpses(zoneid, instanceid)) {
		LogError("Loading player corpses failed");
		return false;
	}

	LogInfo("Loading traps");
	if (!content_db.LoadTraps(short_name, GetInstanceVersion()))
	{
		LogError("Loading traps failed");
		return false;
	}

	LogInfo("Loading adventure flavor text");
	LoadAdventureFlavor();

	LogInfo("Loading ground spawns");
	if (!LoadGroundSpawns())
	{
		LogError("Loading ground spawns failed. continuing");
	}

	LogInfo("Loading World Objects from DB");
	if (!LoadZoneObjects())
	{
		LogError("Loading World Objects failed. continuing");
	}

	LogInfo("Flushing old respawn timers");
	database.QueryDatabase("DELETE FROM `respawn_times` WHERE (`start` + `duration`) < UNIX_TIMESTAMP(NOW())");

	LoadZoneDoors();
	LoadZoneBlockedSpells();

	//clear trader items if we are loading the bazaar
	if (strncasecmp(short_name, "bazaar", 6) == 0) {
		database.DeleteTraderItem(0);
		database.DeleteBuyLines(0);
	}

	LoadLDoNTraps();
	LoadLDoNTrapEntries();
	LoadVeteranRewards();
	LoadAlternateCurrencies();
	LoadNPCEmotes(&NPCEmoteList);

	LoadAlternateAdvancement();

	content_db.LoadGlobalLoot();

	//Load merchant data
	GetMerchantDataForZoneLoad();

	//Load temporary merchant data
	LoadTempMerchantData();

	// Merc data
	if (RuleB(Mercs, AllowMercs)) {
		LoadMercTemplates();
		LoadMercSpells();
	}

	if (RuleB(Zone, LevelBasedEXPMods)) {
		LoadLevelEXPMods();
	}

	petition_list.ClearPetitions();
	petition_list.ReadDatabase();

	LogInfo("Loading dynamic zones");
	DynamicZone::CacheAllFromDatabase();

	LogInfo("Loading active Expeditions");
	Expedition::CacheAllFromDatabase();

	LogInfo("Loading timezone data");
	zone_time.setEQTimeZone(content_db.GetZoneTZ(zoneid, GetInstanceVersion()));

	LogInfo("Init Finished: ZoneID = [{}], Time Offset = [{}]", zoneid, zone_time.getEQTimeZone());

	LoadGrids();
	LoadTickItems();

	//MODDING HOOK FOR ZONE INIT
	mod_init();

	// logging origination information
	LogSys.origination_info.zone_short_name = zone->short_name;
	LogSys.origination_info.zone_long_name  = zone->long_name;
	LogSys.origination_info.instance_id     = zone->instanceid;

	return true;
}

void Zone::ReloadStaticData() {
	LogInfo("Reloading Zone Static Data");

	LogInfo("Reloading static zone points");
	if (!content_db.LoadStaticZonePoints(&zone_point_list, GetShortName(), GetInstanceVersion())) {
		LogError("Loading static zone points failed");
	}

	LogInfo("Reloading traps");
	entity_list.RemoveAllTraps();
	if (!content_db.LoadTraps(GetShortName(), GetInstanceVersion()))
	{
		LogError("Reloading traps failed");
	}

	LogInfo("Reloading ground spawns");
	if (!LoadGroundSpawns())
	{
		LogError("Reloading ground spawns failed. continuing");
	}

	entity_list.RemoveAllObjects();
	LogInfo("Reloading World Objects from DB");
	if (!LoadZoneObjects())
	{
		LogError("Reloading World Objects failed. continuing");
	}

	entity_list.RemoveAllDoors();
	LoadZoneDoors();
	entity_list.RespawnAllDoors();

	LoadVeteranRewards();
	LoadAlternateCurrencies();
	NPCEmoteList.Clear();
	LoadNPCEmotes(&NPCEmoteList);

	//load the zone config file.
	if (!LoadZoneCFG(GetShortName(), GetInstanceVersion())) { // try loading the zone name...
		LoadZoneCFG(
			GetFileName(),
			GetInstanceVersion()
		);
	} // if that fails, try the file name, then load defaults

	content_service.SetExpansionContext()->ReloadContentFlags();


	LogInfo("Zone Static Data Reloaded");
}

bool Zone::LoadZoneCFG(const char* filename, uint16 instance_version)
{

	memset(&newzone_data, 0, sizeof(NewZone_Struct));
	map_name = nullptr;

	if (!content_db.GetZoneCFG(
		ZoneID(filename),
		instance_version,
		&newzone_data,
		can_bind,
		can_combat,
		can_levitate,
		can_castoutdoor,
		is_city,
		is_hotzone,
		allow_mercs,
		max_movement_update_range,
		zone_type,
		default_ruleset,
		&map_name
	)) {
		// If loading a non-zero instance failed, try loading the default
		if (instance_version != 0) {
			safe_delete_array(map_name);
			if (!content_db.GetZoneCFG(
				ZoneID(filename),
				0,
				&newzone_data,
				can_bind,
				can_combat,
				can_levitate,
				can_castoutdoor,
				is_city,
				is_hotzone,
				allow_mercs,
				max_movement_update_range,
				zone_type,
				default_ruleset,
				&map_name
			)) {
				LogError("Error loading the Zone Config");
				return false;
			}
		}
	}

	//overwrite with our internal variables
	strcpy(newzone_data.zone_short_name, GetShortName());
	strcpy(newzone_data.zone_long_name, GetLongName());
	strcpy(newzone_data.zone_short_name2, GetShortName());

	LogInfo(
		"Successfully loaded Zone Config for Zone [{}] ({}) Version [{}] Instance ID [{}]",
		GetShortName(),
		GetLongName(),
		GetInstanceVersion(),
		instance_version
	);

	return true;
}

bool Zone::SaveZoneCFG()
{
	return content_db.SaveZoneCFG(GetZoneID(), GetInstanceVersion(), &newzone_data);
}

void Zone::AddAuth(ServerZoneIncomingClient_Struct* szic) {
	auto zca = new ZoneClientAuth_Struct;
	memset(zca, 0, sizeof(ZoneClientAuth_Struct));
	zca->ip = szic->ip;
	zca->wid = szic->wid;
	zca->accid = szic->accid;
	zca->admin = szic->admin;
	zca->charid = szic->charid;
	zca->lsid = szic->lsid;
	zca->tellsoff = szic->tellsoff;
	strn0cpy(zca->charname, szic->charname, sizeof(zca->charname));
	strn0cpy(zca->lskey, szic->lskey, sizeof(zca->lskey));
	zca->stale = false;
	client_auth_list.Insert(zca);
}

void Zone::RemoveAuth(const char* iCharName, const char* iLSKey)
{
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		ZoneClientAuth_Struct* zca = iterator.GetData();
		if (strcasecmp(zca->charname, iCharName) == 0 && strcasecmp(zca->lskey, iLSKey) == 0) {
			iterator.RemoveCurrent();
			return;
		}
		iterator.Advance();
	}
}

void Zone::RemoveAuth(uint32 lsid)
{
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		ZoneClientAuth_Struct* zca = iterator.GetData();
		if (zca->lsid == lsid) {
			iterator.RemoveCurrent();
			continue;
		}
		iterator.Advance();
	}
}

void Zone::ResetAuth()
{
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		iterator.RemoveCurrent();
	}
}

bool Zone::GetAuth(uint32 iIP, const char* iCharName, uint32* oWID, uint32* oAccID, uint32* oCharID, int16* oStatus, char* oLSKey, bool* oTellsOff) {
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		ZoneClientAuth_Struct* zca = iterator.GetData();
		if (strcasecmp(zca->charname, iCharName) == 0) {
				if(oWID)
				*oWID = zca->wid;
				if(oAccID)
				*oAccID = zca->accid;
				if(oCharID)
				*oCharID = zca->charid;
				if(oStatus)
				*oStatus = zca->admin;
				if(oTellsOff)
				*oTellsOff = zca->tellsoff;
				zca->stale = true;
			return true;
		}
		iterator.Advance();
	}
	return false;
}

uint32 Zone::CountAuth() {
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	int x = 0;
	iterator.Reset();
	while (iterator.MoreElements()) {
		x++;
		iterator.Advance();
	}
	return x;
}

bool Zone::Process() {
	spawn_conditions.Process();

	if (spawn2_timer.Check()) {

		LinkedListIterator<Spawn2 *> iterator(spawn2_list);

		EQ::InventoryProfile::CleanDirty();

		iterator.Reset();
		while (iterator.MoreElements()) {
			if (iterator.GetData()->Process()) {
				iterator.Advance();
			}
			else {
				iterator.RemoveCurrent();
			}
		}

		if (adv_data && !did_adventure_actions) {
			DoAdventureActions();
		}

		if (GetNpcPositionUpdateDistance() == 0) {
			CalculateNpcUpdateDistanceSpread();
		}
	}

	if (hot_reload_timer.Check() && IsQuestHotReloadQueued()) {

		LogHotReloadDetail("Hot reload timer check...");

		bool perform_reload = true;

		if (RuleB(HotReload, QuestsRepopWhenPlayersNotInCombat)) {
			for (auto &it : entity_list.GetClientList()) {
				auto client = it.second;
				if (client->GetAggroCount() > 0) {
					perform_reload = false;
					break;
				}
			}
		}

		if (perform_reload) {
			ZoneReload::HotReloadQuests();
		}
	}

	if(initgrids_timer.Check()) {
		//delayed grid loading stuff.
		initgrids_timer.Disable();
		LinkedListIterator<Spawn2*> iterator(spawn2_list);

		iterator.Reset();
		while (iterator.MoreElements()) {
			iterator.GetData()->LoadGrid();
			iterator.Advance();
		}
	}

	if(!staticzone) {
		if (autoshutdown_timer.Check()) {
			StartShutdownTimer();
			if (numclients == 0) {
				return false;
			}
		}
	}

	if(GetInstanceID() > 0)
	{
		if(Instance_Timer != nullptr && Instance_Shutdown_Timer == nullptr)
		{
			if(Instance_Timer->Check())
			{
				auto dz = GetDynamicZone();
				if (dz)
				{
					dz->RemoveAllMembers(); // entity list will teleport clients out immediately
				}

				// instance shutting down, move corpses to graveyard or non-instanced zone at same coords
				entity_list.MovePlayerCorpsesToGraveyard(true);
				entity_list.GateAllClientsToSafeReturn();
				database.DeleteInstance(GetInstanceID());
				Instance_Shutdown_Timer = new Timer(20000); //20 seconds
			}

			if(adv_data == nullptr)
			{
				if(Instance_Warning_timer == nullptr)
				{
					uint32 rem_time = Instance_Timer->GetRemainingTime();
					uint32_t minutes_warning = 0;
					if(rem_time < 60000 && rem_time > 55000)
					{
						minutes_warning = 1;
					}
					else if(rem_time < 300000 && rem_time > 295000)
					{
						minutes_warning = 5;
					}
					else if(rem_time < 900000 && rem_time > 895000)
					{
						minutes_warning = 15;
					}

					if (minutes_warning > 0)
					{
						// expedition expire warnings are handled by world
						auto expedition = Expedition::FindCachedExpeditionByZoneInstance(GetZoneID(), GetInstanceID());
						if (!expedition)
						{
							entity_list.ExpeditionWarning(minutes_warning);
							Instance_Warning_timer = new Timer(10000);
						}
					}
				}
				else if(Instance_Warning_timer->Check())
				{
					safe_delete(Instance_Warning_timer);
				}
			}
		}
		else if(Instance_Shutdown_Timer != nullptr)
		{
			if(Instance_Shutdown_Timer->Check())
			{
				StartShutdownTimer();
				return false;
			}
		}
	}

	if(Weather_Timer->Check())
	{
		Weather_Timer->Disable();
		ChangeWeather();
	}

	if(qGlobals)
	{
		if(qglobal_purge_timer.Check())
		{
			qGlobals->PurgeExpiredGlobals();
		}
	}

	if (clientauth_timer.Check()) {
		LinkedListIterator<ZoneClientAuth_Struct*> iterator2(client_auth_list);

		iterator2.Reset();
		while (iterator2.MoreElements()) {
			if (iterator2.GetData()->stale)
				iterator2.RemoveCurrent();
			else {
				iterator2.GetData()->stale = true;
				iterator2.Advance();
			}
		}
	}

	mMovementManager->Process();

	return true;
}

void Zone::ChangeWeather()
{
	if(!HasWeather())
	{
		Weather_Timer->Disable();
		return;
	}

	int chance = zone->random.Int(0, 3);
	uint8 rainchance = zone->newzone_data.rain_chance[chance];
	uint8 rainduration = zone->newzone_data.rain_duration[chance];
	uint8 snowchance = zone->newzone_data.snow_chance[chance];
	uint8 snowduration = zone->newzone_data.snow_duration[chance];
	uint32 weathertimer = 0;
	uint16 tmpweather = zone->random.Int(0, 100);
	uint8 duration = 0;
	uint8 tmpOldWeather = zone->zone_weather;
	bool changed = false;

	if(tmpOldWeather == 0)
	{
		if(rainchance > 0 || snowchance > 0)
		{
			uint8 intensity = zone->random.Int(1, 10);
			if((rainchance > snowchance) || (rainchance == snowchance))
			{
				//It's gunna rain!
				if(rainchance >= tmpweather)
				{
					if(rainduration == 0)
						duration = 1;
					else
						duration = rainduration*3; //Duration is 1 EQ hour which is 3 earth minutes.

					weathertimer = (duration*60)*1000;
					Weather_Timer->Start(weathertimer);
					zone->zone_weather = 1;
					zone->weather_intensity = intensity;
					changed = true;
				}
			}
			else
			{
				//It's gunna snow!
				if(snowchance >= tmpweather)
				{
					if(snowduration == 0)
						duration = 1;
					else
						duration = snowduration*3;
					weathertimer = (duration*60)*1000;
					Weather_Timer->Start(weathertimer);
					zone->zone_weather = 2;
					zone->weather_intensity = intensity;
					changed = true;
				}
			}
		}
	}
	else
	{
		changed = true;
		//We've had weather, now taking a break
		if(tmpOldWeather == 1)
		{
			if(rainduration == 0)
				duration = 1;
			else
				duration = rainduration*3; //Duration is 1 EQ hour which is 3 earth minutes.

			weathertimer = (duration*60)*1000;
			Weather_Timer->Start(weathertimer);
			zone->weather_intensity = 0;
		}
		else if(tmpOldWeather == 2)
		{
			if(snowduration == 0)
				duration = 1;
			else
				duration = snowduration*3; //Duration is 1 EQ hour which is 3 earth minutes.

			weathertimer = (duration*60)*1000;
			Weather_Timer->Start(weathertimer);
			zone->weather_intensity = 0;
		}
	}

	if(changed == false)
	{
		if(weathertimer == 0)
		{
			uint32 weatherTimerRule = RuleI(Zone, WeatherTimer);
			weathertimer = weatherTimerRule*1000;
			Weather_Timer->Start(weathertimer);
		}
		LogDebug("The next weather check for zone: [{}] will be in [{}] seconds", zone->GetShortName(), Weather_Timer->GetRemainingTime()/1000);
	}
	else
	{
		LogDebug("The weather for zone: [{}] has changed. Old weather was = [{}]. New weather is = [{}] The next check will be in [{}] seconds. Rain chance: [{}], Rain duration: [{}], Snow chance [{}], Snow duration: [{}]", zone->GetShortName(), tmpOldWeather, zone_weather,Weather_Timer->GetRemainingTime()/1000,rainchance,rainduration,snowchance,snowduration);
		weatherSend();
		if (zone->weather_intensity == 0)
		{
			zone->zone_weather = 0;
		}
	}
}

bool Zone::HasWeather()
{
	uint8 rain1 = zone->newzone_data.rain_chance[0];
	uint8 rain2 = zone->newzone_data.rain_chance[1];
	uint8 rain3 = zone->newzone_data.rain_chance[2];
	uint8 rain4 = zone->newzone_data.rain_chance[3];
	uint8 snow1 = zone->newzone_data.snow_chance[0];
	uint8 snow2 = zone->newzone_data.snow_chance[1];
	uint8 snow3 = zone->newzone_data.snow_chance[2];
	uint8 snow4 = zone->newzone_data.snow_chance[3];

	if(rain1 == 0 && rain2 == 0 && rain3 == 0 && rain4 == 0 && snow1 == 0 && snow2 == 0 && snow3 == 0 && snow4 == 0)
		return false;
	else
		return true;
}

void Zone::StartShutdownTimer(uint32 set_time) {
	if (set_time > autoshutdown_timer.GetRemainingTime()) {
		if (set_time == (RuleI(Zone, AutoShutdownDelay))) {
			set_time = static_cast<uint32>(content_db.getZoneShutDownDelay(GetZoneID(), GetInstanceVersion()));
		}

		autoshutdown_timer.SetTimer(set_time);
		LogDebug("Zone::StartShutdownTimer set to {}", set_time);
	}

	LogDebug("Zone::StartShutdownTimer trigger - set_time: [{}] remaining_time: [{}] diff: [{}]", set_time, autoshutdown_timer.GetRemainingTime(), (set_time - autoshutdown_timer.GetRemainingTime()));
}

bool Zone::Depop(bool StartSpawnTimer) {
	std::map<uint32,NPCType *>::iterator itr;
	entity_list.Depop(StartSpawnTimer);
	entity_list.ClearTrapPointers();
	entity_list.UpdateAllTraps(false);
	/* Refresh npctable (cache), getting current info from database. */
	while(!npctable.empty()) {
		itr = npctable.begin();
		delete itr->second;
		itr->second = nullptr;
		npctable.erase(itr);
	}

	// clear spell cache
	database.ClearNPCSpells();

	zone->spawn_group_list.ReloadSpawnGroups();

	return true;
}

void Zone::ClearNPCTypeCache(int id) {
	if (id <= 0) {
		auto iter = npctable.begin();
		while (iter != npctable.end()) {
			delete iter->second;
			iter->second = nullptr;
			++iter;
		}
		npctable.clear();
	}
	else {
		auto iter = npctable.begin();
		while (iter != npctable.end()) {
			if (iter->first == (uint32)id) {
				delete iter->second;
				iter->second = nullptr;
				npctable.erase(iter);
				return;
			}
			++iter;
		}
	}
}

void Zone::Repop()
{
	if (!Depop()) {
		return;
	}

	LinkedListIterator<Spawn2 *> iterator(spawn2_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		iterator.RemoveCurrent();
	}

	npc_scale_manager->LoadScaleData();

	entity_list.ClearTrapPointers();

	quest_manager.ClearAllTimers();

	LogInfo("Loading spawn groups");
	if (!content_db.LoadSpawnGroups(short_name, GetInstanceVersion(), &spawn_group_list)) {
		LogError("Loading spawn groups failed");
	}

	LogInfo("Loading spawn conditions");
	if (!spawn_conditions.LoadSpawnConditions(short_name, instanceid)) {
		LogError("Loading spawn conditions failed, continuing without them");
	}

	if (!content_db.PopulateZoneSpawnList(zoneid, spawn2_list, GetInstanceVersion())) {
		LogDebug("Error in Zone::Repop: database.PopulateZoneSpawnList failed");
	}

	LoadGrids();

	initgrids_timer.Start();

	entity_list.UpdateAllTraps(true, true);

	//MODDING HOOK FOR REPOP
	mod_repop();
}

void Zone::GetTimeSync()
{
	if (!zone_has_current_time) {
		LogInfo("Requesting world time");
		auto pack = new ServerPacket(ServerOP_GetWorldTime, 1);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Zone::SetDate(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute)
{
	if (worldserver.Connected()) {
		auto pack = new ServerPacket(ServerOP_SetWorldTime, sizeof(eqTimeOfDay));
		eqTimeOfDay* eqtod = (eqTimeOfDay*)pack->pBuffer;
		eqtod->start_eqtime.minute=minute;
		eqtod->start_eqtime.hour=hour;
		eqtod->start_eqtime.day=day;
		eqtod->start_eqtime.month=month;
		eqtod->start_eqtime.year=year;
		eqtod->start_realtime=time(0);
		printf("Setting master date on world server to: %d-%d-%d %d:%d (%d)\n", year, month, day, hour, minute, (int)eqtod->start_realtime);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Zone::SetTime(uint8 hour, uint8 minute, bool update_world /*= true*/)
{
	if (worldserver.Connected()) {
		auto pack = new ServerPacket(ServerOP_SetWorldTime, sizeof(eqTimeOfDay));
		eqTimeOfDay* eq_time_of_day = (eqTimeOfDay*)pack->pBuffer;

		zone_time.GetCurrentEQTimeOfDay(time(0), &eq_time_of_day->start_eqtime);

		eq_time_of_day->start_eqtime.minute = minute;
		eq_time_of_day->start_eqtime.hour = hour;
		eq_time_of_day->start_realtime = time(0);

		/* By Default we update worlds time, but we can optionally no update world which updates the rest of the zone servers */
		if (update_world){
			LogInfo("Setting master time on world server to: {}:{} ({})\n", hour, minute, (int)eq_time_of_day->start_realtime);
			worldserver.SendPacket(pack);

			/* Set Time Localization Flag */
			zone->is_zone_time_localized = false;
		}
		/* When we don't update world, we are localizing ourselves, we become disjointed from normal syncs and set time locally */
		else{

			LogInfo("Setting zone localized time...");

			zone->zone_time.SetCurrentEQTimeOfDay(eq_time_of_day->start_eqtime, eq_time_of_day->start_realtime);
			auto outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
			TimeOfDay_Struct* time_of_day = (TimeOfDay_Struct*)outapp->pBuffer;
			zone->zone_time.GetCurrentEQTimeOfDay(time(0), time_of_day);
			entity_list.QueueClients(0, outapp, false);
			safe_delete(outapp);

			/* Set Time Localization Flag */
			zone->is_zone_time_localized = true;
		}

		safe_delete(pack);
	}
}

ZonePoint* Zone::GetClosestZonePoint(const glm::vec3& location, uint32 to, Client* client, float max_distance) {
	LinkedListIterator<ZonePoint*> iterator(zone_point_list);
	ZonePoint* closest_zp = nullptr;
	float closest_dist = FLT_MAX;
	float max_distance2 = max_distance * max_distance;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* zp = iterator.GetData();
		uint32 mask_test = client->ClientVersionBit();
		if (!(zp->client_version_mask & mask_test)) {
			iterator.Advance();
			continue;
		}

		if (zp->target_zone_id == to)
		{
            auto dist = Distance(glm::vec2(zp->x, zp->y), glm::vec2(location));
			if ((zp->x == 999999 || zp->x == -999999) && (zp->y == 999999 || zp->y == -999999))
				dist = 0;

			if (dist < closest_dist)
			{
				closest_zp = zp;
				closest_dist = dist;
			}
		}
		iterator.Advance();
	}

	// if we have a water map and it says we're in a zoneline, lets assume it's just a really big zone line
	// this shouldn't open up any exploits since those situations are detected later on
	if ((zone->HasWaterMap() && !zone->watermap->InZoneLine(glm::vec3(client->GetPosition()))) || (!zone->HasWaterMap() && closest_dist > 400.0f && closest_dist < max_distance2))
	{
		if (client) {
			if (!client->cheat_manager.GetExemptStatus(Port)) {
				client->cheat_manager.CheatDetected(MQZoneUnknownDest, location);
			}
		}
		LogInfo("WARNING: Closest zone point for zone id [{}] is [{}], you might need to update your zone_points table if you dont arrive at the right spot", to, closest_dist);
		LogInfo("<Real Zone Points>. [{}]", to_string(location).c_str());
	}

	if(closest_dist > max_distance2)
		closest_zp = nullptr;

	if(!closest_zp)
		closest_zp = GetClosestZonePointWithoutZone(location.x, location.y, location.z, client);

	return closest_zp;
}

ZonePoint* Zone::GetClosestZonePoint(const glm::vec3& location, const char* to_name, Client* client, float max_distance) {
	if(to_name == nullptr)
		return GetClosestZonePointWithoutZone(location.x, location.y, location.z, client, max_distance);
	return GetClosestZonePoint(location, ZoneID(to_name), client, max_distance);
}

ZonePoint* Zone::GetClosestZonePointWithoutZone(float x, float y, float z, Client* client, float max_distance) {
	LinkedListIterator<ZonePoint*> iterator(zone_point_list);
	ZonePoint* closest_zp = nullptr;
	float closest_dist = FLT_MAX;
	float max_distance2 = max_distance*max_distance;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* zp = iterator.GetData();
		uint32 mask_test = client->ClientVersionBit();

		if(!(zp->client_version_mask & mask_test))
		{
			iterator.Advance();
			continue;
		}

		float delta_x = zp->x - x;
		float delta_y = zp->y - y;
		if(zp->x == 999999 || zp->x == -999999)
			delta_x = 0;
		if(zp->y == 999999 || zp->y == -999999)
			delta_y = 0;

		float dist = delta_x*delta_x+delta_y*delta_y;///*+(zp->z-z)*(zp->z-z)*/;
		if (dist < closest_dist)
		{
			closest_zp = zp;
			closest_dist = dist;
		}
		iterator.Advance();
	}
	if(closest_dist > max_distance2)
		closest_zp = nullptr;

	return closest_zp;
}

bool ZoneDatabase::LoadStaticZonePoints(LinkedList<ZonePoint *> *zone_point_list, const char *zonename, uint32 version)
{
	zone_point_list->Clear();
	zone->numzonepoints = 0;
	zone->virtual_zone_point_list.clear();

	auto zone_points = ZonePointsRepository::GetWhere(content_db,
		fmt::format(
			"zone = '{}' AND (version = {} OR version = -1) {} ORDER BY number",
			zonename,
			version,
			ContentFilterCriteria::apply()
		)
	);

	for (auto &zone_point : zone_points) {
		auto zp = new ZonePoint;

		zp->x                    = zone_point.x;
		zp->y                    = zone_point.y;
		zp->z                    = zone_point.z;
		zp->target_x             = zone_point.target_x;
		zp->target_y             = zone_point.target_y;
		zp->target_z             = zone_point.target_z;
		zp->target_zone_id       = zone_point.target_zone_id;
		zp->heading              = zone_point.heading;
		zp->target_heading       = zone_point.target_heading;
		zp->number               = zone_point.number;
		zp->target_zone_instance = zone_point.target_instance;
		zp->client_version_mask  = zone_point.client_version_mask;
		zp->is_virtual           = zone_point.is_virtual > 0;
		zp->height               = zone_point.height;
		zp->width                = zone_point.width;

		LogZonePoints(
			"Loading ZP x [{}] y [{}] z [{}] heading [{}] target x y z zone_id instance_id [{}] [{}] [{}] [{}] [{}] number [{}] is_virtual [{}] height [{}] width [{}]",
			zp->x,
			zp->y,
			zp->z,
			zp->heading,
			zp->target_x,
			zp->target_y,
			zp->target_z,
			zp->target_zone_id,
			zp->target_zone_instance,
			zp->number,
			zp->is_virtual ? "true" : "false",
			zp->height,
			zp->width
		);

		if (zone_point.is_virtual) {
			zone->virtual_zone_point_list.emplace_back(zone_point);
			safe_delete(zp);
			continue;
		}

		zone_point_list->Insert(zp);
		zone->numzonepoints++;
	}

	return true;
}

bool ZoneDatabase::GetDecayTimes(npcDecayTimes_Struct *npcCorpseDecayTimes)
{
	const std::string query =
	    "SELECT varname, value FROM variables WHERE varname LIKE 'decaytime%%' ORDER BY varname";
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	int index = 0;
	for (auto row = results.begin(); row != results.end(); ++row, ++index) {
		Seperator sep(row[0]);
		npcCorpseDecayTimes[index].minlvl = atoi(sep.arg[1]);
		npcCorpseDecayTimes[index].maxlvl = atoi(sep.arg[2]);

		npcCorpseDecayTimes[index].seconds = std::min(24 * 60 * 60, atoi(row[1]));
	}

	return true;
}

void Zone::weatherSend(Client *client)
{
	auto outapp = new EQApplicationPacket(OP_Weather, 8);
	if (zone_weather > 0) {
		outapp->pBuffer[0] = zone_weather - 1;
	}
	if (zone_weather > 0) {
		outapp->pBuffer[4] = zone->weather_intensity;
	}
	if (client) {
		client->QueuePacket(outapp);
	}
	else {
		entity_list.QueueClients(0, outapp);
	}
	safe_delete(outapp);
}

bool Zone::HasGraveyard() {
	bool Result = false;

	if(graveyard_zoneid() > 0)
		Result = true;

	return Result;
}

void Zone::SetGraveyard(uint32 zoneid, const glm::vec4& graveyardPosition) {
	pgraveyard_zoneid = zoneid;
	m_Graveyard = graveyardPosition;
}

void Zone::LoadZoneBlockedSpells()
{
	if (!blocked_spells) {
		zone_total_blocked_spells = content_db.GetBlockedSpellsCount(GetZoneID());
		if (zone_total_blocked_spells > 0) {
			blocked_spells = new ZoneSpellsBlocked[zone_total_blocked_spells];
			if (!content_db.LoadBlockedSpells(zone_total_blocked_spells, blocked_spells, GetZoneID())) {
				LogError(" Failed to load blocked spells");
				ClearBlockedSpells();
			}
		}
	}
}

void Zone::ClearBlockedSpells()
{
	if (blocked_spells) {
		safe_delete_array(blocked_spells);
		zone_total_blocked_spells = 0;
	}
}

bool Zone::IsSpellBlocked(uint32 spell_id, const glm::vec3 &location)
{
	if (blocked_spells) {
		bool exception = false;
		bool block_all = false;

		for (int x = 0; x < GetZoneTotalBlockedSpells(); x++) {
			if (blocked_spells[x].spellid == spell_id) {
				exception = true;
			}

			if (blocked_spells[x].spellid == 0) {
				block_all = true;
			}
		}

		// If all spells are blocked and this is an exception, it is not blocked
		if (block_all && exception) {
			return false;
		}

		for (int x = 0; x < GetZoneTotalBlockedSpells(); x++) {
			// Spellid of 0 matches all spells
			if (0 != blocked_spells[x].spellid && spell_id != blocked_spells[x].spellid) {
				continue;
			}

			switch (blocked_spells[x].type) {
				case ZoneBlockedSpellTypes::ZoneWide: {
					return true;
					break;
				}
				case ZoneBlockedSpellTypes::Region: {
					if (IsWithinAxisAlignedBox(
						location,
						blocked_spells[x].m_Location - blocked_spells[x].m_Difference,
						blocked_spells[x].m_Location + blocked_spells[x].m_Difference
					)) {
						return true;
					}
					break;
				}
				default: {
					continue;
					break;
				}
			}
		}
	}

	return false;
}

const char *Zone::GetSpellBlockedMessage(uint32 spell_id, const glm::vec3 &location)
{
	if (blocked_spells) {
		for (int x = 0; x < GetZoneTotalBlockedSpells(); x++) {
			if (spell_id != blocked_spells[x].spellid && blocked_spells[x].spellid != 0) {
				continue;
			}
			switch (blocked_spells[x].type) {
				case ZoneBlockedSpellTypes::ZoneWide: {
					return blocked_spells[x].message;
					break;
				}
				case ZoneBlockedSpellTypes::Region: {
					if (IsWithinAxisAlignedBox(
						location,
						blocked_spells[x].m_Location - blocked_spells[x].m_Difference,
						blocked_spells[x].m_Location + blocked_spells[x].m_Difference
					)) {
						return blocked_spells[x].message;
					}
					break;
				}
				default: {
					continue;
					break;
				}
			}
		}
	}
	return "Error: Message String Not Found\0";
}

void Zone::SetInstanceTimer(uint32 new_duration)
{
	if(Instance_Timer)
	{
		Instance_Timer->Start(new_duration * 1000);
	}
}

void Zone::LoadLDoNTraps()
{
	const std::string query   = "SELECT id, type, spell_id, skill, locked FROM ldon_trap_templates";
	auto              results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto lt = new LDoNTrapTemplate;
		lt->id       = atoi(row[0]);
		lt->type     = (LDoNChestTypes) atoi(row[1]);
		lt->spell_id = atoi(row[2]);
		lt->skill    = atoi(row[3]);
		lt->locked   = atoi(row[4]);
		ldon_trap_list[lt->id] = lt;
	}

}

void Zone::LoadLDoNTrapEntries()
{
	const std::string query = "SELECT id, trap_id FROM ldon_trap_entries";
    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
		return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
    {
        uint32 id = atoi(row[0]);
        uint32 trap_id = atoi(row[1]);

        LDoNTrapTemplate *trapTemplate = nullptr;
        auto it = ldon_trap_list.find(trap_id);

        if(it == ldon_trap_list.end())
            continue;

        trapTemplate = ldon_trap_list[trap_id];

        std::list<LDoNTrapTemplate*> temp;
        auto iter = ldon_trap_entry_list.find(id);

        if(iter != ldon_trap_entry_list.end())
            temp = ldon_trap_entry_list[id];

        temp.push_back(trapTemplate);
        ldon_trap_entry_list[id] = temp;
    }

}

void Zone::LoadVeteranRewards()
{
	VeteranRewards.clear();

	InternalVeteranReward current_reward;
	current_reward.claim_id = 0;

    const std::string query = "SELECT claim_id, name, item_id, charges "
                            "FROM veteran_reward_templates "
                            "WHERE reward_slot < 8 and claim_id > 0 "
                            "ORDER by claim_id, reward_slot";
    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

	int index = 0;
    for (auto row = results.begin(); row != results.end(); ++row, ++index)
    {
        uint32 claim = atoi(row[0]);

        if(claim != current_reward.claim_id)
        {
            if(current_reward.claim_id != 0)
            {
                current_reward.claim_count = index;
                current_reward.number_available = 1;
                VeteranRewards.push_back(current_reward);
            }

            index = 0;
            memset(&current_reward, 0, sizeof(InternalVeteranReward));
            current_reward.claim_id = claim;
        }

        strcpy(current_reward.items[index].item_name, row[1]);
        current_reward.items[index].item_id = atoi(row[2]);
        current_reward.items[index].charges = atoi(row[3]);
    }

    if(current_reward.claim_id != 0)
    {
        current_reward.claim_count = index;
        current_reward.number_available = 1;
        VeteranRewards.push_back(current_reward);
    }

}

void Zone::LoadAlternateCurrencies()
{
	AlternateCurrencies.clear();

	AltCurrencyDefinition_Struct current_currency;

    const std::string query = "SELECT id, item_id FROM alternate_currency";
    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
		return;
    }

    for (auto row : results) {
        current_currency.id = std::stoul(row[0]);
        current_currency.item_id = std::stoul(row[1]);
        AlternateCurrencies.push_back(current_currency);
    }

}

void Zone::UpdateQGlobal(uint32 qid, QGlobal newGlobal)
{
	if(newGlobal.npc_id != 0)
		return;

	if(newGlobal.char_id != 0)
		return;

	if(newGlobal.zone_id == GetZoneID() || newGlobal.zone_id == 0)
	{
		if(qGlobals)
		{
			qGlobals->AddGlobal(qid, newGlobal);
		}
		else
		{
			qGlobals = new QGlobalCache();
			qGlobals->AddGlobal(qid, newGlobal);
		}
	}
}

void Zone::DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID)
{
	if(qGlobals)
	{
		qGlobals->RemoveGlobal(name, npcID, charID, zoneID);
	}
}

void Zone::LoadAdventureFlavor()
{
	const std::string query = "SELECT id, text FROM adventure_template_entry_flavor";
	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

    for (auto row = results.begin(); row != results.end(); ++row) {
        uint32 id = atoi(row[0]);
        adventure_entry_list_flavor[id] = row[1];
    }

}

void Zone::DoAdventureCountIncrease()
{
	ServerZoneAdventureDataReply_Struct *sr = (ServerZoneAdventureDataReply_Struct*)adv_data;
	if(sr->count < sr->total)
	{
		sr->count++;
		auto pack = new ServerPacket(ServerOP_AdventureCountUpdate, sizeof(uint16));
		*((uint16*)pack->pBuffer) = instanceid;
		worldserver.SendPacket(pack);
		delete pack;
	}
}

void Zone::DoAdventureAssassinationCountIncrease()
{
	ServerZoneAdventureDataReply_Struct *sr = (ServerZoneAdventureDataReply_Struct*)adv_data;
	if(sr->assa_count < RuleI(Adventure, NumberKillsForBossSpawn))
	{
		sr->assa_count++;
		auto pack = new ServerPacket(ServerOP_AdventureAssaCountUpdate, sizeof(uint16));
		*((uint16*)pack->pBuffer) = instanceid;
		worldserver.SendPacket(pack);
		delete pack;
	}
}

void Zone::DoAdventureActions()
{
	ServerZoneAdventureDataReply_Struct* ds = (ServerZoneAdventureDataReply_Struct*)adv_data;
	if(ds->type == Adventure_Collect)
	{
		int count = (ds->total - ds->count) * 25 / 10;
		entity_list.AddLootToNPCS(ds->data_id, count);
		did_adventure_actions = true;
	}
	else if(ds->type == Adventure_Assassinate)
	{
		if(ds->assa_count >= RuleI(Adventure, NumberKillsForBossSpawn))
		{
			const NPCType* tmp = content_db.LoadNPCTypesData(ds->data_id);
			if(tmp)
			{
				NPC* npc = new NPC(tmp, nullptr, glm::vec4(ds->assa_x, ds->assa_y, ds->assa_z, ds->assa_h), GravityBehavior::Water);
				npc->AddLootTable();
				if (npc->DropsGlobalLoot())
					npc->CheckGlobalLootTables();
				entity_list.AddNPC(npc);
				npc->Shout("Rarrrgh!");
				did_adventure_actions = true;
			}
		}
	}
	else
	{
		did_adventure_actions = true;
	}

}

void Zone::LoadNPCEmotes(LinkedList<NPC_Emote_Struct*>* NPCEmoteList)
{

	NPCEmoteList->Clear();
    const std::string query = "SELECT emoteid, event_, type, text FROM npc_emotes";
    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
    {
	    auto nes = new NPC_Emote_Struct;
	    nes->emoteid = atoi(row[0]);
	    nes->event_ = atoi(row[1]);
	    nes->type = atoi(row[2]);
	    strn0cpy(nes->text, row[3], sizeof(nes->text));
	    NPCEmoteList->Insert(nes);
    }

}

void Zone::ReloadWorld(uint8 global_repop)
{
	entity_list.ClearAreas();
	parse->ReloadQuests();

	if (global_repop) {
		if (global_repop == ReloadWorld::ForceRepop) {
			zone->ClearSpawnTimers();
		}

		zone->Repop();
	}

	worldserver.SendEmoteMessage(
		0,
		0,
		AccountStatus::GMAdmin,
		Chat::Yellow,
		fmt::format(
			"Quests reloaded {}for {}{}.",
			(
				global_repop ?
				(
					global_repop == ReloadWorld::Repop ?
					"and repopped NPCs " :
					"and forcefully repopped NPCs "
				) :
				""
			),
			fmt::format(
				"{} ({})",
				GetLongName(),
				GetZoneID()
			),
			(
				GetInstanceID() ?
				fmt::format(
					" (Instance ID {})",
					GetInstanceID()
				) :
				""
			)
		).c_str()
	);
}

void Zone::ClearSpawnTimers()
{
	LinkedListIterator<Spawn2 *> iterator(spawn2_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		auto query = fmt::format(
			"DELETE FROM respawn_times WHERE id = {} AND instance_id = {}",
			iterator.GetData()->GetID(),
			GetInstanceID()
		);
		auto results = database.QueryDatabase(query);

		iterator.Advance();
	}
}

void Zone::LoadTickItems()
{
	tick_items.clear();

    const std::string query = "SELECT it_itemid, it_chance, it_level, it_qglobal, it_bagslot FROM item_tick";
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        return;
    }


    for (auto row = results.begin(); row != results.end(); ++row) {
        if(atoi(row[0]) == 0)
            continue;

        item_tick_struct ti_tmp;
		ti_tmp.itemid = atoi(row[0]);
		ti_tmp.chance = atoi(row[1]);
		ti_tmp.level = atoi(row[2]);
		ti_tmp.bagslot = (int16)atoi(row[4]);
		ti_tmp.qglobal = std::string(row[3]);
		tick_items[atoi(row[0])] = ti_tmp;

    }

}

uint32 Zone::GetSpawnKillCount(uint32 in_spawnid) {
	LinkedListIterator<Spawn2*> iterator(spawn2_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID() == in_spawnid)
		{
			return(iterator.GetData()->killcount);
		}
		iterator.Advance();
	}
	return 0;
}

void Zone::SetIsHotzone(bool is_hotzone)
{
	Zone::is_hotzone = is_hotzone;
}

void Zone::RequestUCSServerStatus() {
	auto outapp = new ServerPacket(ServerOP_UCSServerStatusRequest, sizeof(UCSServerStatus_Struct));
	auto ucsss = (UCSServerStatus_Struct*)outapp->pBuffer;
	ucsss->available = 0;
	ucsss->port = Config->ZonePort;
	ucsss->unused = 0;
	worldserver.SendPacket(outapp);
	safe_delete(outapp);
}

void Zone::SetUCSServerAvailable(bool ucss_available, uint32 update_timestamp) {
	if (m_last_ucss_update == update_timestamp && m_ucss_available != ucss_available) {
		m_ucss_available = false;
		RequestUCSServerStatus();
		return;
	}
	if (m_last_ucss_update < update_timestamp)
		m_ucss_available = ucss_available;
}

int Zone::GetNpcPositionUpdateDistance() const
{
	return npc_position_update_distance;
}

void Zone::SetNpcPositionUpdateDistance(int in_npc_position_update_distance)
{
	Zone::npc_position_update_distance = in_npc_position_update_distance;
}

void Zone::CalculateNpcUpdateDistanceSpread()
{
	float max_x = 0;
	float max_y = 0;
	float min_x = 0;
	float min_y = 0;

	auto &mob_list = entity_list.GetMobList();

	for (auto &it : mob_list) {
		Mob *entity = it.second;
		if (!entity->IsNPC()) {
			continue;
		}

		if (entity->GetX() <= min_x) {
			min_x = entity->GetX();
		}

		if (entity->GetY() <= min_y) {
			min_y = entity->GetY();
		}

		if (entity->GetX() >= max_x) {
			max_x = entity->GetX();
		}

		if (entity->GetY() >= max_y) {
			max_y = entity->GetY();
		}
	}

	int x_spread        = int(abs(max_x - min_x));
	int y_spread        = int(abs(max_y - min_y));
	int combined_spread = int(abs((x_spread + y_spread) / 2));
	int update_distance = EQ::ClampLower(int(combined_spread / 4), int(zone->GetMaxMovementUpdateRange()));

	SetNpcPositionUpdateDistance(update_distance);

	Log(Logs::General, Logs::Debug,
		"NPC update spread distance set to [%i] combined_spread [%i]",
		update_distance,
		combined_spread
	);
}

bool Zone::IsQuestHotReloadQueued() const
{
	return quest_hot_reload_queued;
}

void Zone::SetQuestHotReloadQueued(bool in_quest_hot_reload_queued)
{
	quest_hot_reload_queued = in_quest_hot_reload_queued;
}

void Zone::LoadGrids()
{
	zone_grids        = GridRepository::GetZoneGrids(GetZoneID());
	zone_grid_entries = GridEntriesRepository::GetZoneGridEntries(GetZoneID());
}

Timer Zone::GetInitgridsTimer()
{
	return initgrids_timer;
}

uint32 Zone::GetInstanceTimeRemaining() const
{
	return instance_time_remaining;
}

void Zone::SetInstanceTimeRemaining(uint32 instance_time_remaining)
{
	Zone::instance_time_remaining = instance_time_remaining;
}

bool Zone::IsZone(uint32 zone_id, uint16 instance_id) const
{
	return (zoneid == zone_id && instanceid == instance_id);
}

DynamicZone* Zone::GetDynamicZone()
{
	if (GetInstanceID() == 0)
	{
		return nullptr;
	}

	// todo: cache dynamic zone id on zone later for faster lookup
	for (const auto& dz_iter : zone->dynamic_zone_cache)
	{
		if (dz_iter.second->IsSameDz(GetZoneID(), GetInstanceID()))
		{
			return dz_iter.second.get();
		}
	}

	return nullptr;
}

uint32 Zone::GetCurrencyID(uint32 item_id)
{
	if (!item_id) {
		return 0;
	}

	for (const auto& alternate_currency : AlternateCurrencies) {
		if (item_id == alternate_currency.item_id) {
			return alternate_currency.id;
		}
	}

	return 0;
}

uint32 Zone::GetCurrencyItemID(uint32 currency_id)
{
	if (!currency_id) {
		return 0;
	}

	for (const auto& alternate_currency : AlternateCurrencies) {
		if (currency_id == alternate_currency.id) {
			return alternate_currency.item_id;
		}
	}

	return 0;
}

std::string Zone::GetZoneDescription()
{
	if (!IsLoaded()) {
		return fmt::format(
			"PID ({})",
			EQ::GetPID()
		);
	}

	return fmt::format(
		"{} ({}){}{}",
		GetLongName(),
		GetZoneID(),
		(
			GetInstanceID() ?
			fmt::format(
				" (Instance ID {})",
				GetInstanceID()
			) :
			""
		),
		(
			GetInstanceVersion() ?
			fmt::format(
				" (Version {})",
				GetInstanceVersion()
			) :
			""
		)
	);
}

void Zone::SendReloadMessage(std::string reload_type)
{
	worldserver.SendEmoteMessage(
		0,
		0,
		AccountStatus::GMAdmin,
		Chat::Yellow,
		fmt::format(
			"{} reloaded for {}.",
			reload_type,
			GetZoneDescription()
		).c_str()
	);
}

void Zone::SendDiscordMessage(int webhook_id, const std::string& message)
{
	if (worldserver.Connected()) {
		auto pack = new ServerPacket(ServerOP_DiscordWebhookMessage, sizeof(DiscordWebhookMessage_Struct) + 1);
		auto *q   = (DiscordWebhookMessage_Struct *) pack->pBuffer;

		strn0cpy(q->message, message.c_str(), 2000);
		q->webhook_id = webhook_id;

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Zone::SendDiscordMessage(const std::string& webhook_name, const std::string &message)
{
	bool not_found = true;
	for (auto & w : LogSys.discord_webhooks) {
		if (w.webhook_name == webhook_name) {
			SendDiscordMessage(w.id, message + "\n");
			not_found = false;
		}
	}

	if (not_found) {
		LogDiscord("[SendDiscordMessage] Did not find valid webhook by webhook name [{}]", webhook_name);
	}
}
