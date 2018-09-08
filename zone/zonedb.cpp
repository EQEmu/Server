
#include "../common/eqemu_logsys.h"
#include "../common/extprofile.h"
#include "../common/item_instance.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"

#include "client.h"
#include "corpse.h"
#include "groups.h"
#include "merc.h"
#include "zone.h"
#include "zonedb.h"
#include "aura.h"

#include <ctime>
#include <iostream>

extern Zone* zone;

ZoneDatabase database;

ZoneDatabase::ZoneDatabase()
: SharedDatabase()
{
	ZDBInitVars();
}

ZoneDatabase::ZoneDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
: SharedDatabase(host, user, passwd, database, port)
{
	ZDBInitVars();
}

void ZoneDatabase::ZDBInitVars() {
	memset(door_isopen_array, 0, sizeof(door_isopen_array));
	npc_spellseffects_cache = 0;
	npc_spellseffects_loadtried = 0;
	max_faction = 0;
	faction_array = nullptr;
}

ZoneDatabase::~ZoneDatabase() {
	if (npc_spellseffects_cache) {
		for (int x = 0; x <= npc_spellseffects_maxid; x++) {
			safe_delete_array(npc_spellseffects_cache[x]);
		}
		safe_delete_array(npc_spellseffects_cache);
	}
	safe_delete_array(npc_spellseffects_loadtried);

	if (faction_array != nullptr) {
		for (int x = 0; x <= max_faction; x++) {
			if (faction_array[x] != 0)
				safe_delete(faction_array[x]);
		}
		safe_delete_array(faction_array);
	}
}

bool ZoneDatabase::SaveZoneCFG(uint32 zoneid, uint16 instance_id, NewZone_Struct* zd) {

	std::string query = StringFormat("UPDATE zone SET underworld = %f, minclip = %f, "
                                    "maxclip = %f, fog_minclip = %f, fog_maxclip = %f, "
                                    "fog_blue = %i, fog_red = %i, fog_green = %i, "
                                    "sky = %i, ztype = %i, zone_exp_multiplier = %f, "
                                    "safe_x = %f, safe_y = %f, safe_z = %f "
                                    "WHERE zoneidnumber = %i AND version = %i",
                                    zd->underworld, zd->minclip,
                                    zd->maxclip, zd->fog_minclip[0], zd->fog_maxclip[0],
                                    zd->fog_blue[0], zd->fog_red[0], zd->fog_green[0],
                                    zd->sky, zd->ztype, zd->zone_exp_multiplier,
                                    zd->safe_x, zd->safe_y, zd->safe_z,
                                    zoneid, instance_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        return false;
	}

	return true;
}

bool ZoneDatabase::GetZoneCFG(uint32 zoneid, uint16 instance_id, NewZone_Struct *zone_data, bool &can_bind, bool &can_combat, bool &can_levitate, bool &can_castoutdoor, bool &is_city, bool &is_hotzone, bool &allow_mercs, uint8 &zone_type, int &ruleset, char **map_filename) {

	*map_filename = new char[100];
	zone_data->zone_id = zoneid;

	std::string query = StringFormat(
		"SELECT "
		"ztype, "					 // 0
		"fog_red, "					 // 1
		"fog_green, "				 // 2
		"fog_blue, "				 // 3
		"fog_minclip, "				 // 4
		"fog_maxclip, "				 // 5
		"fog_red2, "				 // 6
		"fog_green2, "				 // 7
		"fog_blue2, "				 // 8
		"fog_minclip2, "			 // 9
		"fog_maxclip2, "			 // 10
		"fog_red3, "				 // 11
		"fog_green3, "				 // 12
		"fog_blue3, "				 // 13
		"fog_minclip3, "			 // 14
		"fog_maxclip3, "			 // 15
		"fog_red4, "				 // 16
		"fog_green4, "				 // 17
		"fog_blue4, "				 // 18
		"fog_minclip4, "			 // 19
		"fog_maxclip4, "			 // 20
		"fog_density, "				 // 21
		"sky, "						 // 22
		"zone_exp_multiplier, "		 // 23
		"safe_x, "					 // 24
		"safe_y, "					 // 25
		"safe_z, "					 // 26
		"underworld, "				 // 27
		"minclip, "					 // 28
		"maxclip, "					 // 29
		"time_type, "				 // 30
		"canbind, "					 // 31
		"cancombat, "				 // 32
		"canlevitate, "				 // 33
		"castoutdoor, "				 // 34
		"hotzone, "					 // 35
		"ruleset, "					 // 36
		"suspendbuffs, "			 // 37
		"map_file_name, "			 // 38
		"short_name, "				 // 39
		"rain_chance1, "			 // 40
		"rain_chance2, "			 // 41
		"rain_chance3, "			 // 42
		"rain_chance4, "			 // 43
		"rain_duration1, "			 // 44
		"rain_duration2, "			 // 45
		"rain_duration3, "			 // 46
		"rain_duration4, "			 // 47
		"snow_chance1, "			 // 48
		"snow_chance2, "			 // 49
		"snow_chance3, "			 // 50
		"snow_chance4, "			 // 51
		"snow_duration1, "			 // 52
		"snow_duration2, "			 // 53
		"snow_duration3, "			 // 54
		"snow_duration4, "			 // 55
		"gravity, "					 // 56
		"fast_regen_hp, "			 // 57
		"fast_regen_mana, "			 // 58
		"fast_regen_endurance, "	 // 59
		"npc_max_aggro_dist "		 // 60
		"FROM zone WHERE zoneidnumber = %i AND version = %i",
		zoneid, instance_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		strcpy(*map_filename, "default");
		return false;
	}

	if (results.RowCount() == 0) {
		strcpy(*map_filename, "default");
		return false;
	}

	auto row = results.begin();

	memset(zone_data, 0, sizeof(NewZone_Struct));
	zone_data->ztype = atoi(row[0]);
	zone_type = zone_data->ztype;

	int index;
	for (index = 0; index < 4; index++) {
		zone_data->fog_red[index] = atoi(row[1 + index * 5]);
		zone_data->fog_green[index] = atoi(row[2 + index * 5]);
		zone_data->fog_blue[index] = atoi(row[3 + index * 5]);
		zone_data->fog_minclip[index] = atof(row[4 + index * 5]);
		zone_data->fog_maxclip[index] = atof(row[5 + index * 5]);
	}

	zone_data->fog_density = atof(row[21]);
	zone_data->sky = atoi(row[22]);
	zone_data->zone_exp_multiplier = atof(row[23]);
	zone_data->safe_x = atof(row[24]);
	zone_data->safe_y = atof(row[25]);
	zone_data->safe_z = atof(row[26]);
	zone_data->underworld = atof(row[27]);
	zone_data->minclip = atof(row[28]);
	zone_data->maxclip = atof(row[29]);
	zone_data->time_type = atoi(row[30]);

	//not in the DB yet:
	zone_data->gravity = atof(row[56]);
	Log(Logs::General, Logs::Debug, "Zone Gravity is %f", zone_data->gravity);
	allow_mercs = true;

	zone_data->FastRegenHP = atoi(row[57]);
	zone_data->FastRegenMana = atoi(row[58]);
	zone_data->FastRegenEndurance = atoi(row[59]);
	zone_data->NPCAggroMaxDist = atoi(row[60]);

	int bindable = 0;
	bindable = atoi(row[31]);

	can_bind = bindable == 0 ? false : true;
	is_city = bindable == 2 ? true : false;
	can_combat = atoi(row[32]) == 0 ? false : true;
	can_levitate = atoi(row[33]) == 0 ? false : true;
	can_castoutdoor = atoi(row[34]) == 0 ? false : true;
	is_hotzone = atoi(row[35]) == 0 ? false : true;


	ruleset = atoi(row[36]);
	zone_data->SuspendBuffs = atoi(row[37]);

	char *file = row[38];
	if (file)
		strcpy(*map_filename, file);
	else
		strcpy(*map_filename, row[39]);

	for (index = 0; index < 4; index++)
		zone_data->rain_chance[index] = atoi(row[40 + index]);

	for (index = 0; index < 4; index++)
		zone_data->rain_duration[index] = atoi(row[44 + index]);

	for (index = 0; index < 4; index++)
		zone_data->snow_chance[index] = atoi(row[48 + index]);

	for (index = 0; index < 4; index++)
		zone_data->snow_duration[index] = atof(row[52 + index]);

	return true;
}

void ZoneDatabase::UpdateRespawnTime(uint32 spawn2_id, uint16 instance_id, uint32 time_left)
{

	timeval tv;
	gettimeofday(&tv, nullptr);
	uint32 current_time = tv.tv_sec;

	/*	If we pass timeleft as 0 that means we clear from respawn time
			otherwise we update with a REPLACE INTO
	*/

	if(time_left == 0) {
        std::string query = StringFormat("DELETE FROM `respawn_times` WHERE `id` = %u AND `instance_id` = %u", spawn2_id, instance_id);
        QueryDatabase(query);
		return;
	}

    std::string query = StringFormat(
		"REPLACE INTO `respawn_times` "
		"(id, "
		"start, "
		"duration, "
		"instance_id) "
		"VALUES "
		"(%u, "
		"%u, "
		"%u, "
		"%u)",
		spawn2_id,
		current_time,
		time_left,
		instance_id
	);
    QueryDatabase(query);

	return;
}

//Gets the respawn time left in the database for the current spawn id
uint32 ZoneDatabase::GetSpawnTimeLeft(uint32 id, uint16 instance_id)
{
	std::string query = StringFormat("SELECT start, duration FROM respawn_times "
                                    "WHERE id = %lu AND instance_id = %lu",
                                    (unsigned long)id, (unsigned long)zone->GetInstanceID());
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return 0;
    }

    if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();

    timeval tv;
    gettimeofday(&tv, nullptr);
    uint32 resStart = atoi(row[0]);
    uint32 resDuration = atoi(row[1]);

    //compare our values to current time
    if((resStart + resDuration) <= tv.tv_sec) {
        //our current time was expired
        return 0;
    }

    //we still have time left on this timer
    return ((resStart + resDuration) - tv.tv_sec);

}

void ZoneDatabase::UpdateSpawn2Status(uint32 id, uint8 new_status)
{
	std::string query = StringFormat("UPDATE spawn2 SET enabled = %i WHERE id = %lu", new_status, (unsigned long)id);
	QueryDatabase(query);
}

bool ZoneDatabase::logevents(const char* accountname,uint32 accountid,uint8 status,const char* charname, const char* target,const char* descriptiontype, const char* description,int event_nid){

	uint32 len = strlen(description);
	uint32 len2 = strlen(target);
	auto descriptiontext = new char[2 * len + 1];
	auto targetarr = new char[2 * len2 + 1];
	memset(descriptiontext, 0, 2*len+1);
	memset(targetarr, 0, 2*len2+1);
	DoEscapeString(descriptiontext, description, len);
	DoEscapeString(targetarr, target, len2);

	std::string query = StringFormat("INSERT INTO eventlog (accountname, accountid, status, "
                                    "charname, target, descriptiontype, description, event_nid) "
                                    "VALUES('%s', %i, %i, '%s', '%s', '%s', '%s', '%i')",
                                    accountname, accountid, status, charname, targetarr,
                                    descriptiontype, descriptiontext, event_nid);
    safe_delete_array(descriptiontext);
	safe_delete_array(targetarr);
	auto results = QueryDatabase(query);
	if (!results.Success())	{
		return false;
	}

	return true;
}

void ZoneDatabase::RegisterBug(BugReport_Struct* bug_report) {
	if (!bug_report)
		return;

	size_t len = 0;
	char* name_ = nullptr;
	char* ui_ = nullptr;
	char* type_ = nullptr;
	char* target_ = nullptr;
	char* bug_ = nullptr;
	
	len = strlen(bug_report->reporter_name);
	if (len) {
		if (len > 63) // check against db column size
			len = 63;
		name_ = new char[(2 * len + 1)];
		memset(name_, 0, (2 * len + 1));
		DoEscapeString(name_, bug_report->reporter_name, len);
	}

	len = strlen(bug_report->ui_path);
	if (len) {
		if (len > 127)
			len = 127;
		ui_ = new char[(2 * len + 1)];
		memset(ui_, 0, (2 * len + 1));
		DoEscapeString(ui_, bug_report->ui_path, len);
	}

	len = strlen(bug_report->category_name);
	if (len) {
		if (len > 63)
			len = 63;
		type_ = new char[(2 * len + 1)];
		memset(type_, 0, (2 * len + 1));
		DoEscapeString(type_, bug_report->category_name, len);
	}

	len = strlen(bug_report->target_name);
	if (len) {
		if (len > 63)
			len = 63;
		target_ = new char[(2 * len + 1)];
		memset(target_, 0, (2 * len + 1));
		DoEscapeString(target_, bug_report->target_name, len);
	}

	len = strlen(bug_report->bug_report);
	if (len) {
		if (len > 1023)
			len = 1023;
		bug_ = new char[(2 * len + 1)];
		memset(bug_, 0, (2 * len + 1));
		DoEscapeString(bug_, bug_report->bug_report, len);
	}

	//x and y are intentionally swapped because eq is inversexy coords //is this msg out-of-date or are the parameters wrong?
	std::string query = StringFormat(
		"INSERT INTO `bugs` (`zone`, `name`, `ui`, `x`, `y`, `z`, `type`, `flag`, `target`, `bug`, `date`) "
		"VALUES('%s', '%s', '%s', '%.2f', '%.2f', '%.2f', '%s', %d, '%s', '%s', CURDATE())",
		zone->GetShortName(),
		(name_ ? name_ : ""),
		(ui_ ? ui_ : ""),
		bug_report->pos_x,
		bug_report->pos_y,
		bug_report->pos_z,
		(type_ ? type_ : ""),
		bug_report->optional_info_mask,
		(target_ ? target_ : "Unknown Target"),
		(bug_ ? bug_ : "")
	);
	safe_delete_array(name_);
	safe_delete_array(ui_);
	safe_delete_array(type_);
	safe_delete_array(target_);
	safe_delete_array(bug_);
	
	QueryDatabase(query);
}

void ZoneDatabase::RegisterBug(Client* client, BugReport_Struct* bug_report) {
	if (!client || !bug_report)
		return;

	size_t len = 0;
	char* category_name_ = nullptr;
	char* reporter_name_ = nullptr;
	char* ui_path_ = nullptr;
	char* target_name_ = nullptr;
	char* bug_report_ = nullptr;
	char* system_info_ = nullptr;

	len = strlen(bug_report->category_name);
	if (len) {
		if (len > 63) // check against db column size
			len = 63;
		category_name_ = new char[(2 * len + 1)];
		memset(category_name_, 0, (2 * len + 1));
		DoEscapeString(category_name_, bug_report->category_name, len);
	}

	len = strlen(bug_report->reporter_name);
	if (len) {
		if (len > 63)
			len = 63;
		reporter_name_ = new char[(2 * len + 1)];
		memset(reporter_name_, 0, (2 * len + 1));
		DoEscapeString(reporter_name_, bug_report->reporter_name, len);
	}

	len = strlen(bug_report->ui_path);
	if (len) {
		if (len > 127)
			len = 127;
		ui_path_ = new char[(2 * len + 1)];
		memset(ui_path_, 0, (2 * len + 1));
		DoEscapeString(ui_path_, bug_report->ui_path, len);
	}

	len = strlen(bug_report->target_name);
	if (len) {
		if (len > 63)
			len = 63;
		target_name_ = new char[(2 * len + 1)];
		memset(target_name_, 0, (2 * len + 1));
		DoEscapeString(target_name_, bug_report->target_name, len);
	}

	len = strlen(bug_report->bug_report);
	if (len) {
		if (len > 1023)
			len = 1023;
		bug_report_ = new char[(2 * len + 1)];
		memset(bug_report_, 0, (2 * len + 1));
		DoEscapeString(bug_report_, bug_report->bug_report, len);
	}

	len = strlen(bug_report->system_info);
	if (len) {
		if (len > 1023)
			len = 1023;
		system_info_ = new char[(2 * len + 1)];
		memset(system_info_, 0, (2 * len + 1));
		DoEscapeString(system_info_, bug_report->system_info, len);
	}

	std::string query = StringFormat(
		"INSERT INTO `bug_reports` "
		"(`zone`,"
		" `client_version_id`,"
		" `client_version_name`,"
		" `account_id`,"
		" `character_id`,"
		" `character_name`,"
		" `reporter_spoof`,"
		" `category_id`,"
		" `category_name`,"
		" `reporter_name`,"
		" `ui_path`,"
		" `pos_x`,"
		" `pos_y`,"
		" `pos_z`,"
		" `heading`,"
		" `time_played`,"
		" `target_id`,"
		" `target_name`,"
		" `optional_info_mask`,"
		" `_can_duplicate`,"
		" `_crash_bug`,"
		" `_target_info`,"
		" `_character_flags`,"
		" `_unknown_value`,"
		" `bug_report`,"
		" `system_info`) "
		"VALUES "
		"('%s',"
		" '%u',"
		" '%s',"
		" '%u',"
		" '%u',"
		" '%s',"
		" '%u',"
		" '%u',"
		" '%s',"
		" '%s',"
		" '%s',"
		" '%1.1f',"
		" '%1.1f',"
		" '%1.1f',"
		" '%u',"
		" '%u',"
		" '%u',"
		" '%s',"
		" '%u',"
		" '%u',"
		" '%u',"
		" '%u',"
		" '%u',"
		" '%u',"
		" '%s',"
		" '%s')",
		zone->GetShortName(),
		client->ClientVersion(),
		EQEmu::versions::ClientVersionName(client->ClientVersion()),
		client->AccountID(),
		client->CharacterID(),
		client->GetName(),
		(strcmp(client->GetName(), reporter_name_) != 0 ? 1 : 0),
		bug_report->category_id,
		(category_name_ ? category_name_ : ""),
		(reporter_name_ ? reporter_name_ : ""),
		(ui_path_ ? ui_path_ : ""),
		bug_report->pos_x,
		bug_report->pos_y,
		bug_report->pos_z,
		bug_report->heading,
		bug_report->time_played,
		bug_report->target_id,
		(target_name_ ? target_name_ : ""),
		bug_report->optional_info_mask,
		((bug_report->optional_info_mask & EQEmu::bug::infoCanDuplicate) != 0 ? 1 : 0),
		((bug_report->optional_info_mask & EQEmu::bug::infoCrashBug) != 0 ? 1 : 0),
		((bug_report->optional_info_mask & EQEmu::bug::infoTargetInfo) != 0 ? 1 : 0),
		((bug_report->optional_info_mask & EQEmu::bug::infoCharacterFlags) != 0 ? 1 : 0),
		((bug_report->optional_info_mask & EQEmu::bug::infoUnknownValue) != 0 ? 1 : 0),
		(bug_report_ ? bug_report_ : ""),
		(system_info_ ? system_info_ : "")
	);
	safe_delete_array(category_name_);
	safe_delete_array(reporter_name_);
	safe_delete_array(ui_path_);
	safe_delete_array(target_name_);
	safe_delete_array(bug_report_);
	safe_delete_array(system_info_);
	
	auto result = QueryDatabase(query);

	// TODO: Entity dumping [RuleB(Bugs, DumpTargetEntity)]
}

//void ZoneDatabase::UpdateBug(PetitionBug_Struct* bug) {
//
//	uint32 len = strlen(bug->text);
//	auto bugtext = new char[2 * len + 1];
//	memset(bugtext, 0, 2 * len + 1);
//	DoEscapeString(bugtext, bug->text, len);
//
//	std::string query = StringFormat("INSERT INTO bugs (type, name, bugtext, flag) "
//		"VALUES('%s', '%s', '%s', %i)",
//		"Petition", bug->name, bugtext, 25);
//	safe_delete_array(bugtext);
//	QueryDatabase(query);
//}

bool ZoneDatabase::SetSpecialAttkFlag(uint8 id, const char* flag) {

	std::string query = StringFormat("UPDATE npc_types SET npcspecialattks='%s' WHERE id = %i;", flag, id);
    auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return results.RowsAffected() != 0;
}

bool ZoneDatabase::DoorIsOpen(uint8 door_id,const char* zone_name)
{
	if(door_isopen_array[door_id] == 0) {
		SetDoorPlace(1,door_id,zone_name);
		return false;
	}
	else {
		SetDoorPlace(0,door_id,zone_name);
		return true;
	}
}

void ZoneDatabase::SetDoorPlace(uint8 value,uint8 door_id,const char* zone_name)
{
	door_isopen_array[door_id] = value;
}

void ZoneDatabase::GetEventLogs(const char* name,char* target,uint32 account_id,uint8 eventid,char* detail,char* timestamp, CharacterEventLog_Struct* cel)
{
	char modifications[200];
	if(strlen(name) != 0)
		sprintf(modifications,"charname=\'%s\'",name);
	else if(account_id != 0)
		sprintf(modifications,"accountid=%i",account_id);

	if(strlen(target) != 0)
		sprintf(modifications,"%s AND target LIKE \'%%%s%%\'",modifications,target);

	if(strlen(detail) != 0)
		sprintf(modifications,"%s AND description LIKE \'%%%s%%\'",modifications,detail);

	if(strlen(timestamp) != 0)
		sprintf(modifications,"%s AND time LIKE \'%%%s%%\'",modifications,timestamp);

	if(eventid == 0)
		eventid =1;
	sprintf(modifications,"%s AND event_nid=%i",modifications,eventid);

    std::string query = StringFormat("SELECT id, accountname, accountid, status, charname, target, "
                                    "time, descriptiontype, description FROM eventlog WHERE %s", modifications);
    auto results = QueryDatabase(query);
    if (!results.Success())
        return;

	int index = 0;
    for (auto row = results.begin(); row != results.end(); ++row, ++index) {
        if(index == 255)
            break;

        cel->eld[index].id = atoi(row[0]);
        strn0cpy(cel->eld[index].accountname,row[1],64);
        cel->eld[index].account_id = atoi(row[2]);
        cel->eld[index].status = atoi(row[3]);
        strn0cpy(cel->eld[index].charactername,row[4],64);
        strn0cpy(cel->eld[index].targetname,row[5],64);
        sprintf(cel->eld[index].timestamp,"%s",row[6]);
        strn0cpy(cel->eld[index].descriptiontype,row[7],64);
        strn0cpy(cel->eld[index].details,row[8],128);
        cel->eventid = eventid;
        cel->count = index + 1;
    }

}

// Load child objects for a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::LoadWorldContainer(uint32 parentid, EQEmu::ItemInstance* container)
{
	if (!container) {
		Log(Logs::General, Logs::Error, "Programming error: LoadWorldContainer passed nullptr pointer");
		return;
	}

	std::string query = StringFormat("SELECT bagidx, itemid, charges, augslot1, augslot2, augslot3, augslot4, augslot5, augslot6 "
                                    "FROM object_contents WHERE parentid = %i", parentid);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        Log(Logs::General, Logs::Error, "Error in DB::LoadWorldContainer: %s", results.ErrorMessage().c_str());
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row) {
        uint8 index = (uint8)atoi(row[0]);
        uint32 item_id = (uint32)atoi(row[1]);
        int8 charges = (int8)atoi(row[2]);
		uint32 aug[EQEmu::invaug::SOCKET_COUNT];
        aug[0] = (uint32)atoi(row[3]);
        aug[1] = (uint32)atoi(row[4]);
        aug[2] = (uint32)atoi(row[5]);
        aug[3] = (uint32)atoi(row[6]);
        aug[4] = (uint32)atoi(row[7]);
		aug[5] = (uint32)atoi(row[8]);

        EQEmu::ItemInstance* inst = database.CreateItem(item_id, charges);
		if (inst && inst->GetItem()->IsClassCommon()) {
			for (int i = EQEmu::invaug::SOCKET_BEGIN; i <= EQEmu::invaug::SOCKET_END; i++)
                if (aug[i])
                    inst->PutAugment(&database, i, aug[i]);
            // Put item inside world container
            container->PutItem(index, *inst);
            safe_delete(inst);
        }
    }

}

// Save child objects for a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::SaveWorldContainer(uint32 zone_id, uint32 parent_id, const EQEmu::ItemInstance* container)
{
	// Since state is not saved for each world container action, we'll just delete
	// all and save from scratch .. we may come back later to optimize
	if (!container)
		return;

	//Delete all items from container
	DeleteWorldContainer(parent_id,zone_id);

	// Save all 10 items, if they exist
	for (uint8 index = EQEmu::invbag::SLOT_BEGIN; index <= EQEmu::invbag::SLOT_END; index++) {

		EQEmu::ItemInstance* inst = container->GetItem(index);
		if (!inst)
            continue;

        uint32 item_id = inst->GetItem()->ID;
		uint32 augslot[EQEmu::invaug::SOCKET_COUNT] = { 0, 0, 0, 0, 0, 0 };

		if (inst->IsType(EQEmu::item::ItemClassCommon)) {
			for (int i = EQEmu::invaug::SOCKET_BEGIN; i <= EQEmu::invaug::SOCKET_END; i++) {
                EQEmu::ItemInstance *auginst=inst->GetAugment(i);
                augslot[i]=(auginst && auginst->GetItem()) ? auginst->GetItem()->ID : 0;
            }
        }

        std::string query = StringFormat("REPLACE INTO object_contents "
                                        "(zoneid, parentid, bagidx, itemid, charges, "
                                        "augslot1, augslot2, augslot3, augslot4, augslot5, augslot6, droptime) "
                                        "VALUES (%i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, now())",
                                        zone_id, parent_id, index, item_id, inst->GetCharges(),
										augslot[0], augslot[1], augslot[2], augslot[3], augslot[4], augslot[5]);
        auto results = QueryDatabase(query);
        if (!results.Success())
            Log(Logs::General, Logs::Error, "Error in ZoneDatabase::SaveWorldContainer: %s", results.ErrorMessage().c_str());

    }

}

// Remove all child objects inside a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::DeleteWorldContainer(uint32 parent_id, uint32 zone_id)
{
	std::string query = StringFormat("DELETE FROM object_contents WHERE parentid = %i AND zoneid = %i", parent_id, zone_id);
    auto results = QueryDatabase(query);
	if (!results.Success())
		Log(Logs::General, Logs::Error, "Error in ZoneDatabase::DeleteWorldContainer: %s", results.ErrorMessage().c_str());

}

Trader_Struct* ZoneDatabase::LoadTraderItem(uint32 char_id)
{
	auto loadti = new Trader_Struct;
	memset(loadti,0,sizeof(Trader_Struct));

	std::string query = StringFormat("SELECT * FROM trader WHERE char_id = %i ORDER BY slot_id LIMIT 80", char_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		Log(Logs::Detail, Logs::Trading, "Failed to load trader information!\n");
		return loadti;
	}

	loadti->Code = BazaarTrader_ShowItems;
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (atoi(row[5]) >= 80 || atoi(row[4]) < 0) {
			Log(Logs::Detail, Logs::Trading, "Bad Slot number when trying to load trader information!\n");
			continue;
		}

		loadti->Items[atoi(row[5])] = atoi(row[1]);
		loadti->ItemCost[atoi(row[5])] = atoi(row[4]);
	}
	return loadti;
}

TraderCharges_Struct* ZoneDatabase::LoadTraderItemWithCharges(uint32 char_id)
{
	auto loadti = new TraderCharges_Struct;
	memset(loadti,0,sizeof(TraderCharges_Struct));

	std::string query = StringFormat("SELECT * FROM trader WHERE char_id=%i ORDER BY slot_id LIMIT 80", char_id);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		Log(Logs::Detail, Logs::Trading, "Failed to load trader information!\n");
		return loadti;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (atoi(row[5]) >= 80 || atoi(row[5]) < 0) {
			Log(Logs::Detail, Logs::Trading, "Bad Slot number when trying to load trader information!\n");
			continue;
		}

		loadti->ItemID[atoi(row[5])] = atoi(row[1]);
		loadti->SerialNumber[atoi(row[5])] = atoi(row[2]);
		loadti->Charges[atoi(row[5])] = atoi(row[3]);
		loadti->ItemCost[atoi(row[5])] = atoi(row[4]);
	}
	return loadti;
}

EQEmu::ItemInstance* ZoneDatabase::LoadSingleTraderItem(uint32 CharID, int SerialNumber) {
	std::string query = StringFormat("SELECT * FROM trader WHERE char_id = %i AND serialnumber = %i "
                                    "ORDER BY slot_id LIMIT 80", CharID, SerialNumber);
    auto results = QueryDatabase(query);
    if (!results.Success())
        return nullptr;

	if (results.RowCount() == 0) {
        Log(Logs::Detail, Logs::Trading, "Bad result from query\n"); fflush(stdout);
        return nullptr;
    }

    auto row = results.begin();

    int ItemID = atoi(row[1]);
	int Charges = atoi(row[3]);
	int Cost = atoi(row[4]);

	const EQEmu::ItemData *item = database.GetItem(ItemID);

	if(!item) {
		Log(Logs::Detail, Logs::Trading, "Unable to create item\n");
		fflush(stdout);
		return nullptr;
	}

    if (item->NoDrop == 0)
        return nullptr;

    EQEmu::ItemInstance* inst = database.CreateItem(item);
	if(!inst) {
		Log(Logs::Detail, Logs::Trading, "Unable to create item instance\n");
		fflush(stdout);
		return nullptr;
	}

    inst->SetCharges(Charges);
	inst->SetSerialNumber(SerialNumber);
	inst->SetMerchantSlot(SerialNumber);
	inst->SetPrice(Cost);

	if(inst->IsStackable())
		inst->SetMerchantCount(Charges);

	return inst;
}

void ZoneDatabase::SaveTraderItem(uint32 CharID, uint32 ItemID, uint32 SerialNumber, int32 Charges, uint32 ItemCost, uint8 Slot){

	std::string query = StringFormat("REPLACE INTO trader VALUES(%i, %i, %i, %i, %i, %i)",
                                    CharID, ItemID, SerialNumber, Charges, ItemCost, Slot);
    auto results = QueryDatabase(query);
    if (!results.Success())
        Log(Logs::Detail, Logs::None, "[CLIENT] Failed to save trader item: %i for char_id: %i, the error was: %s\n", ItemID, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::UpdateTraderItemCharges(int CharID, uint32 SerialNumber, int32 Charges) {
	Log(Logs::Detail, Logs::Trading, "ZoneDatabase::UpdateTraderItemCharges(%i, %i, %i)", CharID, SerialNumber, Charges);

	std::string query = StringFormat("UPDATE trader SET charges = %i WHERE char_id = %i AND serialnumber = %i",
                                    Charges, CharID, SerialNumber);
    auto results = QueryDatabase(query);
    if (!results.Success())
		Log(Logs::Detail, Logs::None, "[CLIENT] Failed to update charges for trader item: %i for char_id: %i, the error was: %s\n",
                                SerialNumber, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::UpdateTraderItemPrice(int CharID, uint32 ItemID, uint32 Charges, uint32 NewPrice) {

	Log(Logs::Detail, Logs::Trading, "ZoneDatabase::UpdateTraderPrice(%i, %i, %i, %i)", CharID, ItemID, Charges, NewPrice);

	const EQEmu::ItemData *item = database.GetItem(ItemID);

	if(!item)
		return;

	if(NewPrice == 0) {
		Log(Logs::Detail, Logs::Trading, "Removing Trader items from the DB for CharID %i, ItemID %i", CharID, ItemID);

        std::string query = StringFormat("DELETE FROM trader WHERE char_id = %i AND item_id = %i",CharID, ItemID);
        auto results = QueryDatabase(query);
        if (!results.Success())
			Log(Logs::Detail, Logs::None, "[CLIENT] Failed to remove trader item(s): %i for char_id: %i, the error was: %s\n", ItemID, CharID, results.ErrorMessage().c_str());

		return;
	}

    if(!item->Stackable) {
        std::string query = StringFormat("UPDATE trader SET item_cost = %i "
                                        "WHERE char_id = %i AND item_id = %i AND charges=%i",
                                        NewPrice, CharID, ItemID, Charges);
        auto results = QueryDatabase(query);
        if (!results.Success())
            Log(Logs::Detail, Logs::None, "[CLIENT] Failed to update price for trader item: %i for char_id: %i, the error was: %s\n", ItemID, CharID, results.ErrorMessage().c_str());

        return;
    }

    std::string query = StringFormat("UPDATE trader SET item_cost = %i "
                                    "WHERE char_id = %i AND item_id = %i",
                                    NewPrice, CharID, ItemID);
    auto results = QueryDatabase(query);
    if (!results.Success())
            Log(Logs::Detail, Logs::None, "[CLIENT] Failed to update price for trader item: %i for char_id: %i, the error was: %s\n", ItemID, CharID, results.ErrorMessage().c_str());
}

void ZoneDatabase::DeleteTraderItem(uint32 char_id){

	if(char_id==0) {
        const std::string query = "DELETE FROM trader";
        auto results = QueryDatabase(query);
		if (!results.Success())
			Log(Logs::Detail, Logs::None, "[CLIENT] Failed to delete all trader items data, the error was: %s\n", results.ErrorMessage().c_str());

        return;
	}

	std::string query = StringFormat("DELETE FROM trader WHERE char_id = %i", char_id);
	auto results = QueryDatabase(query);
    if (!results.Success())
        Log(Logs::Detail, Logs::None, "[CLIENT] Failed to delete trader item data for char_id: %i, the error was: %s\n", char_id, results.ErrorMessage().c_str());

}
void ZoneDatabase::DeleteTraderItem(uint32 CharID,uint16 SlotID) {

	std::string query = StringFormat("DELETE FROM trader WHERE char_id = %i And slot_id = %i", CharID, SlotID);
	auto results = QueryDatabase(query);
	if (!results.Success())
		Log(Logs::Detail, Logs::None, "[CLIENT] Failed to delete trader item data for char_id: %i, the error was: %s\n",CharID, results.ErrorMessage().c_str());
}

void ZoneDatabase::DeleteBuyLines(uint32 CharID) {

	if(CharID==0) {
        const std::string query = "DELETE FROM buyer";
		auto results = QueryDatabase(query);
        if (!results.Success())
			Log(Logs::Detail, Logs::None, "[CLIENT] Failed to delete all buyer items data, the error was: %s\n",results.ErrorMessage().c_str());

        return;
	}

    std::string query = StringFormat("DELETE FROM buyer WHERE charid = %i", CharID);
	auto results = QueryDatabase(query);
	if (!results.Success())
			Log(Logs::Detail, Logs::None, "[CLIENT] Failed to delete buyer item data for charid: %i, the error was: %s\n",CharID,results.ErrorMessage().c_str());

}

void ZoneDatabase::AddBuyLine(uint32 CharID, uint32 BuySlot, uint32 ItemID, const char* ItemName, uint32 Quantity, uint32 Price) {
	std::string query = StringFormat("REPLACE INTO buyer VALUES(%i, %i, %i, \"%s\", %i, %i)",
                                    CharID, BuySlot, ItemID, ItemName, Quantity, Price);
    auto results = QueryDatabase(query);
	if (!results.Success())
		Log(Logs::Detail, Logs::None, "[CLIENT] Failed to save buline item: %i for char_id: %i, the error was: %s\n", ItemID, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::RemoveBuyLine(uint32 CharID, uint32 BuySlot) {
	std::string query = StringFormat("DELETE FROM buyer WHERE charid = %i AND buyslot = %i", CharID, BuySlot);
    auto results = QueryDatabase(query);
	if (!results.Success())
		Log(Logs::Detail, Logs::None, "[CLIENT] Failed to delete buyslot %i for charid: %i, the error was: %s\n", BuySlot, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::UpdateBuyLine(uint32 CharID, uint32 BuySlot, uint32 Quantity) {
	if(Quantity <= 0) {
		RemoveBuyLine(CharID, BuySlot);
		return;
	}

	std::string query = StringFormat("UPDATE buyer SET quantity = %i WHERE charid = %i AND buyslot = %i", Quantity, CharID, BuySlot);
    auto results = QueryDatabase(query);
	if (!results.Success())
		Log(Logs::Detail, Logs::None, "[CLIENT] Failed to update quantity in buyslot %i for charid: %i, the error was: %s\n", BuySlot, CharID, results.ErrorMessage().c_str());

}

#define StructDist(in, f1, f2) (uint32(&in->f2)-uint32(&in->f1))

bool ZoneDatabase::LoadCharacterData(uint32 character_id, PlayerProfile_Struct* pp, ExtendedProfile_Struct* m_epp){
	std::string query = StringFormat(
		"SELECT                     "
		"`name`,                    "
		"last_name,                 "
		"gender,                    "
		"race,                      "
		"class,                     "
		"`level`,                   "
		"deity,                     "
		"birthday,                  "
		"last_login,                "
		"time_played,               "
		"pvp_status,                "
		"level2,                    "
		"anon,                      "
		"gm,                        "
		"intoxication,              "
		"hair_color,                "
		"beard_color,               "
		"eye_color_1,               "
		"eye_color_2,               "
		"hair_style,                "
		"beard,                     "
		"ability_time_seconds,      "
		"ability_number,            "
		"ability_time_minutes,      "
		"ability_time_hours,        "
		"title,                     "
		"suffix,                    "
		"exp,                       "
		"points,                    "
		"mana,                      "
		"cur_hp,                    "
		"str,                       "
		"sta,                       "
		"cha,                       "
		"dex,                       "
		"`int`,                     "
		"agi,                       "
		"wis,                       "
		"face,                      "
		"y,                         "
		"x,                         "
		"z,                         "
		"heading,                   "
		"pvp2,                      "
		"pvp_type,                  "
		"autosplit_enabled,         "
		"zone_change_count,         "
		"drakkin_heritage,          "
		"drakkin_tattoo,            "
		"drakkin_details,           "
		"toxicity,                  "
		"hunger_level,              "
		"thirst_level,              "
		"ability_up,                "
		"zone_id,                   "
		"zone_instance,             "
		"leadership_exp_on,         "
		"ldon_points_guk,           "
		"ldon_points_mir,           "
		"ldon_points_mmc,           "
		"ldon_points_ruj,           "
		"ldon_points_tak,           "
		"ldon_points_available,     "
		"tribute_time_remaining,    "
		"show_helm,                 "
		"career_tribute_points,     "
		"tribute_points,            "
		"tribute_active,            "
		"endurance,                 "
		"group_leadership_exp,      "
		"raid_leadership_exp,       "
		"group_leadership_points,   "
		"raid_leadership_points,    "
		"air_remaining,             "
		"pvp_kills,                 "
		"pvp_deaths,                "
		"pvp_current_points,        "
		"pvp_career_points,         "
		"pvp_best_kill_streak,      "
		"pvp_worst_death_streak,    "
		"pvp_current_kill_streak,   "
		"aa_points_spent,           "
		"aa_exp,                    "
		"aa_points,                 "
		"group_auto_consent,        "
		"raid_auto_consent,         "
		"guild_auto_consent,        "
		"RestTimer,                 "
		"`e_aa_effects`,			"
		"`e_percent_to_aa`,			"
		"`e_expended_aa_spent`,		"
		"`e_last_invsnapshot`		"
		"FROM                       "
		"character_data             "
		"WHERE `id` = %i         ", character_id);
	auto results = database.QueryDatabase(query); int r = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		strcpy(pp->name, row[r]); r++;											 // "`name`,                    "
		strcpy(pp->last_name, row[r]); r++;										 // "last_name,                 "
		pp->gender = atoi(row[r]); r++;											 // "gender,                    "
		pp->race = atoi(row[r]); r++;											 // "race,                      "
		pp->class_ = atoi(row[r]); r++;											 // "class,                     "
		pp->level = atoi(row[r]); r++;											 // "`level`,                   "
		pp->deity = atoi(row[r]); r++;											 // "deity,                     "
		pp->birthday = atoi(row[r]); r++;										 // "birthday,                  "
		pp->lastlogin = atoi(row[r]); r++;										 // "last_login,                "
		pp->timePlayedMin = atoi(row[r]); r++;									 // "time_played,               "
		pp->pvp = atoi(row[r]); r++;											 // "pvp_status,                "
		pp->level2 = atoi(row[r]); r++;											 // "level2,                    "
		pp->anon = atoi(row[r]); r++;											 // "anon,                      "
		pp->gm = atoi(row[r]); r++;												 // "gm,                        "
		pp->intoxication = atoi(row[r]); r++;									 // "intoxication,              "
		pp->haircolor = atoi(row[r]); r++;										 // "hair_color,                "
		pp->beardcolor = atoi(row[r]); r++;										 // "beard_color,               "
		pp->eyecolor1 = atoi(row[r]); r++;										 // "eye_color_1,               "
		pp->eyecolor2 = atoi(row[r]); r++;										 // "eye_color_2,               "
		pp->hairstyle = atoi(row[r]); r++;										 // "hair_style,                "
		pp->beard = atoi(row[r]); r++;											 // "beard,                     "
		pp->ability_time_seconds = atoi(row[r]); r++;							 // "ability_time_seconds,      "
		pp->ability_number = atoi(row[r]); r++;									 // "ability_number,            "
		pp->ability_time_minutes = atoi(row[r]); r++;							 // "ability_time_minutes,      "
		pp->ability_time_hours = atoi(row[r]); r++;								 // "ability_time_hours,        "
		strcpy(pp->title, row[r]); r++;											 // "title,                     "
		strcpy(pp->suffix, row[r]); r++;										 // "suffix,                    "
		pp->exp = atoi(row[r]); r++;											 // "exp,                       "
		pp->points = atoi(row[r]); r++;											 // "points,                    "
		pp->mana = atoi(row[r]); r++;											 // "mana,                      "
		pp->cur_hp = atoi(row[r]); r++;											 // "cur_hp,                    "
		pp->STR = atoi(row[r]); r++;											 // "str,                       "
		pp->STA = atoi(row[r]); r++;											 // "sta,                       "
		pp->CHA = atoi(row[r]); r++;											 // "cha,                       "
		pp->DEX = atoi(row[r]); r++;											 // "dex,                       "
		pp->INT = atoi(row[r]); r++;											 // "`int`,                     "
		pp->AGI = atoi(row[r]); r++;											 // "agi,                       "
		pp->WIS = atoi(row[r]); r++;											 // "wis,                       "
		pp->face = atoi(row[r]); r++;											 // "face,                      "
		pp->y = atof(row[r]); r++;												 // "y,                         "
		pp->x = atof(row[r]); r++;												 // "x,                         "
		pp->z = atof(row[r]); r++;												 // "z,                         "
		pp->heading = atof(row[r]); r++;										 // "heading,                   "
		pp->pvp2 = atoi(row[r]); r++;											 // "pvp2,                      "
		pp->pvptype = atoi(row[r]); r++;										 // "pvp_type,                  "
		pp->autosplit = atoi(row[r]); r++;										 // "autosplit_enabled,         "
		pp->zone_change_count = atoi(row[r]); r++;								 // "zone_change_count,         "
		pp->drakkin_heritage = atoi(row[r]); r++;								 // "drakkin_heritage,          "
		pp->drakkin_tattoo = atoi(row[r]); r++;									 // "drakkin_tattoo,            "
		pp->drakkin_details = atoi(row[r]); r++;								 // "drakkin_details,           "
		pp->toxicity = atoi(row[r]); r++;										 // "toxicity,                  "
		pp->hunger_level = atoi(row[r]); r++;									 // "hunger_level,              "
		pp->thirst_level = atoi(row[r]); r++;									 // "thirst_level,              "
		pp->ability_up = atoi(row[r]); r++;										 // "ability_up,                "
		pp->zone_id = atoi(row[r]); r++;										 // "zone_id,                   "
		pp->zoneInstance = atoi(row[r]); r++;									 // "zone_instance,             "
		pp->leadAAActive = atoi(row[r]); r++;									 // "leadership_exp_on,         "
		pp->ldon_points_guk = atoi(row[r]); r++;								 // "ldon_points_guk,           "
		pp->ldon_points_mir = atoi(row[r]); r++;								 // "ldon_points_mir,           "
		pp->ldon_points_mmc = atoi(row[r]); r++;								 // "ldon_points_mmc,           "
		pp->ldon_points_ruj = atoi(row[r]); r++;								 // "ldon_points_ruj,           "
		pp->ldon_points_tak = atoi(row[r]); r++;								 // "ldon_points_tak,           "
		pp->ldon_points_available = atoi(row[r]); r++;							 // "ldon_points_available,     "
		pp->tribute_time_remaining = atoi(row[r]); r++;							 // "tribute_time_remaining,    "
		pp->showhelm = atoi(row[r]); r++;										 // "show_helm,                 "
		pp->career_tribute_points = atoi(row[r]); r++;							 // "career_tribute_points,     "
		pp->tribute_points = atoi(row[r]); r++;									 // "tribute_points,            "
		pp->tribute_active = atoi(row[r]); r++;									 // "tribute_active,            "
		pp->endurance = atoi(row[r]); r++;										 // "endurance,                 "
		pp->group_leadership_exp = atoi(row[r]); r++;							 // "group_leadership_exp,      "
		pp->raid_leadership_exp = atoi(row[r]); r++;							 // "raid_leadership_exp,       "
		pp->group_leadership_points = atoi(row[r]); r++;						 // "group_leadership_points,   "
		pp->raid_leadership_points = atoi(row[r]); r++;							 // "raid_leadership_points,    "
		pp->air_remaining = atoi(row[r]); r++;									 // "air_remaining,             "
		pp->PVPKills = atoi(row[r]); r++;										 // "pvp_kills,                 "
		pp->PVPDeaths = atoi(row[r]); r++;										 // "pvp_deaths,                "
		pp->PVPCurrentPoints = atoi(row[r]); r++;								 // "pvp_current_points,        "
		pp->PVPCareerPoints = atoi(row[r]); r++;								 // "pvp_career_points,         "
		pp->PVPBestKillStreak = atoi(row[r]); r++;								 // "pvp_best_kill_streak,      "
		pp->PVPWorstDeathStreak = atoi(row[r]); r++;							 // "pvp_worst_death_streak,    "
		pp->PVPCurrentKillStreak = atoi(row[r]); r++;							 // "pvp_current_kill_streak,   "
		pp->aapoints_spent = atoi(row[r]); r++;									 // "aa_points_spent,           "
		pp->expAA = atoi(row[r]); r++;											 // "aa_exp,                    "
		pp->aapoints = atoi(row[r]); r++;										 // "aa_points,                 "
		pp->groupAutoconsent = atoi(row[r]); r++;								 // "group_auto_consent,        "
		pp->raidAutoconsent = atoi(row[r]); r++;								 // "raid_auto_consent,         "
		pp->guildAutoconsent = atoi(row[r]); r++;								 // "guild_auto_consent,        "
		pp->RestTimer = atoi(row[r]); r++;										 // "RestTimer,                 "
		m_epp->aa_effects = atoi(row[r]); r++;									 // "`e_aa_effects`,			"
		m_epp->perAA = atoi(row[r]); r++;										 // "`e_percent_to_aa`,			"
		m_epp->expended_aa = atoi(row[r]); r++;									 // "`e_expended_aa_spent`,		"
		m_epp->last_invsnapshot_time = atoi(row[r]); r++;						 // "`e_last_invsnapshot`		"
		m_epp->next_invsnapshot_time = m_epp->last_invsnapshot_time + (RuleI(Character, InvSnapshotMinIntervalM) * 60);
	}
	return true;
}

bool ZoneDatabase::LoadCharacterFactionValues(uint32 character_id, faction_map & val_list) {
	std::string query = StringFormat("SELECT `faction_id`, `current_value` FROM `faction_values` WHERE `char_id` = %i", character_id);
	auto results = database.QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) { val_list[atoi(row[0])] = atoi(row[1]); }
	return true;
}

bool ZoneDatabase::LoadCharacterMemmedSpells(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT							"
		"slot_id,						"
		"`spell_id`						"
		"FROM							"
		"`character_memmed_spells`		"
		"WHERE `id` = %u ORDER BY `slot_id`", character_id);
	auto results = database.QueryDatabase(query);
	int i = 0;
	/* Initialize Spells */
	for (i = 0; i < MAX_PP_MEMSPELL; i++){
		pp->mem_spells[i] = 0xFFFFFFFF;
	}
	for (auto row = results.begin(); row != results.end(); ++row) {
		i = atoi(row[0]);
		if (i < MAX_PP_MEMSPELL && atoi(row[1]) <= SPDAT_RECORDS){
			pp->mem_spells[i] = atoi(row[1]);
		}
	}
	return true;
}

bool ZoneDatabase::LoadCharacterSpellBook(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT					"
		"slot_id,				"
		"`spell_id`				"
		"FROM					"
		"`character_spells`		"
		"WHERE `id` = %u ORDER BY `slot_id`", character_id);
	auto results = database.QueryDatabase(query);
	int i = 0;
	/* Initialize Spells */
	for (i = 0; i < MAX_PP_SPELLBOOK; i++){
		pp->spell_book[i] = 0xFFFFFFFF;
	}
	for (auto row = results.begin(); row != results.end(); ++row) {
		i = atoi(row[0]);
		if (i < MAX_PP_SPELLBOOK && atoi(row[1]) <= SPDAT_RECORDS){
			pp->spell_book[i] = atoi(row[1]);
		}
	}
	return true;
}

bool ZoneDatabase::LoadCharacterLanguages(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT					"
		"lang_id,				"
		"`value`				"
		"FROM					"
		"`character_languages`	"
		"WHERE `id` = %u ORDER BY `lang_id`", character_id);
	auto results = database.QueryDatabase(query); int i = 0;
	/* Initialize Languages */
	for (i = 0; i < MAX_PP_LANGUAGE; ++i)
		pp->languages[i] = 0;

	for (auto row = results.begin(); row != results.end(); ++row) {
		i = atoi(row[0]);
		if (i < MAX_PP_LANGUAGE){
			pp->languages[i] = atoi(row[1]);
		}
	}

	return true;
}

bool ZoneDatabase::LoadCharacterLeadershipAA(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat("SELECT slot, rank FROM character_leadership_abilities WHERE `id` = %u", character_id);
	auto results = database.QueryDatabase(query); uint32 slot = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		slot = atoi(row[0]);
		pp->leader_abilities.ranks[slot] = atoi(row[1]);
	}
	return true;
}

bool ZoneDatabase::LoadCharacterDisciplines(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT				  "
		"disc_id			  "
		"FROM				  "
		"`character_disciplines`"
		"WHERE `id` = %u ORDER BY `slot_id`", character_id);
	auto results = database.QueryDatabase(query);
	int i = 0;

	/* Initialize Disciplines */
	memset(pp->disciplines.values, 0, (sizeof(pp->disciplines.values[0]) * MAX_PP_DISCIPLINES));
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (i < MAX_PP_DISCIPLINES)
			pp->disciplines.values[i] = atoi(row[0]);
        ++i;
    }
	return true;
}

bool ZoneDatabase::LoadCharacterSkills(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT				"
		"skill_id,			"
		"`value`			"
		"FROM				"
		"`character_skills` "
		"WHERE `id` = %u ORDER BY `skill_id`", character_id);
	auto results = database.QueryDatabase(query);
	int i = 0;
	/* Initialize Skill */
	for (i = 0; i < MAX_PP_SKILL; ++i)
		pp->skills[i] = 0;

	for (auto row = results.begin(); row != results.end(); ++row) {
		i = atoi(row[0]);
		if (i < MAX_PP_SKILL)
			pp->skills[i] = atoi(row[1]);
	}

	return true;
}

bool ZoneDatabase::LoadCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"SELECT                  "
		"platinum,               "
		"gold,                   "
		"silver,                 "
		"copper,                 "
		"platinum_bank,          "
		"gold_bank,              "
		"silver_bank,            "
		"copper_bank,            "
		"platinum_cursor,        "
		"gold_cursor,            "
		"silver_cursor,          "
		"copper_cursor,          "
		"radiant_crystals,       "
		"career_radiant_crystals,"
		"ebon_crystals,          "
		"career_ebon_crystals    "
		"FROM                    "
		"character_currency      "
		"WHERE `id` = %i         ", character_id);
	auto results = database.QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		pp->platinum = atoi(row[0]);
		pp->gold = atoi(row[1]);
		pp->silver = atoi(row[2]);
		pp->copper = atoi(row[3]);
		pp->platinum_bank = atoi(row[4]);
		pp->gold_bank = atoi(row[5]);
		pp->silver_bank = atoi(row[6]);
		pp->copper_bank = atoi(row[7]);
		pp->platinum_cursor = atoi(row[8]);
		pp->gold_cursor = atoi(row[9]);
		pp->silver_cursor = atoi(row[10]);
		pp->copper_cursor = atoi(row[11]);
		pp->currentRadCrystals = atoi(row[12]);
		pp->careerRadCrystals = atoi(row[13]);
		pp->currentEbonCrystals = atoi(row[14]);
		pp->careerEbonCrystals = atoi(row[15]);
	}
	return true;
}

bool ZoneDatabase::LoadCharacterMaterialColor(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat("SELECT slot, blue, green, red, use_tint, color FROM `character_material` WHERE `id` = %u LIMIT 9", character_id);
	auto results = database.QueryDatabase(query); int i = 0; int r = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		r = 0;
		i = atoi(row[r]); /* Slot */ r++;
		pp->item_tint.Slot[i].Blue = atoi(row[r]); r++;
		pp->item_tint.Slot[i].Green = atoi(row[r]); r++;
		pp->item_tint.Slot[i].Red = atoi(row[r]); r++;
		pp->item_tint.Slot[i].UseTint = atoi(row[r]);
	}
	return true;
}

bool ZoneDatabase::LoadCharacterBandolier(uint32 character_id, PlayerProfile_Struct* pp)
{
	std::string query = StringFormat("SELECT `bandolier_id`, `bandolier_slot`, `item_id`, `icon`, `bandolier_name` FROM `character_bandolier` WHERE `id` = %u LIMIT %u",
		character_id, EQEmu::profile::BANDOLIERS_SIZE);
	auto results = database.QueryDatabase(query); int i = 0; int r = 0; int si = 0;
	for (i = 0; i < EQEmu::profile::BANDOLIERS_SIZE; i++) {
		pp->bandoliers[i].Name[0] = '\0';
		for (int si = 0; si < EQEmu::profile::BANDOLIER_ITEM_COUNT; si++) {
			pp->bandoliers[i].Items[si].ID = 0;
			pp->bandoliers[i].Items[si].Icon = 0;
			pp->bandoliers[i].Items[si].Name[0] = '\0';
		}
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		r = 0;
		i = atoi(row[r]); /* Bandolier ID */ r++;
		si = atoi(row[r]); /* Bandolier Slot */ r++;

		const EQEmu::ItemData* item_data = database.GetItem(atoi(row[r]));
		if (item_data) {
			pp->bandoliers[i].Items[si].ID = item_data->ID; r++;
			pp->bandoliers[i].Items[si].Icon = atoi(row[r]); r++; // Must use db value in case an Ornamentation is assigned
			strncpy(pp->bandoliers[i].Items[si].Name, item_data->Name, 64);
		}
		else {
			pp->bandoliers[i].Items[si].ID = 0; r++;
			pp->bandoliers[i].Items[si].Icon = 0; r++;
			pp->bandoliers[i].Items[si].Name[0] = '\0';
		}
		strcpy(pp->bandoliers[i].Name, row[r]);  r++;

		si++; // What is this for!?
	}
	return true;
}

bool ZoneDatabase::LoadCharacterTribute(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat("SELECT `tier`, `tribute` FROM `character_tribute` WHERE `id` = %u", character_id);
	auto results = database.QueryDatabase(query);
	int i = 0;
	for (i = 0; i < EQEmu::invtype::TRIBUTE_SIZE; i++){
		pp->tributes[i].tribute = 0xFFFFFFFF;
		pp->tributes[i].tier = 0;
	}
	i = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		if(atoi(row[1]) != TRIBUTE_NONE){
			pp->tributes[i].tier = atoi(row[0]);
			pp->tributes[i].tribute = atoi(row[1]);
			i++;
		}
	}
	return true;
}

bool ZoneDatabase::LoadCharacterPotions(uint32 character_id, PlayerProfile_Struct *pp)
{
	std::string query =
	    StringFormat("SELECT `potion_id`, `item_id`, `icon` FROM `character_potionbelt` WHERE `id` = %u LIMIT %u",
		character_id, EQEmu::profile::POTION_BELT_SIZE);
	auto results = database.QueryDatabase(query);
	int i = 0;
	for (i = 0; i < EQEmu::profile::POTION_BELT_SIZE; i++) {
		pp->potionbelt.Items[i].Icon = 0;
		pp->potionbelt.Items[i].ID = 0;
		pp->potionbelt.Items[i].Name[0] = '\0';
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		i = atoi(row[0]);
		const EQEmu::ItemData *item_data = database.GetItem(atoi(row[1]));
		if (!item_data)
			continue;
		pp->potionbelt.Items[i].ID = item_data->ID;
		pp->potionbelt.Items[i].Icon = atoi(row[2]);
		strncpy(pp->potionbelt.Items[i].Name, item_data->Name, 64);
	}

	return true;
}

bool ZoneDatabase::LoadCharacterBindPoint(uint32 character_id, PlayerProfile_Struct *pp)
{
	std::string query = StringFormat("SELECT `slot`, `zone_id`, `instance_id`, `x`, `y`, `z`, `heading` FROM "
					 "`character_bind` WHERE `id` = %u LIMIT 5",
					 character_id);
	auto results = database.QueryDatabase(query);

	if (!results.RowCount()) // SHIT -- this actually isn't good
		return true;

	for (auto row = results.begin(); row != results.end(); ++row) {
		int index = atoi(row[0]);
		if (index < 0 || index > 4)
			continue;

		pp->binds[index].zoneId = atoi(row[1]);
		pp->binds[index].instance_id = atoi(row[2]);
		pp->binds[index].x = atoi(row[3]);
		pp->binds[index].y = atoi(row[4]);
		pp->binds[index].z = atoi(row[5]);
		pp->binds[index].heading = atoi(row[6]);
	}

	return true;
}

bool ZoneDatabase::SaveCharacterLanguage(uint32 character_id, uint32 lang_id, uint32 value){
	std::string query = StringFormat("REPLACE INTO `character_languages` (id, lang_id, value) VALUES (%u, %u, %u)", character_id, lang_id, value); QueryDatabase(query);
	Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterLanguage for character ID: %i, lang_id:%u value:%u done", character_id, lang_id, value);
	return true;
}

bool ZoneDatabase::SaveCharacterBindPoint(uint32 character_id, const BindStruct &bind, uint32 bind_num)
{
	/* Save Home Bind Point */
	std::string query =
	    StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, slot) VALUES (%u, "
			 "%u, %u, %f, %f, %f, %f, %i)",
			 character_id, bind.zoneId, bind.instance_id, bind.x, bind.y, bind.z, bind.heading, bind_num);

	Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterBindPoint for character ID: %i zone_id: %u "
					   "instance_id: %u position: %f %f %f %f bind_num: %u",
		character_id, bind.zoneId, bind.instance_id, bind.x, bind.y, bind.z, bind.heading, bind_num);

	auto results = QueryDatabase(query);
	if (!results.RowsAffected())
		Log(Logs::General, Logs::None, "ERROR Bind Home Save: %s. %s", results.ErrorMessage().c_str(),
			query.c_str());

	return true;
}

bool ZoneDatabase::SaveCharacterMaterialColor(uint32 character_id, uint32 slot_id, uint32 color){
	uint8 red = (color & 0x00FF0000) >> 16;
	uint8 green = (color & 0x0000FF00) >> 8;
	uint8 blue = (color & 0x000000FF);

	std::string query = StringFormat("REPLACE INTO `character_material` (id, slot, red, green, blue, color, use_tint) VALUES (%u, %u, %u, %u, %u, %u, 255)", character_id, slot_id, red, green, blue, color); auto results = QueryDatabase(query);
	Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterMaterialColor for character ID: %i, slot_id: %u color: %u done", character_id, slot_id, color);
	return true;
}

bool ZoneDatabase::SaveCharacterSkill(uint32 character_id, uint32 skill_id, uint32 value){
	std::string query = StringFormat("REPLACE INTO `character_skills` (id, skill_id, value) VALUES (%u, %u, %u)", character_id, skill_id, value); auto results = QueryDatabase(query);
	Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterSkill for character ID: %i, skill_id:%u value:%u done", character_id, skill_id, value);
	return true;
}

bool ZoneDatabase::SaveCharacterDisc(uint32 character_id, uint32 slot_id, uint32 disc_id){
	std::string query = StringFormat("REPLACE INTO `character_disciplines` (id, slot_id, disc_id) VALUES (%u, %u, %u)", character_id, slot_id, disc_id);
	auto results = QueryDatabase(query);
	Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterDisc for character ID: %i, slot:%u disc_id:%u done", character_id, slot_id, disc_id);
	return true;
}

bool ZoneDatabase::SaveCharacterTribute(uint32 character_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat("DELETE FROM `character_tribute` WHERE `id` = %u", character_id);
	QueryDatabase(query);
	/* Save Tributes only if we have values... */
	for (int i = 0; i < EQEmu::invtype::TRIBUTE_SIZE; i++){
		if (pp->tributes[i].tribute > 0 && pp->tributes[i].tribute != TRIBUTE_NONE){
			std::string query = StringFormat("REPLACE INTO `character_tribute` (id, tier, tribute) VALUES (%u, %u, %u)", character_id, pp->tributes[i].tier, pp->tributes[i].tribute);
			QueryDatabase(query);
			Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterTribute for character ID: %i, tier:%u tribute:%u done", character_id, pp->tributes[i].tier, pp->tributes[i].tribute);
		}
	}
	return true;
}

bool ZoneDatabase::SaveCharacterBandolier(uint32 character_id, uint8 bandolier_id, uint8 bandolier_slot, uint32 item_id, uint32 icon, const char* bandolier_name)
{
	char bandolier_name_esc[64];
	DoEscapeString(bandolier_name_esc, bandolier_name, strlen(bandolier_name));
	std::string query = StringFormat("REPLACE INTO `character_bandolier` (id, bandolier_id, bandolier_slot, item_id, icon, bandolier_name) VALUES (%u, %u, %u, %u, %u,'%s')", character_id, bandolier_id, bandolier_slot, item_id, icon, bandolier_name_esc);
	auto results = QueryDatabase(query);
	Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterBandolier for character ID: %i, bandolier_id: %u, bandolier_slot: %u item_id: %u, icon:%u band_name:%s  done", character_id, bandolier_id, bandolier_slot, item_id, icon, bandolier_name);
	return true;
}

bool ZoneDatabase::SaveCharacterPotionBelt(uint32 character_id, uint8 potion_id, uint32 item_id, uint32 icon)
{
	std::string query = StringFormat("REPLACE INTO `character_potionbelt` (id, potion_id, item_id, icon) VALUES (%u, %u, %u, %u)", character_id, potion_id, item_id, icon);
	auto results = QueryDatabase(query);
	return true;
}

bool ZoneDatabase::SaveCharacterLeadershipAA(uint32 character_id, PlayerProfile_Struct* pp){
	uint8 first_entry = 0; std::string query = "";
	for (int i = 0; i < MAX_LEADERSHIP_AA_ARRAY; i++){
		if (pp->leader_abilities.ranks[i] > 0){
			if (first_entry != 1){
				query = StringFormat("REPLACE INTO `character_leadership_abilities` (id, slot, rank) VALUES (%i, %u, %u)", character_id, i, pp->leader_abilities.ranks[i]);
				first_entry = 1;
			}
			query = query + StringFormat(", (%i, %u, %u)", character_id, i, pp->leader_abilities.ranks[i]);
		}
	}
	auto results = QueryDatabase(query);
	return true;
}

bool ZoneDatabase::SaveCharacterInventorySnapshot(uint32 character_id){
	uint32 time_index = time(nullptr);
	std::string query = StringFormat(
		"INSERT INTO inventory_snapshots ("
		" time_index,"
		" charid,"
		" slotid,"
		" itemid,"
		" charges,"
		" color,"
		" augslot1,"
		" augslot2,"
		" augslot3,"
		" augslot4,"
		" augslot5,"
		" augslot6,"
		" instnodrop,"
		" custom_data,"
		" ornamenticon,"
		" ornamentidfile,"
		" ornament_hero_model"
		")"
		" SELECT"
		" %u,"
		" charid,"
		" slotid,"
		" itemid,"
		" charges,"
		" color,"
		" augslot1,"
		" augslot2,"
		" augslot3,"
		" augslot4,"
		" augslot5,"
		" augslot6,"
		" instnodrop,"
		" custom_data,"
		" ornamenticon,"
		" ornamentidfile,"
		" ornament_hero_model"
		" FROM inventory"
		" WHERE charid = %u",
		time_index,
		character_id
	);
	auto results = database.QueryDatabase(query);
	Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterInventorySnapshot %i (%s)", character_id, (results.Success() ? "pass" : "fail"));
	return results.Success();
}

bool ZoneDatabase::SaveCharacterData(uint32 character_id, uint32 account_id, PlayerProfile_Struct* pp, ExtendedProfile_Struct* m_epp){
	
	/* If this is ever zero - the client hasn't fully loaded and potentially crashed during zone */
	if (account_id <= 0)
		return false;
	
	std::string mail_key = database.GetMailKey(character_id);

	clock_t t = std::clock(); /* Function timer start */
	std::string query = StringFormat(
		"REPLACE INTO `character_data` ("
		" id,                        "
		" account_id,                "
		" `name`,                    "
		" last_name,                 "
		" gender,                    "
		" race,                      "
		" class,                     "
		" `level`,                   "
		" deity,                     "
		" birthday,                  "
		" last_login,                "
		" time_played,               "
		" pvp_status,                "
		" level2,                    "
		" anon,                      "
		" gm,                        "
		" intoxication,              "
		" hair_color,                "
		" beard_color,               "
		" eye_color_1,               "
		" eye_color_2,               "
		" hair_style,                "
		" beard,                     "
		" ability_time_seconds,      "
		" ability_number,            "
		" ability_time_minutes,      "
		" ability_time_hours,        "
		" title,                     "
		" suffix,                    "
		" exp,                       "
		" points,                    "
		" mana,                      "
		" cur_hp,                    "
		" str,                       "
		" sta,                       "
		" cha,                       "
		" dex,                       "
		" `int`,                     "
		" agi,                       "
		" wis,                       "
		" face,                      "
		" y,                         "
		" x,                         "
		" z,                         "
		" heading,                   "
		" pvp2,                      "
		" pvp_type,                  "
		" autosplit_enabled,         "
		" zone_change_count,         "
		" drakkin_heritage,          "
		" drakkin_tattoo,            "
		" drakkin_details,           "
		" toxicity,                  "
		" hunger_level,              "
		" thirst_level,              "
		" ability_up,                "
		" zone_id,                   "
		" zone_instance,             "
		" leadership_exp_on,         "
		" ldon_points_guk,           "
		" ldon_points_mir,           "
		" ldon_points_mmc,           "
		" ldon_points_ruj,           "
		" ldon_points_tak,           "
		" ldon_points_available,     "
		" tribute_time_remaining,    "
		" show_helm,                 "
		" career_tribute_points,     "
		" tribute_points,            "
		" tribute_active,            "
		" endurance,                 "
		" group_leadership_exp,      "
		" raid_leadership_exp,       "
		" group_leadership_points,   "
		" raid_leadership_points,    "
		" air_remaining,             "
		" pvp_kills,                 "
		" pvp_deaths,                "
		" pvp_current_points,        "
		" pvp_career_points,         "
		" pvp_best_kill_streak,      "
		" pvp_worst_death_streak,    "
		" pvp_current_kill_streak,   "
		" aa_points_spent,           "
		" aa_exp,                    "
		" aa_points,                 "
		" group_auto_consent,        "
		" raid_auto_consent,         "
		" guild_auto_consent,        "
		" RestTimer,				 "
		" e_aa_effects,				 "
		" e_percent_to_aa,			 "
		" e_expended_aa_spent,		 "
		" e_last_invsnapshot,		 "
		" mailkey					 "
		")							 "
		"VALUES ("
		"%u,"  // id																" id,                        "
		"%u,"  // account_id														" account_id,                "
		"'%s',"  // `name`					  pp->name,								" `name`,                    "
		"'%s',"  // last_name					pp->last_name,						" last_name,                 "
		"%u,"  // gender					  pp->gender,							" gender,                    "
		"%u,"  // race						  pp->race,								" race,                      "
		"%u,"  // class						  pp->class_,							" class,                     "
		"%u,"  // `level`					  pp->level,							" `level`,                   "
		"%u,"  // deity						  pp->deity,							" deity,                     "
		"%u,"  // birthday					  pp->birthday,							" birthday,                  "
		"%u,"  // last_login				  pp->lastlogin,						" last_login,                "
		"%u,"  // time_played				  pp->timePlayedMin,					" time_played,               "
		"%u,"  // pvp_status				  pp->pvp,								" pvp_status,                "
		"%u,"  // level2					  pp->level2,							" level2,                    "
		"%u,"  // anon						  pp->anon,								" anon,                      "
		"%u,"  // gm						  pp->gm,								" gm,                        "
		"%u,"  // intoxication				  pp->intoxication,						" intoxication,              "
		"%u,"  // hair_color				  pp->haircolor,						" hair_color,                "
		"%u,"  // beard_color				  pp->beardcolor,						" beard_color,               "
		"%u,"  // eye_color_1				  pp->eyecolor1,						" eye_color_1,               "
		"%u,"  // eye_color_2				  pp->eyecolor2,						" eye_color_2,               "
		"%u,"  // hair_style				  pp->hairstyle,						" hair_style,                "
		"%u,"  // beard						  pp->beard,							" beard,                     "
		"%u,"  // ability_time_seconds		  pp->ability_time_seconds,				" ability_time_seconds,      "
		"%u,"  // ability_number			  pp->ability_number,					" ability_number,            "
		"%u,"  // ability_time_minutes		  pp->ability_time_minutes,				" ability_time_minutes,      "
		"%u,"  // ability_time_hours		  pp->ability_time_hours,				" ability_time_hours,        "
		"'%s',"  // title						  pp->title,						" title,                     "   "
		"'%s',"  // suffix					  pp->suffix,							" suffix,                    "
		"%u,"  // exp						  pp->exp,								" exp,                       "
		"%u,"  // points					  pp->points,							" points,                    "
		"%u,"  // mana						  pp->mana,								" mana,                      "
		"%u,"  // cur_hp					  pp->cur_hp,							" cur_hp,                    "
		"%u,"  // str						  pp->STR,								" str,                       "
		"%u,"  // sta						  pp->STA,								" sta,                       "
		"%u,"  // cha						  pp->CHA,								" cha,                       "
		"%u,"  // dex						  pp->DEX,								" dex,                       "
		"%u,"  // `int`						  pp->INT,								" `int`,                     "
		"%u,"  // agi						  pp->AGI,								" agi,                       "
		"%u,"  // wis						  pp->WIS,								" wis,                       "
		"%u,"  // face						  pp->face,								" face,                      "
		"%f,"  // y							  pp->y,								" y,                         "
		"%f,"  // x							  pp->x,								" x,                         "
		"%f,"  // z							  pp->z,								" z,                         "
		"%f,"  // heading					  pp->heading,							" heading,                   "
		"%u,"  // pvp2						  pp->pvp2,								" pvp2,                      "
		"%u,"  // pvp_type					  pp->pvptype,							" pvp_type,                  "
		"%u,"  // autosplit_enabled			  pp->autosplit,						" autosplit_enabled,         "
		"%u,"  // zone_change_count			  pp->zone_change_count,				" zone_change_count,         "
		"%u,"  // drakkin_heritage			  pp->drakkin_heritage,					" drakkin_heritage,          "
		"%u,"  // drakkin_tattoo			  pp->drakkin_tattoo,					" drakkin_tattoo,            "
		"%u,"  // drakkin_details			  pp->drakkin_details,					" drakkin_details,           "
		"%i,"  // toxicity					  pp->toxicity,							" toxicity,                  "
		"%i,"  // hunger_level				  pp->hunger_level,						" hunger_level,              "
		"%i,"  // thirst_level				  pp->thirst_level,						" thirst_level,              "
		"%u,"  // ability_up				  pp->ability_up,						" ability_up,                "
		"%u,"  // zone_id					  pp->zone_id,							" zone_id,                   "
		"%u,"  // zone_instance				  pp->zoneInstance,						" zone_instance,             "
		"%u,"  // leadership_exp_on			  pp->leadAAActive,						" leadership_exp_on,         "
		"%u,"  // ldon_points_guk			  pp->ldon_points_guk,					" ldon_points_guk,           "
		"%u,"  // ldon_points_mir			  pp->ldon_points_mir,					" ldon_points_mir,           "
		"%u,"  // ldon_points_mmc			  pp->ldon_points_mmc,					" ldon_points_mmc,           "
		"%u,"  // ldon_points_ruj			  pp->ldon_points_ruj,					" ldon_points_ruj,           "
		"%u,"  // ldon_points_tak			  pp->ldon_points_tak,					" ldon_points_tak,           "
		"%u,"  // ldon_points_available		  pp->ldon_points_available,			" ldon_points_available,     "
		"%u,"  // tribute_time_remaining	  pp->tribute_time_remaining,			" tribute_time_remaining,    "
		"%u,"  // show_helm					  pp->showhelm,							" show_helm,                 "
		"%u,"  // career_tribute_points		  pp->career_tribute_points,			" career_tribute_points,     "
		"%u,"  // tribute_points			  pp->tribute_points,					" tribute_points,            "
		"%u,"  // tribute_active			  pp->tribute_active,					" tribute_active,            "
		"%u,"  // endurance					  pp->endurance,						" endurance,                 "
		"%u,"  // group_leadership_exp		  pp->group_leadership_exp,				" group_leadership_exp,      "
		"%u,"  // raid_leadership_exp		  pp->raid_leadership_exp,				" raid_leadership_exp,       "
		"%u,"  // group_leadership_points	  pp->group_leadership_points,			" group_leadership_points,   "
		"%u,"  // raid_leadership_points	  pp->raid_leadership_points,			" raid_leadership_points,    "
		"%u,"  // air_remaining				  pp->air_remaining,					" air_remaining,             "
		"%u,"  // pvp_kills					  pp->PVPKills,							" pvp_kills,                 "
		"%u,"  // pvp_deaths				  pp->PVPDeaths,						" pvp_deaths,                "
		"%u,"  // pvp_current_points		  pp->PVPCurrentPoints,					" pvp_current_points,        "
		"%u,"  // pvp_career_points			  pp->PVPCareerPoints,					" pvp_career_points,         "
		"%u,"  // pvp_best_kill_streak		  pp->PVPBestKillStreak,				" pvp_best_kill_streak,      "
		"%u,"  // pvp_worst_death_streak	  pp->PVPWorstDeathStreak,				" pvp_worst_death_streak,    "
		"%u,"  // pvp_current_kill_streak	  pp->PVPCurrentKillStreak,				" pvp_current_kill_streak,   "
		"%u,"  // aa_points_spent			  pp->aapoints_spent,					" aa_points_spent,           "
		"%u,"  // aa_exp					  pp->expAA,							" aa_exp,                    "
		"%u,"  // aa_points					  pp->aapoints,							" aa_points,                 "
		"%u,"  // group_auto_consent		  pp->groupAutoconsent,					" group_auto_consent,        "
		"%u,"  // raid_auto_consent			  pp->raidAutoconsent,					" raid_auto_consent,         "
		"%u,"  // guild_auto_consent		  pp->guildAutoconsent,					" guild_auto_consent,        "
		"%u,"  // RestTimer					  pp->RestTimer,						" RestTimer)                 "
		"%u,"  // e_aa_effects
		"%u,"  // e_percent_to_aa
		"%u,"  // e_expended_aa_spent
		"%u,"  // e_last_invsnapshot
		"'%s'" // mailkey					  mail_key
		")",
		character_id,					  // " id,                        "
		account_id,						  // " account_id,                "
		EscapeString(pp->name).c_str(),						  // " `name`,                    "
		EscapeString(pp->last_name).c_str(),					  // " last_name,                 "
		pp->gender,						  // " gender,                    "
		pp->race,						  // " race,                      "
		pp->class_,						  // " class,                     "
		pp->level,						  // " `level`,                   "
		pp->deity,						  // " deity,                     "
		pp->birthday,					  // " birthday,                  "
		pp->lastlogin,					  // " last_login,                "
		pp->timePlayedMin,				  // " time_played,               "
		pp->pvp,						  // " pvp_status,                "
		pp->level2,						  // " level2,                    "
		pp->anon,						  // " anon,                      "
		pp->gm,							  // " gm,                        "
		pp->intoxication,				  // " intoxication,              "
		pp->haircolor,					  // " hair_color,                "
		pp->beardcolor,					  // " beard_color,               "
		pp->eyecolor1,					  // " eye_color_1,               "
		pp->eyecolor2,					  // " eye_color_2,               "
		pp->hairstyle,					  // " hair_style,                "
		pp->beard,						  // " beard,                     "
		pp->ability_time_seconds,		  // " ability_time_seconds,      "
		pp->ability_number,				  // " ability_number,            "
		pp->ability_time_minutes,		  // " ability_time_minutes,      "
		pp->ability_time_hours,			  // " ability_time_hours,        "
		EscapeString(pp->title).c_str(),						  // " title,                     "
		EscapeString(pp->suffix).c_str(),						  // " suffix,                    "
		pp->exp,						  // " exp,                       "
		pp->points,						  // " points,                    "
		pp->mana,						  // " mana,                      "
		pp->cur_hp,						  // " cur_hp,                    "
		pp->STR,						  // " str,                       "
		pp->STA,						  // " sta,                       "
		pp->CHA,						  // " cha,                       "
		pp->DEX,						  // " dex,                       "
		pp->INT,						  // " `int`,                     "
		pp->AGI,						  // " agi,                       "
		pp->WIS,						  // " wis,                       "
		pp->face,						  // " face,                      "
		pp->y,							  // " y,                         "
		pp->x,							  // " x,                         "
		pp->z,							  // " z,                         "
		pp->heading,					  // " heading,                   "
		pp->pvp2,						  // " pvp2,                      "
		pp->pvptype,					  // " pvp_type,                  "
		pp->autosplit,					  // " autosplit_enabled,         "
		pp->zone_change_count,			  // " zone_change_count,         "
		pp->drakkin_heritage,			  // " drakkin_heritage,          "
		pp->drakkin_tattoo,				  // " drakkin_tattoo,            "
		pp->drakkin_details,			  // " drakkin_details,           "
		pp->toxicity,					  // " toxicity,                  "
		pp->hunger_level,				  // " hunger_level,              "
		pp->thirst_level,				  // " thirst_level,              "
		pp->ability_up,					  // " ability_up,                "
		pp->zone_id,					  // " zone_id,                   "
		pp->zoneInstance,				  // " zone_instance,             "
		pp->leadAAActive,				  // " leadership_exp_on,         "
		pp->ldon_points_guk,			  // " ldon_points_guk,           "
		pp->ldon_points_mir,			  // " ldon_points_mir,           "
		pp->ldon_points_mmc,			  // " ldon_points_mmc,           "
		pp->ldon_points_ruj,			  // " ldon_points_ruj,           "
		pp->ldon_points_tak,			  // " ldon_points_tak,           "
		pp->ldon_points_available,		  // " ldon_points_available,     "
		pp->tribute_time_remaining,		  // " tribute_time_remaining,    "
		pp->showhelm,					  // " show_helm,                 "
		pp->career_tribute_points,		  // " career_tribute_points,     "
		pp->tribute_points,				  // " tribute_points,            "
		pp->tribute_active,				  // " tribute_active,            "
		pp->endurance,					  // " endurance,                 "
		pp->group_leadership_exp,		  // " group_leadership_exp,      "
		pp->raid_leadership_exp,		  // " raid_leadership_exp,       "
		pp->group_leadership_points,	  // " group_leadership_points,   "
		pp->raid_leadership_points,		  // " raid_leadership_points,    "
		pp->air_remaining,				  // " air_remaining,             "
		pp->PVPKills,					  // " pvp_kills,                 "
		pp->PVPDeaths,					  // " pvp_deaths,                "
		pp->PVPCurrentPoints,			  // " pvp_current_points,        "
		pp->PVPCareerPoints,			  // " pvp_career_points,         "
		pp->PVPBestKillStreak,			  // " pvp_best_kill_streak,      "
		pp->PVPWorstDeathStreak,		  // " pvp_worst_death_streak,    "
		pp->PVPCurrentKillStreak,		  // " pvp_current_kill_streak,   "
		pp->aapoints_spent,				  // " aa_points_spent,           "
		pp->expAA,						  // " aa_exp,                    "
		pp->aapoints,					  // " aa_points,                 "
		pp->groupAutoconsent,			  // " group_auto_consent,        "
		pp->raidAutoconsent,			  // " raid_auto_consent,         "
		pp->guildAutoconsent,			  // " guild_auto_consent,        "
		pp->RestTimer,					  // " RestTimer)                 "
		m_epp->aa_effects,
		m_epp->perAA,
		m_epp->expended_aa,
		m_epp->last_invsnapshot_time,
		mail_key.c_str()
	);
	auto results = database.QueryDatabase(query);
	Log(Logs::General, Logs::None, "ZoneDatabase::SaveCharacterData %i, done... Took %f seconds", character_id, ((float)(std::clock() - t)) / CLOCKS_PER_SEC);
	return true;
}

bool ZoneDatabase::SaveCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp){
	if (pp->copper < 0) { pp->copper = 0; }
	if (pp->silver < 0) { pp->silver = 0; }
	if (pp->gold < 0) { pp->gold = 0; }
	if (pp->platinum < 0) { pp->platinum = 0; }
	if (pp->copper_bank < 0) { pp->copper_bank = 0; }
	if (pp->silver_bank < 0) { pp->silver_bank = 0; }
	if (pp->gold_bank < 0) { pp->gold_bank = 0; }
	if (pp->platinum_bank < 0) { pp->platinum_bank = 0; }
	if (pp->platinum_cursor < 0) { pp->platinum_cursor = 0; }
	if (pp->gold_cursor < 0) { pp->gold_cursor = 0; }
	if (pp->silver_cursor < 0) { pp->silver_cursor = 0; }
	if (pp->copper_cursor < 0) { pp->copper_cursor = 0; }
	std::string query = StringFormat(
		"REPLACE INTO `character_currency` (id, platinum, gold, silver, copper,"
		"platinum_bank, gold_bank, silver_bank, copper_bank,"
		"platinum_cursor, gold_cursor, silver_cursor, copper_cursor, "
		"radiant_crystals, career_radiant_crystals, ebon_crystals, career_ebon_crystals)"
		"VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u)",
		character_id,
		pp->platinum,
		pp->gold,
		pp->silver,
		pp->copper,
		pp->platinum_bank,
		pp->gold_bank,
		pp->silver_bank,
		pp->copper_bank,
		pp->platinum_cursor,
		pp->gold_cursor,
		pp->silver_cursor,
		pp->copper_cursor,
		pp->currentRadCrystals,
		pp->careerRadCrystals,
		pp->currentEbonCrystals,
		pp->careerEbonCrystals);
	auto results = database.QueryDatabase(query);
	Log(Logs::General, Logs::None, "Saving Currency for character ID: %i, done", character_id);
	return true;
}

bool ZoneDatabase::SaveCharacterAA(uint32 character_id, uint32 aa_id, uint32 current_level, uint32 charges){
	std::string rquery = StringFormat("REPLACE INTO `character_alternate_abilities` (id, aa_id, aa_value, charges)"
		" VALUES (%u, %u, %u, %u)",
		character_id, aa_id, current_level, charges);
	auto results = QueryDatabase(rquery);
	Log(Logs::General, Logs::None, "Saving AA for character ID: %u, aa_id: %u current_level: %u", character_id, aa_id, current_level);
	return true;
}

bool ZoneDatabase::SaveCharacterMemorizedSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	if (spell_id > SPDAT_RECORDS){ return false; }
	std::string query = StringFormat("REPLACE INTO `character_memmed_spells` (id, slot_id, spell_id) VALUES (%u, %u, %u)", character_id, slot_id, spell_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::SaveCharacterSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	if (spell_id > SPDAT_RECORDS){ return false; }
	std::string query = StringFormat("REPLACE INTO `character_spells` (id, slot_id, spell_id) VALUES (%u, %u, %u)", character_id, slot_id, spell_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	std::string query = StringFormat("DELETE FROM `character_spells` WHERE `slot_id` = %u AND `id` = %u", slot_id, character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterDisc(uint32 character_id, uint32 slot_id){
	std::string query = StringFormat("DELETE FROM `character_disciplines` WHERE `slot_id` = %u AND `id` = %u", slot_id, character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterBandolier(uint32 character_id, uint32 band_id){
	std::string query = StringFormat("DELETE FROM `character_bandolier` WHERE `bandolier_id` = %u AND `id` = %u", band_id, character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterLeadershipAAs(uint32 character_id){
	std::string query = StringFormat("DELETE FROM `character_leadership_abilities` WHERE `id` = %u", character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterAAs(uint32 character_id){
	std::string query = StringFormat("DELETE FROM `character_alternate_abilities` WHERE `id` = %u", character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterDye(uint32 character_id){
	std::string query = StringFormat("DELETE FROM `character_material` WHERE `id` = %u", character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::DeleteCharacterMemorizedSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	std::string query = StringFormat("DELETE FROM `character_memmed_spells` WHERE `slot_id` = %u AND `id` = %u", slot_id, character_id);
	QueryDatabase(query);
	return true;
}

bool ZoneDatabase::NoRentExpired(const char* name){
	std::string query = StringFormat("SELECT (UNIX_TIMESTAMP(NOW()) - last_login) FROM `character_data` WHERE name = '%s'", name);
	auto results = QueryDatabase(query);
	if (!results.Success())
        return false;

    if (results.RowCount() != 1)
        return false;

	auto row = results.begin();
	uint32 seconds = atoi(row[0]);

	return (seconds>1800);
}

const NPCType* ZoneDatabase::LoadNPCTypesData(uint32 npc_type_id, bool bulk_load /*= false*/)
{
	const NPCType *npc = nullptr;

	/* If there is a cached NPC entry, load it */
	auto itr = zone->npctable.find(npc_type_id);
	if(itr != zone->npctable.end())
		return itr->second;

	std::string where_condition = "";

	if (bulk_load){
		Log(Logs::General, Logs::Debug, "Performing bulk NPC Types load");
		where_condition = StringFormat(
			"INNER JOIN spawnentry ON npc_types.id = spawnentry.npcID "
			"INNER JOIN spawn2 ON spawnentry.spawngroupID = spawn2.spawngroupID "
			"WHERE spawn2.zone = '%s' and spawn2.version = %u GROUP BY npc_types.id", zone->GetShortName(), zone->GetInstanceVersion());
	}
	else{
		where_condition = StringFormat("WHERE id = %u", npc_type_id);
	}

	std::string query = StringFormat("SELECT "
		"npc_types.id, "
		"npc_types.name, "
		"npc_types.level, "
		"npc_types.race, "
		"npc_types.class, "
		"npc_types.hp, "
		"npc_types.mana, "
		"npc_types.gender, "
		"npc_types.texture, "
		"npc_types.helmtexture, "
		"npc_types.herosforgemodel, "
		"npc_types.size, "
		"npc_types.loottable_id, "
		"npc_types.merchant_id, "
		"npc_types.alt_currency_id, "
		"npc_types.adventure_template_id, "
		"npc_types.trap_template, "
		"npc_types.attack_speed, "
		"npc_types.STR, "
		"npc_types.STA, "
		"npc_types.DEX, "
		"npc_types.AGI, "
		"npc_types._INT, "
		"npc_types.WIS, "
		"npc_types.CHA, "
		"npc_types.MR, "
		"npc_types.CR, "
		"npc_types.DR, "
		"npc_types.FR, "
		"npc_types.PR, "
		"npc_types.Corrup, "
		"npc_types.PhR, "
		"npc_types.mindmg, "
		"npc_types.maxdmg, "
		"npc_types.attack_count, "
		"npc_types.special_abilities, "
		"npc_types.npc_spells_id, "
		"npc_types.npc_spells_effects_id, "
		"npc_types.d_melee_texture1, "
		"npc_types.d_melee_texture2, "
		"npc_types.ammo_idfile, "
		"npc_types.prim_melee_type, "
		"npc_types.sec_melee_type, "
		"npc_types.ranged_type, "
		"npc_types.runspeed, "
		"npc_types.findable, "
		"npc_types.trackable, "
		"npc_types.hp_regen_rate, "
		"npc_types.mana_regen_rate, "
		"npc_types.aggroradius, "
		"npc_types.assistradius, "
		"npc_types.bodytype, "
		"npc_types.npc_faction_id, "
		"npc_types.face, "
		"npc_types.luclin_hairstyle, "
		"npc_types.luclin_haircolor, "
		"npc_types.luclin_eyecolor, "
		"npc_types.luclin_eyecolor2, "
		"npc_types.luclin_beardcolor, "
		"npc_types.luclin_beard, "
		"npc_types.drakkin_heritage, "
		"npc_types.drakkin_tattoo, "
		"npc_types.drakkin_details, "
		"npc_types.armortint_id, "
		"npc_types.armortint_red, "
		"npc_types.armortint_green, "
		"npc_types.armortint_blue, "
		"npc_types.see_invis, "
		"npc_types.see_invis_undead, "
		"npc_types.lastname, "
		"npc_types.qglobal, "
		"npc_types.AC, "
		"npc_types.npc_aggro, "
		"npc_types.spawn_limit, "
		"npc_types.see_hide, "
		"npc_types.see_improved_hide, "
		"npc_types.ATK, "
		"npc_types.Accuracy, "
		"npc_types.Avoidance, "
		"npc_types.slow_mitigation, "
		"npc_types.maxlevel, "
		"npc_types.scalerate, "
		"npc_types.private_corpse, "
		"npc_types.unique_spawn_by_name, "
		"npc_types.underwater, "
		"npc_types.emoteid, "
		"npc_types.spellscale, "
		"npc_types.healscale, "
		"npc_types.no_target_hotkey, "
		"npc_types.raid_target, "
		"npc_types.attack_delay, "
		"npc_types.light, "
		"npc_types.armtexture, "
		"npc_types.bracertexture, "
		"npc_types.handtexture, "
		"npc_types.legtexture, "
		"npc_types.feettexture, "
		"npc_types.ignore_despawn, "
		"npc_types.show_name, "
		"npc_types.untargetable, "
		"npc_types.charm_ac, "
		"npc_types.charm_min_dmg, "
		"npc_types.charm_max_dmg, "
		"npc_types.charm_attack_delay, "
		"npc_types.charm_accuracy_rating, "
		"npc_types.charm_avoidance_rating, "
		"npc_types.charm_atk, "
		"npc_types.skip_global_loot, "
		"npc_types.rare_spawn "
		"FROM npc_types %s",
		where_condition.c_str()
	);

    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return nullptr;
    }

    for (auto row = results.begin(); row != results.end(); ++row) {
		NPCType *temp_npctype_data;
		temp_npctype_data = new NPCType;
		memset (temp_npctype_data, 0, sizeof *temp_npctype_data);

		temp_npctype_data->npc_id = atoi(row[0]);

		strn0cpy(temp_npctype_data->name, row[1], 50);

		temp_npctype_data->level = atoi(row[2]);
		temp_npctype_data->race = atoi(row[3]);
		temp_npctype_data->class_ = atoi(row[4]);
		temp_npctype_data->max_hp = atoi(row[5]);
		temp_npctype_data->cur_hp = temp_npctype_data->max_hp;
		temp_npctype_data->Mana = atoi(row[6]);
		temp_npctype_data->gender = atoi(row[7]);
		temp_npctype_data->texture = atoi(row[8]);
		temp_npctype_data->helmtexture = atoi(row[9]);
		temp_npctype_data->herosforgemodel = atoul(row[10]);
		temp_npctype_data->size = atof(row[11]);
        temp_npctype_data->loottable_id = atoi(row[12]);
		temp_npctype_data->merchanttype = atoi(row[13]);
		temp_npctype_data->alt_currency_type = atoi(row[14]);
		temp_npctype_data->adventure_template = atoi(row[15]);
		temp_npctype_data->trap_template = atoi(row[16]);
		temp_npctype_data->attack_speed = atof(row[17]);
		temp_npctype_data->STR = atoi(row[18]);
		temp_npctype_data->STA = atoi(row[19]);
		temp_npctype_data->DEX = atoi(row[20]);
		temp_npctype_data->AGI = atoi(row[21]);
		temp_npctype_data->INT = atoi(row[22]);
		temp_npctype_data->WIS = atoi(row[23]);
		temp_npctype_data->CHA = atoi(row[24]);
		temp_npctype_data->MR = atoi(row[25]);
		temp_npctype_data->CR = atoi(row[26]);
		temp_npctype_data->DR = atoi(row[27]);
		temp_npctype_data->FR = atoi(row[28]);
		temp_npctype_data->PR = atoi(row[29]);
		temp_npctype_data->Corrup = atoi(row[30]);
		temp_npctype_data->PhR = atoi(row[31]);
		temp_npctype_data->min_dmg = atoi(row[32]);
		temp_npctype_data->max_dmg = atoi(row[33]);
		temp_npctype_data->attack_count = atoi(row[34]);

		if (row[35] != nullptr)
			strn0cpy(temp_npctype_data->special_abilities, row[35], 512);
		else
			temp_npctype_data->special_abilities[0] = '\0';

		temp_npctype_data->npc_spells_id = atoi(row[36]);
		temp_npctype_data->npc_spells_effects_id = atoi(row[37]);
		temp_npctype_data->d_melee_texture1 = atoi(row[38]);
		temp_npctype_data->d_melee_texture2 = atoi(row[39]);
		strn0cpy(temp_npctype_data->ammo_idfile, row[40], 30);
		temp_npctype_data->prim_melee_type = atoi(row[41]);
		temp_npctype_data->sec_melee_type = atoi(row[42]);
		temp_npctype_data->ranged_type = atoi(row[43]);
		temp_npctype_data->runspeed= atof(row[44]);
		temp_npctype_data->findable = atoi(row[45]) == 0? false : true;
		temp_npctype_data->trackable = atoi(row[46]) == 0? false : true;
		temp_npctype_data->hp_regen = atoi(row[47]);
		temp_npctype_data->mana_regen = atoi(row[48]);

		// set default value for aggroradius
        temp_npctype_data->aggroradius = (int32)atoi(row[49]);
		if (temp_npctype_data->aggroradius <= 0)
			temp_npctype_data->aggroradius = 70;

		temp_npctype_data->assistradius = (int32)atoi(row[50]);
		if (temp_npctype_data->assistradius <= 0)
			temp_npctype_data->assistradius = temp_npctype_data->aggroradius;

		if (row[51] && strlen(row[51]))
            temp_npctype_data->bodytype = (uint8)atoi(row[51]);
        else
            temp_npctype_data->bodytype = 0;

		temp_npctype_data->npc_faction_id = atoi(row[52]);

		temp_npctype_data->luclinface = atoi(row[53]);
		temp_npctype_data->hairstyle = atoi(row[54]);
		temp_npctype_data->haircolor = atoi(row[55]);
		temp_npctype_data->eyecolor1 = atoi(row[56]);
		temp_npctype_data->eyecolor2 = atoi(row[57]);
		temp_npctype_data->beardcolor = atoi(row[58]);
		temp_npctype_data->beard = atoi(row[59]);
		temp_npctype_data->drakkin_heritage = atoi(row[60]);
		temp_npctype_data->drakkin_tattoo = atoi(row[61]);
		temp_npctype_data->drakkin_details = atoi(row[62]);

		uint32 armor_tint_id = atoi(row[63]);

		temp_npctype_data->armor_tint.Head.Color = (atoi(row[64]) & 0xFF) << 16;
        temp_npctype_data->armor_tint.Head.Color |= (atoi(row[65]) & 0xFF) << 8;
		temp_npctype_data->armor_tint.Head.Color |= (atoi(row[66]) & 0xFF);
		temp_npctype_data->armor_tint.Head.Color |= (temp_npctype_data->armor_tint.Head.Color) ? (0xFF << 24) : 0;

		if (armor_tint_id != 0) {
			std::string armortint_query = StringFormat(
				"SELECT red1h, grn1h, blu1h, "
				"red2c, grn2c, blu2c, "
				"red3a, grn3a, blu3a, "
				"red4b, grn4b, blu4b, "
				"red5g, grn5g, blu5g, "
				"red6l, grn6l, blu6l, "
				"red7f, grn7f, blu7f, "
				"red8x, grn8x, blu8x, "
				"red9x, grn9x, blu9x "
				"FROM npc_types_tint WHERE id = %d",
				armor_tint_id);
            auto armortint_results = QueryDatabase(armortint_query);
            if (!armortint_results.Success() || armortint_results.RowCount() == 0)
                armor_tint_id = 0;
            else {
                auto armorTint_row = armortint_results.begin();

				for (int index = EQEmu::textures::textureBegin; index <= EQEmu::textures::LastTexture; index++) {
                    temp_npctype_data->armor_tint.Slot[index].Color = atoi(armorTint_row[index * 3]) << 16;
					temp_npctype_data->armor_tint.Slot[index].Color |= atoi(armorTint_row[index * 3 + 1]) << 8;
					temp_npctype_data->armor_tint.Slot[index].Color |= atoi(armorTint_row[index * 3 + 2]);
					temp_npctype_data->armor_tint.Slot[index].Color |= (temp_npctype_data->armor_tint.Slot[index].Color) ? (0xFF << 24) : 0;
                }
            }
        }
		// Try loading npc_types tint fields if armor tint is 0 or query failed to get results
		if (armor_tint_id == 0) {
			for (int index = EQEmu::textures::armorChest; index < EQEmu::textures::materialCount; index++) {
				temp_npctype_data->armor_tint.Slot[index].Color = temp_npctype_data->armor_tint.Slot[0].Color; // odd way to 'zero-out' the array...
			}
		}

		temp_npctype_data->see_invis = atoi(row[67]);
		temp_npctype_data->see_invis_undead = atoi(row[68]) == 0? false: true;	// Set see_invis_undead flag

		if (row[69] != nullptr)
			strn0cpy(temp_npctype_data->lastname, row[69], 32);

		temp_npctype_data->qglobal = atoi(row[70]) == 0? false: true;	// qglobal
		temp_npctype_data->AC = atoi(row[71]);
		temp_npctype_data->npc_aggro = atoi(row[72]) == 0? false: true;
		temp_npctype_data->spawn_limit = atoi(row[73]);
		temp_npctype_data->see_hide = atoi(row[74]) == 0? false: true;
		temp_npctype_data->see_improved_hide = atoi(row[75]) == 0? false: true;
		temp_npctype_data->ATK = atoi(row[76]);
		temp_npctype_data->accuracy_rating = atoi(row[77]);
		temp_npctype_data->avoidance_rating = atoi(row[78]);
		temp_npctype_data->slow_mitigation = atoi(row[79]);
		temp_npctype_data->maxlevel = atoi(row[80]);
		temp_npctype_data->scalerate = atoi(row[81]);
		temp_npctype_data->private_corpse = atoi(row[82]) == 1 ? true: false;
		temp_npctype_data->unique_spawn_by_name = atoi(row[83]) == 1 ? true: false;
		temp_npctype_data->underwater = atoi(row[84]) == 1 ? true: false;
		temp_npctype_data->emoteid = atoi(row[85]);
		temp_npctype_data->spellscale = atoi(row[86]);
		temp_npctype_data->healscale = atoi(row[87]);
		temp_npctype_data->no_target_hotkey = atoi(row[88]) == 1 ? true: false;
		temp_npctype_data->raid_target = atoi(row[89]) == 0 ? false: true;
		temp_npctype_data->attack_delay = atoi(row[90]) * 100; // TODO: fix DB
		temp_npctype_data->light = (atoi(row[91]) & 0x0F);

		temp_npctype_data->armtexture = atoi(row[92]);
		temp_npctype_data->bracertexture = atoi(row[93]);
		temp_npctype_data->handtexture = atoi(row[94]);
		temp_npctype_data->legtexture = atoi(row[95]);
		temp_npctype_data->feettexture = atoi(row[96]);
		temp_npctype_data->ignore_despawn = atoi(row[97]) == 1 ? true : false;
		temp_npctype_data->show_name = atoi(row[98]) != 0 ? true : false;
		temp_npctype_data->untargetable = atoi(row[99]) != 0 ? true : false;

		temp_npctype_data->charm_ac = atoi(row[100]);
		temp_npctype_data->charm_min_dmg = atoi(row[101]);
		temp_npctype_data->charm_max_dmg = atoi(row[102]);
		temp_npctype_data->charm_attack_delay = atoi(row[103]) * 100; // TODO: fix DB
		temp_npctype_data->charm_accuracy_rating = atoi(row[104]);
		temp_npctype_data->charm_avoidance_rating = atoi(row[105]);
		temp_npctype_data->charm_atk = atoi(row[106]);

		temp_npctype_data->skip_global_loot = atoi(row[107]) != 0;
		temp_npctype_data->rare_spawn = atoi(row[108]) != 0;

		// If NPC with duplicate NPC id already in table,
		// free item we attempted to add.
		if (zone->npctable.find(temp_npctype_data->npc_id) != zone->npctable.end()) {
			std::cerr << "Error loading duplicate NPC " << temp_npctype_data->npc_id << std::endl;
			delete temp_npctype_data;
			return nullptr;
		}

        zone->npctable[temp_npctype_data->npc_id] = temp_npctype_data;
        npc = temp_npctype_data;
    }

	return npc;
}

const NPCType* ZoneDatabase::GetMercType(uint32 id, uint16 raceid, uint32 clientlevel)
{
	//need to save based on merc_npc_type & client level
	uint32 merc_type_id = id * 100 + clientlevel;

	// If Merc is already in tree, return it.
	auto itr = zone->merctable.find(merc_type_id);
	if(itr != zone->merctable.end())
		return itr->second;

	//If the id is 0, return nullptr. (sanity check)
	if(id == 0)
        return nullptr;

	// Otherwise, load Merc data on demand
	std::string query = StringFormat("SELECT "
		"m_stats.merc_npc_type_id, "
		"'' AS name, "
		"m_stats.level, "
		"m_types.race_id, "
		"m_subtypes.class_id, "
		"m_stats.hp, "
		"m_stats.mana, "
		"0 AS gender, "
		"m_armorinfo.texture, "
		"m_armorinfo.helmtexture, "
		"m_stats.attack_delay, "
		"m_stats.STR, "
		"m_stats.STA, "
		"m_stats.DEX, "
		"m_stats.AGI, "
		"m_stats._INT, "
		"m_stats.WIS, "
		"m_stats.CHA, "
		"m_stats.MR, "
		"m_stats.CR, "
		"m_stats.DR, "
		"m_stats.FR, "
		"m_stats.PR, "
		"m_stats.Corrup, "
		"m_stats.mindmg, "
		"m_stats.maxdmg, "
		"m_stats.attack_count, "
		"m_stats.special_abilities, "
		"m_weaponinfo.d_melee_texture1, "
		"m_weaponinfo.d_melee_texture2, "
		"m_weaponinfo.prim_melee_type, "
		"m_weaponinfo.sec_melee_type, "
		"m_stats.runspeed, "
		"m_stats.hp_regen_rate, "
		"m_stats.mana_regen_rate, "
		"1 AS bodytype, "
		"m_armorinfo.armortint_id, "
		"m_armorinfo.armortint_red, "
		"m_armorinfo.armortint_green, "
		"m_armorinfo.armortint_blue, "
		"m_stats.AC, "
		"m_stats.ATK, "
		"m_stats.Accuracy, "
		"m_stats.statscale, "
		"m_stats.spellscale, "
		"m_stats.healscale "
		"FROM merc_stats m_stats "
		"INNER JOIN merc_armorinfo m_armorinfo "
		"ON m_stats.merc_npc_type_id = m_armorinfo.merc_npc_type_id "
		"AND m_armorinfo.minlevel <= m_stats.level AND m_armorinfo.maxlevel >= m_stats.level "
		"INNER JOIN merc_weaponinfo m_weaponinfo "
		"ON m_stats.merc_npc_type_id = m_weaponinfo.merc_npc_type_id "
		"AND m_weaponinfo.minlevel <= m_stats.level AND m_weaponinfo.maxlevel >= m_stats.level "
		"INNER JOIN merc_templates m_templates "
		"ON m_templates.merc_npc_type_id = m_stats.merc_npc_type_id "
		"INNER JOIN merc_types m_types "
		"ON m_templates.merc_type_id = m_types.merc_type_id "
		"INNER JOIN merc_subtypes m_subtypes "
		"ON m_templates.merc_subtype_id = m_subtypes.merc_subtype_id "
		"WHERE m_templates.merc_npc_type_id = %d AND m_stats.clientlevel = %d AND m_types.race_id = %d",
		id, clientlevel, raceid); //dual primary keys. one is ID, one is level.

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return nullptr;
	}

	const NPCType *npc = nullptr;

	// Process each row returned.
	for (auto row = results.begin(); row != results.end(); ++row) {
		NPCType *tmpNPCType;
		tmpNPCType = new NPCType;
		memset(tmpNPCType, 0, sizeof *tmpNPCType);

		tmpNPCType->npc_id = atoi(row[0]);

		strn0cpy(tmpNPCType->name, row[1], 50);

		tmpNPCType->level = atoi(row[2]);
		tmpNPCType->race = atoi(row[3]);
		tmpNPCType->class_ = atoi(row[4]);
		tmpNPCType->max_hp = atoi(row[5]);
		tmpNPCType->cur_hp = tmpNPCType->max_hp;
		tmpNPCType->Mana = atoi(row[6]);
		tmpNPCType->gender = atoi(row[7]);
		tmpNPCType->texture = atoi(row[8]);
		tmpNPCType->helmtexture = atoi(row[9]);
		tmpNPCType->attack_delay = atoi(row[10]) * 100; // TODO: fix DB
		tmpNPCType->STR = atoi(row[11]);
		tmpNPCType->STA = atoi(row[12]);
		tmpNPCType->DEX = atoi(row[13]);
		tmpNPCType->AGI = atoi(row[14]);
		tmpNPCType->INT = atoi(row[15]);
		tmpNPCType->WIS = atoi(row[16]);
		tmpNPCType->CHA = atoi(row[17]);
		tmpNPCType->MR = atoi(row[18]);
		tmpNPCType->CR = atoi(row[19]);
		tmpNPCType->DR = atoi(row[20]);
		tmpNPCType->FR = atoi(row[21]);
		tmpNPCType->PR = atoi(row[22]);
		tmpNPCType->Corrup = atoi(row[23]);
		tmpNPCType->min_dmg = atoi(row[24]);
		tmpNPCType->max_dmg = atoi(row[25]);
		tmpNPCType->attack_count = atoi(row[26]);

		if (row[27] != nullptr)
			strn0cpy(tmpNPCType->special_abilities, row[27], 512);
		else
			tmpNPCType->special_abilities[0] = '\0';

		tmpNPCType->d_melee_texture1 = atoi(row[28]);
		tmpNPCType->d_melee_texture2 = atoi(row[29]);
		tmpNPCType->prim_melee_type = atoi(row[30]);
		tmpNPCType->sec_melee_type = atoi(row[31]);
		tmpNPCType->runspeed = atof(row[32]);

		tmpNPCType->hp_regen = atoi(row[33]);
		tmpNPCType->mana_regen = atoi(row[34]);

		tmpNPCType->aggroradius = RuleI(Mercs, AggroRadius);

		if (row[35] && strlen(row[35]))
			tmpNPCType->bodytype = (uint8)atoi(row[35]);
		else
			tmpNPCType->bodytype = 1;

		uint32 armor_tint_id = atoi(row[36]);
		tmpNPCType->armor_tint.Slot[0].Color = (atoi(row[37]) & 0xFF) << 16;
		tmpNPCType->armor_tint.Slot[0].Color |= (atoi(row[38]) & 0xFF) << 8;
		tmpNPCType->armor_tint.Slot[0].Color |= (atoi(row[39]) & 0xFF);
		tmpNPCType->armor_tint.Slot[0].Color |= (tmpNPCType->armor_tint.Slot[0].Color) ? (0xFF << 24) : 0;

		if (armor_tint_id == 0)
			for (int index = EQEmu::textures::armorChest; index <= EQEmu::textures::LastTexture; index++)
				tmpNPCType->armor_tint.Slot[index].Color = tmpNPCType->armor_tint.Slot[0].Color;
		else if (tmpNPCType->armor_tint.Slot[0].Color == 0) {
			std::string armorTint_query = StringFormat("SELECT red1h, grn1h, blu1h, "
								   "red2c, grn2c, blu2c, "
								   "red3a, grn3a, blu3a, "
								   "red4b, grn4b, blu4b, "
								   "red5g, grn5g, blu5g, "
								   "red6l, grn6l, blu6l, "
								   "red7f, grn7f, blu7f, "
								   "red8x, grn8x, blu8x, "
								   "red9x, grn9x, blu9x "
								   "FROM npc_types_tint WHERE id = %d",
								   armor_tint_id);
			auto armorTint_results = QueryDatabase(armorTint_query);
			if (!results.Success() || results.RowCount() == 0)
				armor_tint_id = 0;
			else {
				auto armorTint_row = results.begin();

				for (int index = EQEmu::textures::textureBegin; index <= EQEmu::textures::LastTexture; index++) {
					tmpNPCType->armor_tint.Slot[index].Color = atoi(armorTint_row[index * 3]) << 16;
					tmpNPCType->armor_tint.Slot[index].Color |= atoi(armorTint_row[index * 3 + 1]) << 8;
					tmpNPCType->armor_tint.Slot[index].Color |= atoi(armorTint_row[index * 3 + 2]);
					tmpNPCType->armor_tint.Slot[index].Color |= (tmpNPCType->armor_tint.Slot[index].Color) ? (0xFF << 24) : 0;
				}
			}
		} else
			armor_tint_id = 0;

		tmpNPCType->AC = atoi(row[40]);
		tmpNPCType->ATK = atoi(row[41]);
		tmpNPCType->accuracy_rating = atoi(row[42]);
		tmpNPCType->scalerate = atoi(row[43]);
		tmpNPCType->spellscale = atoi(row[44]);
		tmpNPCType->healscale = atoi(row[45]);

		// If Merc with duplicate NPC id already in table,
		// free item we attempted to add.
		if (zone->merctable.find(merc_type_id) != zone->merctable.end()) {
			delete tmpNPCType;
			return nullptr;
		}

		zone->merctable[merc_type_id] = tmpNPCType;
		npc = tmpNPCType;
	}

	return npc;
}

bool ZoneDatabase::LoadMercInfo(Client *client) {

	std::string query = StringFormat("SELECT MercID, Slot, Name, TemplateID, SuspendedTime, "
                                    "IsSuspended, TimerRemaining, Gender, MercSize, StanceID, HP, Mana, "
                                    "Endurance, Face, LuclinHairStyle, LuclinHairColor, "
                                    "LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, "
                                    "DrakkinHeritage, DrakkinTattoo, DrakkinDetails "
                                    "FROM mercs WHERE OwnerCharacterID = '%i' ORDER BY Slot", client->CharacterID());
    auto results = QueryDatabase(query);
    if (!results.Success())
        return false;

	if(results.RowCount() == 0)
		return false;

    for (auto row = results.begin(); row != results.end(); ++row) {
        uint8 slot = atoi(row[1]);

        if(slot >= MAXMERCS)
            continue;

        client->GetMercInfo(slot).mercid = atoi(row[0]);
        client->GetMercInfo(slot).slot = slot;
        snprintf(client->GetMercInfo(slot).merc_name, 64, "%s", row[2]);
        client->GetMercInfo(slot).MercTemplateID = atoi(row[3]);
        client->GetMercInfo(slot).SuspendedTime = atoi(row[4]);
        client->GetMercInfo(slot).IsSuspended = atoi(row[5]) == 1 ? true : false;
		client->GetMercInfo(slot).MercTimerRemaining = atoi(row[6]);
		client->GetMercInfo(slot).Gender = atoi(row[7]);
		client->GetMercInfo(slot).MercSize = atof(row[8]);
		client->GetMercInfo(slot).State = 5;
		client->GetMercInfo(slot).Stance = atoi(row[9]);
		client->GetMercInfo(slot).hp = atoi(row[10]);
		client->GetMercInfo(slot).mana = atoi(row[11]);
		client->GetMercInfo(slot).endurance = atoi(row[12]);
		client->GetMercInfo(slot).face = atoi(row[13]);
		client->GetMercInfo(slot).luclinHairStyle = atoi(row[14]);
		client->GetMercInfo(slot).luclinHairColor = atoi(row[15]);
		client->GetMercInfo(slot).luclinEyeColor = atoi(row[16]);
		client->GetMercInfo(slot).luclinEyeColor2 = atoi(row[17]);
		client->GetMercInfo(slot).luclinBeardColor = atoi(row[18]);
		client->GetMercInfo(slot).luclinBeard = atoi(row[19]);
		client->GetMercInfo(slot).drakkinHeritage = atoi(row[20]);
		client->GetMercInfo(slot).drakkinTattoo = atoi(row[21]);
		client->GetMercInfo(slot).drakkinDetails = atoi(row[22]);
    }

	return true;
}

bool ZoneDatabase::LoadCurrentMerc(Client *client) {

	uint8 slot = client->GetMercSlot();

	if(slot > MAXMERCS)
        return false;

    std::string query = StringFormat("SELECT MercID, Name, TemplateID, SuspendedTime, "
                                    "IsSuspended, TimerRemaining, Gender, MercSize, StanceID, HP, "
                                    "Mana, Endurance, Face, LuclinHairStyle, LuclinHairColor, "
                                    "LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, "
                                    "LuclinBeard, DrakkinHeritage, DrakkinTattoo, DrakkinDetails "
                                    "FROM mercs WHERE OwnerCharacterID = '%i' AND Slot = '%u'",
                                    client->CharacterID(), slot);
    auto results = database.QueryDatabase(query);

    if(!results.Success())
		return false;

	if(results.RowCount() == 0)
		return false;


    for (auto row = results.begin(); row != results.end(); ++row) {
        client->GetMercInfo(slot).mercid = atoi(row[0]);
        client->GetMercInfo(slot).slot = slot;
        snprintf(client->GetMercInfo(slot).merc_name, 64, "%s", row[1]);
        client->GetMercInfo(slot).MercTemplateID = atoi(row[2]);
        client->GetMercInfo(slot).SuspendedTime = atoi(row[3]);
        client->GetMercInfo(slot).IsSuspended = atoi(row[4]) == 1? true: false;
        client->GetMercInfo(slot).MercTimerRemaining = atoi(row[5]);
		client->GetMercInfo(slot).Gender = atoi(row[6]);
		client->GetMercInfo(slot).MercSize = atof(row[7]);
		client->GetMercInfo(slot).State = atoi(row[8]);
		client->GetMercInfo(slot).hp = atoi(row[9]);
		client->GetMercInfo(slot).mana = atoi(row[10]);
		client->GetMercInfo(slot).endurance = atoi(row[11]);
		client->GetMercInfo(slot).face = atoi(row[12]);
		client->GetMercInfo(slot).luclinHairStyle = atoi(row[13]);
		client->GetMercInfo(slot).luclinHairColor = atoi(row[14]);
		client->GetMercInfo(slot).luclinEyeColor = atoi(row[15]);
		client->GetMercInfo(slot).luclinEyeColor2 = atoi(row[16]);
		client->GetMercInfo(slot).luclinBeardColor = atoi(row[17]);
		client->GetMercInfo(slot).luclinBeard = atoi(row[18]);
		client->GetMercInfo(slot).drakkinHeritage = atoi(row[19]);
		client->GetMercInfo(slot).drakkinTattoo = atoi(row[20]);
		client->GetMercInfo(slot).drakkinDetails = atoi(row[21]);
	}

	return true;
}

bool ZoneDatabase::SaveMerc(Merc *merc) {
	Client *owner = merc->GetMercOwner();

	if(!owner)
		return false;

	if(merc->GetMercID() == 0)
	{
		// New merc record
		std::string query = StringFormat("INSERT INTO mercs "
		"(OwnerCharacterID, Slot, Name, TemplateID, "
		"SuspendedTime, IsSuspended, TimerRemaining, "
		"Gender, MercSize, StanceID, HP, Mana, Endurance, Face, "
		"LuclinHairStyle, LuclinHairColor, LuclinEyeColor, "
		"LuclinEyeColor2, LuclinBeardColor, LuclinBeard, "
		"DrakkinHeritage, DrakkinTattoo, DrakkinDetails) "
		"VALUES('%u', '%u', '%s', '%u', '%u', '%u', '%u', "
		"'%u', '%u', '%f', '%u', '%u', '%u', '%i', '%i', '%i', "
		"'%i', '%i', '%i', '%i', '%i', '%i', '%i')",
		merc->GetMercCharacterID(), owner->GetNumMercs(),
		merc->GetCleanName(), merc->GetMercTemplateID(),
		owner->GetMercInfo().SuspendedTime, merc->IsSuspended(),
		owner->GetMercInfo().MercTimerRemaining, merc->GetGender(),
		merc->GetSize(), merc->GetStance(), merc->GetHP(),
		merc->GetMana(), merc->GetEndurance(), merc->GetLuclinFace(),
		merc->GetHairStyle(), merc->GetHairColor(), merc->GetEyeColor1(),
		merc->GetEyeColor2(), merc->GetBeardColor(),
		merc->GetBeard(), merc->GetDrakkinHeritage(),
		merc->GetDrakkinTattoo(), merc->GetDrakkinDetails());

		auto results = database.QueryDatabase(query);
		if(!results.Success()) {
			owner->Message(13, results.ErrorMessage().c_str());
			return false;
		} else if (results.RowsAffected() != 1) {
			owner->Message(13, "Unable to save merc to the database.");
			return false;
		}

		merc->SetMercID(results.LastInsertedID());
		merc->UpdateMercInfo(owner);
		database.SaveMercBuffs(merc);
		return true;
	}

	// Update existing merc record
	std::string query = StringFormat("UPDATE mercs SET OwnerCharacterID = '%u', Slot = '%u', "
	"Name = '%s', TemplateID = '%u', SuspendedTime = '%u', "
	"IsSuspended = '%u', TimerRemaining = '%u', Gender = '%u', MercSize = '%f', "
	"StanceID = '%u', HP = '%u', Mana = '%u', Endurance = '%u', "
	"Face = '%i', LuclinHairStyle = '%i', LuclinHairColor = '%i', "
	"LuclinEyeColor = '%i', LuclinEyeColor2 = '%i', LuclinBeardColor = '%i', "
	"LuclinBeard = '%i', DrakkinHeritage = '%i', DrakkinTattoo = '%i', "
	"DrakkinDetails = '%i' WHERE MercID = '%u'",
	merc->GetMercCharacterID(), owner->GetMercSlot(), merc->GetCleanName(),
	merc->GetMercTemplateID(), owner->GetMercInfo().SuspendedTime,
	merc->IsSuspended(), owner->GetMercInfo().MercTimerRemaining,
	merc->GetGender(), merc->GetSize(), merc->GetStance(), merc->GetHP(),
	merc->GetMana(), merc->GetEndurance(), merc->GetLuclinFace(),
	merc->GetHairStyle(), merc->GetHairColor(), merc->GetEyeColor1(),
	merc->GetEyeColor2(), merc->GetBeardColor(), merc->GetBeard(),
	merc->GetDrakkinHeritage(), merc->GetDrakkinTattoo(), merc->GetDrakkinDetails(),
	merc->GetMercID());

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		owner->Message(13, results.ErrorMessage().c_str());
		return false;
	} else if (results.RowsAffected() != 1) {
		owner->Message(13, "Unable to save merc to the database.");
		return false;
	}

	merc->UpdateMercInfo(owner);
	database.SaveMercBuffs(merc);

	return true;
}

void ZoneDatabase::SaveMercBuffs(Merc *merc) {

	Buffs_Struct *buffs = merc->GetBuffs();

	// Remove any existing buff saves
    std::string query = StringFormat("DELETE FROM merc_buffs WHERE MercId = %u", merc->GetMercID());
    auto results = database.QueryDatabase(query);
    if(!results.Success()) {
        Log(Logs::General, Logs::Error, "Error While Deleting Merc Buffs before save: %s", results.ErrorMessage().c_str());
        return;
    }

	for (int buffCount = 0; buffCount <= BUFF_COUNT; buffCount++) {
		if(buffs[buffCount].spellid == 0 || buffs[buffCount].spellid == SPELL_UNKNOWN)
            continue;

        int IsPersistent = buffs[buffCount].persistant_buff? 1: 0;

        query = StringFormat("INSERT INTO merc_buffs (MercId, SpellId, CasterLevel, DurationFormula, "
                            "TicsRemaining, PoisonCounters, DiseaseCounters, CurseCounters, "
                            "CorruptionCounters, HitCount, MeleeRune, MagicRune, dot_rune, "
                            "caston_x, Persistent, caston_y, caston_z, ExtraDIChance) "
                            "VALUES (%u, %u, %u, %u, %u, %d, %u, %u, %u, %u, %u, %u, %u, %i, %u, %i, %i, %i);",
                            merc->GetMercID(), buffs[buffCount].spellid, buffs[buffCount].casterlevel,
                            spells[buffs[buffCount].spellid].buffdurationformula, buffs[buffCount].ticsremaining,
                            CalculatePoisonCounters(buffs[buffCount].spellid) > 0 ? buffs[buffCount].counters : 0,
                            CalculateDiseaseCounters(buffs[buffCount].spellid) > 0 ? buffs[buffCount].counters : 0,
                            CalculateCurseCounters(buffs[buffCount].spellid) > 0 ? buffs[buffCount].counters : 0,
                            CalculateCorruptionCounters(buffs[buffCount].spellid) > 0 ? buffs[buffCount].counters : 0,
                            buffs[buffCount].numhits, buffs[buffCount].melee_rune, buffs[buffCount].magic_rune,
                            buffs[buffCount].dot_rune, buffs[buffCount].caston_x, IsPersistent, buffs[buffCount].caston_y,
                            buffs[buffCount].caston_z, buffs[buffCount].ExtraDIChance);
        results = database.QueryDatabase(query);
        if(!results.Success()) {
            Log(Logs::General, Logs::Error, "Error Saving Merc Buffs: %s", results.ErrorMessage().c_str());
            break;
        }
	}
}

void ZoneDatabase::LoadMercBuffs(Merc *merc) {
	Buffs_Struct *buffs = merc->GetBuffs();
	uint32 max_slots = merc->GetMaxBuffSlots();


	bool BuffsLoaded = false;
    std::string query = StringFormat("SELECT SpellId, CasterLevel, DurationFormula, TicsRemaining, "
                                    "PoisonCounters, DiseaseCounters, CurseCounters, CorruptionCounters, "
                                    "HitCount, MeleeRune, MagicRune, dot_rune, caston_x, Persistent, "
                                    "caston_y, caston_z, ExtraDIChance FROM merc_buffs WHERE MercId = %u",
                                    merc->GetMercID());
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		Log(Logs::General, Logs::Error, "Error Loading Merc Buffs: %s", results.ErrorMessage().c_str());
		return;
	}

    int buffCount = 0;
    for (auto row = results.begin(); row != results.end(); ++row, ++buffCount) {
        if(buffCount == BUFF_COUNT)
            break;

        buffs[buffCount].spellid = atoi(row[0]);
        buffs[buffCount].casterlevel = atoi(row[1]);
        buffs[buffCount].ticsremaining = atoi(row[3]);

        if(CalculatePoisonCounters(buffs[buffCount].spellid) > 0)
            buffs[buffCount].counters = atoi(row[4]);

        if(CalculateDiseaseCounters(buffs[buffCount].spellid) > 0)
            buffs[buffCount].counters = atoi(row[5]);

        if(CalculateCurseCounters(buffs[buffCount].spellid) > 0)
            buffs[buffCount].counters = atoi(row[6]);

		if(CalculateCorruptionCounters(buffs[buffCount].spellid) > 0)
            buffs[buffCount].counters = atoi(row[7]);

        buffs[buffCount].numhits = atoi(row[8]);
		buffs[buffCount].melee_rune = atoi(row[9]);
		buffs[buffCount].magic_rune = atoi(row[10]);
		buffs[buffCount].dot_rune = atoi(row[11]);
		buffs[buffCount].caston_x = atoi(row[12]);
		buffs[buffCount].casterid = 0;

        bool IsPersistent = atoi(row[13])? true: false;

        buffs[buffCount].caston_y = atoi(row[13]);
        buffs[buffCount].caston_z = atoi(row[14]);
        buffs[buffCount].ExtraDIChance = atoi(row[15]);

        buffs[buffCount].persistant_buff = IsPersistent;

    }

	query = StringFormat("DELETE FROM merc_buffs WHERE MercId = %u", merc->GetMercID());
    results = database.QueryDatabase(query);
    if(!results.Success())
        Log(Logs::General, Logs::Error, "Error Loading Merc Buffs: %s", results.ErrorMessage().c_str());

}

bool ZoneDatabase::DeleteMerc(uint32 merc_id) {

	if(merc_id == 0)
		return false;

	// TODO: These queries need to be ran together as a transaction.. ie,
	// if one or more fail then they all will fail to commit to the database.
	// ...Not all mercs will have buffs, so why is it required that both deletes succeed?
	std::string query = StringFormat("DELETE FROM merc_buffs WHERE MercId = '%u'", merc_id);
	auto results = database.QueryDatabase(query);
	if(!results.Success())
	{
		Log(Logs::General, Logs::Error, "Error Deleting Merc Buffs: %s", results.ErrorMessage().c_str());
	}

	query = StringFormat("DELETE FROM mercs WHERE MercID = '%u'", merc_id);
	results = database.QueryDatabase(query);
	if(!results.Success())
	{
		Log(Logs::General, Logs::Error, "Error Deleting Merc: %s", results.ErrorMessage().c_str());
		return false;
	}

	return true;
}

void ZoneDatabase::LoadMercEquipment(Merc *merc) {

	std::string query = StringFormat("SELECT item_id FROM merc_inventory "
                                    "WHERE merc_subtype_id = ("
                                    "SELECT merc_subtype_id FROM merc_subtypes "
                                    "WHERE class_id = '%u' AND tier_id = '%u') "
                                    "AND min_level <= %u AND max_level >= %u",
                                    merc->GetClass(), merc->GetTierID(),
                                    merc->GetLevel(), merc->GetLevel());
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		Log(Logs::General, Logs::Error, "Error Loading Merc Inventory: %s", results.ErrorMessage().c_str());
		return;
	}

    int itemCount = 0;
    for(auto row = results.begin(); row != results.end(); ++row) {
		if (itemCount == EQEmu::invslot::EQUIPMENT_COUNT)
            break;

        if(atoi(row[0]) == 0)
            continue;

        merc->AddItem(itemCount, atoi(row[0]));
        itemCount++;
    }
}

uint8 ZoneDatabase::GetGridType(uint32 grid, uint32 zoneid ) {

	std::string query = StringFormat("SELECT type FROM grid WHERE id = %i AND zoneid = %i", grid, zoneid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        return 0;
	}

    if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();

	return atoi(row[0]);
}

void ZoneDatabase::SaveMerchantTemp(uint32 npcid, uint32 slot, uint32 item, uint32 charges){

	std::string query = StringFormat("REPLACE INTO merchantlist_temp (npcid, slot, itemid, charges) "
                                    "VALUES(%d, %d, %d, %d)", npcid, slot, item, charges);
    QueryDatabase(query);
}

void ZoneDatabase::DeleteMerchantTemp(uint32 npcid, uint32 slot){
	std::string query = StringFormat("DELETE FROM merchantlist_temp WHERE npcid=%d AND slot=%d", npcid, slot);
	QueryDatabase(query);
}

bool ZoneDatabase::UpdateZoneSafeCoords(const char* zonename, const glm::vec3& location) {

	std::string query = StringFormat("UPDATE zone SET safe_x='%f', safe_y='%f', safe_z='%f' "
                                    "WHERE short_name='%s';",
                                    location.x, location.y, location.z, zonename);
	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowsAffected() == 0)
		return false;

	return true;
}

uint8 ZoneDatabase::GetUseCFGSafeCoords()
{
	const std::string query = "SELECT value FROM variables WHERE varname='UseCFGSafeCoords'";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

	if (results.RowCount() != 1)
        return 0;

	auto row = results.begin();

    return atoi(row[0]);
}

//New functions for timezone
uint32 ZoneDatabase::GetZoneTZ(uint32 zoneid, uint32 version) {

	std::string query = StringFormat("SELECT timezone FROM zone WHERE zoneidnumber = %i "
                                    "AND (version = %i OR version = 0) ORDER BY version DESC",
                                    zoneid, version);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return 0;
    }

    if (results.RowCount() == 0)
        return 0;

    auto row = results.begin();
    return atoi(row[0]);
}

bool ZoneDatabase::SetZoneTZ(uint32 zoneid, uint32 version, uint32 tz) {

	std::string query = StringFormat("UPDATE zone SET timezone = %i "
                                    "WHERE zoneidnumber = %i AND version = %i",
                                    tz, zoneid, version);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

    return results.RowsAffected() == 1;
}

void ZoneDatabase::RefreshGroupFromDB(Client *client){
	if(!client)
		return;

	Group *group = client->GetGroup();

	if(!group)
		return;

	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = groupActUpdate;

	strcpy(gu->yourname, client->GetName());
	GetGroupLeadershipInfo(group->GetID(), gu->leadersname, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &gu->leader_aas);
	gu->NPCMarkerID = group->GetNPCMarkerID();

	int index = 0;

	std::string query = StringFormat("SELECT name FROM group_id WHERE groupid = %d", group->GetID());
	auto results = QueryDatabase(query);
	if (!results.Success())
	{
	}
	else
	{
		for (auto row = results.begin(); row != results.end(); ++row) {
			if(index >= 6)
				continue;

            if(strcmp(client->GetName(), row[0]) == 0)
				continue;

			strcpy(gu->membername[index], row[0]);
			index++;
		}
	}

	client->QueuePacket(outapp);
	safe_delete(outapp);

	if (client->ClientVersion() >= EQEmu::versions::ClientVersion::SoD) {
		group->NotifyMainTank(client, 1);
		group->NotifyPuller(client, 1);
	}

	group->NotifyMainAssist(client, 1);
	group->NotifyMarkNPC(client);
	group->NotifyAssistTarget(client);
	group->NotifyTankTarget(client);
	group->NotifyPullerTarget(client);
	group->SendMarkedNPCsToMember(client);

}

uint8 ZoneDatabase::GroupCount(uint32 groupid) {

	std::string query = StringFormat("SELECT count(charid) FROM group_id WHERE groupid = %d", groupid);
	auto results = QueryDatabase(query);
    if (!results.Success()) {
        return 0;
    }

    if (results.RowCount() == 0)
        return 0;

    auto row = results.begin();

	return atoi(row[0]);
}

uint8 ZoneDatabase::RaidGroupCount(uint32 raidid, uint32 groupid) {

	std::string query = StringFormat("SELECT count(charid) FROM raid_members "
                                    "WHERE raidid = %d AND groupid = %d;", raidid, groupid);
    auto results = QueryDatabase(query);

    if (!results.Success()) {
        return 0;
    }

    if (results.RowCount() == 0)
        return 0;

    auto row = results.begin();

	return atoi(row[0]);
 }

int32 ZoneDatabase::GetBlockedSpellsCount(uint32 zoneid)
{
	std::string query = StringFormat("SELECT count(*) FROM blocked_spells WHERE zoneid = %d", zoneid);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return -1;
	}

	if (results.RowCount() == 0)
        return -1;

    auto row = results.begin();

	return atoi(row[0]);
}

bool ZoneDatabase::LoadBlockedSpells(int32 blockedSpellsCount, ZoneSpellsBlocked* into, uint32 zoneid)
{
	Log(Logs::General, Logs::Status, "Loading Blocked Spells from database...");

	std::string query = StringFormat("SELECT id, spellid, type, x, y, z, x_diff, y_diff, z_diff, message "
                                    "FROM blocked_spells WHERE zoneid = %d ORDER BY id ASC", zoneid);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

    if (results.RowCount() == 0)
		return true;

    int32 index = 0;
    for(auto row = results.begin(); row != results.end(); ++row, ++index) {
        if(index >= blockedSpellsCount) {
            std::cerr << "Error, Blocked Spells Count of " << blockedSpellsCount << " exceeded." << std::endl;
            break;
        }

        memset(&into[index], 0, sizeof(ZoneSpellsBlocked));
        into[index].spellid = atoi(row[1]);
        into[index].type = atoi(row[2]);
        into[index].m_Location = glm::vec3(atof(row[3]), atof(row[4]), atof(row[5]));
        into[index].m_Difference = glm::vec3(atof(row[6]), atof(row[7]), atof(row[8]));
        strn0cpy(into[index].message, row[9], 255);
    }

	return true;
}

int ZoneDatabase::getZoneShutDownDelay(uint32 zoneID, uint32 version)
{
	std::string query = StringFormat("SELECT shutdowndelay FROM zone "
                                    "WHERE zoneidnumber = %i AND (version=%i OR version=0) "
                                    "ORDER BY version DESC", zoneID, version);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return (RuleI(Zone, AutoShutdownDelay));
    }

    if (results.RowCount() == 0) {
        std::cerr << "Error in getZoneShutDownDelay no result '" << query << "' " << std::endl;
        return (RuleI(Zone, AutoShutdownDelay));
    }

    auto row = results.begin();

    return atoi(row[0]);
}

uint32 ZoneDatabase::GetKarma(uint32 acct_id)
{
    std::string query = StringFormat("SELECT `karma` FROM `account` WHERE `id` = '%i' LIMIT 1", acct_id);
    auto results = QueryDatabase(query);
	if (!results.Success())
		return 0;

	for (auto row = results.begin(); row != results.end(); ++row) {
		return atoi(row[0]);
	}

	return 0;
}

void ZoneDatabase::UpdateKarma(uint32 acct_id, uint32 amount)
{
	std::string query = StringFormat("UPDATE account SET karma = %i WHERE id = %i", amount, acct_id);
    QueryDatabase(query);
}

void ZoneDatabase::ListAllInstances(Client* client, uint32 charid)
{
	if(!client)
		return;

	std::string query = StringFormat("SELECT instance_list.id, zone, version "
                                    "FROM instance_list JOIN instance_list_player "
                                    "ON instance_list.id = instance_list_player.id "
                                    "WHERE instance_list_player.charid = %lu",
                                    (unsigned long)charid);
    auto results = QueryDatabase(query);
    if (!results.Success())
        return;

    char name[64];
    database.GetCharName(charid, name);
    client->Message(0, "%s is part of the following instances:", name);

    for (auto row = results.begin(); row != results.end(); ++row) {
        client->Message(0, "%s - id: %lu, version: %lu", database.GetZoneName(atoi(row[1])),
				(unsigned long)atoi(row[0]), (unsigned long)atoi(row[2]));
    }
}

void ZoneDatabase::QGlobalPurge()
{
	const std::string query = "DELETE FROM quest_globals WHERE expdate < UNIX_TIMESTAMP()";
	database.QueryDatabase(query);
}

void ZoneDatabase::InsertDoor(uint32 ddoordbid, uint16 ddoorid, const char* ddoor_name, const glm::vec4& position, uint8 dopentype, uint16 dguildid, uint32 dlockpick, uint32 dkeyitem, uint8 ddoor_param, uint8 dinvert, int dincline, uint16 dsize, bool ddisabletimer){

	std::string query = StringFormat("REPLACE INTO doors (id, doorid, zone, version, name, "
                                    "pos_x, pos_y, pos_z, heading, opentype, guild, lockpick, "
                                    "keyitem, disable_timer, door_param, invert_state, incline, size) "
                                    "VALUES('%i', '%i', '%s', '%i', '%s', '%f', '%f', "
                                    "'%f', '%f', '%i', '%i', '%i', '%i', '%i', '%i', '%i', '%i', '%i')",
                                    ddoordbid, ddoorid, zone->GetShortName(), zone->GetInstanceVersion(),
                                    ddoor_name, position.x, position.y, position.z, position.w,
									dopentype, dguildid, dlockpick, dkeyitem, (ddisabletimer ? 1 : 0), ddoor_param, dinvert, dincline, dsize);
    QueryDatabase(query);
}

void ZoneDatabase::LoadAltCurrencyValues(uint32 char_id, std::map<uint32, uint32> &currency) {

	std::string query = StringFormat("SELECT currency_id, amount "
                                    "FROM character_alt_currency "
                                    "WHERE char_id = '%u'", char_id);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
        currency[atoi(row[0])] = atoi(row[1]);

}

void ZoneDatabase::UpdateAltCurrencyValue(uint32 char_id, uint32 currency_id, uint32 value) {

	std::string query = StringFormat("REPLACE INTO character_alt_currency (char_id, currency_id, amount) "
                                    "VALUES('%u', '%u', '%u')", char_id, currency_id, value);
	database.QueryDatabase(query);

}

void ZoneDatabase::SaveBuffs(Client *client) {

	std::string query = StringFormat("DELETE FROM `character_buffs` WHERE `character_id` = '%u'", client->CharacterID());
	database.QueryDatabase(query);

	uint32 buff_count = client->GetMaxBuffSlots();
	Buffs_Struct *buffs = client->GetBuffs();

	for (int index = 0; index < buff_count; index++) {
		if(buffs[index].spellid == SPELL_UNKNOWN)
            continue;

		query = StringFormat("INSERT INTO `character_buffs` (character_id, slot_id, spell_id, "
                            "caster_level, caster_name, ticsremaining, counters, numhits, melee_rune, "
                            "magic_rune, persistent, dot_rune, caston_x, caston_y, caston_z, ExtraDIChance, "
							"instrument_mod) "
                            "VALUES('%u', '%u', '%u', '%u', '%s', '%d', '%u', '%u', '%u', '%u', '%u', '%u', "
                            "'%i', '%i', '%i', '%i', '%i')", client->CharacterID(), index, buffs[index].spellid,
                            buffs[index].casterlevel, buffs[index].caster_name, buffs[index].ticsremaining,
                            buffs[index].counters, buffs[index].numhits, buffs[index].melee_rune,
                            buffs[index].magic_rune, buffs[index].persistant_buff, buffs[index].dot_rune,
                            buffs[index].caston_x, buffs[index].caston_y, buffs[index].caston_z,
                            buffs[index].ExtraDIChance, buffs[index].instrument_mod);
       QueryDatabase(query);
	}
}

void ZoneDatabase::LoadBuffs(Client *client)
{

	Buffs_Struct *buffs = client->GetBuffs();
	uint32 max_slots = client->GetMaxBuffSlots();

	for (int index = 0; index < max_slots; ++index)
		buffs[index].spellid = SPELL_UNKNOWN;

	std::string query = StringFormat("SELECT spell_id, slot_id, caster_level, caster_name, ticsremaining, "
					 "counters, numhits, melee_rune, magic_rune, persistent, dot_rune, "
					 "caston_x, caston_y, caston_z, ExtraDIChance, instrument_mod "
					 "FROM `character_buffs` WHERE `character_id` = '%u'",
					 client->CharacterID());
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		uint32 slot_id = atoul(row[1]);
		if (slot_id >= client->GetMaxBuffSlots())
			continue;

		uint32 spell_id = atoul(row[0]);
		if (!IsValidSpell(spell_id))
			continue;

		Client *caster = entity_list.GetClientByName(row[3]);
		uint32 caster_level = atoi(row[2]);
		int32 ticsremaining = atoi(row[4]);
		uint32 counters = atoul(row[5]);
		uint32 numhits = atoul(row[6]);
		uint32 melee_rune = atoul(row[7]);
		uint32 magic_rune = atoul(row[8]);
		uint8 persistent = atoul(row[9]);
		uint32 dot_rune = atoul(row[10]);
		int32 caston_x = atoul(row[11]);
		int32 caston_y = atoul(row[12]);
		int32 caston_z = atoul(row[13]);
		int32 ExtraDIChance = atoul(row[14]);
		uint32 instrument_mod = atoul(row[15]);

		buffs[slot_id].spellid = spell_id;
		buffs[slot_id].casterlevel = caster_level;

		if (caster) {
			buffs[slot_id].casterid = caster->GetID();
			strcpy(buffs[slot_id].caster_name, caster->GetName());
			buffs[slot_id].client = true;
		} else {
			buffs[slot_id].casterid = 0;
			strcpy(buffs[slot_id].caster_name, "");
			buffs[slot_id].client = false;
		}

		buffs[slot_id].ticsremaining = ticsremaining;
		buffs[slot_id].counters = counters;
		buffs[slot_id].numhits = numhits;
		buffs[slot_id].melee_rune = melee_rune;
		buffs[slot_id].magic_rune = magic_rune;
		buffs[slot_id].persistant_buff = persistent ? true : false;
		buffs[slot_id].dot_rune = dot_rune;
		buffs[slot_id].caston_x = caston_x;
		buffs[slot_id].caston_y = caston_y;
		buffs[slot_id].caston_z = caston_z;
		buffs[slot_id].ExtraDIChance = ExtraDIChance;
		buffs[slot_id].RootBreakChance = 0;
		buffs[slot_id].UpdateClient = false;
		buffs[slot_id].instrument_mod = instrument_mod;
	}

	// We load up to the most our client supports
	max_slots = EQEmu::constants::Lookup(client->ClientVersion())->LongBuffs;
	for (int index = 0; index < max_slots; ++index) {
		if (!IsValidSpell(buffs[index].spellid))
			continue;

		for (int effectIndex = 0; effectIndex < 12; ++effectIndex) {

			if (spells[buffs[index].spellid].effectid[effectIndex] == SE_Charm) {
				buffs[index].spellid = SPELL_UNKNOWN;
				break;
			}

			if (spells[buffs[index].spellid].effectid[effectIndex] == SE_Illusion) {
				if (buffs[index].persistant_buff)
					break;

				buffs[index].spellid = SPELL_UNKNOWN;
				break;
			}
		}
	}
}

void ZoneDatabase::SaveAuras(Client *c)
{
	auto query = StringFormat("DELETE FROM `character_auras` WHERE `id` = %u", c->CharacterID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	const auto &auras = c->GetAuraMgr();
	for (int i = 0; i < auras.count; ++i) {
		auto aura = auras.auras[i].aura;
		if (aura && aura->AuraZones()) {
			query = StringFormat("INSERT INTO `character_auras` (id, slot, spell_id) VALUES(%u, %d, %d)",
					     c->CharacterID(), i, aura->GetAuraID());
			auto results = database.QueryDatabase(query);
			if (!results.Success())
				return;
		}
	}
}

void ZoneDatabase::LoadAuras(Client *c)
{
	auto query = StringFormat("SELECT `spell_id` FROM `character_auras` WHERE `id` = %u ORDER BY `slot`", c->CharacterID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	for (auto row = results.begin(); row != results.end(); ++row)
		c->MakeAura(atoi(row[0]));
}

void ZoneDatabase::SavePetInfo(Client *client)
{
	PetInfo *petinfo = nullptr;

	std::string query = StringFormat("DELETE FROM `character_pet_buffs` WHERE `char_id` = %u", client->CharacterID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	query = StringFormat("DELETE FROM `character_pet_inventory` WHERE `char_id` = %u", client->CharacterID());
	results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	for (int pet = 0; pet < 2; pet++) {
		petinfo = client->GetPetInfo(pet);
		if (!petinfo)
			continue;

		query = StringFormat("INSERT INTO `character_pet_info` "
				"(`char_id`, `pet`, `petname`, `petpower`, `spell_id`, `hp`, `mana`, `size`) "
				"VALUES (%u, %u, '%s', %i, %u, %u, %u, %f) "
				"ON DUPLICATE KEY UPDATE `petname` = '%s', `petpower` = %i, `spell_id` = %u, "
				"`hp` = %u, `mana` = %u, `size` = %f",
				client->CharacterID(), pet, petinfo->Name, petinfo->petpower, petinfo->SpellID,
				petinfo->HP, petinfo->Mana, petinfo->size, // and now the ON DUPLICATE ENTRIES
				petinfo->Name, petinfo->petpower, petinfo->SpellID, petinfo->HP, petinfo->Mana, petinfo->size);
		results = database.QueryDatabase(query);
		if (!results.Success())
			return;
		query.clear();

		// pet buffs!
		int max_slots = RuleI(Spells, MaxTotalSlotsPET);
		for (int index = 0; index < max_slots; index++) {
			if (petinfo->Buffs[index].spellid == SPELL_UNKNOWN || petinfo->Buffs[index].spellid == 0)
				continue;
			if (query.length() == 0)
				query = StringFormat("INSERT INTO `character_pet_buffs` "
						"(`char_id`, `pet`, `slot`, `spell_id`, `caster_level`, "
						"`ticsremaining`, `counters`, `instrument_mod`) "
						"VALUES (%u, %u, %u, %u, %u, %d, %d, %u)",
						client->CharacterID(), pet, index, petinfo->Buffs[index].spellid,
						petinfo->Buffs[index].level, petinfo->Buffs[index].duration,
						petinfo->Buffs[index].counters, petinfo->Buffs[index].bard_modifier);
			else
				query += StringFormat(", (%u, %u, %u, %u, %u, %d, %d, %u)",
						client->CharacterID(), pet, index, petinfo->Buffs[index].spellid,
						petinfo->Buffs[index].level, petinfo->Buffs[index].duration,
						petinfo->Buffs[index].counters, petinfo->Buffs[index].bard_modifier);
		}
		database.QueryDatabase(query);
		query.clear();

		// pet inventory!
		for (int index = EQEmu::invslot::EQUIPMENT_BEGIN; index <= EQEmu::invslot::EQUIPMENT_END; index++) {
			if (!petinfo->Items[index])
				continue;

			if (query.length() == 0)
				query = StringFormat("INSERT INTO `character_pet_inventory` "
						"(`char_id`, `pet`, `slot`, `item_id`) "
						"VALUES (%u, %u, %u, %u)",
						client->CharacterID(), pet, index, petinfo->Items[index]);
			else
				query += StringFormat(", (%u, %u, %u, %u)", client->CharacterID(), pet, index, petinfo->Items[index]);
		}
		database.QueryDatabase(query);
	}
}

void ZoneDatabase::RemoveTempFactions(Client *client) {

	std::string query = StringFormat("DELETE FROM faction_values "
                                    "WHERE temp = 1 AND char_id = %u",
                                    client->CharacterID());
	QueryDatabase(query);
}

void ZoneDatabase::UpdateItemRecastTimestamps(uint32 char_id, uint32 recast_type, uint32 timestamp)
{
	std::string query =
	    StringFormat("REPLACE INTO character_item_recast (id, recast_type, timestamp) VALUES (%u, %u, %u)", char_id,
			 recast_type, timestamp);
	QueryDatabase(query);
}

void ZoneDatabase::LoadPetInfo(Client *client)
{

	// Load current pet and suspended pet
	PetInfo *petinfo = client->GetPetInfo(0);
	PetInfo *suspended = client->GetPetInfo(1);

	memset(petinfo, 0, sizeof(PetInfo));
	memset(suspended, 0, sizeof(PetInfo));

	std::string query = StringFormat("SELECT `pet`, `petname`, `petpower`, `spell_id`, "
					 "`hp`, `mana`, `size` FROM `character_pet_info` "
					 "WHERE `char_id` = %u",
					 client->CharacterID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	PetInfo *pi;
	for (auto row = results.begin(); row != results.end(); ++row) {
		uint16 pet = atoi(row[0]);

		if (pet == 0)
			pi = petinfo;
		else if (pet == 1)
			pi = suspended;
		else
			continue;

		strncpy(pi->Name, row[1], 64);
		pi->petpower = atoi(row[2]);
		pi->SpellID = atoi(row[3]);
		pi->HP = atoul(row[4]);
		pi->Mana = atoul(row[5]);
		pi->size = atof(row[6]);
	}

	query = StringFormat("SELECT `pet`, `slot`, `spell_id`, `caster_level`, `castername`, "
			     "`ticsremaining`, `counters`, `instrument_mod` FROM `character_pet_buffs` "
			     "WHERE `char_id` = %u",
			     client->CharacterID());
	results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		uint16 pet = atoi(row[0]);
		if (pet == 0)
			pi = petinfo;
		else if (pet == 1)
			pi = suspended;
		else
			continue;

		uint32 slot_id = atoul(row[1]);
		if (slot_id >= RuleI(Spells, MaxTotalSlotsPET))
			continue;

		uint32 spell_id = atoul(row[2]);
		if (!IsValidSpell(spell_id))
			continue;

		uint32 caster_level = atoi(row[3]);
		int caster_id = 0;
		// The castername field is currently unused
		int32 ticsremaining = atoi(row[5]);
		uint32 counters = atoul(row[6]);
		uint8 bard_mod = atoul(row[7]);

		pi->Buffs[slot_id].spellid = spell_id;
		pi->Buffs[slot_id].level = caster_level;
		pi->Buffs[slot_id].player_id = caster_id;
		pi->Buffs[slot_id].effect_type = 2; // Always 2 in buffs struct for real buffs

		pi->Buffs[slot_id].duration = ticsremaining;
		pi->Buffs[slot_id].counters = counters;
		pi->Buffs[slot_id].bard_modifier = bard_mod;
	}

	query = StringFormat("SELECT `pet`, `slot`, `item_id` "
			     "FROM `character_pet_inventory` "
			     "WHERE `char_id`=%u",
			     client->CharacterID());
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		uint16 pet = atoi(row[0]);
		if (pet == 0)
			pi = petinfo;
		else if (pet == 1)
			pi = suspended;
		else
			continue;

		int slot = atoi(row[1]);
		if (slot < EQEmu::invslot::EQUIPMENT_BEGIN || slot > EQEmu::invslot::EQUIPMENT_END)
			continue;

		pi->Items[slot] = atoul(row[2]);
	}
}

bool ZoneDatabase::GetFactionData(FactionMods* fm, uint32 class_mod, uint32 race_mod, uint32 deity_mod, int32 faction_id) {
	if (faction_id <= 0 || faction_id > (int32) max_faction)
		return false;

	if (faction_array[faction_id] == 0){
		return false;
	}

	fm->base = faction_array[faction_id]->base;

	if(class_mod > 0) {
		char str[32];
		sprintf(str, "c%u", class_mod);

		std::map<std::string, int16>::const_iterator iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->class_mod = iter->second;
		} else {
			fm->class_mod = 0;
		}
	} else {
		fm->class_mod = 0;
	}

	if(race_mod > 0) {
		char str[32];
		sprintf(str, "r%u", race_mod);

		auto iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->race_mod = iter->second;
		} else {
			fm->race_mod = 0;
		}
	} else {
		fm->race_mod = 0;
	}

	if(deity_mod > 0) {
		char str[32];
		sprintf(str, "d%u", deity_mod);

		auto iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->deity_mod = iter->second;
		} else {
			fm->deity_mod = 0;
		}
	} else {
		fm->deity_mod = 0;
	}

	return true;
}

//o--------------------------------------------------------------
//| Name: GetFactionName; Dec. 16
//o--------------------------------------------------------------
//| Notes: Retrieves the name of the specified faction .Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::GetFactionName(int32 faction_id, char* name, uint32 buflen) {
	if ((faction_id <= 0) || faction_id > int32(max_faction) ||(faction_array[faction_id] == 0))
		return false;
	if (faction_array[faction_id]->name[0] != 0) {
		strn0cpy(name, faction_array[faction_id]->name, buflen);
		return true;
	}
	return false;

}

//o--------------------------------------------------------------
//| Name: GetNPCFactionList; Dec. 16, 2001
//o--------------------------------------------------------------
//| Purpose: Gets a list of faction_id's and values bound to the npc_id. Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::GetNPCFactionList(uint32 npcfaction_id, int32* faction_id, int32* value, uint8* temp, int32* primary_faction) {
	if (npcfaction_id <= 0) {
		if (primary_faction)
			*primary_faction = npcfaction_id;
		return true;
	}
	const NPCFactionList* nfl = GetNPCFactionEntry(npcfaction_id);
	if (!nfl)
		return false;
	if (primary_faction)
		*primary_faction = nfl->primaryfaction;
	for (int i=0; i<MAX_NPC_FACTIONS; i++) {
		faction_id[i] = nfl->factionid[i];
		value[i] = nfl->factionvalue[i];
		temp[i] = nfl->factiontemp[i];
	}
	return true;
}

//o--------------------------------------------------------------
//| Name: SetCharacterFactionLevel; Dec. 20, 2001
//o--------------------------------------------------------------
//| Purpose: Update characters faction level with specified faction_id to specified value. Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::SetCharacterFactionLevel(uint32 char_id, int32 faction_id, int32 value, uint8 temp, faction_map &val_list)
{

	std::string query;

	if(temp == 2)
		temp = 0;

	if(temp == 3)
		temp = 1;

	query = StringFormat("INSERT INTO `faction_values` "
						"(`char_id`, `faction_id`, `current_value`, `temp`) "
						"VALUES (%i, %i, %i, %i) "
						"ON DUPLICATE KEY UPDATE `current_value`=%i,`temp`=%i",
						char_id, faction_id, value, temp, value, temp);
    auto results = QueryDatabase(query);

	if (!results.Success())
		return false;
	else
		val_list[faction_id] = value;

	return true;
}

bool ZoneDatabase::LoadFactionData()
{
	std::string query = "SELECT MAX(id) FROM faction_list";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

    if (results.RowCount() == 0)
        return false;

    auto row = results.begin();

	max_faction = row[0] ? atoi(row[0]) : 0;
    faction_array = new Faction*[max_faction+1];
    for(unsigned int index=0; index<max_faction; index++)
        faction_array[index] = nullptr;

    query = "SELECT id, name, base FROM faction_list";
    results = QueryDatabase(query);
    if (!results.Success()) {
        return false;
    }

    for (row = results.begin(); row != results.end(); ++row) {
        uint32 index = atoi(row[0]);
		faction_array[index] = new Faction;
		strn0cpy(faction_array[index]->name, row[1], 50);
		faction_array[index]->base = atoi(row[2]);

        query = StringFormat("SELECT `mod`, `mod_name` FROM `faction_list_mod` WHERE faction_id = %u", index);
        auto modResults = QueryDatabase(query);
        if (!modResults.Success())
            continue;

		for (auto modRow = modResults.begin(); modRow != modResults.end(); ++modRow)
            faction_array[index]->mods[modRow[1]] = atoi(modRow[0]);
    }

	return true;
}

bool ZoneDatabase::GetFactionIdsForNPC(uint32 nfl_id, std::list<struct NPCFaction*> *faction_list, int32* primary_faction) {
	if (nfl_id <= 0) {
		std::list<struct NPCFaction*>::iterator cur,end;
		cur = faction_list->begin();
		end = faction_list->end();
		for(; cur != end; ++cur) {
			struct NPCFaction* tmp = *cur;
			safe_delete(tmp);
		}

		faction_list->clear();
		if (primary_faction)
			*primary_faction = nfl_id;
		return true;
	}
	const NPCFactionList* nfl = GetNPCFactionEntry(nfl_id);
	if (!nfl)
		return false;
	if (primary_faction)
		*primary_faction = nfl->primaryfaction;

	std::list<struct NPCFaction*>::iterator cur,end;
	cur = faction_list->begin();
	end = faction_list->end();
	for(; cur != end; ++cur) {
		struct NPCFaction* tmp = *cur;
		safe_delete(tmp);
	}
	faction_list->clear();
	for (int i=0; i<MAX_NPC_FACTIONS; i++) {
		struct NPCFaction *pFac;
		if (nfl->factionid[i]) {
			pFac = new struct NPCFaction;
			pFac->factionID = nfl->factionid[i];
			pFac->value_mod = nfl->factionvalue[i];
			pFac->npc_value = nfl->factionnpcvalue[i];
			pFac->temp = nfl->factiontemp[i];
			faction_list->push_back(pFac);
		}
	}
	return true;
}

/*  Corpse Queries */

bool ZoneDatabase::DeleteGraveyard(uint32 zone_id, uint32 graveyard_id) {
	std::string query = StringFormat( "UPDATE `zone` SET `graveyard_id` = 0 WHERE `zone_idnumber` = %u AND `version` = 0", zone_id);
	auto results = QueryDatabase(query);

	query = StringFormat("DELETE FROM `graveyard` WHERE `id` = %u",  graveyard_id);
	auto results2 = QueryDatabase(query);

	if (results.Success() && results2.Success()){
		return true;
	}

	return false;
}

uint32 ZoneDatabase::AddGraveyardIDToZone(uint32 zone_id, uint32 graveyard_id) {
	std::string query = StringFormat(
		"UPDATE `zone` SET `graveyard_id` = %u WHERE `zone_idnumber` = %u AND `version` = 0",
		graveyard_id, zone_id
	);
	auto results = QueryDatabase(query);
	return zone_id;
}

uint32 ZoneDatabase::CreateGraveyardRecord(uint32 graveyard_zone_id, const glm::vec4& position) {
	std::string query = StringFormat("INSERT INTO `graveyard` "
                                    "SET `zone_id` = %u, `x` = %1.1f, `y` = %1.1f, `z` = %1.1f, `heading` = %1.1f",
                                    graveyard_zone_id, position.x, position.y, position.z, position.w);
	auto results = QueryDatabase(query);
	if (results.Success())
		return results.LastInsertedID();

	return 0;
}
uint32 ZoneDatabase::SendCharacterCorpseToGraveyard(uint32 dbid, uint32 zone_id, uint16 instance_id, const glm::vec4& position) {

	double xcorpse = (position.x + zone->random.Real(-20,20));
	double ycorpse = (position.y + zone->random.Real(-20,20));

	std::string query = StringFormat("UPDATE `character_corpses` "
                                    "SET `zone_id` = %u, `instance_id` = 0, "
                                    "`x` = %1.1f, `y` = %1.1f, `z` = %1.1f, `heading` = %1.1f, "
                                    "`was_at_graveyard` = 1 "
                                    "WHERE `id` = %d",
                                    zone_id, xcorpse, ycorpse, position.z, position.w, dbid);
	QueryDatabase(query);
	return dbid;
}

uint32 ZoneDatabase::GetCharacterCorpseDecayTimer(uint32 corpse_db_id){
	std::string query = StringFormat("SELECT(UNIX_TIMESTAMP() - UNIX_TIMESTAMP(time_of_death)) FROM `character_corpses` WHERE `id` = %d AND NOT `time_of_death` = 0", corpse_db_id);
	auto results = QueryDatabase(query);
	auto row = results.begin();
	if (results.Success() && results.RowsAffected() != 0)
		return atoul(row[0]);

	return 0;
}

uint32 ZoneDatabase::UpdateCharacterCorpse(uint32 db_id, uint32 char_id, const char* char_name, uint32 zone_id, uint16 instance_id, PlayerCorpse_Struct* dbpc, const glm::vec4& position, bool is_rezzed) {
	std::string query = StringFormat("UPDATE `character_corpses` "
                                    "SET `charname` = '%s', `zone_id` = %u, `instance_id` = %u, `charid` = %d, "
                                    "`x` = %1.1f,`y` =	%1.1f,`z` =	%1.1f, `heading` = %1.1f, "
                                    "`is_locked` = %d, `exp` = %u, `size` = %f, `level` = %u, "
                                    "`race` = %u, `gender` = %u, `class` = %u, `deity` = %u, "
                                    "`texture` = %u, `helm_texture` = %u, `copper` = %u, "
                                    "`silver` = %u, `gold` = %u, `platinum` = %u, `hair_color`  = %u, "
                                    "`beard_color` = %u, `eye_color_1` = %u, `eye_color_2` = %u, "
                                    "`hair_style`  = %u, `face` = %u, `beard` = %u, `drakkin_heritage` = %u, "
                                    "`drakkin_tattoo`  = %u, `drakkin_details` = %u, `wc_1` = %u, "
                                    "`wc_2` = %u, `wc_3` = %u, `wc_4` = %u, `wc_5` = %u, `wc_6` = %u, "
                                    "`wc_7` = %u, `wc_8` = %u, `wc_9` = %u "
                                    "WHERE `id` = %u",
                                    EscapeString(char_name).c_str(), zone_id, instance_id, char_id,
                                    position.x, position.y, position.z, position.w,
                                    dbpc->locked, dbpc->exp, dbpc->size, dbpc->level, dbpc->race,
                                    dbpc->gender, dbpc->class_, dbpc->deity, dbpc->texture,
                                    dbpc->helmtexture, dbpc->copper, dbpc->silver, dbpc->gold,
                                    dbpc->plat, dbpc->haircolor, dbpc->beardcolor, dbpc->eyecolor1,
                                    dbpc->eyecolor2, dbpc->hairstyle, dbpc->face, dbpc->beard,
                                    dbpc->drakkin_heritage, dbpc->drakkin_tattoo, dbpc->drakkin_details,
                                    dbpc->item_tint.Head.Color, dbpc->item_tint.Chest.Color, dbpc->item_tint.Arms.Color,
                                    dbpc->item_tint.Wrist.Color, dbpc->item_tint.Hands.Color, dbpc->item_tint.Legs.Color,
                                    dbpc->item_tint.Feet.Color, dbpc->item_tint.Primary.Color, dbpc->item_tint.Secondary.Color,
                                    db_id);
	auto results = QueryDatabase(query);

	return db_id;
}

void ZoneDatabase::MarkCorpseAsRezzed(uint32 db_id) {
	std::string query = StringFormat("UPDATE `character_corpses` SET `is_rezzed` = 1 WHERE `id` = %i", db_id);
	auto results = QueryDatabase(query);
}

uint32 ZoneDatabase::SaveCharacterCorpse(uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, PlayerCorpse_Struct* dbpc, const glm::vec4& position) {
	/* Dump Basic Corpse Data */
	std::string query = StringFormat(
		"INSERT INTO `character_corpses` "
		"SET `charname` = '%s',  "
		"`zone_id` =	%u,  "
		"`instance_id` =	%u,  "
		"`charid` = %d, "
		"`x` =	%1.1f,  "
		"`y` = %1.1f,  "
		"`z` = %1.1f,  "
		"`heading` = %1.1f, "
		"`time_of_death` = NOW(),  "
		"`is_buried` =	0,  "
		"`is_locked` = %d, "
		"`exp` = %u,  "
		"`size` = %f,  "
		"`level` = %u,  "
		"`race` = %u,  "
		"`gender` = %u, "
		"`class` = %u,  "
		"`deity` = %u,  "
		"`texture` = %u,  "
		"`helm_texture` = %u, "
		"`copper` = %u,  "
		"`silver` = %u, "
		"`gold` = %u, "
		"`platinum` = %u, "
		"`hair_color`  = %u, "
		"`beard_color` = %u, "
		"`eye_color_1` = %u, "
		"`eye_color_2` = %u, "
		"`hair_style`  = %u, "
		"`face` = %u, "
		"`beard` = %u, "
		"`drakkin_heritage` = %u, "
		"`drakkin_tattoo` = %u, "
		"`drakkin_details` = %u, "
		"`wc_1` = %u, "
		"`wc_2` = %u, "
		"`wc_3` = %u, "
		"`wc_4` = %u, "
		"`wc_5` = %u, "
		"`wc_6` = %u, "
		"`wc_7` = %u, "
		"`wc_8` = %u, "
		"`wc_9`	= %u ",
		EscapeString(charname).c_str(),
		zoneid,
		instanceid,
		charid,
		position.x,
		position.y,
		position.z,
		position.w,
		dbpc->locked,
		dbpc->exp,
		dbpc->size,
		dbpc->level,
		dbpc->race,
		dbpc->gender,
		dbpc->class_,
		dbpc->deity,
		dbpc->texture,
		dbpc->helmtexture,
		dbpc->copper,
		dbpc->silver,
		dbpc->gold,
		dbpc->plat,
		dbpc->haircolor,
		dbpc->beardcolor,
		dbpc->eyecolor1,
		dbpc->eyecolor2,
		dbpc->hairstyle,
		dbpc->face,
		dbpc->beard,
		dbpc->drakkin_heritage,
		dbpc->drakkin_tattoo,
		dbpc->drakkin_details,
		dbpc->item_tint.Head.Color,
		dbpc->item_tint.Chest.Color,
		dbpc->item_tint.Arms.Color,
		dbpc->item_tint.Wrist.Color,
		dbpc->item_tint.Hands.Color,
		dbpc->item_tint.Legs.Color,
		dbpc->item_tint.Feet.Color,
		dbpc->item_tint.Primary.Color,
		dbpc->item_tint.Secondary.Color
	);
	auto results = QueryDatabase(query);
	uint32 last_insert_id = results.LastInsertedID();

	std::string corpse_items_query;
	/* Dump Items from Inventory */
	uint8 first_entry = 0;
	for (unsigned int i = 0; i < dbpc->itemcount; i++) {
		if (first_entry != 1){
			corpse_items_query = StringFormat("REPLACE INTO `character_corpse_items` \n"
				" (corpse_id, equip_slot, item_id, charges, aug_1, aug_2, aug_3, aug_4, aug_5, aug_6, attuned) \n"
				" VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u) \n",
				last_insert_id,
				dbpc->items[i].equip_slot,
				dbpc->items[i].item_id,
				dbpc->items[i].charges,
				dbpc->items[i].aug_1,
				dbpc->items[i].aug_2,
				dbpc->items[i].aug_3,
				dbpc->items[i].aug_4,
				dbpc->items[i].aug_5,
				dbpc->items[i].aug_6,
				dbpc->items[i].attuned
			);
			first_entry = 1;
		}
		else{
			corpse_items_query = corpse_items_query + StringFormat(", (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u) \n",
				last_insert_id,
				dbpc->items[i].equip_slot,
				dbpc->items[i].item_id,
				dbpc->items[i].charges,
				dbpc->items[i].aug_1,
				dbpc->items[i].aug_2,
				dbpc->items[i].aug_3,
				dbpc->items[i].aug_4,
				dbpc->items[i].aug_5,
				dbpc->items[i].aug_6,
				dbpc->items[i].attuned
			);
		}
	}
	if (!corpse_items_query.empty())
		QueryDatabase(corpse_items_query);

	return last_insert_id;
}

uint32 ZoneDatabase::GetCharacterBuriedCorpseCount(uint32 char_id) {
	std::string query = StringFormat("SELECT COUNT(*) FROM `character_corpses` WHERE `charid` = '%u' AND `is_buried` = 1", char_id);
	auto results = QueryDatabase(query);

	for (auto row = results.begin(); row != results.end(); ++row) {
		return atoi(row[0]);
	}
	return 0;
}

uint32 ZoneDatabase::GetCharacterCorpseCount(uint32 char_id) {
	std::string query = StringFormat("SELECT COUNT(*) FROM `character_corpses` WHERE `charid` = '%u'", char_id);
	auto results = QueryDatabase(query);

	for (auto row = results.begin(); row != results.end(); ++row) {
		return atoi(row[0]);
	}
	return 0;
}

uint32 ZoneDatabase::GetCharacterCorpseID(uint32 char_id, uint8 corpse) {
	std::string query = StringFormat("SELECT `id` FROM `character_corpses` WHERE `charid` = '%u'", char_id);
	auto results = QueryDatabase(query);

	for (auto row = results.begin(); row != results.end(); ++row) {
		for (int i = 0; i < corpse; i++) {
			return atoul(row[0]);
		}
	}
	return 0;
}

uint32 ZoneDatabase::GetCharacterCorpseItemCount(uint32 corpse_id){
	std::string query = StringFormat("SELECT COUNT(*) FROM character_corpse_items WHERE `corpse_id` = %u",
		corpse_id
	);
	auto results = QueryDatabase(query);
	auto row = results.begin();
	if (results.Success() && results.RowsAffected() != 0){
		return atoi(row[0]);
	}
	return 0;
}

uint32 ZoneDatabase::GetCharacterCorpseItemAt(uint32 corpse_id, uint16 slotid) {
	Corpse* tmp = LoadCharacterCorpse(corpse_id);
	uint32 itemid = 0;

	if (tmp) {
		itemid = tmp->GetWornItem(slotid);
		tmp->DepopPlayerCorpse();
	}
	return itemid;
}

bool ZoneDatabase::LoadCharacterCorpseData(uint32 corpse_id, PlayerCorpse_Struct* pcs){
	std::string query = StringFormat(
		"SELECT           \n"
		"is_locked,       \n"
		"exp,             \n"
		"size,            \n"
		"`level`,         \n"
		"race,            \n"
		"gender,          \n"
		"class,           \n"
		"deity,           \n"
		"texture,         \n"
		"helm_texture,    \n"
		"copper,          \n"
		"silver,          \n"
		"gold,            \n"
		"platinum,        \n"
		"hair_color,      \n"
		"beard_color,     \n"
		"eye_color_1,     \n"
		"eye_color_2,     \n"
		"hair_style,      \n"
		"face,            \n"
		"beard,           \n"
		"drakkin_heritage,\n"
		"drakkin_tattoo,  \n"
		"drakkin_details, \n"
		"wc_1,            \n"
		"wc_2,            \n"
		"wc_3,            \n"
		"wc_4,            \n"
		"wc_5,            \n"
		"wc_6,            \n"
		"wc_7,            \n"
		"wc_8,            \n"
		"wc_9             \n"
		"FROM             \n"
		"character_corpses\n"
		"WHERE `id` = %u  LIMIT 1\n",
		corpse_id
	);
	auto results = QueryDatabase(query);
	uint16 i = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		pcs->locked = atoi(row[i++]);						// is_locked,
		pcs->exp = atoul(row[i++]);							// exp,
		pcs->size = atoi(row[i++]);							// size,
		pcs->level = atoi(row[i++]);						// `level`,
		pcs->race = atoi(row[i++]);							// race,
		pcs->gender = atoi(row[i++]);						// gender,
		pcs->class_ = atoi(row[i++]);						// class,
		pcs->deity = atoi(row[i++]);						// deity,
		pcs->texture = atoi(row[i++]);						// texture,
		pcs->helmtexture = atoi(row[i++]);					// helm_texture,
		pcs->copper = atoul(row[i++]);						// copper,
		pcs->silver = atoul(row[i++]);						// silver,
		pcs->gold = atoul(row[i++]);						// gold,
		pcs->plat = atoul(row[i++]);						// platinum,
		pcs->haircolor = atoi(row[i++]);					// hair_color,
		pcs->beardcolor = atoi(row[i++]);					// beard_color,
		pcs->eyecolor1 = atoi(row[i++]);					// eye_color_1,
		pcs->eyecolor2 = atoi(row[i++]);					// eye_color_2,
		pcs->hairstyle = atoi(row[i++]);					// hair_style,
		pcs->face = atoi(row[i++]);							// face,
		pcs->beard = atoi(row[i++]);						// beard,
		pcs->drakkin_heritage = atoul(row[i++]);			// drakkin_heritage,
		pcs->drakkin_tattoo = atoul(row[i++]);				// drakkin_tattoo,
		pcs->drakkin_details = atoul(row[i++]);				// drakkin_details,
		pcs->item_tint.Head.Color = atoul(row[i++]);		// wc_1,
		pcs->item_tint.Chest.Color = atoul(row[i++]);		// wc_2,
		pcs->item_tint.Arms.Color = atoul(row[i++]);		// wc_3,
		pcs->item_tint.Wrist.Color = atoul(row[i++]);		// wc_4,
		pcs->item_tint.Hands.Color = atoul(row[i++]);		// wc_5,
		pcs->item_tint.Legs.Color = atoul(row[i++]);		// wc_6,
		pcs->item_tint.Feet.Color = atoul(row[i++]);		// wc_7,
		pcs->item_tint.Primary.Color = atoul(row[i++]);		// wc_8,
		pcs->item_tint.Secondary.Color = atoul(row[i++]);	// wc_9
	}
	query = StringFormat(
		"SELECT                       \n"
		"equip_slot,                  \n"
		"item_id,                     \n"
		"charges,                     \n"
		"aug_1,                       \n"
		"aug_2,                       \n"
		"aug_3,                       \n"
		"aug_4,                       \n"
		"aug_5,                       \n"
		"aug_6,                       \n"
		"attuned                      \n"
		"FROM                         \n"
		"character_corpse_items       \n"
		"WHERE `corpse_id` = %u\n"
		,
		corpse_id
	);
	results = QueryDatabase(query);

	i = 0;
	pcs->itemcount = results.RowCount();
	uint16 r = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		memset(&pcs->items[i], 0, sizeof (player_lootitem::ServerLootItem_Struct));
		pcs->items[i].equip_slot = atoi(row[r++]);		// equip_slot,
		pcs->items[i].item_id = atoul(row[r++]); 		// item_id,
		pcs->items[i].charges = atoi(row[r++]); 		// charges,
		pcs->items[i].aug_1 = atoi(row[r++]); 			// aug_1,
		pcs->items[i].aug_2 = atoi(row[r++]); 			// aug_2,
		pcs->items[i].aug_3 = atoi(row[r++]); 			// aug_3,
		pcs->items[i].aug_4 = atoi(row[r++]); 			// aug_4,
		pcs->items[i].aug_5 = atoi(row[r++]); 			// aug_5,
		pcs->items[i].aug_6 = atoi(row[r++]); 			// aug_6,
		pcs->items[i].attuned = atoi(row[r++]); 		// attuned,
		r = 0;
		i++;
	}

	return true;
}

Corpse* ZoneDatabase::SummonBuriedCharacterCorpses(uint32 char_id, uint32 dest_zone_id, uint16 dest_instance_id, const glm::vec4& position) {
	Corpse* corpse = nullptr;
	std::string query = StringFormat("SELECT `id`, `charname`, `time_of_death`, `is_rezzed` "
                                    "FROM `character_corpses` "
                                    "WHERE `charid` = '%u' AND `is_buried` = 1 "
                                    "ORDER BY `time_of_death` LIMIT 1",
                                    char_id);
	auto results = QueryDatabase(query);

	for (auto row = results.begin(); row != results.end(); ++row) {
		corpse = Corpse::LoadCharacterCorpseEntity(
			atoul(row[0]), 			 // uint32 in_dbid
			char_id, 				 // uint32 in_charid
			row[1], 				 // char* in_charname
			position,
			row[2], 				 // char* time_of_death
			atoi(row[3]) == 1, 		 // bool rezzed
			false					 // bool was_at_graveyard
		);
		if (!corpse)
            continue;

        entity_list.AddCorpse(corpse);
        corpse->SetDecayTimer(RuleI(Character, CorpseDecayTimeMS));
        corpse->Spawn();
        if (!UnburyCharacterCorpse(corpse->GetCorpseDBID(), dest_zone_id, dest_instance_id, position))
            Log(Logs::General, Logs::Error, "Unable to unbury a summoned player corpse for character id %u.", char_id);
	}

	return corpse;
}

bool ZoneDatabase::SummonAllCharacterCorpses(uint32 char_id, uint32 dest_zone_id, uint16 dest_instance_id, const glm::vec4& position) {
	Corpse* corpse = nullptr;
	int CorpseCount = 0;

	std::string query = StringFormat(
		"UPDATE character_corpses SET zone_id = %i, instance_id = %i, x = %f, y = %f, z = %f, heading = %f, is_buried = 0, was_at_graveyard = 0 WHERE charid = %i",
		dest_zone_id, dest_instance_id, position.x, position.y, position.z, position.w, char_id
	);
	auto results = QueryDatabase(query);

	query = StringFormat(
		"SELECT `id`, `charname`, `time_of_death`, `is_rezzed` FROM `character_corpses` WHERE `charid` = '%u'"
		"ORDER BY time_of_death",
		char_id);
	results = QueryDatabase(query);

	for (auto row = results.begin(); row != results.end(); ++row) {
		corpse = Corpse::LoadCharacterCorpseEntity(
			atoul(row[0]),
			char_id,
			row[1],
			position,
			row[2],
			atoi(row[3]) == 1,
			false);

		if (corpse) {
			entity_list.AddCorpse(corpse);
			corpse->SetDecayTimer(RuleI(Character, CorpseDecayTimeMS));
			corpse->Spawn();
			++CorpseCount;
		}
		else{
			Log(Logs::General, Logs::Error, "Unable to construct a player corpse for character id %u.", char_id);
		}
	}

	return (CorpseCount > 0);
}

bool ZoneDatabase::UnburyCharacterCorpse(uint32 db_id, uint32 new_zone_id, uint16 new_instance_id, const glm::vec4& position) {
	std::string query = StringFormat("UPDATE `character_corpses` "
                                    "SET `is_buried` = 0, `zone_id` = %u, `instance_id` = %u, "
                                    "`x` = %f, `y` = %f, `z` = %f, `heading` = %f, "
                                    "`time_of_death` = Now(), `was_at_graveyard` = 0 "
                                    "WHERE `id` = %u",
                                    new_zone_id, new_instance_id,
                                    position.x, position.y, position.z, position.w, db_id);
	auto results = QueryDatabase(query);
	if (results.Success() && results.RowsAffected() != 0)
		return true;

	return false;
}

Corpse* ZoneDatabase::LoadCharacterCorpse(uint32 player_corpse_id) {
	Corpse* NewCorpse = 0;
	std::string query = StringFormat(
		"SELECT `id`, `charid`, `charname`, `x`, `y`, `z`, `heading`, `time_of_death`, `is_rezzed`, `was_at_graveyard` FROM `character_corpses` WHERE `id` = '%u' LIMIT 1",
		player_corpse_id
	);
	auto results = QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
        auto position = glm::vec4(atof(row[3]), atof(row[4]), atof(row[5]), atof(row[6]));
		NewCorpse = Corpse::LoadCharacterCorpseEntity(
				atoul(row[0]), 		 // id					  uint32 in_dbid
				atoul(row[1]),		 // charid				  uint32 in_charid
				row[2], 			 //	char_name
				position,
				row[7],				 // time_of_death		  char* time_of_death
				atoi(row[8]) == 1, 	 // is_rezzed			  bool rezzed
				atoi(row[9])		 // was_at_graveyard	  bool was_at_graveyard
			);
		entity_list.AddCorpse(NewCorpse);
	}
	return NewCorpse;
}

bool ZoneDatabase::LoadCharacterCorpses(uint32 zone_id, uint16 instance_id) {
	std::string query;
	if (!RuleB(Zone, EnableShadowrest)){
		query = StringFormat("SELECT id, charid, charname, x, y, z, heading, time_of_death, is_rezzed, was_at_graveyard FROM character_corpses WHERE zone_id='%u' AND instance_id='%u'", zone_id, instance_id);
	}
	else{
		query = StringFormat("SELECT id, charid, charname, x, y, z, heading, time_of_death, is_rezzed, 0 as was_at_graveyard FROM character_corpses WHERE zone_id='%u' AND instance_id='%u' AND is_buried=0", zone_id, instance_id);
	}

	auto results = QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
        auto position = glm::vec4(atof(row[3]), atof(row[4]), atof(row[5]), atof(row[6]));
		entity_list.AddCorpse(
			 Corpse::LoadCharacterCorpseEntity(
				atoul(row[0]), 		  // id					  uint32 in_dbid
				atoul(row[1]), 		  // charid				  uint32 in_charid
				row[2], 			  //					  char_name
				position,
				row[7], 			  // time_of_death		  char* time_of_death
				atoi(row[8]) == 1, 	  // is_rezzed			  bool rezzed
				atoi(row[9]))
		);
	}

	return true;
}

uint32 ZoneDatabase::GetFirstCorpseID(uint32 char_id) {
	std::string query = StringFormat("SELECT `id` FROM `character_corpses` WHERE `charid` = '%u' AND `is_buried` = 0 ORDER BY `time_of_death` LIMIT 1", char_id);
	auto results = QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		return atoi(row[0]);
	}
	return 0;
}

bool ZoneDatabase::DeleteItemOffCharacterCorpse(uint32 db_id, uint32 equip_slot, uint32 item_id){
	std::string query = StringFormat("DELETE FROM `character_corpse_items` WHERE `corpse_id` = %u AND equip_slot = %u AND item_id = %u", db_id, equip_slot, item_id);
	auto results = QueryDatabase(query);
	if (results.Success() && results.RowsAffected() != 0){
		return true;
	}
	return false;
}

bool ZoneDatabase::BuryCharacterCorpse(uint32 db_id) {
	std::string query = StringFormat("UPDATE `character_corpses` SET `is_buried` = 1 WHERE `id` = %u", db_id);
	auto results = QueryDatabase(query);
	if (results.Success() && results.RowsAffected() != 0){
		return true;
	}
	return false;
}

bool ZoneDatabase::BuryAllCharacterCorpses(uint32 char_id) {
	std::string query = StringFormat("SELECT `id` FROM `character_corpses` WHERE `charid` = %u", char_id);
	auto results = QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		BuryCharacterCorpse(atoi(row[0]));
		return true;
	}
	return false;
}

bool ZoneDatabase::DeleteCharacterCorpse(uint32 db_id) {
	std::string query = StringFormat("DELETE FROM `character_corpses` WHERE `id` = %d", db_id);
	auto results = QueryDatabase(query);
	if (results.Success() && results.RowsAffected() != 0)
		return true;

	return false;
}

uint32 ZoneDatabase::LoadSaylinkID(const char* saylink_text, bool auto_insert)
{
	if (!saylink_text || saylink_text[0] == '\0')
		return 0;
	
	std::string query = StringFormat("SELECT `id` FROM `saylink` WHERE `phrase` = '%s' LIMIT 1", saylink_text);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return 0;
	if (!results.RowCount()) {
		if (auto_insert)
			return SaveSaylinkID(saylink_text);
		else
			return 0;
	}

	auto row = results.begin();
	return atoi(row[0]);
}

uint32 ZoneDatabase::SaveSaylinkID(const char* saylink_text)
{
	if (!saylink_text || saylink_text[0] == '\0')
		return 0;

	std::string query = StringFormat("INSERT INTO `saylink` (`phrase`) VALUES ('%s')", saylink_text);
	auto results = QueryDatabase(query);
	if (!results.Success())
		return 0;
	
	return results.LastInsertedID();
}
