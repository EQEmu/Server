
#include "zonedb.h"
#include "../common/Item.h"
#include "../common/StringUtil.h"
#include "../common/extprofile.h"
#include "../common/guilds.h"
#include "../common/rulesys.h"
#include "zone.h"
#include "client.h"
#include "merc.h"
#include "groups.h"
#include "raids.h"
#include <iostream>
#include <string>
#include <sstream>

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
	npc_spells_maxid = 0;
	npc_spellseffects_maxid = 0;
	npc_spells_cache = 0;
	npc_spellseffects_cache = 0;
	npc_spells_loadtried = 0;
	npc_spellseffects_loadtried = 0;
	max_faction = 0;
	faction_array = nullptr;
}

ZoneDatabase::~ZoneDatabase() {
	unsigned int x;
	if (npc_spells_cache) {
		for (x=0; x<=npc_spells_maxid; x++) {
			safe_delete_array(npc_spells_cache[x]);
		}
		safe_delete_array(npc_spells_cache);
	}
	safe_delete_array(npc_spells_loadtried);

	if (npc_spellseffects_cache) {
		for (x=0; x<=npc_spellseffects_maxid; x++) {
			safe_delete_array(npc_spellseffects_cache[x]);
		}
		safe_delete_array(npc_spellseffects_cache);
	}
	safe_delete_array(npc_spellseffects_loadtried);

	if (faction_array != nullptr) {
		for (x=0; x <= max_faction; x++) {
			if (faction_array[x] != 0)
				safe_delete(faction_array[x]);
		}
		safe_delete_array(faction_array);
	}
}

bool ZoneDatabase::SaveZoneCFG(uint32 zoneid, uint16 instance_id, NewZone_Struct* zd){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if (!RunQuery(query, MakeAnyLenString(&query, "update zone set underworld=%f,minclip=%f,"
		"maxclip=%f,fog_minclip=%f,fog_maxclip=%f,fog_blue=%i,fog_red=%i,fog_green=%i,sky=%i,"
		"ztype=%i,zone_exp_multiplier=%f,safe_x=%f,safe_y=%f,safe_z=%f "
		"where zoneidnumber=%i and version=%i",
		zd->underworld,zd->minclip,
		zd->maxclip,zd->fog_minclip[0],zd->fog_maxclip[0],zd->fog_blue[0],zd->fog_red[0],zd->fog_green[0],zd->sky,
		zd->ztype,zd->zone_exp_multiplier,
		zd->safe_x,zd->safe_y,zd->safe_z,
		zoneid, instance_id),errbuf))	{
			LogFile->write(EQEMuLog::Error, "Error in SaveZoneCFG query %s: %s", query, errbuf);
			safe_delete_array(query);
			return false;
	}
	safe_delete_array(query);
	return true;
}

bool ZoneDatabase::GetZoneCFG(uint32 zoneid, uint16 instance_id, NewZone_Struct *zone_data, bool &can_bind, bool &can_combat, bool &can_levitate, bool &can_castoutdoor, bool &is_city, bool &is_hotzone, bool &allow_mercs, uint8 &zone_type, int &ruleset, char **map_filename) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int i=0;
	int b=0;
	bool good = false;
	*map_filename = new char[100];
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT ztype,"
		"fog_red,fog_green,fog_blue,fog_minclip,fog_maxclip,"
		"fog_red2,fog_green2,fog_blue2,fog_minclip2,fog_maxclip2,"
		"fog_red3,fog_green3,fog_blue3,fog_minclip3,fog_maxclip3,"
		"fog_red4,fog_green4,fog_blue4,fog_minclip4,fog_maxclip4,fog_density,"
		"sky,zone_exp_multiplier,safe_x,safe_y,safe_z,underworld,"
		"minclip,maxclip,time_type,canbind,cancombat,canlevitate,"
		"castoutdoor,hotzone,ruleset,suspendbuffs,map_file_name,short_name,"
		"rain_chance1,rain_chance2,rain_chance3,rain_chance4,"
		"rain_duration1,rain_duration2,rain_duration3,rain_duration4,"
		"snow_chance1,snow_chance2,snow_chance3,snow_chance4,"
		"snow_duration1,snow_duration2,snow_duration3,snow_duration4"
		" from zone where zoneidnumber=%i and version=%i",zoneid, instance_id), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if(row)
		{
			int r = 0;
			memset(zone_data,0,sizeof(NewZone_Struct));
			zone_data->ztype=atoi(row[r++]);

			for(i=0;i<4;i++){
				zone_data->fog_red[i]=atoi(row[r++]);
				zone_data->fog_green[i]=atoi(row[r++]);
				zone_data->fog_blue[i]=atoi(row[r++]);
				zone_data->fog_minclip[i]=atof(row[r++]);
				zone_data->fog_maxclip[i]=atof(row[r++]);
			}

			zone_data->fog_density = atof(row[r++]);;
			zone_data->sky=atoi(row[r++]);
			zone_data->zone_exp_multiplier=atof(row[r++]);
			zone_data->safe_x=atof(row[r++]);
			zone_data->safe_y=atof(row[r++]);
			zone_data->safe_z=atof(row[r++]);
			zone_data->underworld=atof(row[r++]);
			zone_data->minclip=atof(row[r++]);
			zone_data->maxclip=atof(row[r++]);

			zone_data->time_type=atoi(row[r++]);
//not in the DB yet:
			zone_data->gravity = 0.4;

			b = atoi(row[r++]);
			can_bind = b==0?false:true;
			is_city = b==2?true:false;
			can_combat = atoi(row[r++])==0?false:true;
			can_levitate = atoi(row[r++])==0?false:true;
			can_castoutdoor = atoi(row[r++])==0?false:true;
			is_hotzone = atoi(row[r++])==0?false:true;
			allow_mercs = true;
			zone_type = zone_data->ztype;
			ruleset = atoi(row[r++]);
			zone_data->SuspendBuffs = atoi(row[r++]);
			char *file = row[r++];
			if(file)
			{
				strcpy(*map_filename, file);
			}
			else
			{
				strcpy(*map_filename, row[r++]);
			}
			for(i=0;i<4;i++){
				zone_data->rain_chance[i]=atoi(row[r++]);
			}
			for(i=0;i<4;i++){
				zone_data->rain_duration[i]=atoi(row[r++]);
			}
			for(i=0;i<4;i++){
				zone_data->snow_chance[i]=atoi(row[r++]);
			}
			for(i=0;i<4;i++){
				zone_data->snow_duration[i]=atof(row[r++]);
			}
			good = true;
		}
		mysql_free_result(result);
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in GetZoneCFG query %s: %s", query, errbuf);
		strcpy(*map_filename, "default");
	}
	safe_delete_array(query);

	zone_data->zone_id = zoneid;

	return(good);
}

//updates or clears the respawn time in the database for the current spawn id
void ZoneDatabase::UpdateSpawn2Timeleft(uint32 id, uint16 instance_id, uint32 timeleft)
{
	timeval tv;
	gettimeofday(&tv, nullptr);
	uint32 cur = tv.tv_sec;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	//if we pass timeleft as 0 that means we clear from respawn time
	//otherwise we update with a REPLACE INTO
	if(timeleft == 0)
	{
		if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM respawn_times WHERE id=%lu "
			"AND instance_id=%lu",(unsigned long)id, (unsigned long)instance_id),errbuf))
		{
			LogFile->write(EQEMuLog::Error, "Error in UpdateTimeLeft query %s: %s", query, errbuf);
		}
		safe_delete_array(query);
	}
	else
	{
		if (!RunQuery(query, MakeAnyLenString(&query, "REPLACE INTO respawn_times (id,start,duration,instance_id) "
			"VALUES(%lu,%lu,%lu,%lu)",(unsigned long)id, (unsigned long)cur, (unsigned long)timeleft, (unsigned long)instance_id),errbuf))
		{
			LogFile->write(EQEMuLog::Error, "Error in UpdateTimeLeft query %s: %s", query, errbuf);
		}
		safe_delete_array(query);
	}
	return;
}

//Gets the respawn time left in the database for the current spawn id
uint32 ZoneDatabase::GetSpawnTimeLeft(uint32 id, uint16 instance_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	MakeAnyLenString(&query, "SELECT start, duration FROM respawn_times WHERE id=%lu AND instance_id=%lu",
		(unsigned long)id, (unsigned long)zone->GetInstanceID());

	if (RunQuery(query, strlen(query), errbuf, &result))
	{
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if(row)
		{
			timeval tv;
			gettimeofday(&tv, nullptr);
			uint32 resStart = atoi(row[0]);
			uint32 resDuration = atoi(row[1]);

			//compare our values to current time
			if((resStart + resDuration) <= tv.tv_sec)
			{
				//our current time was expired
				mysql_free_result(result);
				return 0;
			}
			else
			{
				//we still have time left on this timer
				mysql_free_result(result);
				return ((resStart + resDuration) - tv.tv_sec);
			}
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in GetSpawnTimeLeft query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return 0;
	}
	return 0;
}

void ZoneDatabase::UpdateSpawn2Status(uint32 id, uint8 new_status)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "UPDATE spawn2 SET enabled=%i WHERE id=%lu", new_status, (unsigned long)id),errbuf))
	{
		LogFile->write(EQEMuLog::Error, "Error in UpdateSpawn2Status query %s: %s", query, errbuf);
	}
	safe_delete_array(query);
	return;
}

bool ZoneDatabase::logevents(const char* accountname,uint32 accountid,uint8 status,const char* charname, const char* target,const char* descriptiontype, const char* description,int event_nid){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 len = strlen(description);
	uint32 len2 = strlen(target);
	char* descriptiontext = new char[2*len+1];
	char* targetarr = new char[2*len2+1];
	memset(descriptiontext, 0, 2*len+1);
	memset(targetarr, 0, 2*len2+1);
	DoEscapeString(descriptiontext, description, len);
	DoEscapeString(targetarr, target, len2);
	if (!RunQuery(query, MakeAnyLenString(&query, "Insert into eventlog (accountname,accountid,status,charname,target,descriptiontype,description,event_nid) values('%s',%i,%i,'%s','%s','%s','%s','%i')", accountname,accountid,status,charname,targetarr,descriptiontype,descriptiontext,event_nid), errbuf))	{
		std::cerr << "Error in logevents" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);
	safe_delete_array(descriptiontext);
	safe_delete_array(targetarr);
	return true;
}


void ZoneDatabase::UpdateBug(BugStruct* bug){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;


	uint32 len = strlen(bug->bug);
	char* bugtext = nullptr;
	if(len > 0)
	{
		bugtext = new char[2*len+1];
		memset(bugtext, 0, 2*len+1);
		DoEscapeString(bugtext, bug->bug, len);
	}

	len = strlen(bug->ui);
	char* uitext = nullptr;
	if(len > 0)
	{
		uitext = new char[2*len+1];
		memset(uitext, 0, 2*len+1);
		DoEscapeString(uitext, bug->ui, len);
	}

	len = strlen(bug->target_name);
	char* targettext = nullptr;
	if(len > 0)
	{
		targettext = new char[2*len+1];
		memset(targettext, 0, 2*len+1);
		DoEscapeString(targettext, bug->target_name, len);
	}

	//x and y are intentionally swapped because eq is inversexy coords
	if (!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO bugs (zone, name, ui, x, y, z, type, flag, target, bug, date) "
		"values('%s', '%s', '%s', '%.2f', '%.2f', '%.2f', '%s', %d, '%s', '%s', CURDATE())", zone->GetShortName(), bug->name,
		uitext==nullptr?"":uitext, bug->y, bug->x, bug->z, bug->chartype, bug->type, targettext==nullptr?"Unknown Target":targettext,
		bugtext==nullptr?"":bugtext), errbuf)) {
		std::cerr << "Error in UpdateBug" << query << "' " << errbuf << std::endl;
	}
	safe_delete_array(query);
	safe_delete_array(bugtext);
	safe_delete_array(uitext);
	safe_delete_array(targettext);
}

void ZoneDatabase::UpdateBug(PetitionBug_Struct* bug){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 len = strlen(bug->text);
	char* bugtext = new char[2*len+1];
	memset(bugtext, 0, 2*len+1);
	DoEscapeString(bugtext, bug->text, len);
	if (!RunQuery(query, MakeAnyLenString(&query, "Insert into bugs (type,name,bugtext,flag) values('%s','%s','%s',%i)","Petition",bug->name,bugtext,25), errbuf))	{
		std::cerr << "Error in UpdateBug" << query << "' " << errbuf << std::endl;
	}
	safe_delete_array(query);
	safe_delete_array(bugtext);
}


bool ZoneDatabase::GetAccountInfoForLogin_result(MYSQL_RES* result, int16* admin, char* account_name, uint32* lsaccountid, uint8* gmspeed, bool* revoked,bool* gmhideme, uint32* account_creation) {
	MYSQL_ROW row;
	if (mysql_num_rows(result) == 1) {
		row = mysql_fetch_row(result);
		if (admin)
			*admin = atoi(row[0]);
		if (account_name)
			strcpy(account_name, row[1]);
		if (lsaccountid) {

			if (row[2])
				*lsaccountid = atoi(row[2]);
			else
				*lsaccountid = 0;


		}
		if (gmspeed)
			*gmspeed = atoi(row[3]);
		if (revoked)
			*revoked = atoi(row[4]);
		if(gmhideme)
			*gmhideme = atoi(row[5]);
		if(account_creation)
			*account_creation = atoul(row[6]);

		return true;
	}
	else {
		return false;
	}
}


bool ZoneDatabase::SetSpecialAttkFlag(uint8 id, const char* flag) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32	affected_rows = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE npc_types SET npcspecialattks='%s' WHERE id=%i;",flag,id), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	if (affected_rows == 0) {
		return false;
	}

	return true;
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
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	uint32 count = 0;
	char modifications[200];
	if(strlen(name) != 0)
		sprintf(modifications,"charname=\'%s\'",name);
	else if(account_id != 0)
		sprintf(modifications,"accountid=%i",account_id);

	if(strlen(target) != 0)
		sprintf(modifications,"%s AND target like \'%%%s%%\'",modifications,target);

	if(strlen(detail) != 0)
		sprintf(modifications,"%s AND description like \'%%%s%%\'",modifications,detail);

	if(strlen(timestamp) != 0)
		sprintf(modifications,"%s AND time like \'%%%s%%\'",modifications,timestamp);

	if(eventid == 0)
		eventid =1;
	sprintf(modifications,"%s AND event_nid=%i",modifications,eventid);

	MakeAnyLenString(&query, "SELECT id,accountname,accountid,status,charname,target,time,descriptiontype,description FROM eventlog where %s",modifications);
	if (RunQuery(query, strlen(query), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)))
		{
			if(count > 255)
				break;
			cel->eld[count].id = atoi(row[0]);
			strn0cpy(cel->eld[count].accountname,row[1],64);
			cel->eld[count].account_id = atoi(row[2]);
			cel->eld[count].status = atoi(row[3]);
			strn0cpy(cel->eld[count].charactername,row[4],64);
			strn0cpy(cel->eld[count].targetname,row[5],64);
			sprintf(cel->eld[count].timestamp,"%s",row[6]);
			strn0cpy(cel->eld[count].descriptiontype,row[7],64);
			strn0cpy(cel->eld[count].details,row[8],128);
			cel->eventid = eventid;
			count++;
			cel->count = count;
		}
		mysql_free_result(result);
	}
	else
	{
		// TODO: Invalid item length in database
		safe_delete_array(query);
	}
}

// Load child objects for a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::LoadWorldContainer(uint32 parentid, ItemInst* container)
{
	if (!container) {
		LogFile->write(EQEMuLog::Error, "Programming error: LoadWorldContainer passed nullptr pointer");
		return;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	//const Item_Struct* item = nullptr;
	//ItemInst* inst = nullptr;

	uint32 len_query = MakeAnyLenString(&query, "select "
		"bagidx,itemid,charges,augslot1,augslot2,augslot3,augslot4,augslot5 from object_contents where parentid=%i", parentid);

	if (RunQuery(query, len_query, errbuf, &result)) {
		while ((row = mysql_fetch_row(result))) {
			uint8 index = (uint8)atoi(row[0]);
			uint32 item_id = (uint32)atoi(row[1]);
			int8 charges = (int8)atoi(row[2]);
			uint32 aug[5];
			aug[0]	= (uint32)atoi(row[3]);
			aug[1]	= (uint32)atoi(row[4]);
			aug[2]	= (uint32)atoi(row[5]);
			aug[3]	= (uint32)atoi(row[6]);
			aug[4]	= (uint32)atoi(row[7]);

			ItemInst* inst = database.CreateItem(item_id, charges);
			if (inst) {
				if (inst->GetItem()->ItemClass == ItemClassCommon) {
					for(int i=0;i<5;i++) {
						if (aug[i]) {
								inst->PutAugment(&database, i, aug[i]);
						}
					}
				}
				// Put item inside world container
				container->PutItem(index, *inst);
				safe_delete(inst);
			}
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in DB::LoadWorldContainer: %s", errbuf);
	}

	safe_delete_array(query);
}

// Save child objects for a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::SaveWorldContainer(uint32 zone_id, uint32 parent_id, const ItemInst* container)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	// Since state is not saved for each world container action, we'll just delete
	// all and save from scratch .. we may come back later to optimize
	//DeleteWorldContainer(parent_id);

	if (!container) {
		return;
	}
	//Delete all items from container
	DeleteWorldContainer(parent_id,zone_id);
	// Save all 10 items, if they exist
	for (uint8 index=0; index<10; index++) {
		ItemInst* inst = container->GetItem(index);
		if (inst) {
			uint32 item_id = inst->GetItem()->ID;
			uint32 augslot[5] = { 0, 0, 0, 0, 0 };
			if (inst->IsType(ItemClassCommon)) {
				for(int i=0;i<5;i++) {
					ItemInst *auginst=inst->GetAugment(i);
					augslot[i]=(auginst && auginst->GetItem()) ? auginst->GetItem()->ID : 0;
				}
			}
			uint32 len_query = MakeAnyLenString(&query,

				"replace into object_contents (zoneid,parentid,bagidx,itemid,charges,augslot1,augslot2,augslot3,augslot4,augslot5,droptime) values (%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,now())",
				zone_id, parent_id, index, item_id, inst->GetCharges(),augslot[0],augslot[1],augslot[2],augslot[3],augslot[4]);

			if (!RunQuery(query, len_query, errbuf)) {
				LogFile->write(EQEMuLog::Error, "Error in ZoneDatabase::SaveWorldContainer: %s", errbuf);
			}
			safe_delete_array(query);
		}

	}
}

// Remove all child objects inside a world container (i.e., forge, bag dropped to ground, etc)
void ZoneDatabase::DeleteWorldContainer(uint32 parent_id,uint32 zone_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	uint32 len_query = MakeAnyLenString(&query,
		"delete from object_contents where parentid=%i and zoneid=%i", parent_id,zone_id);
	if (!RunQuery(query, len_query, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Error in ZoneDatabase::DeleteWorldContainer: %s", errbuf);
	}

	safe_delete_array(query);
}

Trader_Struct* ZoneDatabase::LoadTraderItem(uint32 char_id){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	Trader_Struct* loadti = new Trader_Struct;
	memset(loadti,0,sizeof(Trader_Struct));
	if (RunQuery(query,MakeAnyLenString(&query, "select * from trader where char_id=%i order by slot_id limit 80",char_id),errbuf,&result)){
		safe_delete_array(query);
		loadti->Code = BazaarTrader_ShowItems;
		while ((row = mysql_fetch_row(result))) {
			if(atoi(row[5])>=80 || atoi(row[4])<0)
				_log(TRADING__CLIENT, "Bad Slot number when trying to load trader information!\n");
			else{
				loadti->Items[atoi(row[5])] = atoi(row[1]);
				loadti->ItemCost[atoi(row[5])] = atoi(row[4]);
			}
		}
		mysql_free_result(result);
	}
	else{
		safe_delete_array(query);
		_log(TRADING__CLIENT, "Failed to load trader information!\n");
	}
	return loadti;
}

TraderCharges_Struct* ZoneDatabase::LoadTraderItemWithCharges(uint32 char_id){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	TraderCharges_Struct* loadti = new TraderCharges_Struct;
	memset(loadti,0,sizeof(TraderCharges_Struct));
	if (RunQuery(query,MakeAnyLenString(&query, "select * from trader where char_id=%i order by slot_id limit 80",char_id),errbuf,&result)){
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result))) {
			if(atoi(row[5])>=80 || atoi(row[5])<0)
				_log(TRADING__CLIENT, "Bad Slot number when trying to load trader information!\n");
			else{
				loadti->ItemID[atoi(row[5])] = atoi(row[1]);
				loadti->SerialNumber[atoi(row[5])] = atoi(row[2]);
				loadti->Charges[atoi(row[5])] = atoi(row[3]);
				loadti->ItemCost[atoi(row[5])] = atoi(row[4]);
			}
		}
		mysql_free_result(result);
	}
	else{
		safe_delete_array(query);
		_log(TRADING__CLIENT, "Failed to load trader information!\n");
	}
	return loadti;
}

ItemInst* ZoneDatabase::LoadSingleTraderItem(uint32 CharID, int SerialNumber) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query,MakeAnyLenString(&query, "select * from trader where char_id=%i and serialnumber=%i order by slot_id limit 80",
						CharID, SerialNumber),errbuf,&result)){
		safe_delete_array(query);

		if (mysql_num_rows(result) != 1) {
			_log(TRADING__CLIENT, "Bad result from query\n"); fflush(stdout);
			return nullptr;
		}
		row = mysql_fetch_row(result);
		int ItemID = atoi(row[1]);
		int Charges = atoi(row[3]);
		int Cost = atoi(row[4]);

		const Item_Struct *item=database.GetItem(ItemID);

		if(!item) {
			_log(TRADING__CLIENT, "Unable to create item\n"); fflush(stdout);
			return nullptr;
		}

		if (item && (item->NoDrop!=0)) {
			ItemInst* inst = database.CreateItem(item);
			if(!inst) {
				_log(TRADING__CLIENT, "Unable to create item instance\n"); fflush(stdout);
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
	}

	return nullptr;


}

void ZoneDatabase::SaveTraderItem(uint32 CharID, uint32 ItemID, uint32 SerialNumber, int32 Charges, uint32 ItemCost, uint8 Slot){

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	if (!(RunQuery(query,MakeAnyLenString(&query, "replace INTO trader VALUES(%i,%i,%i,%i,%i,%i)",
							CharID, ItemID, SerialNumber, Charges, ItemCost, Slot),errbuf)))
		_log(TRADING__CLIENT, "Failed to save trader item: %i for char_id: %i, the error was: %s\n", ItemID, CharID, errbuf);

	safe_delete_array(query);
}

void ZoneDatabase::UpdateTraderItemCharges(int CharID, uint32 SerialNumber, int32 Charges) {

	_log(TRADING__CLIENT, "ZoneDatabase::UpdateTraderItemCharges(%i, %i, %i)", CharID, SerialNumber, Charges);
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	if (!(RunQuery(query,MakeAnyLenString(&query, "update trader set charges=%i where char_id=%i and serialnumber=%i",
						Charges, CharID, SerialNumber),errbuf)))
		_log(TRADING__CLIENT, "Failed to update charges for trader item: %i for char_id: %i, the error was: %s\n",
						SerialNumber, CharID, errbuf);

	safe_delete_array(query);

}

void ZoneDatabase::UpdateTraderItemPrice(int CharID, uint32 ItemID, uint32 Charges, uint32 NewPrice) {

	_log(TRADING__CLIENT, "ZoneDatabase::UpdateTraderPrice(%i, %i, %i, %i)", CharID, ItemID, Charges, NewPrice);

	const Item_Struct *item = database.GetItem(ItemID);

	if(!item)
		return;

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* Query = 0;

	if(NewPrice == 0) {
		_log(TRADING__CLIENT, "Removing Trader items from the DB for CharID %i, ItemID %i", CharID, ItemID);

		if (!(RunQuery(Query,MakeAnyLenString(&Query, "delete from trader where char_id=%i and item_id=%i",
								CharID, ItemID),errbuf)))

			_log(TRADING__CLIENT, "Failed to remove trader item(s): %i for char_id: %i, the error was: %s\n",
							ItemID, CharID, errbuf);

		safe_delete_array(Query);

		return;
	}
	else {
		if(!item->Stackable) {
			if (!(RunQuery(Query,MakeAnyLenString(&Query, "update trader set item_cost=%i where char_id=%i and item_id=%i"
								" and charges=%i", NewPrice, CharID, ItemID, Charges),errbuf)))

				_log(TRADING__CLIENT, "Failed to update price for trader item: %i for char_id: %i, the error was: %s\n",
								ItemID, CharID, errbuf);
		}
		else {
			if (!(RunQuery(Query,MakeAnyLenString(&Query, "update trader set item_cost=%i where char_id=%i and item_id=%i",
								NewPrice, CharID, ItemID),errbuf)))

				_log(TRADING__CLIENT, "Failed to update price for trader item: %i for char_id: %i, the error was: %s\n",
								ItemID, CharID, errbuf);
		}

		safe_delete_array(Query);
	}

}

void ZoneDatabase::DeleteTraderItem(uint32 char_id){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	if(char_id==0){
		if (!(RunQuery(query,MakeAnyLenString(&query, "delete from trader"),errbuf)))
			_log(TRADING__CLIENT, "Failed to delete all trader items data, the error was: %s\n",errbuf);
	}
	else{
		if (!(RunQuery(query,MakeAnyLenString(&query, "delete from trader where char_id=%i",char_id),errbuf)))
			_log(TRADING__CLIENT, "Failed to delete trader item data for char_id: %i, the error was: %s\n",char_id,errbuf);
	}
	safe_delete_array(query);
}
void ZoneDatabase::DeleteTraderItem(uint32 CharID,uint16 SlotID){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	if (!(RunQuery(query,MakeAnyLenString(&query, "delete from trader where char_id=%i and slot_id=%i",CharID, SlotID),errbuf)))
		_log(TRADING__CLIENT, "Failed to delete trader item data for char_id: %i, the error was: %s\n",CharID, errbuf);
	safe_delete_array(query);
}

void ZoneDatabase::DeleteBuyLines(uint32 CharID){

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	if(CharID==0){
		if (!(RunQuery(query,MakeAnyLenString(&query, "delete from buyer"),errbuf)))
			_log(TRADING__CLIENT, "Failed to delete all buyer items data, the error was: %s\n",errbuf);
	}
	else{
		if (!(RunQuery(query,MakeAnyLenString(&query, "delete from buyer where charid=%i",CharID),errbuf)))
			_log(TRADING__CLIENT, "Failed to delete buyer item data for charid: %i, the error was: %s\n",CharID,errbuf);
	}
	safe_delete_array(query);
}

void ZoneDatabase::AddBuyLine(uint32 CharID, uint32 BuySlot, uint32 ItemID, const char* ItemName, uint32 Quantity, uint32 Price) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	if (!(RunQuery(query,MakeAnyLenString(&query, "replace INTO buyer VALUES(%i,%i, %i,\"%s\",%i,%i)",
							CharID, BuySlot, ItemID, ItemName, Quantity, Price),errbuf)))
		_log(TRADING__CLIENT, "Failed to save buline item: %i for char_id: %i, the error was: %s\n", ItemID, CharID, errbuf);

	safe_delete_array(query);
}

void ZoneDatabase::RemoveBuyLine(uint32 CharID, uint32 BuySlot) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	if (!(RunQuery(query,MakeAnyLenString(&query, "delete from buyer where charid=%i and buyslot=%i", CharID, BuySlot), errbuf)))
		_log(TRADING__CLIENT, "Failed to delete buyslot %i for charid: %i, the error was: %s\n", BuySlot, CharID, errbuf);

	safe_delete_array(query);
}

void ZoneDatabase::UpdateBuyLine(uint32 CharID, uint32 BuySlot, uint32 Quantity) {

	if(Quantity <= 0) {
		RemoveBuyLine(CharID, BuySlot);
		return;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	if (!(RunQuery(query,MakeAnyLenString(&query, "update buyer set quantity=%i where charid=%i and buyslot=%i",
							Quantity, CharID, BuySlot), errbuf)))
		_log(TRADING__CLIENT, "Failed to update quantity in buyslot %i for charid: %i, the error was: %s\n", BuySlot, CharID, errbuf);

	safe_delete_array(query);

}

bool ZoneDatabase::GetCharacterInfoForLogin(const char* name, uint32* character_id,
char* current_zone, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext,
uint32* pplen, uint32* guilddbid, uint8* guildrank,
uint8 *class_, uint8 *level, bool *LFP, bool *LFG, uint8 *NumXTargets, uint8 *firstlogon) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 querylen;
	MYSQL_RES *result;

	bool ret = false;

	//DO NOT FORGET TO EDIT Client::Handle_Connect_OP_ZoneEntry if you change this.

	if (character_id && *character_id) {
		// searching by ID should be a lil bit faster
		querylen = MakeAnyLenString(&query, "SELECT id,profile,zonename,x,y,z,guild_id,rank,extprofile,class,level,lfp,lfg,instanceid,xtargets,firstlogon FROM character_ LEFT JOIN guild_members ON id=char_id WHERE id=%i", *character_id);
	}
	else {
		querylen = MakeAnyLenString(&query, "SELECT id,profile,zonename,x,y,z,guild_id,rank,extprofile,class,level,lfp,lfg,instanceid,xtargets,firstlogon FROM character_ LEFT JOIN guild_members ON id=char_id WHERE name='%s'", name);
	}

	if (RunQuery(query, querylen, errbuf, &result)) {
		ret = GetCharacterInfoForLogin_result(result, character_id, current_zone, pp, inv, ext, pplen, guilddbid, guildrank, class_, level, LFP, LFG, NumXTargets, firstlogon);
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "GetCharacterInfoForLogin query '%s' %s", query, errbuf);
	}

	safe_delete_array(query);
	return ret;
}

#define StructDist(in, f1, f2) (uint32(&in->f2)-uint32(&in->f1))

// Process results of GetCharacterInfoForLogin()
// Query this processes: SELECT id,profile,zonename,x,y,z,guild,guildrank,extprofile,class,level FROM character_ WHERE id=%i
bool ZoneDatabase::GetCharacterInfoForLogin_result(MYSQL_RES* result,
	uint32* character_id, char* current_zone, PlayerProfile_Struct* pp, Inventory* inv,
	ExtendedProfile_Struct *ext, uint32* pplen, uint32* guilddbid, uint8* guildrank,
	uint8 *class_, uint8 *level, bool *LFP, bool *LFG, uint8 *NumXTargets, uint8* firstlogon) {

	MYSQL_ROW row;
	unsigned long* lengths;

	if (mysql_num_rows(result) == 1) {
		row = mysql_fetch_row(result);
		lengths = mysql_fetch_lengths(result);
		if (pp && pplen) {
			if (lengths[1] == sizeof(PlayerProfile_Struct)) {
				memcpy(pp, row[1], sizeof(PlayerProfile_Struct));
			} else {
				LogFile->write(EQEMuLog::Error, "Player profile length mismatch in GetCharacterInfo Expected: %i, Got: %i",
					sizeof(PlayerProfile_Struct), lengths[1]);
				return false;
			}

			*pplen = lengths[1];
			pp->zone_id = GetZoneID(row[2]);
			pp->zoneInstance = atoi(row[13]);

			pp->x = atof(row[3]);
			pp->y = atof(row[4]);
			pp->z = atof(row[5]);

			pp->lastlogin = time(nullptr);

			if (pp->x == -1 && pp->y == -1 && pp->z == -1)
				GetSafePoints(pp->zone_id, database.GetInstanceVersion(pp->zoneInstance), &pp->x, &pp->y, &pp->z);
		}

		uint32 char_id = atoi(row[0]);
		if (RuleB(Character, SharedBankPlat))
			pp->platinum_shared = database.GetSharedPlatinum(GetAccountIDByChar(char_id));
		if (character_id)
			*character_id = char_id;
		if (current_zone)
			strcpy(current_zone, row[2]);

		if (guilddbid) {
			if(row[6] != nullptr)
				*guilddbid = atoi(row[6]);
			else
				*guilddbid = GUILD_NONE;
		}
		if (guildrank) {
			if(row[7] != nullptr)
				*guildrank = atoi(row[7]);
			else
				*guildrank = GUILD_RANK_NONE;
		}

		if(ext) {
			//SetExtendedProfile handles any conversion
			SetExtendedProfile(ext, row[8], lengths[8]);
		}

		if(class_)
			*class_ = atoi(row[9]);

		if(level)
			*level = atoi(row[10]);

		if(LFP)
			*LFP = atoi(row[11]);

		if(LFG)
			*LFG = atoi(row[12]);

		if(NumXTargets)
		{
			*NumXTargets = atoi(row[14]);
		}


		if(firstlogon)
		{
			*firstlogon = atoi(row[15]);
		}

		// Fix use_tint, previously it was set to 1 for a dyed slot, client wants it set to 0xFF
		for(int i = 0; i<9; i++)
			if(pp->item_tint[i].rgb.use_tint == 1)
				pp->item_tint[i].rgb.use_tint = 0xFF;

		// Retrieve character inventory
		return GetInventory(char_id, inv);
	}

	return false;
}

bool ZoneDatabase::NoRentExpired(const char* name){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, "Select (UNIX_TIMESTAMP(NOW())-timelaston) from character_ where name='%s'", name), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			uint32 seconds = atoi(row[0]);
			mysql_free_result(result);
			return (seconds>1800);
		}
	}
	return false;
}

/* Searches npctable for matching id, and returns the item if found,
 * or nullptr otherwise. If id passed is 0, loads all npc_types for
 * the current zone, returning the last item added.
 */
const NPCType* ZoneDatabase::GetNPCType (uint32 id) {
	const NPCType *npc=nullptr;
	std::map<uint32,NPCType *>::iterator itr;

	// If NPC is already in tree, return it.
	if((itr = zone->npctable.find(id)) != zone->npctable.end())
		return itr->second;

		// Otherwise, get NPCs from database.
		char errbuf[MYSQL_ERRMSG_SIZE];
		char *query = 0;
		MYSQL_RES *result;
		MYSQL_ROW row;


		// If id is 0, load all npc_types for the current zone,
		// according to spawn2.
		const char *basic_query = "SELECT "
			"npc_types.id,"
			"npc_types.name,"
			"npc_types.level,"
			"npc_types.race,"
			"npc_types.class,"
			"npc_types.hp,"
			"npc_types.mana,"
			"npc_types.gender,"
			"npc_types.texture,"
			"npc_types.helmtexture,"
			"npc_types.size,"
			"npc_types.loottable_id,"
			"npc_types.merchant_id,"
			"npc_types.alt_currency_id,"
			"npc_types.adventure_template_id,"
			"npc_types.trap_template,"
			"npc_types.attack_speed,"
			"npc_types.STR,"
			"npc_types.STA,"
			"npc_types.DEX,"
			"npc_types.AGI,"
			"npc_types._INT,"
			"npc_types.WIS,"
			"npc_types.CHA,"
			"npc_types.MR,"
			"npc_types.CR,"
			"npc_types.DR,"
			"npc_types.FR,"
			"npc_types.PR,"
			"npc_types.Corrup,"
			"npc_types.PhR,"
			"npc_types.mindmg,"
			"npc_types.maxdmg,"
			"npc_types.attack_count,"
			"npc_types.special_abilities,"
			"npc_types.npc_spells_id,"
			"npc_types.npc_spells_effects_id,"
			"npc_types.d_meele_texture1,"
			"npc_types.d_meele_texture2,"
			"npc_types.ammo_idfile,"
			"npc_types.prim_melee_type,"
			"npc_types.sec_melee_type,"
			"npc_types.ranged_type,"
			"npc_types.runspeed,"
			"npc_types.findable,"
			"npc_types.trackable,"
			"npc_types.hp_regen_rate,"
			"npc_types.mana_regen_rate,"
			"npc_types.aggroradius,"
			"npc_types.assistradius,"
			"npc_types.bodytype,"
			"npc_types.npc_faction_id,"
			"npc_types.face,"
			"npc_types.luclin_hairstyle,"
			"npc_types.luclin_haircolor,"
			"npc_types.luclin_eyecolor,"
			"npc_types.luclin_eyecolor2,"
			"npc_types.luclin_beardcolor,"
			"npc_types.luclin_beard,"
			"npc_types.drakkin_heritage,"
			"npc_types.drakkin_tattoo,"
			"npc_types.drakkin_details,"
			"npc_types.armortint_id,"
			"npc_types.armortint_red,"
			"npc_types.armortint_green,"
			"npc_types.armortint_blue,"
			"npc_types.see_invis,"
			"npc_types.see_invis_undead,"
			"npc_types.lastname,"
			"npc_types.qglobal,"
			"npc_types.AC,"
			"npc_types.npc_aggro,"
			"npc_types.spawn_limit,"
			"npc_types.see_hide,"
			"npc_types.see_improved_hide,"
			"npc_types.ATK,"
			"npc_types.Accuracy,"
			"npc_types.Avoidance,"
			"npc_types.slow_mitigation,"
			"npc_types.maxlevel,"
			"npc_types.scalerate,"
			"npc_types.private_corpse,"
			"npc_types.unique_spawn_by_name,"
			"npc_types.underwater,"
			"npc_types.emoteid,"
			"npc_types.spellscale,"
			"npc_types.healscale,"
			"npc_types.no_target_hotkey";

		MakeAnyLenString(&query, "%s FROM npc_types WHERE id=%d", basic_query, id);

		if (RunQuery(query, strlen(query), errbuf, &result)) {
			// Process each row returned.
			while((row = mysql_fetch_row(result))) {
				NPCType *tmpNPCType;
				tmpNPCType = new NPCType;
				memset (tmpNPCType, 0, sizeof *tmpNPCType);

				int r = 0;
				tmpNPCType->npc_id = atoi(row[r++]);

				strn0cpy(tmpNPCType->name, row[r++], 50);

				tmpNPCType->level = atoi(row[r++]);
				tmpNPCType->race = atoi(row[r++]);
				tmpNPCType->class_ = atoi(row[r++]);
				tmpNPCType->max_hp = atoi(row[r++]);
				tmpNPCType->cur_hp = tmpNPCType->max_hp;
				tmpNPCType->Mana = atoi(row[r++]);
				tmpNPCType->gender = atoi(row[r++]);
				tmpNPCType->texture = atoi(row[r++]);
				tmpNPCType->helmtexture = atoi(row[r++]);
				tmpNPCType->size = atof(row[r++]);
				tmpNPCType->loottable_id = atoi(row[r++]);
				tmpNPCType->merchanttype = atoi(row[r++]);
				tmpNPCType->alt_currency_type = atoi(row[r++]);
				tmpNPCType->adventure_template = atoi(row[r++]);
				tmpNPCType->trap_template = atoi(row[r++]);
				tmpNPCType->attack_speed = atof(row[r++]);
				tmpNPCType->STR = atoi(row[r++]);
				tmpNPCType->STA = atoi(row[r++]);
				tmpNPCType->DEX = atoi(row[r++]);
				tmpNPCType->AGI = atoi(row[r++]);
				tmpNPCType->INT = atoi(row[r++]);
				tmpNPCType->WIS = atoi(row[r++]);
				tmpNPCType->CHA = atoi(row[r++]);
				tmpNPCType->MR = atoi(row[r++]);
				tmpNPCType->CR = atoi(row[r++]);
				tmpNPCType->DR = atoi(row[r++]);
				tmpNPCType->FR = atoi(row[r++]);
				tmpNPCType->PR = atoi(row[r++]);
				tmpNPCType->Corrup = atoi(row[r++]);
				tmpNPCType->PhR = atoi(row[r++]);
				tmpNPCType->min_dmg = atoi(row[r++]);
				tmpNPCType->max_dmg = atoi(row[r++]);
				tmpNPCType->attack_count = atoi(row[r++]);
				strn0cpy(tmpNPCType->special_abilities, row[r++], 512);
				tmpNPCType->npc_spells_id = atoi(row[r++]);
				tmpNPCType->npc_spells_effects_id = atoi(row[r++]);
				tmpNPCType->d_meele_texture1 = atoi(row[r++]);
				tmpNPCType->d_meele_texture2 = atoi(row[r++]);
				strn0cpy(tmpNPCType->ammo_idfile, row[r++], 30);
				tmpNPCType->prim_melee_type = atoi(row[r++]);
				tmpNPCType->sec_melee_type = atoi(row[r++]);
				tmpNPCType->ranged_type = atoi(row[r++]);
				tmpNPCType->runspeed= atof(row[r++]);
				tmpNPCType->findable = atoi(row[r++]) == 0? false : true;
				tmpNPCType->trackable = atoi(row[r++]) == 0? false : true;
				tmpNPCType->hp_regen = atoi(row[r++]);
				tmpNPCType->mana_regen = atoi(row[r++]);

				tmpNPCType->aggroradius = (int32)atoi(row[r++]);
				// set defaultvalue for aggroradius
				if (tmpNPCType->aggroradius <= 0)
					tmpNPCType->aggroradius = 70;
				tmpNPCType->assistradius = (int32)atoi(row[r++]);
				if (tmpNPCType->assistradius <= 0)
					tmpNPCType->assistradius = tmpNPCType->aggroradius;

				if (row[r] && strlen(row[r]))
					tmpNPCType->bodytype = (uint8)atoi(row[r]);
				else
					tmpNPCType->bodytype = 0;
				r++;

				tmpNPCType->npc_faction_id = atoi(row[r++]);

				tmpNPCType->luclinface = atoi(row[r++]);
				tmpNPCType->hairstyle = atoi(row[r++]);
				tmpNPCType->haircolor = atoi(row[r++]);
				tmpNPCType->eyecolor1 = atoi(row[r++]);
				tmpNPCType->eyecolor2 = atoi(row[r++]);
				tmpNPCType->beardcolor = atoi(row[r++]);
				tmpNPCType->beard = atoi(row[r++]);
				tmpNPCType->drakkin_heritage = atoi(row[r++]);
				tmpNPCType->drakkin_tattoo = atoi(row[r++]);
				tmpNPCType->drakkin_details = atoi(row[r++]);
				uint32 armor_tint_id = atoi(row[r++]);
				tmpNPCType->armor_tint[0] = (atoi(row[r++]) & 0xFF) << 16;
				tmpNPCType->armor_tint[0] |= (atoi(row[r++]) & 0xFF) << 8;
				tmpNPCType->armor_tint[0] |= (atoi(row[r++]) & 0xFF);
				tmpNPCType->armor_tint[0] |= (tmpNPCType->armor_tint[0]) ? (0xFF << 24) : 0;
				
				int i;
				if (armor_tint_id > 0)
				{
					if (tmpNPCType->armor_tint[0] == 0)
					{
						char at_errbuf[MYSQL_ERRMSG_SIZE];
						char *at_query = nullptr;
						MYSQL_RES *at_result = nullptr;
						MYSQL_ROW at_row;

						MakeAnyLenString(&at_query,
						"SELECT "
						"red1h,grn1h,blu1h,"
						"red2c,grn2c,blu2c,"
						"red3a,grn3a,blu3a,"
						"red4b,grn4b,blu4b,"
						"red5g,grn5g,blu5g,"
						"red6l,grn6l,blu6l,"
						"red7f,grn7f,blu7f,"
						"red8x,grn8x,blu8x,"
						"red9x,grn9x,blu9x "
						"FROM npc_types_tint WHERE id=%d", armor_tint_id);

						if (RunQuery(at_query, strlen(at_query), at_errbuf, &at_result))
						{
							if ((at_row = mysql_fetch_row(at_result)))
							{
								for (i = 0; i < _MaterialCount; i++)
								{
									tmpNPCType->armor_tint[i] = atoi(at_row[i * 3]) << 16;
									tmpNPCType->armor_tint[i] |= atoi(at_row[i * 3 + 1]) << 8;
									tmpNPCType->armor_tint[i] |= atoi(at_row[i * 3 + 2]);
									tmpNPCType->armor_tint[i] |= (tmpNPCType->armor_tint[i]) ? (0xFF << 24) : 0;
								}
							}
							else
							{
								armor_tint_id = 0;
							}
						}
						else
						{
							armor_tint_id = 0;
						}

						if (at_result)
						{
							mysql_free_result(at_result);
						}

						safe_delete_array(at_query);
					}
					else
					{
						armor_tint_id = 0;
					}
				}

				if (armor_tint_id == 0)
				{
					for (i = 1; i < _MaterialCount; i++)
					{
						tmpNPCType->armor_tint[i] = tmpNPCType->armor_tint[0];
					}
				}

				tmpNPCType->see_invis = atoi(row[r++]);
				tmpNPCType->see_invis_undead = atoi(row[r++])==0?false:true;	// Set see_invis_undead flag
				if (row[r] != nullptr)
					strn0cpy(tmpNPCType->lastname, row[r], 32);
				r++;

				tmpNPCType->qglobal = atoi(row[r++])==0?false:true;	// qglobal
				tmpNPCType->AC = atoi(row[r++]);
				tmpNPCType->npc_aggro = atoi(row[r++])==0?false:true;
				tmpNPCType->spawn_limit = atoi(row[r++]);
				tmpNPCType->see_hide = atoi(row[r++])==0?false:true;
				tmpNPCType->see_improved_hide = atoi(row[r++])==0?false:true;
				tmpNPCType->ATK = atoi(row[r++]);
				tmpNPCType->accuracy_rating = atoi(row[r++]);
				tmpNPCType->avoidance_rating = atoi(row[r++]);
				tmpNPCType->slow_mitigation = atoi(row[r++]);
				tmpNPCType->maxlevel = atoi(row[r++]);
				tmpNPCType->scalerate = atoi(row[r++]);
				tmpNPCType->private_corpse = atoi(row[r++]) == 1 ? true : false;
				tmpNPCType->unique_spawn_by_name = atoi(row[r++]) == 1 ? true : false;
				tmpNPCType->underwater = atoi(row[r++]) == 1 ? true : false;
				tmpNPCType->emoteid = atoi(row[r++]);
				tmpNPCType->spellscale = atoi(row[r++]);
				tmpNPCType->healscale = atoi(row[r++]);
				tmpNPCType->no_target_hotkey = atoi(row[r++]) == 1 ? true : false;
				
				// If NPC with duplicate NPC id already in table,
				// free item we attempted to add.
				if (zone->npctable.find(tmpNPCType->npc_id) != zone->npctable.end())
				{
					std::cerr << "Error loading duplicate NPC " << tmpNPCType->npc_id << std::endl;
					delete tmpNPCType;
					npc = nullptr;
				} else {
					zone->npctable[tmpNPCType->npc_id]=tmpNPCType;
					npc = tmpNPCType;
				}

//				Sleep(0);
			}

			if (result) {
				mysql_free_result(result);
			}
		} else
			std::cerr << "Error loading NPCs from database. Bad query: " << errbuf << std::endl;
		safe_delete_array(query);

	return npc;
}


const NPCType* ZoneDatabase::GetMercType(uint32 id, uint16 raceid, uint32 clientlevel) {
	const NPCType *npc=nullptr;
	std::map<uint32,NPCType *>::iterator itr;

	//need to save based on merc_npc_type & client level
	uint32 merc_type_id = id * 100 + clientlevel;

	// If NPC is already in tree, return it.
	if((itr = zone->merctable.find(merc_type_id)) != zone->merctable.end())
		return itr->second;
	//If the NPC type is 0, return nullptr. (sanity check)
	if(id == 0)
	return nullptr;

	// Otherwise, get NPCs from database.
		char errbuf[MYSQL_ERRMSG_SIZE];
		char *query = 0;
		MYSQL_RES *result;
		MYSQL_ROW row;


		// If id is 0, load all npc_types for the current zone,
		// according to spawn2.
		const char *basic_query = "SELECT "
			"vwMercNpcTypes.merc_npc_type_id,"
			"vwMercNpcTypes.name,"
			//"vwMercNpcTypes.clientlevel,"
			"vwMercNpcTypes.level,"
			"vwMercNpcTypes.race_id,"
			"vwMercNpcTypes.class_id,"
			"vwMercNpcTypes.hp,"
			"vwMercNpcTypes.mana,"
			"vwMercNpcTypes.gender,"
			"vwMercNpcTypes.texture,"
			"vwMercNpcTypes.helmtexture,"
			//"vwMercNpcTypes.size,"
			// "vwMercNpcTypes.loottable_id,"
			// "vwMercNpcTypes.merchant_id,"
			// "vwMercNpcTypes.alt_currency_id,"
			// "vwMercNpcTypes.adventure_template_id,"
			// "vwMercNpcTypes.trap_template,"
			"vwMercNpcTypes.attack_speed,"
			"vwMercNpcTypes.STR,"
			"vwMercNpcTypes.STA,"
			"vwMercNpcTypes.DEX,"
			"vwMercNpcTypes.AGI,"
			"vwMercNpcTypes._INT,"
			"vwMercNpcTypes.WIS,"
			"vwMercNpcTypes.CHA,"
			"vwMercNpcTypes.MR,"
			"vwMercNpcTypes.CR,"
			"vwMercNpcTypes.DR,"
			"vwMercNpcTypes.FR,"
			"vwMercNpcTypes.PR,"
			"vwMercNpcTypes.Corrup,"
			"vwMercNpcTypes.mindmg,"
			"vwMercNpcTypes.maxdmg,"
			"vwMercNpcTypes.attack_count,"
			"vwMercNpcTypes.special_abilities,"
			// "vwMercNpcTypes.npc_spells_id,"
			"vwMercNpcTypes.d_meele_texture1,"
			"vwMercNpcTypes.d_meele_texture2,"
			"vwMercNpcTypes.prim_melee_type,"
			"vwMercNpcTypes.sec_melee_type,"
			"vwMercNpcTypes.runspeed,"
			// "vwMercNpcTypes.findable,"
			// "vwMercNpcTypes.trackable,"
			"vwMercNpcTypes.hp_regen_rate,"
			"vwMercNpcTypes.mana_regen_rate,"
			// "vwMercNpcTypes.aggroradius,"
			"vwMercNpcTypes.bodytype,"
			// "vwMercNpcTypes.npc_faction_id,"
			//"vwMercNpcTypes.face,"
			//"vwMercNpcTypes.luclin_hairstyle,"
			//"vwMercNpcTypes.luclin_haircolor,"
			//"vwMercNpcTypes.luclin_eyecolor,"
			//"vwMercNpcTypes.luclin_eyecolor2,"
			//"vwMercNpcTypes.luclin_beardcolor,"
			//"vwMercNpcTypes.luclin_beard,"
			//"vwMercNpcTypes.drakkin_heritage,"
			//"vwMercNpcTypes.drakkin_tattoo,"
			//"vwMercNpcTypes.drakkin_details,"
			"vwMercNpcTypes.armortint_id,"
			"vwMercNpcTypes.armortint_red,"
			"vwMercNpcTypes.armortint_green,"
			"vwMercNpcTypes.armortint_blue,"
			// "vwMercNpcTypes.see_invis,"
			// "vwMercNpcTypes.see_invis_undead,"
			// "vwMercNpcTypes.lastname,"
			// "vwMercNpcTypes.qglobal,"
			"vwMercNpcTypes.AC,"
			// "vwMercNpcTypes.npc_aggro,"
			// "vwMercNpcTypes.spawn_limit,"
			// "vwMercNpcTypes.see_hide,"
			// "vwMercNpcTypes.see_improved_hide,"
			"vwMercNpcTypes.ATK,"
			"vwMercNpcTypes.Accuracy,"
			"vwMercNpcTypes.spellscale,"
			"vwMercNpcTypes.healscale";
			// "vwMercNpcTypes.slow_mitigation,"
			// "vwMercNpcTypes.maxlevel,"
			// "vwMercNpcTypes.scalerate,"
			// "vwMercNpcTypes.private_corpse,"
			// "vwMercNpcTypes.unique_spawn_by_name,"
			// "vwMercNpcTypes.underwater,"
			// "vwMercNpcTypes.emoteid";

		MakeAnyLenString(&query, "%s FROM vwMercNpcTypes WHERE merc_npc_type_id=%d AND clientlevel=%d AND race_id = %d", basic_query, id, clientlevel, raceid); //dual primary keys. one is ID, one is level.

		if (RunQuery(query, strlen(query), errbuf, &result)) {
			// Process each row returned.
			while((row = mysql_fetch_row(result))) {
				NPCType *tmpNPCType;
				tmpNPCType = new NPCType;
				memset (tmpNPCType, 0, sizeof *tmpNPCType);

				int r = 0;
				tmpNPCType->npc_id = atoi(row[r++]);

				strn0cpy(tmpNPCType->name, row[r++], 50);

				tmpNPCType->level = atoi(row[r++]);
				tmpNPCType->race = atoi(row[r++]);
				tmpNPCType->class_ = atoi(row[r++]);
				tmpNPCType->max_hp = atoi(row[r++]);
				tmpNPCType->cur_hp = tmpNPCType->max_hp;
				tmpNPCType->Mana = atoi(row[r++]);
				tmpNPCType->gender = atoi(row[r++]);
				tmpNPCType->texture = atoi(row[r++]);
				tmpNPCType->helmtexture = atoi(row[r++]);
				//tmpNPCType->size = atof(row[r++]);
				//tmpNPCType->loottable_id = atoi(row[r++]);
				//tmpNPCType->merchanttype = atoi(row[r++]);
				//tmpNPCType->alt_currency_type = atoi(row[r++]);
				//tmpNPCType->adventure_template = atoi(row[r++]);
				//tmpNPCType->trap_template = atoi(row[r++]);
				tmpNPCType->attack_speed = atof(row[r++]);
				tmpNPCType->STR = atoi(row[r++]);
				tmpNPCType->STA = atoi(row[r++]);
				tmpNPCType->DEX = atoi(row[r++]);
				tmpNPCType->AGI = atoi(row[r++]);
				tmpNPCType->INT = atoi(row[r++]);
				tmpNPCType->WIS = atoi(row[r++]);
				tmpNPCType->CHA = atoi(row[r++]);
				tmpNPCType->MR = atoi(row[r++]);
				tmpNPCType->CR = atoi(row[r++]);
				tmpNPCType->DR = atoi(row[r++]);
				tmpNPCType->FR = atoi(row[r++]);
				tmpNPCType->PR = atoi(row[r++]);
				tmpNPCType->Corrup = atoi(row[r++]);
				tmpNPCType->min_dmg = atoi(row[r++]);
				tmpNPCType->max_dmg = atoi(row[r++]);
				tmpNPCType->attack_count = atoi(row[r++]);
				strn0cpy(tmpNPCType->special_abilities, row[r++], 512);
				//tmpNPCType->npc_spells_id = atoi(row[r++]);
				tmpNPCType->d_meele_texture1 = atoi(row[r++]);
				tmpNPCType->d_meele_texture2 = atoi(row[r++]);
				tmpNPCType->prim_melee_type = atoi(row[r++]);
				tmpNPCType->sec_melee_type = atoi(row[r++]);
				tmpNPCType->runspeed= atof(row[r++]);
				//tmpNPCType->findable = atoi(row[r++]) == 0? false : true;
				//tmpNPCType->trackable = atoi(row[r++]) == 0? false : true;
				tmpNPCType->hp_regen = atoi(row[r++]);
				tmpNPCType->mana_regen = atoi(row[r++]);

				//tmpNPCType->aggroradius = (int32)atoi(row[r++]);
				tmpNPCType->aggroradius = RuleI(Mercs, AggroRadius);
				// set defaultvalue for aggroradius
				//if (tmpNPCType->aggroradius <= 0)
				//	tmpNPCType->aggroradius = 70;

				if (row[r] && strlen(row[r]))
					tmpNPCType->bodytype = (uint8)atoi(row[r]);
				else
					tmpNPCType->bodytype = 1;
				r++;

				//tmpNPCType->npc_faction_id = atoi(row[r++]);

				//tmpNPCType->luclinface = atoi(row[r++]);
				//tmpNPCType->hairstyle = atoi(row[r++]);
				//tmpNPCType->haircolor = atoi(row[r++]);
				//tmpNPCType->eyecolor1 = atoi(row[r++]);
				//tmpNPCType->eyecolor2 = atoi(row[r++]);
				//tmpNPCType->beardcolor = atoi(row[r++]);
				//tmpNPCType->beard = atoi(row[r++]);
				//tmpNPCType->drakkin_heritage = atoi(row[r++]);
				//tmpNPCType->drakkin_tattoo = atoi(row[r++]);
				//tmpNPCType->drakkin_details = atoi(row[r++]);
				uint32 armor_tint_id = atoi(row[r++]);
				tmpNPCType->armor_tint[0] = (atoi(row[r++]) & 0xFF) << 16;
				tmpNPCType->armor_tint[0] |= (atoi(row[r++]) & 0xFF) << 8;
				tmpNPCType->armor_tint[0] |= (atoi(row[r++]) & 0xFF);
				tmpNPCType->armor_tint[0] |= (tmpNPCType->armor_tint[0]) ? (0xFF << 24) : 0;

				int i;
				if (armor_tint_id > 0)
				{
					if (tmpNPCType->armor_tint[0] == 0)
					{
						char at_errbuf[MYSQL_ERRMSG_SIZE];
						char *at_query = nullptr;
						MYSQL_RES *at_result = nullptr;
						MYSQL_ROW at_row;

						MakeAnyLenString(&at_query,
						"SELECT "
						"red1h,grn1h,blu1h,"
						"red2c,grn2c,blu2c,"
						"red3a,grn3a,blu3a,"
						"red4b,grn4b,blu4b,"
						"red5g,grn5g,blu5g,"
						"red6l,grn6l,blu6l,"
						"red7f,grn7f,blu7f,"
						"red8x,grn8x,blu8x,"
						"red9x,grn9x,blu9x "
						"FROM npc_types_tint WHERE id=%d", armor_tint_id);

						if (RunQuery(at_query, strlen(at_query), at_errbuf, &at_result))
						{
							if ((at_row = mysql_fetch_row(at_result)))
							{
								for (i = 0; i < _MaterialCount; i++)
								{
									tmpNPCType->armor_tint[i] = atoi(at_row[i * 3]) << 16;
									tmpNPCType->armor_tint[i] |= atoi(at_row[i * 3 + 1]) << 8;
									tmpNPCType->armor_tint[i] |= atoi(at_row[i * 3 + 2]);
									tmpNPCType->armor_tint[i] |= (tmpNPCType->armor_tint[i]) ? (0xFF << 24) : 0;
								}
							}
							else
							{
								armor_tint_id = 0;
							}
						}
						else
						{
							armor_tint_id = 0;
						}

						if (at_result)
						{
							mysql_free_result(at_result);
						}

						safe_delete_array(at_query);
					}
					else
					{
						armor_tint_id = 0;
					}
				}

				if (armor_tint_id == 0)
				{
					for (i = 1; i < _MaterialCount; i++)
					{
						tmpNPCType->armor_tint[i] = tmpNPCType->armor_tint[0];
					}
				}

				//tmpNPCType->see_invis = atoi(row[r++]);
				//tmpNPCType->see_invis_undead = atoi(row[r++])==0?false:true;	// Set see_invis_undead flag
				//if (row[r] != nullptr)
				//	strn0cpy(tmpNPCType->lastname, row[r], 32);
				//r++;

				//tmpNPCType->qglobal = atoi(row[r++])==0?false:true;	// qglobal
				tmpNPCType->AC = atoi(row[r++]);
				//tmpNPCType->npc_aggro = atoi(row[r++])==0?false:true;
				//tmpNPCType->spawn_limit = atoi(row[r++]);
				//tmpNPCType->see_hide = atoi(row[r++])==0?false:true;
				//tmpNPCType->see_improved_hide = atoi(row[r++])==0?false:true;
				tmpNPCType->ATK = atoi(row[r++]);
				tmpNPCType->accuracy_rating = atoi(row[r++]);
				//tmpNPCType->slow_mitigation = atof(row[r++]);
				//tmpNPCType->maxlevel = atoi(row[r++]);
				tmpNPCType->scalerate = RuleI(Mercs, ScaleRate);
				//tmpNPCType->private_corpse = atoi(row[r++]) == 1 ? true : false;
				//tmpNPCType->unique_spawn_by_name = atoi(row[r++]) == 1 ? true : false;
				//tmpNPCType->underwater = atoi(row[r++]) == 1 ? true : false;
				//tmpNPCType->emoteid = atoi(row[r++]);
				tmpNPCType->spellscale = atoi(row[r++]);
				tmpNPCType->healscale = atoi(row[r++]);

				// If NPC with duplicate NPC id already in table,
				// free item we attempted to add.
				if (zone->merctable.find(tmpNPCType->npc_id * 100 + clientlevel) != zone->merctable.end())
				{
					delete tmpNPCType;
					npc = nullptr;
				} else {
					zone->merctable[tmpNPCType->npc_id * 100 + clientlevel]=tmpNPCType;
					npc = tmpNPCType;
				}

//				Sleep(0);
			}

			if (result) {
				mysql_free_result(result);
			}
		} else
			std::cerr << "Error loading NPCs from database. Bad query: " << errbuf << std::endl;
		safe_delete_array(query);

	return npc;
}

bool ZoneDatabase::LoadMercInfo(Client *c) {
	bool loaded = false;

	if(c->GetEPP().merc_name[0] != 0) {
		std::string errorMessage;
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;
		//char name[64];

		//CleanMobName(c->GetEPP().merc_name, name);

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT MercID, Slot, Name, TemplateID, SuspendedTime, IsSuspended, TimerRemaining, Gender, StanceID, HP, Mana, Endurance, Face, LuclinHairStyle, LuclinHairColor, LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, DrakkinHeritage, DrakkinTattoo, DrakkinDetails FROM mercs WHERE OwnerCharacterID = '%i' ORDER BY Slot", c->CharacterID()), TempErrorMessageBuffer, &DatasetResult)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			while(DataRow = mysql_fetch_row(DatasetResult)) {
				uint8 slot = atoi(DataRow[1]);

				if(slot >= MAXMERCS) {
					continue;
				}

				c->GetMercInfo(slot).mercid = atoi(DataRow[0]);
				c->GetMercInfo(slot).slot = slot;
				snprintf(c->GetMercInfo(slot).merc_name, 64, "%s", std::string(DataRow[2]).c_str());
				c->GetMercInfo(slot).MercTemplateID = atoi(DataRow[3]);
				c->GetMercInfo(slot).SuspendedTime = atoi(DataRow[4]);
				c->GetMercInfo(slot).IsSuspended = atoi(DataRow[5]) == 1 ? true : false;
				c->GetMercInfo(slot).MercTimerRemaining = atoi(DataRow[6]);
				c->GetMercInfo(slot).Gender = atoi(DataRow[7]);
				c->GetMercInfo(slot).State = 5;
				c->GetMercInfo(slot).Stance = atoi(DataRow[8]);
				c->GetMercInfo(slot).hp = atoi(DataRow[9]);
				c->GetMercInfo(slot).mana = atoi(DataRow[10]);
				c->GetMercInfo(slot).endurance = atoi(DataRow[11]);
				c->GetMercInfo(slot).face = atoi(DataRow[12]);
				c->GetMercInfo(slot).luclinHairStyle = atoi(DataRow[13]);
				c->GetMercInfo(slot).luclinHairColor = atoi(DataRow[14]);
				c->GetMercInfo(slot).luclinEyeColor = atoi(DataRow[15]);
				c->GetMercInfo(slot).luclinEyeColor2 = atoi(DataRow[16]);
				c->GetMercInfo(slot).luclinBeardColor = atoi(DataRow[17]);
				c->GetMercInfo(slot).luclinBeard = atoi(DataRow[18]);
				c->GetMercInfo(slot).drakkinHeritage = atoi(DataRow[19]);
				c->GetMercInfo(slot).drakkinTattoo = atoi(DataRow[20]);
				c->GetMercInfo(slot).drakkinDetails = atoi(DataRow[21]);
				loaded = true;
			}

			mysql_free_result(DatasetResult);
		}

		safe_delete_array(Query);
	}

	return loaded;
}

bool ZoneDatabase::LoadCurrentMerc(Client *c) {
	bool loaded = false;

	if(c->GetEPP().merc_name[0] != 0) {
		std::string errorMessage;
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
		MYSQL_RES* DatasetResult;
		MYSQL_ROW DataRow;
		//char name[64];

		uint8 slot = c->GetMercSlot();

		if(slot > MAXMERCS) {
			return false;
		}

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT MercID, Name, TemplateID, SuspendedTime, IsSuspended, TimerRemaining, Gender, StanceID, HP, Mana, Endurance, Face, LuclinHairStyle, LuclinHairColor, LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, DrakkinHeritage, DrakkinTattoo, DrakkinDetails FROM mercs WHERE OwnerCharacterID = '%i' AND Slot = '%u'", c->CharacterID(), slot), TempErrorMessageBuffer, &DatasetResult)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			while(DataRow = mysql_fetch_row(DatasetResult)) {
				c->GetMercInfo(slot).mercid = atoi(DataRow[0]);
				c->GetMercInfo(slot).slot = slot;
				snprintf(c->GetMercInfo(slot).merc_name, 64, "%s", std::string(DataRow[1]).c_str());
				c->GetMercInfo(slot).MercTemplateID = atoi(DataRow[2]);
				c->GetMercInfo(slot).SuspendedTime = atoi(DataRow[3]);
				c->GetMercInfo(slot).IsSuspended = atoi(DataRow[4]) == 1 ? true : false;
				c->GetMercInfo(slot).MercTimerRemaining = atoi(DataRow[5]);
				c->GetMercInfo(slot).Gender = atoi(DataRow[6]);
				c->GetMercInfo(slot).State = atoi(DataRow[7]);
				c->GetMercInfo(slot).hp = atoi(DataRow[8]);
				c->GetMercInfo(slot).mana = atoi(DataRow[9]);
				c->GetMercInfo(slot).endurance = atoi(DataRow[10]);
				c->GetMercInfo(slot).face = atoi(DataRow[11]);
				c->GetMercInfo(slot).luclinHairStyle = atoi(DataRow[12]);
				c->GetMercInfo(slot).luclinHairColor = atoi(DataRow[13]);
				c->GetMercInfo(slot).luclinEyeColor = atoi(DataRow[14]);
				c->GetMercInfo(slot).luclinEyeColor2 = atoi(DataRow[15]);
				c->GetMercInfo(slot).luclinBeardColor = atoi(DataRow[16]);
				c->GetMercInfo(slot).luclinBeard = atoi(DataRow[17]);
				c->GetMercInfo(slot).drakkinHeritage = atoi(DataRow[18]);
				c->GetMercInfo(slot).drakkinTattoo = atoi(DataRow[19]);
				c->GetMercInfo(slot).drakkinDetails = atoi(DataRow[20]);
				loaded = true;
			}

			mysql_free_result(DatasetResult);
		}

		safe_delete_array(Query);
	}

	return loaded;
}

bool ZoneDatabase::SaveMerc(Merc *merc) {
	Client *owner = merc->GetMercOwner();
	bool Result = false;
	std::string errorMessage;

	if(!owner) {
		return false;
	}

	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
	uint32 affectedRows = 0;

	if(merc->GetMercID() == 0) {
		// New merc record
		uint32 TempNewMercID = 0;
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "INSERT INTO mercs (OwnerCharacterID, Slot, Name, TemplateID, SuspendedTime, IsSuspended, TimerRemaining, Gender, StanceID, HP, Mana, Endurance, Face, LuclinHairStyle, LuclinHairColor, LuclinEyeColor, LuclinEyeColor2, LuclinBeardColor, LuclinBeard, DrakkinHeritage, DrakkinTattoo, DrakkinDetails) VALUES('%u', '%u', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%i', '%i', '%i', '%i', '%i', '%i', '%i', '%i', '%i', '%i')", merc->GetMercCharacterID(), owner->GetNumMercs(), merc->GetCleanName(), merc->GetMercTemplateID(), owner->GetMercInfo().SuspendedTime, merc->IsSuspended(), owner->GetMercInfo().MercTimerRemaining, merc->GetGender(), merc->GetStance(), merc->GetHP(), merc->GetMana(), merc->GetEndurance(), merc->GetLuclinFace(), merc->GetHairStyle(), merc->GetHairColor(), merc->GetEyeColor1(), merc->GetEyeColor2(), merc->GetBeardColor(), merc->GetBeard(), merc->GetDrakkinHeritage(), merc->GetDrakkinTattoo(), merc->GetDrakkinDetails() ), TempErrorMessageBuffer, 0, &affectedRows, &TempNewMercID)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			merc->SetMercID(TempNewMercID);
			Result = true;
		}
	}
	else {
		// Update existing merc record
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE mercs SET OwnerCharacterID = '%u', Slot = '%u', Name = '%s', TemplateID = '%u', SuspendedTime = '%u', IsSuspended = '%u', TimerRemaining = '%u', Gender = '%u', StanceID = '%u', HP = '%u', Mana = '%u', Endurance = '%u', Face = '%i', LuclinHairStyle = '%i', LuclinHairColor = '%i', LuclinEyeColor = '%i', LuclinEyeColor2 = '%i', LuclinBeardColor = '%i', LuclinBeard = '%i', DrakkinHeritage = '%i', DrakkinTattoo = '%i', DrakkinDetails = '%i' WHERE MercID = '%u'", merc->GetMercCharacterID(), owner->GetMercSlot(), merc->GetCleanName(), merc->GetMercTemplateID(), owner->GetMercInfo().SuspendedTime, merc->IsSuspended(), owner->GetMercInfo().MercTimerRemaining, merc->GetGender(), merc->GetStance(), merc->GetHP(), merc->GetMana(), merc->GetEndurance(), merc->GetLuclinFace(), merc->GetHairStyle(), merc->GetHairColor(), merc->GetEyeColor1(), merc->GetEyeColor2(), merc->GetBeardColor(), merc->GetBeard(), merc->GetDrakkinHeritage(), merc->GetDrakkinTattoo(), merc->GetDrakkinDetails(), merc->GetMercID()), TempErrorMessageBuffer, 0, &affectedRows)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		else {
			Result = true;
			//time(&_startTotalPlayTime);
		}
	}

	safe_delete_array(Query);

	if(!errorMessage.empty() || (Result && affectedRows != 1)) {
		if(owner && !errorMessage.empty())
			owner->Message(13, errorMessage.c_str());
		else if(owner)
			owner->Message(13, std::string("Unable to save merc to the database.").c_str());

		Result = false;
	}
	else {
		merc->UpdateMercInfo(owner);
		database.SaveMercBuffs(merc);
		//database.SaveMercStance(this);
		//database.SaveMercTimers(this);
	}

	return Result;
}

void ZoneDatabase::SaveMercBuffs(Merc *merc) {
	Buffs_Struct *buffs = merc->GetBuffs();
	std::string errorMessage;
	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
	int BuffCount = 0;
	int InsertCount = 0;

	uint32 buff_count = merc->GetMaxBuffSlots();
	while(BuffCount < BUFF_COUNT) {
		if(buffs[BuffCount].spellid > 0 && buffs[BuffCount].spellid != SPELL_UNKNOWN) {
			if(InsertCount == 0) {
				// Remove any existing buff saves
				if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM merc_buffs WHERE MercId = %u", merc->GetMercID()), TempErrorMessageBuffer)) {
					errorMessage = std::string(TempErrorMessageBuffer);
					safe_delete(Query);
					Query = 0;
					break;
				}
			}

			int IsPersistent = 0;

			if(buffs[BuffCount].persistant_buff)
				IsPersistent = 1;
			else
				IsPersistent = 0;

			if(!database.RunQuery(Query, MakeAnyLenString(&Query, "INSERT INTO merc_buffs (MercId, SpellId, CasterLevel, DurationFormula, "
				"TicsRemaining, PoisonCounters, DiseaseCounters, CurseCounters, CorruptionCounters, HitCount, MeleeRune, MagicRune, "
				"dot_rune, caston_x, Persistent, caston_y, caston_z, ExtraDIChance) VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %i, %u, %i, %i, %i);",
				merc->GetMercID(), buffs[BuffCount].spellid, buffs[BuffCount].casterlevel, spells[buffs[BuffCount].spellid].buffdurationformula,
				buffs[BuffCount].ticsremaining,
				CalculatePoisonCounters(buffs[BuffCount].spellid) > 0 ? buffs[BuffCount].counters : 0,
				CalculateDiseaseCounters(buffs[BuffCount].spellid) > 0 ? buffs[BuffCount].counters : 0,
				CalculateCurseCounters(buffs[BuffCount].spellid) > 0 ? buffs[BuffCount].counters : 0,
				CalculateCorruptionCounters(buffs[BuffCount].spellid) > 0 ? buffs[BuffCount].counters : 0,
				buffs[BuffCount].numhits, buffs[BuffCount].melee_rune, buffs[BuffCount].magic_rune,
				buffs[BuffCount].dot_rune,
				buffs[BuffCount].caston_x, 
				IsPersistent, 
				buffs[BuffCount].caston_y, 
				buffs[BuffCount].caston_z,
				buffs[BuffCount].ExtraDIChance), TempErrorMessageBuffer)) {
				errorMessage = std::string(TempErrorMessageBuffer);
				safe_delete(Query);
				Query = 0;
				break;
			}
			else {
				safe_delete(Query);
				Query = 0;
				InsertCount++;
			}
		}

		BuffCount++;
	}

	if(!errorMessage.empty()) {
		LogFile->write(EQEMuLog::Error, "Error Saving Merc Buffs: %s", errorMessage.c_str());
	}
}

void ZoneDatabase::LoadMercBuffs(Merc *merc) {
	Buffs_Struct *buffs = merc->GetBuffs();
	uint32 max_slots = merc->GetMaxBuffSlots();
	std::string errorMessage;
	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	bool BuffsLoaded = false;

	if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT SpellId, CasterLevel, DurationFormula, TicsRemaining, PoisonCounters, DiseaseCounters, CurseCounters, CorruptionCounters, HitCount, MeleeRune, MagicRune, dot_rune, caston_x, Persistent, caston_y, caston_z, ExtraDIChance FROM merc_buffs WHERE MercId = %u", merc->GetMercID()), TempErrorMessageBuffer, &DatasetResult)) {
		errorMessage = std::string(TempErrorMessageBuffer);
	}
	else {
		int BuffCount = 0;

		while(DataRow = mysql_fetch_row(DatasetResult)) {
			if(BuffCount == BUFF_COUNT)
				break;

			buffs[BuffCount].spellid = atoi(DataRow[0]);
			buffs[BuffCount].casterlevel = atoi(DataRow[1]);
			buffs[BuffCount].ticsremaining = atoi(DataRow[3]);

			if(CalculatePoisonCounters(buffs[BuffCount].spellid) > 0) {
				buffs[BuffCount].counters = atoi(DataRow[4]);
			} else if(CalculateDiseaseCounters(buffs[BuffCount].spellid) > 0) {
				buffs[BuffCount].counters = atoi(DataRow[5]);
			} else if(CalculateCurseCounters(buffs[BuffCount].spellid) > 0) {
				buffs[BuffCount].counters = atoi(DataRow[6]);
			} else if(CalculateCorruptionCounters(buffs[BuffCount].spellid) > 0) {
				buffs[BuffCount].counters = atoi(DataRow[7]);
			}
			buffs[BuffCount].numhits = atoi(DataRow[8]);
			buffs[BuffCount].melee_rune = atoi(DataRow[9]);
			buffs[BuffCount].magic_rune = atoi(DataRow[10]);
			buffs[BuffCount].dot_rune = atoi(DataRow[11]);
			buffs[BuffCount].caston_x = atoi(DataRow[12]);
			buffs[BuffCount].casterid = 0;

			bool IsPersistent = false;

			if(atoi(DataRow[13]))
				IsPersistent = true;

			buffs[BuffCount].caston_y = atoi(DataRow[13]);
			buffs[BuffCount].caston_z = atoi(DataRow[14]);
			buffs[BuffCount].ExtraDIChance = atoi(DataRow[15]);

			buffs[BuffCount].persistant_buff = IsPersistent;

			BuffCount++;
		}

		mysql_free_result(DatasetResult);

		BuffsLoaded = true;
	}

	safe_delete_array(Query);

	if(errorMessage.empty() && BuffsLoaded) {
		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM merc_buffs WHERE MercId = %u", merc->GetMercID()), TempErrorMessageBuffer)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		safe_delete_array(Query);
	}

	if(!errorMessage.empty()) {
		LogFile->write(EQEMuLog::Error, "Error Loading Merc Buffs: %s", errorMessage.c_str());
	}
}

bool ZoneDatabase::DeleteMerc(uint32 merc_id) {
	std::string errorMessage;
	bool Result = false;
	int TempCounter = 0;

	if(merc_id > 0) {
		char* Query = 0;
		char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];

		// TODO: These queries need to be ran together as a transaction.. ie, if one or more fail then they all will fail to commit to the database.

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM merc_buffs WHERE MercID = '%u'", merc_id), TempErrorMessageBuffer)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		else
			TempCounter++;

		safe_delete_array(Query);

		if(!database.RunQuery(Query, MakeAnyLenString(&Query, "DELETE FROM mercs WHERE MercID = '%u'", merc_id), TempErrorMessageBuffer)) {
			errorMessage = std::string(TempErrorMessageBuffer);
		}
		else
			TempCounter++;

		safe_delete_array(Query);

		if(TempCounter == 2)
			Result = true;
	}

	if(!errorMessage.empty()) {
		LogFile->write(EQEMuLog::Error, "Error Deleting Merc: %s", errorMessage.c_str());
	}

	return Result;
}

void ZoneDatabase::LoadMercEquipment(Merc *merc) {
	std::string errorMessage;
	char* Query = 0;
	char TempErrorMessageBuffer[MYSQL_ERRMSG_SIZE];
	MYSQL_RES* DatasetResult;
	MYSQL_ROW DataRow;

	if(!database.RunQuery(Query, MakeAnyLenString(&Query, "SELECT item_id FROM merc_inventory WHERE merc_subtype_id = (SELECT merc_subtype_id FROM merc_subtypes WHERE class_id = '%u' AND tier_id = '%u') AND min_level <= %u AND max_level >= %u", merc->GetClass(), merc->GetTierID(), merc->GetLevel(), merc->GetLevel()), TempErrorMessageBuffer, &DatasetResult)) {
		errorMessage = std::string(TempErrorMessageBuffer);
	}
	else {
		int itemCount = 0;

		while(DataRow = mysql_fetch_row(DatasetResult)) {
			if (itemCount == EmuConstants::EQUIPMENT_SIZE)
				break;

			if(atoi(DataRow[0]) > 0) {
				merc->AddItem(itemCount, atoi(DataRow[0]));

				itemCount++;
			}
		}

		mysql_free_result(DatasetResult);
	}

	safe_delete_array(Query);
	Query = 0;

	if(!errorMessage.empty()) {
		LogFile->write(EQEMuLog::Error, "Error Loading Merc Inventory: %s", errorMessage.c_str());
	}
}

uint8 ZoneDatabase::GetGridType(uint32 grid, uint32 zoneid ) {
	char *query = 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	MYSQL_RES *result;
	MYSQL_ROW row;
	int type = 0;
	if (RunQuery(query, MakeAnyLenString(&query,"SELECT type from grid where id = %i and zoneid = %i",grid,zoneid),errbuf,&result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			type = atoi( row[0] );
		}
			mysql_free_result(result);
	} else {
		std::cerr << "Error in GetGridType query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
	}
	return type;
}



void ZoneDatabase::SaveMerchantTemp(uint32 npcid, uint32 slot, uint32 item, uint32 charges){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "replace into merchantlist_temp (npcid,slot,itemid,charges) values(%d,%d,%d,%d)", npcid, slot, item, charges), errbuf)) {
		std::cerr << "Error in SaveMerchantTemp query '" << query << "' " << errbuf << std::endl;
	}
	safe_delete_array(query);
}
void ZoneDatabase::DeleteMerchantTemp(uint32 npcid, uint32 slot){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "delete from merchantlist_temp where npcid=%d and slot=%d", npcid, slot), errbuf)) {
		std::cerr << "Error in DeleteMerchantTemp query '" << query << "' " << errbuf << std::endl;
	}
	safe_delete_array(query);
}


bool ZoneDatabase::UpdateZoneSafeCoords(const char* zonename, float x=0, float y=0, float z=0) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32	affected_rows = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE zone SET safe_x='%f', safe_y='%f', safe_z='%f' WHERE short_name='%s';", x, y, z, zonename), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	if (affected_rows == 0)
	{
		return false;
	}

	return true;
}


uint8 ZoneDatabase::GetUseCFGSafeCoords()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT value FROM variables WHERE varname='UseCFGSafeCoords'"), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);

			uint8 usecoords = atoi(row[0]);
			mysql_free_result(result);
			return usecoords;
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

		std::cerr << "Error in GetUseCFGSafeCoords query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	return 0;

}


uint32 ZoneDatabase::GetServerFilters(char* name, ServerSideFilters_Struct *ssfs) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;

	MYSQL_ROW row;


	unsigned long* lengths;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT serverfilters FROM account WHERE name='%s'", name), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			lengths = mysql_fetch_lengths(result);
			if (lengths[0] == sizeof(ServerSideFilters_Struct)) {
				memcpy(ssfs, row[0], sizeof(ServerSideFilters_Struct));
			}
			else {
				std::cerr << "Player profile length mismatch in ServerSideFilters" << std::endl;
				mysql_free_result(result);
				return 0;
			}
		}
		else {
			mysql_free_result(result);
			return 0;

		}
		uint32 len = lengths[0];
		mysql_free_result(result);
		return len;
	}
	else {
		std::cerr << "Error in ServerSideFilters query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return 0;
	}

	return 0;
}

bool ZoneDatabase::SetServerFilters(char* name, ServerSideFilters_Struct *ssfs) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char query[256+sizeof(ServerSideFilters_Struct)*2+1];
	char* end = query;

	//if (strlen(name) > 15)
	//	return false;

	/*for (int i=0; i<strlen(name); i++)
	{
	if ((name[i] < 'a' || name[i] > 'z') &&
	(name[i] < 'A' || name[i] > 'Z') &&
	(name[i] < '0' || name[i] > '9'))
	return 0;
}*/


	end += sprintf(end, "UPDATE account SET serverfilters=");
	*end++ = '\'';
	end += DoEscapeString(end, (char*)ssfs, sizeof(ServerSideFilters_Struct));
	*end++ = '\'';
	end += sprintf(end," WHERE name='%s'", name);

	uint32 affected_rows = 0;
	if (!RunQuery(query, (uint32) (end - query), errbuf, 0, &affected_rows)) {
		std::cerr << "Error in SetServerSideFilters query " << errbuf << std::endl;
		return false;
	}

	if (affected_rows == 0) {
		return false;
	}

	return true;
}


//New functions for timezone
uint32 ZoneDatabase::GetZoneTZ(uint32 zoneid, uint32 version) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT timezone FROM zone WHERE zoneidnumber=%i AND (version=%i OR version=0) ORDER BY version DESC", zoneid, version), errbuf, &result))
	{
		safe_delete_array(query);
		if (mysql_num_rows(result) > 0) {
			row = mysql_fetch_row(result);
			uint32 tmp = atoi(row[0]);
			mysql_free_result(result);
			return tmp;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetZoneTZ query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
	}
	return 0;
}

bool ZoneDatabase::SetZoneTZ(uint32 zoneid, uint32 version, uint32 tz) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "UPDATE zone SET timezone=%i WHERE zoneidnumber=%i AND version=%i", tz, zoneid, version), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);

		if (affected_rows == 1)
			return true;
		else
			return false;
	}
	else {
		std::cerr << "Error in SetZoneTZ query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	return false;
}
//End new timezone functions.

/*
 solar: this is never actually called, client_process starts an async query
 instead and uses GetAccountInfoForLogin_result to process it..
 */
bool ZoneDatabase::GetAccountInfoForLogin(uint32 account_id, int16* admin, char* account_name, uint32* lsaccountid, uint8* gmspeed, bool* revoked,bool* gmhideme) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT status, name, lsaccount_id, gmspeed, revoked, hideme FROM account WHERE id=%i", account_id), errbuf, &result)) {
		safe_delete_array(query);
		bool ret = GetAccountInfoForLogin_result(result, admin, account_name, lsaccountid, gmspeed, revoked,gmhideme);
		mysql_free_result(result);
		return ret;
	}
	else
	{
		std::cerr << "Error in GetAccountInfoForLogin query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	return false;
}

void ZoneDatabase::RefreshGroupFromDB(Client *c){
	if(!c){
		return;
	}

	Group *g = c->GetGroup();

	if(!g){
		return;
	}

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = groupActUpdate;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	strcpy(gu->yourname, c->GetName());
	GetGroupLeadershipInfo(g->GetID(), gu->leadersname, nullptr, nullptr, nullptr, nullptr, &gu->leader_aas);
	gu->NPCMarkerID = g->GetNPCMarkerID();

	int index = 0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT name from group_id where groupid=%d", g->GetID()), errbuf, &result)) {
		while((row = mysql_fetch_row(result))){
			if(index < 6){
				if(strcmp(c->GetName(), row[0]) != 0){
					strcpy(gu->membername[index], row[0]);
					index++;
				}
			}
		}
		mysql_free_result(result);
	}
	else
	{
		printf("Error in group update query: %s\n", errbuf);
	}
	safe_delete_array(query);

	c->QueuePacket(outapp);
	safe_delete(outapp);

	if(c->GetClientVersion() >= EQClientSoD) {
		g->NotifyMainTank(c, 1);
		g->NotifyPuller(c, 1);
	}

	g->NotifyMainAssist(c, 1);

	g->NotifyMarkNPC(c);
	g->NotifyAssistTarget(c);
	g->NotifyTankTarget(c);
	g->NotifyPullerTarget(c);
	g->SendMarkedNPCsToMember(c);

}

uint8 ZoneDatabase::GroupCount(uint32 groupid){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint8 count=0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT count(charid) FROM group_id WHERE groupid=%d", groupid), errbuf, &result)) {
		if((row = mysql_fetch_row(result))!=nullptr)
			count = atoi(row[0]);
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in ZoneDatabase::GroupCount query '%s': %s", query, errbuf);
	}
	safe_delete_array(query);
	return count;
}

 uint8 ZoneDatabase::RaidGroupCount(uint32 raidid, uint32 groupid)
 {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint8 count=0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT count(charid) FROM raid_members WHERE raidid=%d AND groupid=%d;", raidid, groupid), errbuf, &result)) {
		if((row = mysql_fetch_row(result))!=nullptr)
			count = atoi(row[0]);
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in ZoneDatabase::RaidGroupCount query '%s': %s", query, errbuf);
	}
	safe_delete_array(query);
	return count;
 }

int32 ZoneDatabase::GetBlockedSpellsCount(uint32 zoneid)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	sprintf(query, "SELECT count(*) FROM blocked_spells WHERE zoneid=%d", zoneid);
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row != nullptr && row[0] != 0) {
			int32 ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetBlockedSpellsCount query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return -1;
	}

	return -1;
}

bool ZoneDatabase::LoadBlockedSpells(int32 blockedSpellsCount, ZoneSpellsBlocked* into, uint32 zoneid)
{
	LogFile->write(EQEMuLog::Status, "Loading Blocked Spells from database...");

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	MakeAnyLenString(&query, "SELECT id, spellid, type, x, y, z, x_diff, y_diff, z_diff, message "
		"FROM blocked_spells WHERE zoneid=%d ORDER BY id asc", zoneid);
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		int32 r;
		for(r = 0; (row = mysql_fetch_row(result)); r++) {
			if(r >= blockedSpellsCount) {
				std::cerr << "Error, Blocked Spells Count of " << blockedSpellsCount << " exceeded." << std::endl;
				break;
			}
			memset(&into[r], 0, sizeof(ZoneSpellsBlocked));
			if(row){
				into[r].spellid = atoi(row[1]);
				into[r].type = atoi(row[2]);
				into[r].x = atof(row[3]);
				into[r].y = atof(row[4]);
				into[r].z = atof(row[5]);
				into[r].xdiff = atof(row[6]);
				into[r].ydiff = atof(row[7]);
				into[r].zdiff = atof(row[8]);
				strn0cpy(into[r].message, row[9], 255);
			}
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error in LoadBlockedSpells query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}
	return true;
}

int ZoneDatabase::getZoneShutDownDelay(uint32 zoneID, uint32 version)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT shutdowndelay FROM zone WHERE zoneidnumber=%i AND (version=%i OR version=0) ORDER BY version DESC", zoneID, version), errbuf, &result))
	{
		if (mysql_num_rows(result) > 0) {
			row = mysql_fetch_row(result);
			int retVal = atoi(row[0]);

			mysql_free_result(result);
			safe_delete_array(query);
			return (retVal);
		}
		else {
			std::cerr << "Error in getZoneShutDownDelay no result '" << query << "' " << errbuf << std::endl;
			mysql_free_result(result);
			safe_delete_array(query);
			return (RuleI(Zone, AutoShutdownDelay));
		}
	}
	else {
		std::cerr << "Error in getZoneShutDownDelay query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
	}
	return (RuleI(Zone, AutoShutdownDelay));
}

uint32 ZoneDatabase::GetKarma(uint32 acct_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 ret_val = 0;

	if (!RunQuery(query,MakeAnyLenString(&query, "select `karma` from `account` where `id`='%i' limit 1",
		acct_id),errbuf,&result))
	{
		safe_delete_array(query);
		return 0;
	}

	safe_delete_array(query);
	row = mysql_fetch_row(result);

	ret_val = atoi(row[0]);

	mysql_free_result(result);

	return ret_val;
}

void ZoneDatabase::UpdateKarma(uint32 acct_id, uint32 amount)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "UPDATE account set karma=%i where id=%i", amount, acct_id), errbuf, 0, &affected_rows)){
		safe_delete_array(query);}
	else {
		std::cerr << "Error in UpdateKarma query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
	}
}

void ZoneDatabase::ListAllInstances(Client* c, uint32 charid)
{
	if(!c)
		return;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;


	if (RunQuery(query,MakeAnyLenString(&query, "SELECT instance_list.id, zone, version FROM instance_list JOIN"
		" instance_list_player ON instance_list.id = instance_list_player.id"
		" WHERE instance_list_player.charid=%lu", (unsigned long)charid),errbuf,&result))
	{
		safe_delete_array(query);

		char name[64];
		database.GetCharName(charid, name);
		c->Message(0, "%s is part of the following instances:", name);
		while(row = mysql_fetch_row(result))
		{
			c->Message(0, "%s - id: %lu, version: %lu", database.GetZoneName(atoi(row[1])),
				(unsigned long)atoi(row[0]), (unsigned long)atoi(row[2]));
		}

		mysql_free_result(result);
	}
	else
	{
		safe_delete_array(query);
	}
}

void ZoneDatabase::QGlobalPurge()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM quest_globals WHERE expdate < UNIX_TIMESTAMP()"),
		errbuf);
	safe_delete_array(query);
}

void ZoneDatabase::InsertDoor(uint32 ddoordbid, uint16 ddoorid, const char* ddoor_name, float dxpos, float dypos, float dzpos, float dheading, uint8 dopentype, uint16 dguildid, uint32 dlockpick, uint32 dkeyitem, uint8 ddoor_param, uint8 dinvert, int dincline, uint16 dsize){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	if (!RunQuery(query, MakeAnyLenString(&query, "replace into doors (id, doorid,zone,version,name,pos_x,pos_y,pos_z,heading,opentype,guild,lockpick,keyitem,door_param,invert_state,incline,size) values('%i','%i','%s','%i', '%s','%f','%f','%f','%f','%i','%i','%i', '%i','%i','%i','%i','%i')", ddoordbid ,ddoorid ,zone->GetShortName(), zone->GetInstanceVersion(), ddoor_name, dxpos, dypos, dzpos, dheading, dopentype, dguildid, dlockpick, dkeyitem, ddoor_param, dinvert, dincline, dsize), errbuf))	{
		std::cerr << "Error in InsertDoor" << query << "' " << errbuf << std::endl;
	}
	safe_delete_array(query);
}

void ZoneDatabase::LoadAltCurrencyValues(uint32 char_id, std::map<uint32, uint32> &currency) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT currency_id, amount FROM character_alt_currency where char_id='%u'", char_id), errbuf, &result)) {
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result)))
		{
			currency[atoi(row[0])] = atoi(row[1]);
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in LoadAltCurrencyValues query '%s': %s", query, errbuf);
		safe_delete_array(query);
	}
}

void ZoneDatabase::UpdateAltCurrencyValue(uint32 char_id, uint32 currency_id, uint32 value) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	database.RunQuery(query, MakeAnyLenString(&query, "REPLACE INTO character_alt_currency (char_id, currency_id, amount)"
		" VALUES('%u', '%u', '%u')", char_id, currency_id, value),
		errbuf);
	safe_delete_array(query);
}

void ZoneDatabase::SaveBuffs(Client *c) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `character_buffs` WHERE `character_id`='%u'", c->CharacterID()),
		errbuf);

	uint32 buff_count = c->GetMaxBuffSlots();
	Buffs_Struct *buffs = c->GetBuffs();
	for (int i = 0; i < buff_count; i++) {
		if(buffs[i].spellid != SPELL_UNKNOWN) {
			if(!database.RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `character_buffs` (character_id, slot_id, spell_id, "
				"caster_level, caster_name, ticsremaining, counters, numhits, melee_rune, magic_rune, persistent, dot_rune, "
				"caston_x, caston_y, caston_z, ExtraDIChance) VALUES('%u', '%u', '%u', '%u', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%i', '%i', '%i', '%i')",
				c->CharacterID(), i, buffs[i].spellid, buffs[i].casterlevel, buffs[i].caster_name, buffs[i].ticsremaining,
				buffs[i].counters, buffs[i].numhits, buffs[i].melee_rune, buffs[i].magic_rune, buffs[i].persistant_buff,
				buffs[i].dot_rune, buffs[i].caston_x, buffs[i].caston_y, buffs[i].caston_z, buffs[i].ExtraDIChance),
				errbuf)) {
				LogFile->write(EQEMuLog::Error, "Error in SaveBuffs query '%s': %s", query, errbuf);
			}
		}
	}
	safe_delete_array(query);
}

void ZoneDatabase::LoadBuffs(Client *c) {
	Buffs_Struct *buffs = c->GetBuffs();
	uint32 max_slots = c->GetMaxBuffSlots();
	for(int i = 0; i < max_slots; ++i) {
		buffs[i].spellid = SPELL_UNKNOWN;
	}


	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT spell_id, slot_id, caster_level, caster_name, ticsremaining, counters, "
		"numhits, melee_rune, magic_rune, persistent, dot_rune, caston_x, caston_y, caston_z, ExtraDIChance FROM `character_buffs` WHERE "
		"`character_id`='%u'",
		c->CharacterID()), errbuf, &result))
	{
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result)))
		{
			uint32 slot_id = atoul(row[1]);
			if(slot_id >= c->GetMaxBuffSlots()) {
				continue;
			}

			uint32 spell_id = atoul(row[0]);
			if(!IsValidSpell(spell_id)) {
				continue;
			}

			Client *caster = entity_list.GetClientByName(row[3]);
			uint32 caster_level = atoi(row[2]);
			uint32 ticsremaining = atoul(row[4]);
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

			buffs[slot_id].spellid = spell_id;
			buffs[slot_id].casterlevel = caster_level;
			if(caster) {
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

		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in LoadBuffs query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return;
	}

	max_slots = c->GetMaxBuffSlots();
	for(int i = 0; i < max_slots; ++i) {
		if(!IsValidSpell(buffs[i].spellid)) {
			continue;
		}

		for(int j = 0; j < 12; ++j) {
			bool cont = false;
			switch(spells[buffs[i].spellid].effectid[j]) {
			case SE_Charm:
				buffs[i].spellid = SPELL_UNKNOWN;
				cont = true;
				break;
			case SE_Illusion:
				if(!buffs[i].persistant_buff) {
					buffs[i].spellid = SPELL_UNKNOWN;
					cont = true;
				}
				break;
			}

			if(cont) {
				break;
			}
		}
	}
}

void ZoneDatabase::SavePetInfo(Client *c) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	int i = 0;
	PetInfo *petinfo = c->GetPetInfo(0);
	PetInfo *suspended = c->GetPetInfo(1);

	if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `character_pet_buffs` WHERE `char_id`=%u", c->CharacterID()),
		errbuf)) {
		safe_delete_array(query);
		return;
	}
	safe_delete_array(query);
	if (!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `character_pet_inventory` WHERE `char_id`=%u", c->CharacterID()),
		errbuf)) {
		safe_delete_array(query);
		// error report
		return;
	}
	safe_delete_array(query);

	if(!database.RunQuery(query, MakeAnyLenString(&query,
		"INSERT INTO `character_pet_info` (`char_id`, `pet`, `petname`, `petpower`, `spell_id`, `hp`, `mana`, `size`) "
		"values (%u, 0, '%s', %i, %u, %u, %u, %f) "
		"ON DUPLICATE KEY UPDATE `petname`='%s', `petpower`=%i, `spell_id`=%u, `hp`=%u, `mana`=%u, `size`=%f",
		c->CharacterID(), petinfo->Name, petinfo->petpower, petinfo->SpellID, petinfo->HP, petinfo->Mana, petinfo->size,
		petinfo->Name, petinfo->petpower, petinfo->SpellID, petinfo->HP, petinfo->Mana, petinfo->size),
		errbuf))
	{
		safe_delete_array(query);
		return;
	}
	safe_delete_array(query);

	for(i=0; i < RuleI(Spells, MaxTotalSlotsPET); i++) {
		if (petinfo->Buffs[i].spellid != SPELL_UNKNOWN && petinfo->Buffs[i].spellid != 0) {
			database.RunQuery(query, MakeAnyLenString(&query,
				"INSERT INTO `character_pet_buffs` (`char_id`, `pet`, `slot`, `spell_id`, `caster_level`, "
				"`ticsremaining`, `counters`) values "
				"(%u, 0, %u, %u, %u, %u, %d)",
				c->CharacterID(), i, petinfo->Buffs[i].spellid, petinfo->Buffs[i].level, petinfo->Buffs[i].duration,
				petinfo->Buffs[i].counters),
				errbuf);
			safe_delete_array(query);
		}
		if (suspended->Buffs[i].spellid != SPELL_UNKNOWN && suspended->Buffs[i].spellid != 0) {
			database.RunQuery(query, MakeAnyLenString(&query,
				"INSERT INTO `character_pet_buffs` (`char_id`, `pet`, `slot`, `spell_id`, `caster_level`, "
				"`ticsremaining`, `counters`) values "
				"(%u, 1, %u, %u, %u, %u, %d)",
				c->CharacterID(), i, suspended->Buffs[i].spellid, suspended->Buffs[i].level, suspended->Buffs[i].duration,
				suspended->Buffs[i].counters),
				errbuf);
			safe_delete_array(query);
		}
	}

	for (i = 0; i<EmuConstants::EQUIPMENT_SIZE; i++) {
		if(petinfo->Items[i]) {
			database.RunQuery(query, MakeAnyLenString(&query,
				"INSERT INTO `character_pet_inventory` (`char_id`, `pet`, `slot`, `item_id`) values (%u, 0, %u, %u)",
				c->CharacterID(), i, petinfo->Items[i]), errbuf);
			// should check for errors
			safe_delete_array(query);
		}
	}


	if(!database.RunQuery(query, MakeAnyLenString(&query,
		"INSERT INTO `character_pet_info` (`char_id`, `pet`, `petname`, `petpower`, `spell_id`, `hp`, `mana`, `size`) "
		"values (%u, 1, '%s', %u, %u, %u, %u, %f) "
		"ON DUPLICATE KEY UPDATE `petname`='%s', `petpower`=%i, `spell_id`=%u, `hp`=%u, `mana`=%u, `size`=%f",
		c->CharacterID(), suspended->Name, suspended->petpower, suspended->SpellID, suspended->HP, suspended->Mana, suspended->size,
		suspended->Name, suspended->petpower, suspended->SpellID, suspended->HP, suspended->Mana, suspended->size),
		errbuf))
	{
		safe_delete_array(query);
		return;
	}
	safe_delete_array(query);

	for (i = 0; i<EmuConstants::EQUIPMENT_SIZE; i++) {
		if(suspended->Items[i]) {
			database.RunQuery(query, MakeAnyLenString(&query,
				"INSERT INTO `character_pet_inventory` (`char_id`, `pet`, `slot`, `item_id`) values (%u, 1, %u, %u)",
				c->CharacterID(), i, suspended->Items[i]), errbuf);
			// should check for errors
			safe_delete_array(query);
		}
	}

}

void ZoneDatabase::RemoveTempFactions(Client *c){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "DELETE FROM faction_values WHERE temp = 1 AND char_id=%u", c->CharacterID()), errbuf)) {
		std::cerr << "Error in RemoveTempFactions query '" << query << "' " << errbuf << std::endl;
	}
	safe_delete_array(query);
}

void ZoneDatabase::LoadPetInfo(Client *c) {
	// Load current pet and suspended pet
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	PetInfo *petinfo = c->GetPetInfo(0);
	PetInfo *suspended = c->GetPetInfo(1);
	PetInfo *pi;
	uint16 pet;

	memset(petinfo, 0, sizeof(PetInfo));
	memset(suspended, 0, sizeof(PetInfo));

	if(database.RunQuery(query, MakeAnyLenString(&query,
		"SELECT `pet`, `petname`, `petpower`, `spell_id`, `hp`, `mana`, `size` from `character_pet_info` where `char_id`=%u",
		c->CharacterID()), errbuf, &result))
	{
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result))) {
			pet = atoi(row[0]);
			if (pet == 0)
				pi = petinfo;
			else if (pet == 1)
				pi = suspended;
			else
				continue;

			strncpy(pi->Name,row[1],64);
			pi->petpower = atoi(row[2]);
			pi->SpellID = atoi(row[3]);
			pi->HP = atoul(row[4]);
			pi->Mana = atoul(row[5]);
			pi->size = atof(row[6]);
		}
		mysql_free_result(result);
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in LoadPetInfo query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return;
	}


	if (RunQuery(query, MakeAnyLenString(&query,
		"SELECT `pet`, `slot`, `spell_id`, `caster_level`, `castername`, "
		"`ticsremaining`, `counters` FROM `character_pet_buffs` "
		"WHERE `char_id`=%u",
		c->CharacterID()), errbuf, &result))
	{
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result)))
		{
			pet = atoi(row[0]);
			if (pet == 0)
				pi = petinfo;
			else if (pet == 1)
				pi = suspended;
			else
				continue;

			uint32 slot_id = atoul(row[1]);
			if(slot_id >= RuleI(Spells, MaxTotalSlotsPET)) {
				continue;
			}

			uint32 spell_id = atoul(row[2]);
			if(!IsValidSpell(spell_id)) {
				continue;
			}
			uint32 caster_level = atoi(row[3]);
			int caster_id = 0;
			// The castername field is currently unused
			//Client *caster = entity_list.GetClientByName(row[4]);
			//if (caster) { caster_id = caster->GetID(); }
			uint32 ticsremaining = atoul(row[5]);
			uint32 counters = atoul(row[6]);

			pi->Buffs[slot_id].spellid = spell_id;
			pi->Buffs[slot_id].level = caster_level;
			pi->Buffs[slot_id].player_id = caster_id;
			pi->Buffs[slot_id].slotid = 2;	// Always 2 in buffs struct for real buffs

			pi->Buffs[slot_id].duration = ticsremaining;
			pi->Buffs[slot_id].counters = counters;
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in LoadPetInfo query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return;
	}

	if (database.RunQuery(query, MakeAnyLenString(&query,
		"SELECT `pet`, `slot`, `item_id` FROM `character_pet_inventory` WHERE `char_id`=%u",
		c->CharacterID()), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result))) {
			pet = atoi(row[0]);
			if (pet == 0)
				pi = petinfo;
			else if (pet == 1)
				pi = suspended;
			else
				continue;
			
			int slot = atoi(row[1]);
			if (slot < 0 || slot > EmuConstants::EQUIPMENT_SIZE) // if (slot == 22) { zone.TriggerRandomCrash(); }
				continue;

			pi->Items[slot] = atoul(row[2]);
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in LoadPetInfo query '%s': %s", query, errbuf);
		safe_delete_array(query);
			return;
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

		std::map<std::string, int16>::iterator iter = faction_array[faction_id]->mods.find(str);
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

		std::map<std::string, int16>::iterator iter = faction_array[faction_id]->mods.find(str);
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

bool ZoneDatabase::LoadFactionValues(uint32 char_id, faction_map & val_list) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT faction_id,current_value FROM faction_values WHERE char_id = %i",char_id), errbuf, &result)) {
		safe_delete_array(query);
		bool ret = LoadFactionValues_result(result, val_list);
		mysql_free_result(result);
		return ret;
	}
	else {
		std::cerr << "Error in LoadFactionValues query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
	}
	return false;
}

bool ZoneDatabase::LoadFactionValues_result(MYSQL_RES* result, faction_map & val_list) {
	MYSQL_ROW row;
	while((row = mysql_fetch_row(result))) {
		val_list[atoi(row[0])] = atoi(row[1]);
	}
	return true;
}

//o--------------------------------------------------------------
//| Name: GetFactionName; rembrant, Dec. 16
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
//| Name: GetNPCFactionList; rembrant, Dec. 16, 2001
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
//| Name: SetCharacterFactionLevel; rembrant, Dec. 20, 2001
//o--------------------------------------------------------------
//| Purpose: Update characters faction level with specified faction_id to specified value. Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::SetCharacterFactionLevel(uint32 char_id, int32 faction_id, int32 value, uint8 temp, faction_map &val_list)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;

	if (!RunQuery(query, MakeAnyLenString(&query,
		"DELETE FROM faction_values WHERE char_id=%i AND faction_id = %i",
		char_id, faction_id), errbuf)) {
		std::cerr << "Error in SetCharacterFactionLevel query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	if(value == 0)
	{
		safe_delete_array(query);
		return true;
	}

	if(temp == 2)
		temp = 0;

	if(temp == 3)
		temp = 1;

	if (!RunQuery(query, MakeAnyLenString(&query,
		"INSERT INTO faction_values (char_id,faction_id,current_value,temp) VALUES (%i,%i,%i,%i)",
		char_id, faction_id,value,temp), errbuf, 0, &affected_rows)) {
		std::cerr << "Error in SetCharacterFactionLevel query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);

	if (affected_rows == 0)
	{
		return false;
	}

	val_list[faction_id] = value;
	return(true);
}

bool ZoneDatabase::LoadFactionData()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT MAX(id) FROM faction_list");


	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row && row[0])
		{
			max_faction = atoi(row[0]);
			faction_array = new Faction*[max_faction+1];
			for(unsigned int i=0; i<max_faction; i++)
			{
				faction_array[i] = nullptr;
			}
			mysql_free_result(result);

			MakeAnyLenString(&query, "SELECT id,name,base FROM faction_list");
			if (RunQuery(query, strlen(query), errbuf, &result))
			{
				safe_delete_array(query);
				while((row = mysql_fetch_row(result)))
				{
					uint32 index = atoi(row[0]);
					faction_array[index] = new Faction;
					strn0cpy(faction_array[index]->name, row[1], 50);
					faction_array[index]->base = atoi(row[2]);

					char sec_errbuf[MYSQL_ERRMSG_SIZE];
					MYSQL_RES *sec_result;
					MYSQL_ROW sec_row;
					MakeAnyLenString(&query, "SELECT `mod`, `mod_name` FROM `faction_list_mod` WHERE faction_id=%u", index);
					if (RunQuery(query, strlen(query), sec_errbuf, &sec_result)) {
						while((sec_row = mysql_fetch_row(sec_result)))
						{
							faction_array[index]->mods[sec_row[1]] = atoi(sec_row[0]);
						}
						mysql_free_result(sec_result);
					}
					safe_delete_array(query);
				}
				mysql_free_result(result);
			}
			else {
				std::cerr << "Error in LoadFactionData '" << query << "' " << errbuf << std::endl;
				safe_delete_array(query);
				return false;
			}
		}
		else {
			mysql_free_result(result);
		}
	}
	else {
		std::cerr << "Error in LoadFactionData '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
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
