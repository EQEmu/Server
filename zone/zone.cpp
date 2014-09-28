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
#include "../common/debug.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <time.h>
#include <math.h>

#ifdef _WINDOWS
#include <process.h>
#define	snprintf	_snprintf
#define	vsnprintf	_vsnprintf
#else
#include <pthread.h>
#include "../common/unix.h"
#endif

#include "masterentity.h"
#include "../common/features.h"
#include "spawngroup.h"
#include "spawn2.h"
#include "zone.h"
#include "worldserver.h"
#include "npc.h"
#include "net.h"
#include "../common/seperator.h"
#include "../common/packet_dump_file.h"
#include "../common/eq_stream_factory.h"
#include "../common/eq_stream.h"
#include "../common/string_util.h"
#include "zone_config.h"
#include "../common/breakdowns.h"
#include "map.h"
#include "water_map.h"
#include "object.h"
#include "petitions.h"
#include "pathing.h"
#include "event_codes.h"
#include "client_logs.h"
#include "../common/rulesys.h"
#include "guild_mgr.h"
#include "quest_parser_collection.h"

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif


extern WorldServer worldserver;
extern Zone* zone;
extern uint32 numclients;
extern NetConnection net;
extern uint16 adverrornum;
extern PetitionList petition_list;
Mutex MZoneShutdown;
extern bool staticzone;
Zone* zone = 0;
volatile bool ZoneLoaded = false;
extern QuestParserCollection* parse;

bool Zone::Bootup(uint32 iZoneID, uint32 iInstanceID, bool iStaticZone) {
	const char* zonename = database.GetZoneName(iZoneID);

	if (iZoneID == 0 || zonename == 0)
		return false;
	if (zone != 0 || ZoneLoaded) {
		std::cerr << "Error: Zone::Bootup call when zone already booted!" << std::endl;
		worldserver.SetZone(0);
		return false;
	}

	LogFile->write(EQEMuLog::Status, "Booting %s (%d:%d)", zonename, iZoneID, iInstanceID);

	numclients = 0;
	zone = new Zone(iZoneID, iInstanceID, zonename);

	//init the zone, loads all the data, etc
	if (!zone->Init(iStaticZone)) {
		safe_delete(zone);
		std::cerr << "Zone->Init failed" << std::endl;
		worldserver.SetZone(0);
		return false;
	}
	zone->zonemap = Map::LoadMapFile(zone->map_name);
	zone->watermap = WaterMap::LoadWaterMapfile(zone->map_name);
	zone->pathing = PathManager::LoadPathFile(zone->map_name); 

	char tmp[10];
	if (database.GetVariable("loglevel",tmp, 9)) {
		int log_levels[4];
		if (atoi(tmp)>9){ //Server is using the new code
			for(int i=0;i<4;i++){
				if (((int)tmp[i]>=48) && ((int)tmp[i]<=57))
					log_levels[i]=(int)tmp[i]-48; //get the value to convert it to an int from the ascii value
				else
					log_levels[i]=0; //set to zero on a bogue char
			}
			zone->loglevelvar = log_levels[0];
			LogFile->write(EQEMuLog::Status, "General logging level: %i", zone->loglevelvar);
			zone->merchantvar = log_levels[1];
			LogFile->write(EQEMuLog::Status, "Merchant logging level: %i", zone->merchantvar);
			zone->tradevar = log_levels[2];
			LogFile->write(EQEMuLog::Status, "Trade logging level: %i", zone->tradevar);
			zone->lootvar = log_levels[3];
			LogFile->write(EQEMuLog::Status, "Loot logging level: %i", zone->lootvar);
		}
		else {
			zone->loglevelvar = uint8(atoi(tmp)); //continue supporting only command logging (for now)
			zone->merchantvar = 0;
			zone->tradevar = 0;
			zone->lootvar = 0;
		}
	}

	ZoneLoaded = true;

	worldserver.SetZone(iZoneID, iInstanceID);
	if(iInstanceID != 0)
	{
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureZoneData, sizeof(uint16));
		*((uint16*)pack->pBuffer) = iInstanceID;
		worldserver.SendPacket(pack);
		delete pack;
	}

	LogFile->write(EQEMuLog::Normal, "---- Zone server %s, listening on port:%i ----", zonename, ZoneConfig::get()->ZonePort);
	LogFile->write(EQEMuLog::Status, "Zone Bootup: %s (%i: %i)", zonename, iZoneID, iInstanceID);
	parse->Init();
	UpdateWindowTitle();
	zone->GetTimeSync();

	return true;
}

//this really loads the objects into entity_list
bool Zone::LoadZoneObjects() {

	std::string query = StringFormat("SELECT id, zoneid, xpos, ypos, zpos, heading, "
                                    "itemid, charges, objectname, type, icon, unknown08, "
                                    "unknown10, unknown20, unknown24, unknown76 fROM object "
                                    "WHERE zoneid = %i AND (version = %u OR version = -1)",
                                    zoneid, instanceversion);
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
		LogFile->write(EQEMuLog::Error, "Error Loading Objects from DB: %s",results.ErrorMessage().c_str());
		return false;
    }

    LogFile->write(EQEMuLog::Status, "Loading Objects from DB...");
    for (auto row = results.begin(); row != results.end(); ++row) {
        if (atoi(row[9]) == 0)
        {
            // Type == 0 - Static Object
            const char* shortname = database.GetZoneName(atoi(row[1]), false); // zoneid -> zone_shortname

            if (!shortname)
                continue;

            Door d;
            memset(&d, 0, sizeof(d));

            strn0cpy(d.zone_name, shortname, sizeof(d.zone_name));
            d.db_id = 1000000000 + atoi(row[0]); // Out of range of normal use for doors.id
            d.door_id = -1; // Client doesn't care if these are all the same door_id
            d.pos_x = atof(row[2]); // xpos
            d.pos_y = atof(row[3]); // ypos
            d.pos_z = atof(row[4]); // zpos
            d.heading = atof(row[5]); // heading

            strn0cpy(d.door_name, row[8], sizeof(d.door_name)); // objectname
            // Strip trailing "_ACTORDEF" if present. Client won't accept it for doors.
            int len = strlen(d.door_name);
            if ((len > 9) && (memcmp(&d.door_name[len - 9], "_ACTORDEF", 10) == 0))
                d.door_name[len - 9] = '\0';

            memcpy(d.dest_zone, "NONE", 5);

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

            d.incline = atoi(row[13]); // unknown20 = optional model incline value
            d.client_version_mask = 0xFFFFFFFF; //We should load the mask from the zone.

            Doors* door = new Doors(&d);
            entity_list.AddDoor(door);
        }

        Object_Struct data = {0};
        uint32 id = 0;
        uint32 icon = 0;
        uint32 type = 0;
        uint32 itemid = 0;
        uint32 idx = 0;
        int16 charges = 0;

        id	= (uint32)atoi(row[0]);
        data.zone_id = atoi(row[1]);
        data.x = atof(row[2]);
        data.y = atof(row[3]);
        data.z = atof(row[4]);
        data.heading = atof(row[5]);
		itemid = (uint32)atoi(row[6]);
		charges	= (int16)atoi(row[7]);
        strcpy(data.object_name, row[8]);
        type = (uint8)atoi(row[9]);
        icon = (uint32)atoi(row[10]);
		data.object_type = type;
		data.linked_list_addr[0] = 0;
        data.linked_list_addr[1] = 0;
        data.unknown008	= (uint32)atoi(row[11]);
        data.unknown010	= (uint32)atoi(row[12]);
        data.unknown020	= (uint32)atoi(row[13]);
        data.unknown024	= (uint32)atoi(row[14]);
        data.unknown076	= (uint32)atoi(row[15]);
        data.unknown084	= 0;

        ItemInst* inst = nullptr;
        //FatherNitwit: this dosent seem to work...
        //tradeskill containers do not have an itemid of 0... at least what I am seeing
        if (itemid == 0) {
            // Generic tradeskill container
            inst = new ItemInst(ItemInstWorldContainer);
        }
        else {
            // Groundspawn object
            inst = database.CreateItem(itemid);
        }

        //Father Nitwit's fix... not perfect...
        if(inst == nullptr && type != OT_DROPPEDITEM) {
            inst = new ItemInst(ItemInstWorldContainer);
        }

        // Load child objects if container
        if (inst && inst->IsType(ItemClassContainer)) {
            database.LoadWorldContainer(id, inst);
        }

        Object* object = new Object(id, type, icon, data, inst);
        entity_list.AddObject(object, false);
        if(type == OT_DROPPEDITEM && itemid != 0)
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
	LogFile->write(EQEMuLog::Status, "Loading Ground Spawns from DB...");
	database.LoadGroundSpawns(zoneid, GetInstanceVersion(), &groundspawn);
	uint32 ix=0;
	char* name=0;
	uint32 gsnumber=0;
	for(gsindex=0;gsindex<50;gsindex++){
		if(groundspawn.spawn[gsindex].item>0 && groundspawn.spawn[gsindex].item<500000){
			ItemInst* inst = nullptr;
			inst = database.CreateItem(groundspawn.spawn[gsindex].item);
			gsnumber=groundspawn.spawn[gsindex].max_allowed;
			ix=0;
			if(inst){
				name = groundspawn.spawn[gsindex].name;
				for(ix=0;ix<gsnumber;ix++){
					Object* object = new Object(inst,name,groundspawn.spawn[gsindex].max_x,groundspawn.spawn[gsindex].min_x,groundspawn.spawn[gsindex].max_y,groundspawn.spawn[gsindex].min_y,groundspawn.spawn[gsindex].max_z,groundspawn.spawn[gsindex].heading,groundspawn.spawn[gsindex].respawntimer);//new object with id of 10000+
					entity_list.AddObject(object, false);
				}
				safe_delete(inst);
			}
		}
	}
	return(true);
}

int Zone::SaveTempItem(uint32 merchantid, uint32 npcid, uint32 item, int32 charges, bool sold){
	int freeslot = 0;
	std::list<MerchantList> merlist = merchanttable[merchantid];
	std::list<MerchantList>::const_iterator itr;
	uint32 i = 1;
	for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
		MerchantList ml = *itr;
		if(ml.item == item)
			return 0;

		// Account for merchant lists with gaps in them.
		if(ml.slot >= i)
			i = ml.slot + 1;

	}
	std::list<TempMerchantList> tmp_merlist = tmpmerchanttable[npcid];
	std::list<TempMerchantList>::const_iterator tmp_itr;
	bool update_charges = false;
	TempMerchantList ml;
	while(freeslot == 0 && !update_charges){
		freeslot = i;
		for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
			ml = *tmp_itr;
			if(ml.item == item){
				update_charges = true;
				freeslot = 0;
				break;
			}
			if((ml.slot == i) || (ml.origslot==i)) {
				freeslot=0;
			}
		}
		i++;
	}
	if(update_charges){
		tmp_merlist.clear();
		std::list<TempMerchantList> oldtmp_merlist = tmpmerchanttable[npcid];
		for (tmp_itr = oldtmp_merlist.begin(); tmp_itr != oldtmp_merlist.end(); ++tmp_itr) {
			TempMerchantList ml2 = *tmp_itr;
			if(ml2.item != item)
				tmp_merlist.push_back(ml2);
		}
		if(sold)
			ml.charges = ml.charges + charges;
		else
			ml.charges = charges;
		if(!ml.origslot)
			ml.origslot = ml.slot;
		if(charges>0){
			database.SaveMerchantTemp(npcid, ml.origslot, item, ml.charges);
			tmp_merlist.push_back(ml);
		}
		else{
			database.DeleteMerchantTemp(npcid,ml.origslot);
		}
		tmpmerchanttable[npcid] = tmp_merlist;

		if(sold)
			return ml.slot;

	}
	if(freeslot){
		if(charges<0) //sanity check only, shouldnt happen
			charges = 0x7FFF;
		database.SaveMerchantTemp(npcid, freeslot, item, charges);
		tmp_merlist = tmpmerchanttable[npcid];
		TempMerchantList ml2;
		ml2.charges = charges;
		ml2.item = item;
		ml2.npcid = npcid;
		ml2.slot = freeslot;
		ml2.origslot = ml2.slot;
		tmp_merlist.push_back(ml2);
		tmpmerchanttable[npcid] = tmp_merlist;
	}
	return freeslot;
}

uint32 Zone::GetTempMerchantQuantity(uint32 NPCID, uint32 Slot) {

	std::list<TempMerchantList> TmpMerchantList = tmpmerchanttable[NPCID];
	std::list<TempMerchantList>::const_iterator Iterator;

	for (Iterator = TmpMerchantList.begin(); Iterator != TmpMerchantList.end(); ++Iterator)
		if((*Iterator).slot == Slot)
			return (*Iterator).charges;

	return 0;
}

void Zone::LoadTempMerchantData(){
	LogFile->write(EQEMuLog::Status, "Loading Temporary Merchant Lists...");
	std::string query = StringFormat(
		"SELECT								   "
		"ml.npcid,							   "
		"ml.slot,							   "
		"ml.charges,						   "
		"ml.itemid							   "
		"FROM								   "
		"merchantlist_temp ml,				   "
		"spawnentry se,						   "
		"spawn2 s2							   "
		"WHERE								   "
		"ml.npcid = se.npcid				   "
		"AND se.spawngroupid = s2.spawngroupid "
		"AND s2.zone = '%s' AND s2.version = %i", GetShortName(), GetInstanceVersion());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogFile->write(EQEMuLog::Error, "Error in LoadTempMerchantData query '%s' %s", query.c_str(), results.ErrorMessage().c_str());
		return;
	}
	std::map<uint32, std::list<TempMerchantList> >::iterator cur;
	uint32 npcid = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		TempMerchantList ml;
		ml.npcid = atoul(row[0]);
		if (npcid != ml.npcid){
			cur = tmpmerchanttable.find(ml.npcid);
			if (cur == tmpmerchanttable.end()) {
				std::list<TempMerchantList> empty;
				tmpmerchanttable[ml.npcid] = empty;
				cur = tmpmerchanttable.find(ml.npcid);
			}
			npcid = ml.npcid;
		}
		ml.slot = atoul(row[1]);
		ml.charges = atoul(row[2]);
		ml.item = atoul(row[3]);
		ml.origslot = ml.slot;
		cur->second.push_back(ml);
	}
	pQueuedMerchantsWorkID = 0;
}

void Zone::LoadNewMerchantData(uint32 merchantid){

	std::list<MerchantList> merlist;
	std::string query = StringFormat("SELECT item, slot, faction_required, level_required, alt_currency_cost, "
                                    "classes_required FROM merchantlist WHERE merchantid=%d", merchantid);
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in LoadNewMerchantData query '%s' %s", query.c_str(), results.ErrorMessage().c_str());
        return;
    }

    for(auto row = results.begin(); row != results.end(); ++row) {
        MerchantList ml;
        ml.id = merchantid;
        ml.item = atoul(row[0]);
        ml.slot = atoul(row[1]);
        ml.faction_required = atoul(row[2]);
        ml.level_required = atoul(row[3]);
        ml.alt_currency_cost = atoul(row[3]);
        ml.classes_required = atoul(row[4]);
        merlist.push_back(ml);
    }

    merchanttable[merchantid] = merlist;
}

void Zone::GetMerchantDataForZoneLoad(){
	LogFile->write(EQEMuLog::Status, "Loading Merchant Lists...");
	std::string query = StringFormat(												   
		"SELECT																		   "
		"ml.merchantid,																   "
		"ml.slot,																	   "
		"ml.item,																	   "
		"ml.faction_required,														   "
		"ml.level_required,															   "
		"ml.alt_currency_cost,														   "
		"ml.classes_required,														   "
		"ml.probability																   "
		"FROM																		   "
		"merchantlist AS ml,														   "
		"npc_types AS nt,															   "
		"spawnentry AS se,															   "
		"spawn2 AS s2																   "
		"WHERE nt.merchant_id = ml.merchantid AND nt.id = se.npcid					   "
		"AND se.spawngroupid = s2.spawngroupid AND s2.zone = '%s' AND s2.version = %i  ", GetShortName(), GetInstanceVersion());
	auto results = database.QueryDatabase(query); 
	std::map<uint32, std::list<MerchantList> >::iterator cur;
	uint32 npcid = 0;
	if (results.RowCount() == 0){ LogFile->write(EQEMuLog::Error, "Error in loading Merchant Data for zone");  return; }
	for (auto row = results.begin(); row != results.end(); ++row) { 
		MerchantList ml;
		ml.id = atoul(row[0]);
		if (npcid != ml.id){
			cur = merchanttable.find(ml.id);
			if (cur == merchanttable.end()) {
				std::list<MerchantList> empty;
				merchanttable[ml.id] = empty;
				cur = merchanttable.find(ml.id);
			}
			npcid = ml.id;
		}

		std::list<MerchantList>::iterator iter = cur->second.begin();
		bool found = false;
		while (iter != cur->second.end()) {
			if ((*iter).item == ml.id) {
				found = true;
				break;
			}
			++iter;
		}

		if (found) {
			continue;
		}

		ml.slot = atoul(row[1]);
		ml.item = atoul(row[2]);
		ml.faction_required = atoul(row[3]);
		ml.level_required = atoul(row[4]);
		ml.alt_currency_cost = atoul(row[5]);
		ml.classes_required = atoul(row[6]);
		ml.probability = atoul(row[7]);
		cur->second.push_back(ml);
	}

}

void Zone::LoadMercTemplates(){

	std::list<MercStanceInfo> merc_stances;
	merc_templates.clear();
    std::string query = "SELECT `class_id`, `proficiency_id`, `stance_id`, `isdefault` FROM "
                        "`merc_stance_entries` ORDER BY `class_id`, `proficiency_id`, `stance_id`";
    auto results = database.QueryDatabase(query);
    if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error in ZoneDatabase::LoadMercTemplates()");
	else {
		for (auto row = results.begin(); row != results.end(); ++row) {
			MercStanceInfo tempMercStanceInfo;

			tempMercStanceInfo.ClassID = atoi(row[0]);
			tempMercStanceInfo.ProficiencyID = atoi(row[1]);
			tempMercStanceInfo.StanceID = atoi(row[2]);
			tempMercStanceInfo.IsDefault = atoi(row[3]);

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
        LogFile->write(EQEMuLog::Error, "Error in ZoneDatabase::LoadMercTemplates()");
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
        LogFile->write(EQEMuLog::Error, "Error in ZoneDatabase::LoadEXPLevelMods()");
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
        LogFile->write(EQEMuLog::Error, "Error in Zone::LoadMercSpells()");
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

    if(MERC_DEBUG > 0)
        LogFile->write(EQEMuLog::Debug, "Mercenary Debug: Loaded %i merc spells.", merc_spells_list[1].size() + merc_spells_list[2].size() + merc_spells_list[9].size() + merc_spells_list[12].size());

}

bool Zone::IsLoaded() {
	return ZoneLoaded;
}

void Zone::Shutdown(bool quite)
{
	if (!ZoneLoaded)
		return;

	std::list<Mob*> mob_list;
	entity_list.GetMobList(mob_list);
	std::list<Mob*>::iterator mob_itr = mob_list.begin();
	while (mob_itr != mob_list.end()) {
		Mob* mob_inst = *mob_itr;
		mob_inst->AI_Stop();
		mob_inst->AI_ShutDown();
		++mob_itr;
	}

	std::map<uint32,NPCType *>::iterator itr;
	while(zone->npctable.size()) {
		itr=zone->npctable.begin();
		delete itr->second;
		zone->npctable.erase(itr);
	}

	while(zone->merctable.size()) {
		itr=zone->merctable.begin();
		delete itr->second;
		zone->merctable.erase(itr);
	}

	zone->adventure_entry_list_flavor.clear();

	std::map<uint32,LDoNTrapTemplate*>::iterator itr4;
	while(zone->ldon_trap_list.size())
	{
		itr4 = zone->ldon_trap_list.begin();
		delete itr4->second;
		zone->ldon_trap_list.erase(itr4);
	}
	zone->ldon_trap_entry_list.clear();

	LogFile->write(EQEMuLog::Status, "Zone Shutdown: %s (%i)", zone->GetShortName(), zone->GetZoneID());
	petition_list.ClearPetitions();
	zone->GotCurTime(false);
	if (!quite)
		LogFile->write(EQEMuLog::Normal, "Zone shutdown: going to sleep");
	ZoneLoaded = false;

	zone->ResetAuth();
	safe_delete(zone);
	entity_list.ClearAreas();
	parse->ReloadQuests(true);
	UpdateWindowTitle();
}

void Zone::LoadZoneDoors(const char* zone, int16 version)
{
	LogFile->write(EQEMuLog::Status, "Loading doors for %s ...", zone);

	uint32 maxid;
	int32 count = database.GetDoorsCount(&maxid, zone, version);
	if(count < 1) {
		LogFile->write(EQEMuLog::Status, "... No doors loaded.");
		return;
	}

	Door *dlist = new Door[count];

	if(!database.LoadDoors(count, dlist, zone, version)) {
		LogFile->write(EQEMuLog::Error, "... Failed to load doors.");
		delete[] dlist;
		return;
	}

	int r;
	Door *d = dlist;
	for(r = 0; r < count; r++, d++) {
		Doors* newdoor = new Doors(d);
		entity_list.AddDoor(newdoor);
	}
	delete[] dlist;
}

Zone::Zone(uint32 in_zoneid, uint32 in_instanceid, const char* in_short_name)
:	initgrids_timer(10000),
	autoshutdown_timer((RuleI(Zone, AutoShutdownDelay))),
	clientauth_timer(AUTHENTICATION_TIMEOUT * 1000),
	spawn2_timer(1000),
	qglobal_purge_timer(30000),
	hotzone_timer(120000)
{
	zoneid = in_zoneid;
	instanceid = in_instanceid;
	instanceversion = database.GetInstanceVersion(instanceid);
	zonemap = nullptr;
	watermap = nullptr;
	pathing = nullptr;
	qGlobals = nullptr;
	default_ruleset = 0;

	loglevelvar = 0;
	merchantvar = 0;
	tradevar = 0;
	lootvar = 0;

	if(RuleB(TaskSystem, EnableTaskSystem)) {
		taskmanager->LoadProximities(zoneid);
	}

	short_name = strcpy(new char[strlen(in_short_name)+1], in_short_name);
	strlwr(short_name);
	memset(file_name, 0, sizeof(file_name));
	long_name = 0;
	aggroedmobs =0;

	psafe_x = 0;
	psafe_y = 0;
	psafe_z = 0;
	pgraveyard_id = 0;
	pgraveyard_zoneid = 0;
	pgraveyard_x = 0;
	pgraveyard_y = 0;
	pgraveyard_z = 0;
	pgraveyard_heading = 0;
	pMaxClients = 0;
	pQueuedMerchantsWorkID = 0;
	pvpzone = false;
	if(database.GetServerType() == 1)
		pvpzone = true;
	database.GetZoneLongName(short_name, &long_name, file_name, &psafe_x, &psafe_y, &psafe_z, &pgraveyard_id, &pMaxClients);
	if(graveyard_id() > 0)
	{
		LogFile->write(EQEMuLog::Debug, "Graveyard ID is %i.", graveyard_id());
		bool GraveYardLoaded = database.GetZoneGraveyard(graveyard_id(), &pgraveyard_zoneid, &pgraveyard_x, &pgraveyard_y, &pgraveyard_z, &pgraveyard_heading);
		if(GraveYardLoaded)
			LogFile->write(EQEMuLog::Debug, "Loaded a graveyard for zone %s: graveyard zoneid is %u x is %f y is %f z is %f heading is %f.", short_name, graveyard_zoneid(), graveyard_x(), graveyard_y(), graveyard_z(), graveyard_heading());
		else
			LogFile->write(EQEMuLog::Error, "Unable to load the graveyard id %i for zone %s.", graveyard_id(), short_name);
	}
	if (long_name == 0) {
		long_name = strcpy(new char[18], "Long zone missing");
	}
	autoshutdown_timer.Start(AUTHENTICATION_TIMEOUT * 1000, false);
	Weather_Timer = new Timer(60000);
	Weather_Timer->Start();
	LogFile->write(EQEMuLog::Debug, "The next weather check for zone: %s will be in %i seconds.", short_name, Weather_Timer->GetRemainingTime()/1000);
	zone_weather = 0;
	weather_intensity = 0;
	blocked_spells = nullptr;
	totalBS = 0;
	aas = nullptr;
	totalAAs = 0;
	gottime = false;

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
}

Zone::~Zone() {
	spawn2_list.Clear();
	safe_delete(zonemap);
	safe_delete(watermap);
	safe_delete(pathing);
	if (worldserver.Connected()) {
		worldserver.SetZone(0);
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

	if(aas != nullptr) {
		int r;
		for(r = 0; r < totalAAs; r++) {
			uchar *data = (uchar *) aas[r];
			safe_delete_array(data);
		}
		safe_delete_array(aas);
	}
#ifdef CLIENT_LOGS
	client_logs.clear();
#endif

	safe_delete(GuildBanks);
}

//Modified for timezones.
bool Zone::Init(bool iStaticZone) {
	SetStaticZone(iStaticZone);

	LogFile->write(EQEMuLog::Status, "Loading spawn conditions...");
	if(!spawn_conditions.LoadSpawnConditions(short_name, instanceid)) {
		LogFile->write(EQEMuLog::Error, "Loading spawn conditions failed, continuing without them.");
	}

	LogFile->write(EQEMuLog::Status, "Loading static zone points...");
	if (!database.LoadStaticZonePoints(&zone_point_list, short_name, GetInstanceVersion())) {
		LogFile->write(EQEMuLog::Error, "Loading static zone points failed.");
		return false;
	}

	LogFile->write(EQEMuLog::Status, "Loading spawn groups...");
	if (!database.LoadSpawnGroups(short_name, GetInstanceVersion(), &spawn_group_list)) {
		LogFile->write(EQEMuLog::Error, "Loading spawn groups failed.");
		return false;
	}

	LogFile->write(EQEMuLog::Status, "Loading spawn2 points...");
	if (!database.PopulateZoneSpawnList(zoneid, spawn2_list, GetInstanceVersion()))
	{
		LogFile->write(EQEMuLog::Error, "Loading spawn2 points failed.");
		return false;
	}

	LogFile->write(EQEMuLog::Status, "Loading player corpses...");
	if (!database.LoadPlayerCorpses(zoneid, instanceid)) {
		LogFile->write(EQEMuLog::Error, "Loading player corpses failed.");
		return false;
	}

	LogFile->write(EQEMuLog::Status, "Loading traps...");
	if (!database.LoadTraps(short_name, GetInstanceVersion()))
	{
		LogFile->write(EQEMuLog::Error, "Loading traps failed.");
		return false;
	}

	LogFile->write(EQEMuLog::Status, "Loading adventure flavor text...");
	LoadAdventureFlavor();

	LogFile->write(EQEMuLog::Status, "Loading ground spawns...");
	if (!LoadGroundSpawns())
	{
		LogFile->write(EQEMuLog::Error, "Loading ground spawns failed. continuing.");
	}

	LogFile->write(EQEMuLog::Status, "Loading World Objects from DB...");
	if (!LoadZoneObjects())
	{
		LogFile->write(EQEMuLog::Error, "Loading World Objects failed. continuing.");
	}

	//load up the zone's doors (prints inside)
	zone->LoadZoneDoors(zone->GetShortName(), zone->GetInstanceVersion());
	zone->LoadBlockedSpells(zone->GetZoneID());

	//clear trader items if we are loading the bazaar
	if(strncasecmp(short_name,"bazaar",6)==0) {
		database.DeleteTraderItem(0);
		database.DeleteBuyLines(0);
	}

	zone->LoadLDoNTraps();
	zone->LoadLDoNTrapEntries();
	zone->LoadVeteranRewards();
	zone->LoadAlternateCurrencies();
	zone->LoadNPCEmotes(&NPCEmoteList);

	//Load AA information
	adverrornum = 500;
	LoadAAs();

	//Load merchant data
	adverrornum = 501;
	zone->GetMerchantDataForZoneLoad();

	//Load temporary merchant data
	adverrornum = 502;
	zone->LoadTempMerchantData();

	// Merc data
	if (RuleB(Mercs, AllowMercs)) {
		zone->LoadMercTemplates();
		zone->LoadMercSpells();
	}

	if (RuleB(Zone, LevelBasedEXPMods))
		zone->LoadLevelEXPMods();

	adverrornum = 503;
	petition_list.ClearPetitions();
	petition_list.ReadDatabase();

	//load the zone config file.
	if (!LoadZoneCFG(zone->GetShortName(), zone->GetInstanceVersion(), true)) // try loading the zone name...
		LoadZoneCFG(zone->GetFileName(), zone->GetInstanceVersion()); // if that fails, try the file name, then load defaults

	if(RuleManager::Instance()->GetActiveRulesetID() != default_ruleset)
	{
		std::string r_name = RuleManager::Instance()->GetRulesetName(&database, default_ruleset);
		if(r_name.size() > 0)
		{
			RuleManager::Instance()->LoadRules(&database, r_name.c_str());
		}
	}

	LogFile->write(EQEMuLog::Status, "Loading timezone data...");
	zone->zone_time.setEQTimeZone(database.GetZoneTZ(zoneid, GetInstanceVersion()));

	LogFile->write(EQEMuLog::Status, "Init Finished: ZoneID = %d, Time Offset = %d", zoneid, zone->zone_time.getEQTimeZone());

	LoadTickItems();

	//MODDING HOOK FOR ZONE INIT
	mod_init();

	return true;
}

void Zone::ReloadStaticData() {
	LogFile->write(EQEMuLog::Status, "Reloading Zone Static Data...");

	LogFile->write(EQEMuLog::Status, "Reloading static zone points...");
	zone_point_list.Clear();
	if (!database.LoadStaticZonePoints(&zone_point_list, GetShortName(), GetInstanceVersion())) {
		LogFile->write(EQEMuLog::Error, "Loading static zone points failed.");
	}

	LogFile->write(EQEMuLog::Status, "Reloading traps...");
	entity_list.RemoveAllTraps();
	if (!database.LoadTraps(GetShortName(), GetInstanceVersion()))
	{
		LogFile->write(EQEMuLog::Error, "Reloading traps failed.");
	}

	LogFile->write(EQEMuLog::Status, "Reloading ground spawns...");
	if (!LoadGroundSpawns())
	{
		LogFile->write(EQEMuLog::Error, "Reloading ground spawns failed. continuing.");
	}

	entity_list.RemoveAllObjects();
	LogFile->write(EQEMuLog::Status, "Reloading World Objects from DB...");
	if (!LoadZoneObjects())
	{
		LogFile->write(EQEMuLog::Error, "Reloading World Objects failed. continuing.");
	}

	entity_list.RemoveAllDoors();
	zone->LoadZoneDoors(zone->GetShortName(), zone->GetInstanceVersion());
	entity_list.RespawnAllDoors();

	zone->LoadVeteranRewards();
	zone->LoadAlternateCurrencies();
	NPCEmoteList.Clear();
	zone->LoadNPCEmotes(&NPCEmoteList);

	//load the zone config file.
	if (!LoadZoneCFG(zone->GetShortName(), zone->GetInstanceVersion(), true)) // try loading the zone name...
		LoadZoneCFG(zone->GetFileName(), zone->GetInstanceVersion()); // if that fails, try the file name, then load defaults

	LogFile->write(EQEMuLog::Status, "Zone Static Data Reloaded.");
}

bool Zone::LoadZoneCFG(const char* filename, uint16 instance_id, bool DontLoadDefault)
{
	memset(&newzone_data, 0, sizeof(NewZone_Struct));
	if(instance_id == 0)
	{
		map_name = nullptr;
		if(!database.GetZoneCFG(database.GetZoneID(filename), 0, &newzone_data, can_bind,
			can_combat, can_levitate, can_castoutdoor, is_city, is_hotzone, allow_mercs, zone_type, default_ruleset, &map_name))
		{
			LogFile->write(EQEMuLog::Error, "Error loading the Zone Config.");
			return false;
		}
	}
	else
	{
		//Fall back to base zone if we don't find the instance version.
		map_name = nullptr;
		if(!database.GetZoneCFG(database.GetZoneID(filename), instance_id, &newzone_data, can_bind,
			can_combat, can_levitate, can_castoutdoor, is_city, is_hotzone, allow_mercs, zone_type, default_ruleset, &map_name))
		{
			safe_delete_array(map_name);
			if(!database.GetZoneCFG(database.GetZoneID(filename), 0, &newzone_data, can_bind,
			can_combat, can_levitate, can_castoutdoor, is_city, is_hotzone, allow_mercs, zone_type, default_ruleset, &map_name))
			{
				LogFile->write(EQEMuLog::Error, "Error loading the Zone Config.");
				return false;
			}
		}
	}

	//overwrite with our internal variables
	strcpy(newzone_data.zone_short_name, GetShortName());
	strcpy(newzone_data.zone_long_name, GetLongName());
	strcpy(newzone_data.zone_short_name2, GetShortName());

	LogFile->write(EQEMuLog::Status, "Successfully loaded Zone Config.");
	return true;
}

bool Zone::SaveZoneCFG() {
	return database.SaveZoneCFG(GetZoneID(), GetInstanceVersion(), &newzone_data);
}

void Zone::AddAuth(ServerZoneIncommingClient_Struct* szic) {
	ZoneClientAuth_Struct* zca = new ZoneClientAuth_Struct;
	memset(zca, 0, sizeof(ZoneClientAuth_Struct));
	zca->ip = szic->ip;
	zca->wid = szic->wid;
	zca->accid = szic->accid;
	zca->admin = szic->admin;
	zca->charid = szic->charid;
	zca->tellsoff = szic->tellsoff;
	strn0cpy(zca->charname, szic->charname, sizeof(zca->charname));
	strn0cpy(zca->lskey, szic->lskey, sizeof(zca->lskey));
	zca->stale = false;
	client_auth_list.Insert(zca);
}

void Zone::RemoveAuth(const char* iCharName)
{
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		ZoneClientAuth_Struct* zca = iterator.GetData();
		if (strcasecmp(zca->charname, iCharName) == 0) {
		iterator.RemoveCurrent();
		return;
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

	if(spawn2_timer.Check()) {
		LinkedListIterator<Spawn2*> iterator(spawn2_list);

		Inventory::CleanDirty();

		iterator.Reset();
		while (iterator.MoreElements()) {
			if (iterator.GetData()->Process()) {
				iterator.Advance();
			}
			else {
				iterator.RemoveCurrent();
			}
		}
		if(adv_data && !did_adventure_actions)
		{
			DoAdventureActions();
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
				entity_list.GateAllClients();
				database.DeleteInstance(GetInstanceID());
				Instance_Shutdown_Timer = new Timer(20000); //20 seconds
			}

			if(adv_data == nullptr)
			{
				if(Instance_Warning_timer == nullptr)
				{
					uint32 rem_time = Instance_Timer->GetRemainingTime();
					if(rem_time < 60000 && rem_time > 55000)
					{
						entity_list.ExpeditionWarning(1);
						Instance_Warning_timer = new Timer(10000);
					}
					else if(rem_time < 300000 && rem_time > 295000)
					{
						entity_list.ExpeditionWarning(5);
						Instance_Warning_timer = new Timer(10000);
					}
					else if(rem_time < 900000 && rem_time > 895000)
					{
						entity_list.ExpeditionWarning(15);
						Instance_Warning_timer = new Timer(10000);
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

	if(Weather_Timer->Check()){
		Weather_Timer->Disable();
		this->ChangeWeather();
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

	if(hotzone_timer.Check()) { UpdateHotzone(); }

	return true;
}

void Zone::ChangeWeather()
{
	if(!HasWeather())
	{
		Weather_Timer->Disable();
		return;
	}

	int chance = MakeRandomInt(0, 3);
	uint8 rainchance = zone->newzone_data.rain_chance[chance];
	uint8 rainduration = zone->newzone_data.rain_duration[chance];
	uint8 snowchance = zone->newzone_data.snow_chance[chance];
	uint8 snowduration = zone->newzone_data.snow_duration[chance];
	uint32 weathertimer = 0;
	uint16 tmpweather = MakeRandomInt(0, 100);
	uint8 duration = 0;
	uint8 tmpOldWeather = zone->zone_weather;
	bool changed = false;

	if(tmpOldWeather == 0)
	{
		if(rainchance > 0 || snowchance > 0)
		{
			uint8 intensity = MakeRandomInt(1, 10);
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
			zone->zone_weather = 0;
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
			zone->zone_weather = 0;
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
		LogFile->write(EQEMuLog::Debug, "The next weather check for zone: %s will be in %i seconds.", zone->GetShortName(), Weather_Timer->GetRemainingTime()/1000);
	}
	else
	{
		LogFile->write(EQEMuLog::Debug, "The weather for zone: %s has changed. Old weather was = %i. New weather is = %i The next check will be in %i seconds. Rain chance: %i, Rain duration: %i, Snow chance %i, Snow duration: %i", zone->GetShortName(), tmpOldWeather, zone_weather,Weather_Timer->GetRemainingTime()/1000,rainchance,rainduration,snowchance,snowduration);
		this->weatherSend();
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
		if (set_time == (RuleI(Zone, AutoShutdownDelay)))
		{
			set_time = database.getZoneShutDownDelay(GetZoneID(), GetInstanceVersion());
		}
		autoshutdown_timer.Start(set_time, false);
	}
}

bool Zone::Depop(bool StartSpawnTimer) {
	std::map<uint32,NPCType *>::iterator itr;
	entity_list.Depop(StartSpawnTimer);

#ifdef DEPOP_INVALIDATES_NPC_TYPES_CACHE
	// Refresh npctable, getting current info from database.
	while(npctable.size()) {
		itr=npctable.begin();
		delete itr->second;
		npctable.erase(itr);
	}
#endif

	return true;
}

void Zone::ClearNPCTypeCache(int id) {
	if (id <= 0) {
		auto iter = npctable.begin();
		while (iter != npctable.end()) {
			delete iter->second;
			++iter;
		}
		npctable.clear();
	}
	else {
		auto iter = npctable.begin();
		while (iter != npctable.end()) {
			if (iter->first == (uint32)id) {
				delete iter->second;
				npctable.erase(iter);
				return;
			}
			++iter;
		}
	}
}

void Zone::Repop(uint32 delay) {

	if(!Depop())
		return;

	LinkedListIterator<Spawn2*> iterator(spawn2_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		iterator.RemoveCurrent();
	}

	quest_manager.ClearAllTimers();

	if (!database.PopulateZoneSpawnList(zoneid, spawn2_list, GetInstanceVersion(), delay))
		LogFile->write(EQEMuLog::Debug, "Error in Zone::Repop: database.PopulateZoneSpawnList failed");

	initgrids_timer.Start();

	//MODDING HOOK FOR REPOP
	mod_repop();
}

void Zone::GetTimeSync()
{
	if (worldserver.Connected() && !gottime) {
		ServerPacket* pack = new ServerPacket(ServerOP_GetWorldTime, 0);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Zone::SetDate(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute)
{
	if (worldserver.Connected()) {
		ServerPacket* pack = new ServerPacket(ServerOP_SetWorldTime, sizeof(eqTimeOfDay));
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

void Zone::SetTime(uint8 hour, uint8 minute)
{
	if (worldserver.Connected()) {
		ServerPacket* pack = new ServerPacket(ServerOP_SetWorldTime, sizeof(eqTimeOfDay));
		eqTimeOfDay* eqtod = (eqTimeOfDay*)pack->pBuffer;
		zone_time.getEQTimeOfDay(time(0), &eqtod->start_eqtime);
		eqtod->start_eqtime.minute=minute;
		eqtod->start_eqtime.hour=hour;
		eqtod->start_realtime=time(0);
		printf("Setting master time on world server to: %d:%d (%d)\n", hour, minute, (int)eqtod->start_realtime);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

ZonePoint* Zone::GetClosestZonePoint(float x, float y, float z, uint32 to, Client* client, float max_distance) {
	LinkedListIterator<ZonePoint*> iterator(zone_point_list);
	ZonePoint* closest_zp = 0;
	float closest_dist = FLT_MAX;
	float max_distance2 = max_distance * max_distance;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* zp = iterator.GetData();
		uint32 mask_test = client->GetClientVersionBit();
		if(!(zp->client_version_mask & mask_test))
		{
			iterator.Advance();
			continue;
		}

		if (zp->target_zone_id == to)
		{
			float delta_x = zp->x - x;
			float delta_y = zp->y - y;
			if(zp->x == 999999 || zp->x == -999999)
				delta_x = 0;
			if(zp->y == 999999 || zp->y == -999999)
				delta_y = 0;

			float dist = sqrt(delta_x * delta_x + delta_y * delta_y);
			if (dist < closest_dist)
			{
				closest_zp = zp;
				closest_dist = dist;
			}
		}
		iterator.Advance();
	}

	if(closest_dist > 400.0f && closest_dist < max_distance2)
	{
		if(client)
			client->CheatDetected(MQZoneUnknownDest, x, y, z); // Someone is trying to use /zone
		LogFile->write(EQEMuLog::Status, "WARNING: Closest zone point for zone id %d is %f, you might need to update your zone_points table if you dont arrive at the right spot.", to, closest_dist);
		LogFile->write(EQEMuLog::Status, "<Real Zone Points>. %f x %f y %f z ", x, y, z);
	}

	if(closest_dist > max_distance2)
		closest_zp = nullptr;

	if(!closest_zp)
		closest_zp = GetClosestZonePointWithoutZone(x, y, z, client);

	return closest_zp;
}

ZonePoint* Zone::GetClosestZonePoint(float x, float y, float z, const char* to_name, Client* client, float max_distance) {
	if(to_name == nullptr)
		return GetClosestZonePointWithoutZone(x,y,z, client, max_distance);
	return GetClosestZonePoint(x, y, z, database.GetZoneID(to_name), client, max_distance);
}

ZonePoint* Zone::GetClosestZonePointWithoutZone(float x, float y, float z, Client* client, float max_distance) {
	LinkedListIterator<ZonePoint*> iterator(zone_point_list);
	ZonePoint* closest_zp = 0;
	float closest_dist = FLT_MAX;
	float max_distance2 = max_distance*max_distance;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* zp = iterator.GetData();
		uint32 mask_test = client->GetClientVersionBit();

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

bool ZoneDatabase::LoadStaticZonePoints(LinkedList<ZonePoint*>* zone_point_list, const char* zonename, uint32 version)
{

	zone_point_list->Clear();
	zone->numzonepoints = 0;
	std::string query = StringFormat("SELECT x, y, z, target_x, target_y, "
                                    "target_z, target_zone_id, heading, target_heading, "
                                    "number, target_instance, client_version_mask "
                                    "FROM zone_points WHERE zone='%s' AND (version=%i OR version=-1) "
                                    "ORDER BY number", zonename, version);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        std::cerr << "Error1 in LoadStaticZonePoints query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

    for (auto row = results.begin(); row != results.end(); ++row) {
        ZonePoint* zp = new ZonePoint;

        zp->x = atof(row[0]);
        zp->y = atof(row[1]);
        zp->z = atof(row[2]);
        zp->target_x = atof(row[3]);
        zp->target_y = atof(row[4]);
        zp->target_z = atof(row[5]);
        zp->target_zone_id = atoi(row[6]);
        zp->heading = atof(row[7]);
        zp->target_heading = atof(row[8]);
        zp->number = atoi(row[9]);
        zp->target_zone_instance = atoi(row[10]);
        zp->client_version_mask = (uint32)strtoul(row[11], nullptr, 0);

        zone_point_list->Insert(zp);

        zone->numzonepoints++;
    }

	return true;
}

void Zone::SpawnStatus(Mob* client) {
	LinkedListIterator<Spawn2*> iterator(spawn2_list);

	uint32 x = 0;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->timer.GetRemainingTime() == 0xFFFFFFFF)
			client->Message(0, "  %d: %1.1f, %1.1f, %1.1f: disabled", iterator.GetData()->GetID(), iterator.GetData()->GetX(), iterator.GetData()->GetY(), iterator.GetData()->GetZ());
		else
			client->Message(0, "  %d: %1.1f, %1.1f, %1.1f: %1.2f", iterator.GetData()->GetID(), iterator.GetData()->GetX(), iterator.GetData()->GetY(), iterator.GetData()->GetZ(), (float)iterator.GetData()->timer.GetRemainingTime() / 1000);

		x++;
		iterator.Advance();
	}
	client->Message(0, "%i spawns listed.", x);
}

void Zone::ShowEnabledSpawnStatus(Mob* client)
{
	LinkedListIterator<Spawn2*> iterator(spawn2_list);
	int x = 0;
	int iEnabledCount = 0;

	iterator.Reset();

	while(iterator.MoreElements())
	{
		if (iterator.GetData()->timer.GetRemainingTime() != 0xFFFFFFFF)
		{
			client->Message(0, "  %d: %1.1f, %1.1f, %1.1f: %1.2f", iterator.GetData()->GetID(), iterator.GetData()->GetX(), iterator.GetData()->GetY(), iterator.GetData()->GetZ(), (float)iterator.GetData()->timer.GetRemainingTime() / 1000);
			iEnabledCount++;
		}

		x++;
		iterator.Advance();
	}

	client->Message(0, "%i of %i spawns listed.", iEnabledCount, x);
}

void Zone::ShowDisabledSpawnStatus(Mob* client)
{
	LinkedListIterator<Spawn2*> iterator(spawn2_list);
	int x = 0;
	int iDisabledCount = 0;

	iterator.Reset();

	while(iterator.MoreElements())
	{
		if (iterator.GetData()->timer.GetRemainingTime() == 0xFFFFFFFF)
		{
			client->Message(0, "  %d: %1.1f, %1.1f, %1.1f: disabled", iterator.GetData()->GetID(), iterator.GetData()->GetX(), iterator.GetData()->GetY(), iterator.GetData()->GetZ());
			iDisabledCount++;
		}

		x++;
		iterator.Advance();
	}

	client->Message(0, "%i of %i spawns listed.", iDisabledCount, x);
}

void Zone::ShowSpawnStatusByID(Mob* client, uint32 spawnid)
{
	LinkedListIterator<Spawn2*> iterator(spawn2_list);
	int x = 0;
	int iSpawnIDCount = 0;

	iterator.Reset();

	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == spawnid)
		{
			if (iterator.GetData()->timer.GetRemainingTime() == 0xFFFFFFFF)
				client->Message(0, "  %d: %1.1f, %1.1f, %1.1f: disabled", iterator.GetData()->GetID(), iterator.GetData()->GetX(), iterator.GetData()->GetY(), iterator.GetData()->GetZ());
			else
				client->Message(0, "  %d: %1.1f, %1.1f, %1.1f: %1.2f", iterator.GetData()->GetID(), iterator.GetData()->GetX(), iterator.GetData()->GetY(), iterator.GetData()->GetZ(), (float)iterator.GetData()->timer.GetRemainingTime() / 1000);

			iSpawnIDCount++;

			break;
		}

		x++;
		iterator.Advance();
	}

	if(iSpawnIDCount > 0)
		client->Message(0, "%i of %i spawns listed.", iSpawnIDCount, x);
	else
		client->Message(0, "No matching spawn id was found in this zone.");
}


bool Zone::RemoveSpawnEntry(uint32 spawnid)
{
	LinkedListIterator<Spawn2*> iterator(spawn2_list);


	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID() == spawnid)
		{
			iterator.RemoveCurrent();
			return true;
		}
		else
		iterator.Advance();
	}
return false;
}

bool Zone::RemoveSpawnGroup(uint32 in_id) {
	if(spawn_group_list.RemoveSpawnGroup(in_id))
		return true;
	else
		return false;
}


// Added By Hogie
bool ZoneDatabase::GetDecayTimes(npcDecayTimes_Struct* npcCorpseDecayTimes) {

	const std::string query = "SELECT varname, value FROM variables WHERE varname LIKE 'decaytime%%' ORDER BY varname";
	auto results = QueryDatabase(query);
	if (!results.Success())
        return false;

	int index = 0;
    for (auto row = results.begin(); row != results.end(); ++row, ++index) {
        Seperator sep(row[0]);
        npcCorpseDecayTimes[index].minlvl = atoi(sep.arg[1]);
        npcCorpseDecayTimes[index].maxlvl = atoi(sep.arg[2]);

        if (atoi(row[1]) > 7200)
            npcCorpseDecayTimes[index].seconds = 720;
        else
            npcCorpseDecayTimes[index].seconds = atoi(row[1]);
    }

	return true;
}

void Zone::weatherSend()
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Weather, 8);
	if(zone_weather>0)
		outapp->pBuffer[0] = zone_weather-1;
	if(zone_weather>0)
		outapp->pBuffer[4] = zone->weather_intensity;
	entity_list.QueueClients(0, outapp);
	safe_delete(outapp);
}

bool Zone::HasGraveyard() {
	bool Result = false;

	if(graveyard_zoneid() > 0)
		Result = true;

	return Result;
}

void Zone::SetGraveyard(uint32 zoneid, uint32 x, uint32 y, uint32 z, uint32 heading) {
	pgraveyard_zoneid = zoneid;
	pgraveyard_x = x;
	pgraveyard_y = y;
	pgraveyard_z = z;
	pgraveyard_heading = heading;
}

void Zone::LoadBlockedSpells(uint32 zoneid)
{
	if(!blocked_spells)
	{
		totalBS = database.GetBlockedSpellsCount(zoneid);
		if(totalBS > 0){
			blocked_spells = new ZoneSpellsBlocked[totalBS];
			if(!database.LoadBlockedSpells(totalBS, blocked_spells, zoneid))
			{
				LogFile->write(EQEMuLog::Error, "... Failed to load blocked spells.");
				ClearBlockedSpells();
			}
		}
	}
}

void Zone::ClearBlockedSpells()
{
	if(blocked_spells){
		safe_delete_array(blocked_spells);
		totalBS = 0;
	}
}

bool Zone::IsSpellBlocked(uint32 spell_id, float nx, float ny, float nz)
{
	if (blocked_spells)
	{
		bool exception = false;
		bool block_all = false;
		for (int x = 0; x < totalBS; x++)
		{
			if (blocked_spells[x].spellid == spell_id)
			{
				exception = true;
			}

			if (blocked_spells[x].spellid == 0)
			{
				block_all = true;
			}
		}

		for (int x = 0; x < totalBS; x++)
		{
			// If spellid is 0, block all spells in the zone
			if (block_all)
			{
				// If the same zone has entries other than spellid 0, they act as exceptions and are allowed
				if (exception)
				{
					return false;
				}
				else
				{
					return true;
				}
			}
			else
			{
				if (spell_id != blocked_spells[x].spellid)
				{
					continue;
				}

				switch (blocked_spells[x].type)
				{
					case 1:
					{
						return true;
						break;
					}
					case 2:
					{
						if ((( nx >= (blocked_spells[x].x-blocked_spells[x].xdiff)) && (nx <= (blocked_spells[x].x+blocked_spells[x].xdiff))) &&
							(( ny >= (blocked_spells[x].y-blocked_spells[x].ydiff)) && (ny <= (blocked_spells[x].y+blocked_spells[x].ydiff))) &&
							(( nz >= (blocked_spells[x].z-blocked_spells[x].zdiff)) && (nz <= (blocked_spells[x].z+blocked_spells[x].zdiff))))
						{
							return true;
						}
						break;
					}
					default:
					{
						continue;
						break;
					}
				}
			}
		}
	}
	return false;
}

const char* Zone::GetSpellBlockedMessage(uint32 spell_id, float nx, float ny, float nz)
{
	if(blocked_spells)
	{
		for(int x = 0; x < totalBS; x++)
		{
			if(spell_id != blocked_spells[x].spellid && blocked_spells[x].spellid != 0)
				continue;

			switch(blocked_spells[x].type)
			{
				case 1:
				{
					return blocked_spells[x].message;
					break;
				}
				case 2:
				{
					if((( nx > (blocked_spells[x].x-blocked_spells[x].xdiff)) && (nx < (blocked_spells[x].x+blocked_spells[x].xdiff))) &&
						(( ny > (blocked_spells[x].y-blocked_spells[x].ydiff)) && (ny < (blocked_spells[x].y+blocked_spells[x].ydiff))) &&
						(( nz > (blocked_spells[x].z-blocked_spells[x].zdiff)) && (nz < (blocked_spells[x].z+blocked_spells[x].zdiff))))
					{
						return blocked_spells[x].message;
					}
					break;
				}
				default:
				{
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
	const std::string query = "SELECT id, type, spell_id, skill, locked FROM ldon_trap_templates";
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in Zone::LoadLDoNTraps: %s (%s)", query.c_str(), results.ErrorMessage().c_str());
		return;
    }

    for (auto row = results.begin();row != results.end(); ++row) {
        LDoNTrapTemplate *lt = new LDoNTrapTemplate;
        lt->id = atoi(row[0]);
        lt->type = (LDoNChestTypes)atoi(row[1]);
        lt->spell_id = atoi(row[2]);
        lt->skill = atoi(row[3]);
        lt->locked = atoi(row[4]);
        ldon_trap_list[lt->id] = lt;
    }

}

void Zone::LoadLDoNTrapEntries()
{
	const std::string query = "SELECT id, trap_id FROM ldon_trap_entries";
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in Zone::LoadLDoNTrapEntries: %s (%s)", query.c_str(), results.ErrorMessage().c_str());
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
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in Zone::LoadVeteranRewards: %s (%s)", query.c_str(), results.ErrorMessage().c_str());
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
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in Zone::LoadAlternateCurrencies: %s (%s)", query.c_str(), results.ErrorMessage().c_str());
		return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
    {
        current_currency.id = atoi(row[0]);
        current_currency.item_id = atoi(row[1]);
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
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in Zone::LoadAdventureFlavor: %s (%s)", query.c_str(), results.ErrorMessage().c_str());
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
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureCountUpdate, sizeof(uint16));
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
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureAssaCountUpdate, sizeof(uint16));
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
			const NPCType* tmp = database.GetNPCType(ds->data_id);
			if(tmp)
			{
				NPC* npc = new NPC(tmp, 0, ds->assa_x, ds->assa_y, ds->assa_z, ds->assa_h, FlyMode3);
				npc->AddLootTable();
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
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in Zone::LoadNPCEmotes: %s (%s)", query.c_str(), results.ErrorMessage().c_str());
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
    {
        NPC_Emote_Struct* nes = new NPC_Emote_Struct;
        nes->emoteid = atoi(row[0]);
        nes->event_ = atoi(row[1]);
        nes->type = atoi(row[2]);
        strn0cpy(nes->text, row[3], sizeof(nes->text));
        NPCEmoteList->Insert(nes);
    }

}

void Zone::ReloadWorld(uint32 Option){
	if(Option == 1){
		zone->Repop(0);
		entity_list.ClearAreas();
		parse->ReloadQuests();
	}
}

void Zone::LoadTickItems()
{
	tick_items.clear();

    const std::string query = "SELECT it_itemid, it_chance, it_level, it_qglobal, it_bagslot FROM item_tick";
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in Zone::LoadTickItems: %s (%s)", query.c_str(), results.ErrorMessage().c_str());
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

void Zone::UpdateHotzone()
{
    std::string query = StringFormat("SELECT hotzone FROM zone WHERE short_name = '%s'", GetShortName());
    auto results = database.QueryDatabase(query);
    if (!results.Success())
        return;

    if (results.RowCount() == 0)
        return;

    auto row = results.begin();

    is_hotzone = atoi(row[0]) == 0 ? false: true;
}
