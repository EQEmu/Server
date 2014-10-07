/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "worlddb.h"
//#include "../common/item.h"
#include "../common/string_util.h"
#include "../common/eq_packet_structs.h"
#include "../common/item.h"
#include "../common/rulesys.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include "sof_char_create_data.h"

WorldDatabase database;
extern std::vector<RaceClassAllocation> character_create_allocations;
extern std::vector<RaceClassCombos> character_create_race_class_combos;


// solar: the current stuff is at the bottom of this function
void WorldDatabase::GetCharSelectInfo(uint32 account_id, CharacterSelect_Struct* cs, uint32 ClientVersion) {
	Inventory *inv;
	uint8 has_home = 0; 
	uint8 has_bind = 0;

	/* Initialize Variables */
	for (int i=0; i<10; i++) {
		strcpy(cs->name[i], "<none>");
		cs->zone[i] = 0;
		cs->level[i] = 0;
		cs->tutorial[i] = 0;
		cs->gohome[i] = 0;
	}

	/* Get Character Info */
	std::string cquery = StringFormat(
		"SELECT                     "  
		"`id`,                      "  // 0
		"name,                      "  // 1
		"gender,                    "  // 2
		"race,                      "  // 3
		"class,                     "  // 4
		"`level`,                   "  // 5
		"deity,                     "  // 6
		"last_login,                "  // 7
		"time_played,               "  // 8
		"hair_color,                "  // 9
		"beard_color,               "  // 10
		"eye_color_1,               "  // 11
		"eye_color_2,               "  // 12
		"hair_style,                "  // 13
		"beard,                     "  // 14
		"face,                      "  // 15
		"drakkin_heritage,          "  // 16
		"drakkin_tattoo,            "  // 17
		"drakkin_details,           "  // 18
		"zone_id		            "  // 19
		"FROM                       "  
		"character_data             "  
		"WHERE `account_id` = %i ORDER BY `name` LIMIT 10   ", account_id);
	auto results = database.QueryDatabase(cquery); int char_num = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		PlayerProfile_Struct pp;
		memset(&pp, 0, sizeof(PlayerProfile_Struct));

		uint32 character_id = atoi(row[0]);
		strcpy(cs->name[char_num], row[1]);
		uint8 lvl = atoi(row[5]);
		cs->level[char_num] = lvl;
		cs->class_[char_num] = atoi(row[4]);
		cs->race[char_num] = atoi(row[3]);
		cs->gender[char_num] = atoi(row[2]);
		cs->deity[char_num] = atoi(row[6]);
		cs->zone[char_num] = atoi(row[19]);
		cs->face[char_num] = atoi(row[15]);
		cs->haircolor[char_num] = atoi(row[9]);
		cs->beardcolor[char_num] = atoi(row[10]);
		cs->eyecolor2[char_num] = atoi(row[12]);
		cs->eyecolor1[char_num] = atoi(row[11]);
		cs->hairstyle[char_num] = atoi(row[13]);
		cs->beard[char_num] = atoi(row[14]);
		cs->drakkin_heritage[char_num] = atoi(row[16]);
		cs->drakkin_tattoo[char_num] = atoi(row[17]);
		cs->drakkin_details[char_num] = atoi(row[18]);

		if (RuleB(World, EnableTutorialButton) && (lvl <= RuleI(World, MaxLevelForTutorial)))
			cs->tutorial[char_num] = 1;

		if (RuleB(World, EnableReturnHomeButton)) {
			int now = time(nullptr);
			if ((now - atoi(row[7])) >= RuleI(World, MinOfflineTimeToReturnHome))
				cs->gohome[char_num] = 1;
		}

		/* Set Bind Point Data for any character that may possibly be missing it for any reason */
		cquery = StringFormat("SELECT `zone_id`, `instance_id`, `x`, `y`, `z`, `heading`, `is_home` FROM `character_bind`  WHERE `id` = %i LIMIT 2", character_id); 
		auto results_bind = database.QueryDatabase(cquery); has_home = 0; has_bind = 0;
		for (auto row_b = results_bind.begin(); row_b != results_bind.end(); ++row_b) {
			if (row_b[6] && atoi(row_b[6]) == 1){ has_home = 1; }
			if (row_b[6] && atoi(row_b[6]) == 0){ has_bind = 1; } 
		}

		if (has_home == 0 || has_bind == 0){
			cquery = StringFormat("SELECT `zone_id`, `bind_id`, `x`, `y`, `z` FROM `start_zones` WHERE `player_class` = %i AND `player_deity` = %i AND `player_race` = %i",
				cs->class_[char_num], cs->deity[char_num], cs->race[char_num]);
			auto results_bind = database.QueryDatabase(cquery);
			for (auto row_d = results_bind.begin(); row_d != results_bind.end(); ++row_d) {
				/* If a bind_id is specified, make them start there */
				if (atoi(row_d[1]) != 0) { 
					pp.binds[4].zoneId = (uint32)atoi(row_d[1]);
					GetSafePoints(pp.binds[4].zoneId, 0, &pp.binds[4].x, &pp.binds[4].y, &pp.binds[4].z);
				}
				/* Otherwise, use the zone and coordinates given */
				else {	
					pp.binds[4].zoneId = (uint32)atoi(row_d[0]);
					float x = atof(row_d[2]); 
					float y = atof(row_d[3]); 
					float z = atof(row_d[4]); 
					if (x == 0 && y == 0 && z == 0){ GetSafePoints(pp.binds[4].zoneId, 0, &x, &y, &z); } 
					pp.binds[4].x = x; pp.binds[4].y = y; pp.binds[4].z = z;
					
				}
			}
			pp.binds[0] = pp.binds[4];
			/* If no home bind set, set it */
			if (has_home == 0){
				std::string query = StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, is_home)"
					" VALUES (%u, %u, %u, %f, %f, %f, %f, %i)", 
					character_id, pp.binds[4].zoneId, 0, pp.binds[4].x, pp.binds[4].y, pp.binds[4].z, pp.binds[4].heading, 1); 
				auto results_bset = QueryDatabase(query); ThrowDBError(results_bset.ErrorMessage(), "WorldDatabase::GetCharSelectInfo Set Home Point", query);
			}
			/* If no regular bind set, set it */
			if (has_bind == 0){
				std::string query = StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, is_home)"
					" VALUES (%u, %u, %u, %f, %f, %f, %f, %i)", 
					character_id, pp.binds[0].zoneId, 0, pp.binds[0].x, pp.binds[0].y, pp.binds[0].z, pp.binds[0].heading, 0); 
				auto results_bset = QueryDatabase(query); ThrowDBError(results_bset.ErrorMessage(), "WorldDatabase::GetCharSelectInfo Set Bind Point", query);
			}
		}
		/* Bind End */

		/*
			Character's equipped items
			@merth: Haven't done bracer01/bracer02 yet.
			Also: this needs a second look after items are a little more solid
			NOTE: items don't have a color, players MAY have a tint, if the
			use_tint part is set. otherwise use the regular color
		*/

		/* Load Character Material Data for Char Select */
		cquery = StringFormat("SELECT slot, red, green, blue, use_tint, color FROM `character_material` WHERE `id` = %u", character_id);
		auto results_b = database.QueryDatabase(cquery); uint8 slot = 0;
		for (auto row_b = results_b.begin(); row_b != results_b.end(); ++row_b) {
			slot = atoi(row_b[0]); 
			pp.item_tint[slot].rgb.red = atoi(row_b[1]);
			pp.item_tint[slot].rgb.green = atoi(row_b[2]);
			pp.item_tint[slot].rgb.blue = atoi(row_b[3]);
			pp.item_tint[slot].rgb.use_tint = atoi(row_b[4]);
		}

		/* Load Inventory */
		inv = new Inventory;
		if (GetInventory(account_id, cs->name[char_num], inv)) {
			for (uint8 material = 0; material <= 8; material++) {
				uint32 color = 0;
				ItemInst *item = inv->GetItem(Inventory::CalcSlotFromMaterial(material));
				if (item == 0)
					continue;

				cs->equip[char_num][material] = item->GetItem()->Material;

				if (pp.item_tint[material].rgb.use_tint){ color = pp.item_tint[material].color; }
				else{ color = item->GetItem()->Color; }

				cs->cs_colors[char_num][material].color = color; 

				/* Weapons are handled a bit differently */
				if ((material == MaterialPrimary) || (material == MaterialSecondary)) {
					if (strlen(item->GetItem()->IDFile) > 2) {
						uint32 idfile = atoi(&item->GetItem()->IDFile[2]);
						if (material == MaterialPrimary)
							cs->primary[char_num] = idfile;
						else
							cs->secondary[char_num] = idfile;
					}
				}
			}
		}
		else {
			printf("Error loading inventory for %s\n", cs->name[char_num]);
		}
		safe_delete(inv);
		if (++char_num > 10)
			break;
	}

	return;
}

int WorldDatabase::MoveCharacterToBind(int CharID, uint8 bindnum) {
	/*  if an invalid bind point is specified, use the primary bind */
	if (bindnum > 4){ bindnum = 0; }
	int is_home = 0;
	if (bindnum == 4){ is_home = 1; }

	std::string query = StringFormat("SELECT `zone_id` FROM `character_bind` WHERE `id` = %u AND `is_home` = %u LIMIT 1", CharID, is_home);
	auto results = database.QueryDatabase(query); int i = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		return atoi(row[0]);
	}
}

bool WorldDatabase::GetStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row = 0;
	int rows;

	if(!in_pp || !in_cc)
		return false;

	in_pp->x = in_pp->y = in_pp->z = in_pp->heading = in_pp->zone_id = 0;
	in_pp->binds[0].x = in_pp->binds[0].y = in_pp->binds[0].z = in_pp->binds[0].zoneId = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "SELECT x,y,z,heading,zone_id,bind_id FROM start_zones WHERE player_choice=%i AND player_class=%i "
			"AND player_deity=%i AND player_race=%i",
			in_cc->start_zone,
			in_cc->class_,
			in_cc->deity,
			in_cc->race), errbuf, &result))
	{
		LogFile->write(EQEMuLog::Error, "Start zone query failed: %s : %s\n", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	LogFile->write(EQEMuLog::Status, "Start zone query: %s\n", query);
	safe_delete_array(query);

	if((rows = mysql_num_rows(result)) > 0)
		row = mysql_fetch_row(result);

	if(row)
	{
		LogFile->write(EQEMuLog::Status, "Found starting location in start_zones");
		in_pp->x = atof(row[0]);
		in_pp->y = atof(row[1]);
		in_pp->z = atof(row[2]);
		in_pp->heading = atof(row[3]);
		in_pp->zone_id = atoi(row[4]);
		in_pp->binds[0].zoneId = atoi(row[5]);
	}
	else
	{
		printf("No start_zones entry in database, using defaults\n");
		switch(in_cc->start_zone)
		{
			case 0:
			{
				in_pp->zone_id = 24;	// erudnext
				in_pp->binds[0].zoneId = 38;	// tox
				break;
			}
			case 1:
			{
				in_pp->zone_id =2;	// qeynos2
				in_pp->binds[0].zoneId = 2;	// qeynos2
				break;
			}
			case 2:
			{
				in_pp->zone_id =29;	// halas
				in_pp->binds[0].zoneId = 30;	// everfrost
				break;
			}
			case 3:
			{
				in_pp->zone_id =19;	// rivervale
				in_pp->binds[0].zoneId = 20;	// kithicor
				break;
			}
			case 4:
			{
				in_pp->zone_id =9;	// freportw
				in_pp->binds[0].zoneId = 9;	// freportw
				break;
			}
			case 5:
			{
				in_pp->zone_id =40;	// neriaka
				in_pp->binds[0].zoneId = 25;	// nektulos
				break;
			}
			case 6:
			{
				in_pp->zone_id =52;	// gukta
				in_pp->binds[0].zoneId = 46;	// innothule
				break;
			}
			case 7:
			{
				in_pp->zone_id =49;	// oggok
				in_pp->binds[0].zoneId = 47;	// feerrott
				break;
			}
			case 8:
			{
				in_pp->zone_id =60;	// kaladima
				in_pp->binds[0].zoneId = 68;	// butcher
				break;
			}
			case 9:
			{
				in_pp->zone_id =54;	// gfaydark
				in_pp->binds[0].zoneId = 54;	// gfaydark
				break;
			}
			case 10:
			{
				in_pp->zone_id =61;	// felwithea
				in_pp->binds[0].zoneId = 54;	// gfaydark
				break;
			}
			case 11:
			{
				in_pp->zone_id =55;	// akanon
				in_pp->binds[0].zoneId = 56;	// steamfont
				break;
			}
			case 12:
			{
				in_pp->zone_id =82;	// cabwest
				in_pp->binds[0].zoneId = 78;	// fieldofbone
				break;
			}
			case 13:
			{
				in_pp->zone_id =155;	// sharvahl
				in_pp->binds[0].zoneId = 155;	// sharvahl
				break;
			}
		}
	}

	if(in_pp->x == 0 && in_pp->y == 0 && in_pp->z == 0)
		database.GetSafePoints(in_pp->zone_id, 0, &in_pp->x, &in_pp->y, &in_pp->z);

	if(in_pp->binds[0].x == 0 && in_pp->binds[0].y == 0 && in_pp->binds[0].z == 0)
		database.GetSafePoints(in_pp->binds[0].zoneId, 0, &in_pp->binds[0].x, &in_pp->binds[0].y, &in_pp->binds[0].z);
	if(result)
		mysql_free_result(result);
	return true;
}

bool WorldDatabase::GetStartZoneSoF(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc)
{

	// SoF doesn't send the player_choice field in character creation, it now sends the real zoneID instead.
	//
	// For SoF, search for an entry in start_zones with a matching zone_id, class, race and deity.
	//
	// For now, if no row matching row is found, send them to Crescent Reach, as that is probably the most likely
	// reason for no match being found.
	//
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row = 0;
	int rows;

	if(!in_pp || !in_cc)
		return false;

	in_pp->x = in_pp->y = in_pp->z = in_pp->heading = in_pp->zone_id = 0;
	in_pp->binds[0].x = in_pp->binds[0].y = in_pp->binds[0].z = in_pp->binds[0].zoneId = 0;

	if(!RunQuery(query, MakeAnyLenString(&query, "SELECT x, y, z, heading, bind_id "
		" FROM start_zones "
		" WHERE zone_id = %i "
		" AND player_class = %i "
		" AND player_deity = %i" 
		" AND player_race = %i",
		in_cc->start_zone,
		in_cc->class_,
		in_cc->deity,
		in_cc->race), errbuf, &result))
	{
		LogFile->write(EQEMuLog::Status, "SoF Start zone query failed: %s : %s\n", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	LogFile->write(EQEMuLog::Status, "SoF Start zone query: %s\n", query);
	safe_delete_array(query);

	if((rows = mysql_num_rows(result)) > 0)
		row = mysql_fetch_row(result);

	if(row)
	{
		LogFile->write(EQEMuLog::Status, "Found starting location in start_zones");
		in_pp->x = atof(row[0]);
		in_pp->y = atof(row[1]);
		in_pp->z = atof(row[2]);
		in_pp->heading = atof(row[3]);
		in_pp->zone_id = in_cc->start_zone;
		in_pp->binds[0].zoneId = atoi(row[4]);
	}
	else
	{
		printf("No start_zones entry in database, using defaults\n");

		if(in_cc->start_zone == RuleI(World, TutorialZoneID))
			in_pp->zone_id = in_cc->start_zone;
		else {
			in_pp->x = in_pp->binds[0].x = -51;
			in_pp->y = in_pp->binds[0].y = -20;
			in_pp->z = in_pp->binds[0].z = 0.79;
			in_pp->zone_id = in_pp->binds[0].zoneId = 394; // Crescent Reach.
		}

	}

	if(in_pp->x == 0 && in_pp->y == 0 && in_pp->z == 0)
		database.GetSafePoints(in_pp->zone_id, 0, &in_pp->x, &in_pp->y, &in_pp->z);

	if(in_pp->binds[0].x == 0 && in_pp->binds[0].y == 0 && in_pp->binds[0].z == 0)
		database.GetSafePoints(in_pp->binds[0].zoneId, 0, &in_pp->binds[0].x, &in_pp->binds[0].y, &in_pp->binds[0].z);
	if(result)
		mysql_free_result(result);
	return true;
}

void WorldDatabase::GetLauncherList(std::vector<std::string> &rl) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	rl.clear();

	if (RunQuery(query, MakeAnyLenString(&query,
			"SELECT name FROM launcher" )
		, errbuf, &result))
	{
		while ((row = mysql_fetch_row(result))) {
			rl.push_back(row[0]);
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "WorldDatabase::GetLauncherList: %s", errbuf);
	}
	safe_delete_array(query);
}

void WorldDatabase::SetMailKey(int CharID, int IPAddress, int MailKey) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	char MailKeyString[17];

	if(RuleB(Chat, EnableMailKeyIPVerification) == true)
		sprintf(MailKeyString, "%08X%08X", IPAddress, MailKey);
	else
		sprintf(MailKeyString, "%08X", MailKey);

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE `character_data` SET mailkey = '%s' WHERE id='%i'",
							MailKeyString, CharID), errbuf))

		LogFile->write(EQEMuLog::Error, "WorldDatabase::SetMailKey(%i, %s) : %s", CharID, MailKeyString, errbuf);

	safe_delete_array(query);

}

bool WorldDatabase::GetCharacterLevel(const char *name, int &level)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(RunQuery(query, MakeAnyLenString(&query, "SELECT `level` FROM `character_data` WHERE `name` = '%s'", name), errbuf, &result))
	{
		if(row = mysql_fetch_row(result))
		{
			level = atoi(row[0]);
			mysql_free_result(result);
			safe_delete_array(query);
			return true;
		}
		mysql_free_result(result);
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "WorldDatabase::GetCharacterLevel: %s", errbuf);
	}
	safe_delete_array(query);
	return false;
}

bool WorldDatabase::LoadCharacterCreateAllocations() {
	character_create_allocations.clear();

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(RunQuery(query, MakeAnyLenString(&query, "SELECT * FROM char_create_point_allocations order by id"), errbuf, &result)) {
		safe_delete_array(query);
		while(row = mysql_fetch_row(result)) {
			RaceClassAllocation allocate;
			int r = 0;
			allocate.Index = atoi(row[r++]);
			allocate.BaseStats[0] = atoi(row[r++]);
			allocate.BaseStats[3] = atoi(row[r++]);
			allocate.BaseStats[1] = atoi(row[r++]);
			allocate.BaseStats[2] = atoi(row[r++]);
			allocate.BaseStats[4] = atoi(row[r++]);
			allocate.BaseStats[5] = atoi(row[r++]);
			allocate.BaseStats[6] = atoi(row[r++]);
			allocate.DefaultPointAllocation[0] = atoi(row[r++]);
			allocate.DefaultPointAllocation[3] = atoi(row[r++]);
			allocate.DefaultPointAllocation[1] = atoi(row[r++]);
			allocate.DefaultPointAllocation[2] = atoi(row[r++]);
			allocate.DefaultPointAllocation[4] = atoi(row[r++]);
			allocate.DefaultPointAllocation[5] = atoi(row[r++]);
			allocate.DefaultPointAllocation[6] = atoi(row[r++]);
			character_create_allocations.push_back(allocate);
		}
		mysql_free_result(result);
	} else {
		safe_delete_array(query);
		return false;
	}

	return true;
}

bool WorldDatabase::LoadCharacterCreateCombos() {
	character_create_race_class_combos.clear();

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(RunQuery(query, MakeAnyLenString(&query, "select * from char_create_combinations order by race, class, deity, start_zone"), errbuf, &result)) {
		safe_delete_array(query);
		while(row = mysql_fetch_row(result)) {
			RaceClassCombos combo;
			int r = 0;
			combo.AllocationIndex = atoi(row[r++]);
			combo.Race = atoi(row[r++]);
			combo.Class = atoi(row[r++]);
			combo.Deity = atoi(row[r++]);
			combo.Zone = atoi(row[r++]);
			combo.ExpansionRequired = atoi(row[r++]);
			character_create_race_class_combos.push_back(combo);
		}
		mysql_free_result(result);
	} else {
		safe_delete_array(query);
		return false;
	}

	return true;
}

