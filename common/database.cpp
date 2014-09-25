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
#include "../common/rulesys.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errmsg.h>
#include <mysqld_error.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <cmath>
#include <map>

// Disgrace: for windows compile
#ifdef _WINDOWS
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "unix.h"
#include <netinet/in.h>
#include <sys/time.h>
#endif

#include "database.h"
#include "eq_packet_structs.h"
#include "guilds.h"
#include "string_util.h"
#include "extprofile.h"
extern Client client;

#ifdef _WINDOWS
#if _MSC_VER > 1700 // greater than 2012 (2013+)
#	define _ISNAN_(a) std::isnan(a)
#else
#	include <float.h>
#	define _ISNAN_(a) _isnan(a)
#endif
#else
#	define _ISNAN_(a) std::isnan(a)
#endif

/*
Establish a connection to a mysql database with the supplied parameters

	Added a very simple .ini file parser - Bounce

	Modify to use for win32 & linux - misanthropicfiend
*/
Database::Database ()
{
	DBInitVars();
}

/*
Establish a connection to a mysql database with the supplied parameters
*/

Database::Database(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
{
	DBInitVars();
	Connect(host, user, passwd, database, port);
}

bool Database::Connect(const char* host, const char* user, const char* passwd, const char* database, uint32 port) {
	uint32 errnum= 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(host, user, passwd, database, port, &errnum, errbuf)) {
		LogFile->write(EQEMuLog::Error, "Failed to connect to database: Error: %s", errbuf);

		return false; 
	}
	else {
		LogFile->write(EQEMuLog::Status, "Using database '%s' at %s:%d",database,host,port);
		return true;
	}
}

void Database::DBInitVars() { 
	varcache_array = 0;
	varcache_max = 0;
	varcache_lastupdate = 0;
}
/*

Close the connection to the database
*/
Database::~Database()
{
	unsigned int x;
	if (varcache_array) {
		for (x=0; x<varcache_max; x++) {
			safe_delete_array(varcache_array[x]);
		}
		safe_delete_array(varcache_array);
	}
}

/*
Check if there is an account with name "name" and password "password"
Return the account id or zero if no account matches.
Zero will also be returned if there is a database error.
*/
uint32 Database::CheckLogin(const char* name, const char* password, int16* oStatus) {

	if(strlen(name) >= 50 || strlen(password) >= 50)
		return(0);

	char tmpUN[100];
	char tmpPW[100];

	DoEscapeString(tmpUN, name, strlen(name));
	DoEscapeString(tmpPW, password, strlen(password));

	std::string query = StringFormat("SELECT id, status FROM account WHERE name='%s' AND password is not null "
		"and length(password) > 0 and (password='%s' or password=MD5('%s'))",
		tmpUN, tmpPW, tmpPW);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in CheckLogin query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if(results.RowCount() == 0)
		return 0;

	auto row = results.begin();

	uint32 id = atoi(row[0]);

	if (oStatus)
		*oStatus = atoi(row[1]);

	return id;
}

//Get Banned IP Address List - Only return false if the incoming connection's IP address is not present in the banned_ips table.
bool Database::CheckBannedIPs(const char* loginIP)
{
	std::string query = StringFormat("SELECT ip_address FROM Banned_IPs WHERE ip_address='%s'", loginIP);

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in CheckBannedIPs query '" << query << "' " << results.ErrorMessage() << std::endl;
		return true;
	}

	if (results.RowCount() != 0)
		return true;

	return false;
}

bool Database::AddBannedIP(char* bannedIP, const char* notes) {
	std::string query = StringFormat("INSERT into Banned_IPs SET ip_address='%s', notes='%s'", bannedIP, notes); 
	auto results = QueryDatabase(query); 
	if (!results.Success()) {
		std::cerr << "Error in Database::AddBannedIP query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	} 
	return true;
}

 bool Database::CheckGMIPs(const char* ip_address, uint32 account_id) {
	std::string query = StringFormat("SELECT * FROM `gm_ips` WHERE `ip_address` = '%s' AND `account_id` = %i", ip_address, account_id); 
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 1)
		return true;

	return false;
}

bool Database::AddGMIP(char* ip_address, char* name) {
	std::string query = StringFormat("INSERT into `gm_ips` SET `ip_address` = '%s', `name` = '%s'", ip_address, name); 
	auto results = QueryDatabase(query); 
	return results.Success();
}

void Database::LoginIP(uint32 AccountID, const char* LoginIP) {
	std::string query = StringFormat("INSERT INTO account_ip SET accid=%i, ip='%s' ON DUPLICATE KEY UPDATE count=count+1, lastused=now()", AccountID, LoginIP); 
	auto results = QueryDatabase(query); 
	if (!results.Success())
		std::cerr << "Error in Log IP query '" << query << "' " << results.ErrorMessage() << std::endl;
}

int16 Database::CheckStatus(uint32 account_id) {
	std::string query = StringFormat("SELECT `status`, UNIX_TIMESTAMP(`suspendeduntil`) as `suspendeduntil`, UNIX_TIMESTAMP() as `current`"
							" FROM `account` WHERE `id` = %i", account_id);

	auto results = QueryDatabase(query); 
	if (!results.Success()) {
		std::cerr << "Error in CheckStatus query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;
	
	auto row = results.begin(); 
	int16 status = atoi(row[0]); 
	int32 suspendeduntil = 0;

	// MariaDB initalizes with NULL if unix_timestamp() is out of range
	if (row[1] != nullptr) {
		suspendeduntil = atoi(row[1]);
	}

	int32 current = atoi(row[2]);

	if(suspendeduntil > current)
		return -1;

	return status;
}

uint32 Database::CreateAccount(const char* name, const char* password, int16 status, uint32 lsaccount_id) {
	std::string query;

	if (password)
		query = StringFormat("INSERT INTO account SET name='%s', password='%s', status=%i, lsaccount_id=%i, time_creation=UNIX_TIMESTAMP();",name,password,status, lsaccount_id);
	else
		query = StringFormat("INSERT INTO account SET name='%s', status=%i, lsaccount_id=%i, time_creation=UNIX_TIMESTAMP();",name, status, lsaccount_id);

	std::cerr << "Account Attempting to be created:" << name << " " << (int16) status << std::endl; 
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in CreateAccount query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.LastInsertedID() == 0)
	{
		std::cerr << "Error in CreateAccount query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	return results.LastInsertedID();
}

bool Database::DeleteAccount(const char* name) {
	std::string query = StringFormat("DELETE FROM account WHERE name='%s';",name); 
	std::cout << "Account Attempting to be deleted:" << name << std::endl;

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in DeleteAccount query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	return results.RowsAffected() == 1;
}

bool Database::SetLocalPassword(uint32 accid, const char* password) {
	std::string query = StringFormat("UPDATE account SET password=MD5('%s') where id=%i;", EscapeString(password).c_str(), accid);

	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in SetLocalPassword query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	return true;
}

bool Database::SetAccountStatus(const char* name, int16 status) {
	std::string query = StringFormat("UPDATE account SET status=%i WHERE name='%s';", status, name);

	std::cout << "Account being GM Flagged:" << name << ", Level: " << (int16) status << std::endl;

	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowsAffected() == 0)
	{
		std::cout << "Account: " << name << " does not exist, therefore it cannot be flagged\n";
		return false; 
	}

	return true;
}

/* This initially creates the character during character create */
bool Database::ReserveName(uint32 account_id, char* name) {
	std::string query = StringFormat("INSERT INTO `character_data` SET `account_id` = %i, `name` = '%s'", account_id, name); 
	auto results = QueryDatabase(query);
	if (!results.Success() || results.ErrorMessage() != ""){ return false; } 
	return true;
}

bool Database::ThrowDBError(std::string ErrorMessage, std::string query_title, std::string query){  
	if (ErrorMessage != ""){ 
		std::cout << "\nERROR " << query_title << ": " << ErrorMessage << "\n\n" << query << "\n" << std::endl; 

		/* Write to log file */
		std::ofstream log("eqemu_query_error_log.txt", std::ios_base::app | std::ios_base::out);
		log << "ERROR " << query_title << ": " << ErrorMessage << "\n" << query << "\n";
		log.close();

		return true; 
	}
	return false;
}

/*
	Delete the character with the name "name"
	returns false on failure, true otherwise
*/
bool Database::DeleteCharacter(char *name) {
	uint32 charid = 0;
	printf("Database::DeleteCharacter name : %s \n", name);
	if(!name ||	!strlen(name)) {
		std::cerr << "DeleteCharacter: request to delete without a name (empty char slot)" << std::endl;
		return false;
	}

	/* Get id from character_data before deleting record so we can clean up the rest of the tables */
	std::string query = StringFormat("SELECT `id` from `character_data` WHERE `name` = '%s'", name);
	auto results = QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) { charid = atoi(row[0]); }
	if (charid <= 0){ std::cerr << "Database::DeleteCharacter :: Character not found, stopping delete...\n"; return false; }

	query = StringFormat("DELETE FROM `quest_globals` WHERE `charid` = '%d'", charid); results = QueryDatabase(query);			  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_activities` WHERE `charid` = '%d'", charid); results = QueryDatabase(query);	  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_enabledtasks` WHERE `charid` = '%d'", charid); results = QueryDatabase(query);	  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `completed_tasks` WHERE `charid` = '%d'", charid); results = QueryDatabase(query);		  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `friends` WHERE `charid` = '%d'", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `mail` WHERE `charid` = '%d'", charid); results = QueryDatabase(query);					  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `timers` WHERE `char_id` = '%d'", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `inventory` WHERE `charid` = '%d'", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `char_recipe_list` WHERE `char_id` = '%d'", charid); results = QueryDatabase(query);		  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `adventure_stats` WHERE `player_id` ='%d'", charid); results = QueryDatabase(query);		  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `zone_flags` WHERE `charID` = '%d'", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `titles` WHERE `char_id` = '%d'", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `player_titlesets` WHERE `char_id` = '%d'", charid); results = QueryDatabase(query);		  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `keyring` WHERE `char_id` = '%d'", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `faction_values` WHERE `char_id` = '%d'", charid); results = QueryDatabase(query);		  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `instance_list_player` WHERE `charid` = '%d'", charid); results = QueryDatabase(query);	  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_data` WHERE `id` = '%d'", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_skills` WHERE `id` = %u", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_languages` WHERE `id` = %u", charid); results = QueryDatabase(query);			  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_bind` WHERE `id` = %u", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_alternate_abilities` WHERE `id` = %u", charid); results = QueryDatabase(query);  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_currency` WHERE `id` = %u", charid); results = QueryDatabase(query);			  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_data` WHERE `id` = %u", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_spells` WHERE `id` = %u", charid); results = QueryDatabase(query);				  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_memmed_spells` WHERE `id` = %u", charid); results = QueryDatabase(query);		  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_disciplines` WHERE `id` = %u", charid); results = QueryDatabase(query);		  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_material` WHERE `id` = %u", charid); results = QueryDatabase(query);			  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_tribute` WHERE `id` = %u", charid); results = QueryDatabase(query);			  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_bandolier` WHERE `id` = %u", charid); results = QueryDatabase(query);			  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_potionbelt` WHERE `id` = %u", charid); results = QueryDatabase(query);			  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_inspect_messages` WHERE `id` = %u", charid); results = QueryDatabase(query);	  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_leadership_abilities` WHERE `id` = %u", charid); results = QueryDatabase(query); ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
	query = StringFormat("DELETE FROM `character_alt_currency` WHERE `char_id` = '%d'", charid); results = QueryDatabase(query);  ThrowDBError(results.ErrorMessage(), "Database::DeleteCharacter", query);
#ifdef BOTS																														 
	query = StringFormat("DELETE FROM `guild_members` WHERE `char_id` = '%d' AND GetMobTypeById(%i) = 'C'", charid);
#else																															 
	query = StringFormat("DELETE FROM `guild_members` WHERE `char_id` = '%d'", charid);
#endif																															 
	QueryDatabase(query);
	
	return true;
}

bool Database::SaveCharacterCreate(uint32 character_id, uint32 account_id, PlayerProfile_Struct* pp){
	std::string query = StringFormat(
		"REPLACE INTO `character_data` ("
		"id,"
		"account_id,"
		"`name`,"
		"last_name,"
		"gender,"
		"race,"
		"class,"
		"`level`,"
		"deity,"
		"birthday,"
		"last_login,"
		"time_played,"
		"pvp_status,"
		"level2,"
		"anon,"
		"gm,"
		"intoxication,"
		"hair_color,"
		"beard_color,"
		"eye_color_1,"
		"eye_color_2,"
		"hair_style,"
		"beard,"
		"ability_time_seconds,"
		"ability_number,"
		"ability_time_minutes,"
		"ability_time_hours,"
		"title,"
		"suffix,"
		"exp,"
		"points,"
		"mana,"
		"cur_hp,"
		"str,"
		"sta,"
		"cha,"
		"dex,"
		"`int`,"
		"agi,"
		"wis,"
		"face,"
		"y,"
		"x,"
		"z,"
		"heading,"
		"pvp2,"
		"pvp_type,"
		"autosplit_enabled,"
		"zone_change_count,"
		"drakkin_heritage,"
		"drakkin_tattoo,"
		"drakkin_details,"
		"toxicity,"
		"hunger_level,"
		"thirst_level,"
		"ability_up,"
		"zone_id,"
		"zone_instance,"
		"leadership_exp_on,"
		"ldon_points_guk,"
		"ldon_points_mir,"
		"ldon_points_mmc,"
		"ldon_points_ruj,"
		"ldon_points_tak,"
		"ldon_points_available,"
		"tribute_time_remaining,"
		"show_helm,"
		"career_tribute_points,"
		"tribute_points,"
		"tribute_active,"
		"endurance,"
		"group_leadership_exp,"
		"raid_leadership_exp,"
		"group_leadership_points,"
		"raid_leadership_points,"
		"air_remaining,"
		"pvp_kills,"
		"pvp_deaths,"
		"pvp_current_points,"
		"pvp_career_points,"
		"pvp_best_kill_streak,"
		"pvp_worst_death_streak,"
		"pvp_current_kill_streak,"
		"aa_points_spent,"
		"aa_exp,"
		"aa_points,"
		"group_auto_consent,"
		"raid_auto_consent,"
		"guild_auto_consent,"
		"RestTimer) "
		"VALUES ("
		"%u,"  // id					
		"%u,"  // account_id			
		"'%s',"  // `name`				
		"'%s',"  // last_name			
		"%u,"  // gender				
		"%u,"  // race					
		"%u,"  // class					
		"%u,"  // `level`				
		"%u,"  // deity					
		"%u,"  // birthday				
		"%u,"  // last_login			
		"%u,"  // time_played			
		"%u,"  // pvp_status			
		"%u,"  // level2				
		"%u,"  // anon					
		"%u,"  // gm					
		"%u,"  // intoxication			
		"%u,"  // hair_color			
		"%u,"  // beard_color			
		"%u,"  // eye_color_1			
		"%u,"  // eye_color_2			
		"%u,"  // hair_style			
		"%u,"  // beard					
		"%u,"  // ability_time_seconds	
		"%u,"  // ability_number		
		"%u,"  // ability_time_minutes	
		"%u,"  // ability_time_hours	
		"'%s',"  // title				
		"'%s',"  // suffix				
		"%u,"  // exp					
		"%u,"  // points				
		"%u,"  // mana					
		"%u,"  // cur_hp				
		"%u,"  // str					
		"%u,"  // sta					
		"%u,"  // cha					
		"%u,"  // dex					
		"%u,"  // `int`					
		"%u,"  // agi					
		"%u,"  // wis					
		"%u,"  // face					
		"%f,"  // y						
		"%f,"  // x						
		"%f,"  // z						
		"%f,"  // heading				
		"%u,"  // pvp2					
		"%u,"  // pvp_type				
		"%u,"  // autosplit_enabled		
		"%u,"  // zone_change_count		
		"%u,"  // drakkin_heritage		
		"%u,"  // drakkin_tattoo		
		"%u,"  // drakkin_details		
		"%i,"  // toxicity				
		"%i,"  // hunger_level			
		"%i,"  // thirst_level			
		"%u,"  // ability_up			
		"%u,"  // zone_id				
		"%u,"  // zone_instance			
		"%u,"  // leadership_exp_on		
		"%u,"  // ldon_points_guk		
		"%u,"  // ldon_points_mir		
		"%u,"  // ldon_points_mmc		
		"%u,"  // ldon_points_ruj		
		"%u,"  // ldon_points_tak		
		"%u,"  // ldon_points_available	
		"%u,"  // tribute_time_remaining
		"%u,"  // show_helm				
		"%u,"  // career_tribute_points	
		"%u,"  // tribute_points		
		"%u,"  // tribute_active		
		"%u,"  // endurance				
		"%u,"  // group_leadership_exp	
		"%u,"  // raid_leadership_exp	
		"%u,"  // group_leadership_point
		"%u,"  // raid_leadership_points
		"%u,"  // air_remaining			
		"%u,"  // pvp_kills				
		"%u,"  // pvp_deaths			
		"%u,"  // pvp_current_points	
		"%u,"  // pvp_career_points		
		"%u,"  // pvp_best_kill_streak	
		"%u,"  // pvp_worst_death_streak
		"%u,"  // pvp_current_kill_strea
		"%u,"  // aa_points_spent		
		"%u,"  // aa_exp				
		"%u,"  // aa_points				
		"%u,"  // group_auto_consent	
		"%u,"  // raid_auto_consent		
		"%u,"  // guild_auto_consent	
		"%u"  // RestTimer				
		")",
		character_id,					  // " id,                        "
		account_id,						  // " account_id,                "
		EscapeString(pp->name).c_str(),	  // " `name`,                    "
		EscapeString(pp->last_name).c_str(), // " last_name,              "
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
		EscapeString(pp->title).c_str(),  // " title,                     "
		EscapeString(pp->suffix).c_str(), // " suffix,                    "
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
		pp->RestTimer					  // " RestTimer)                 "
	);
	auto results = QueryDatabase(query);
	ThrowDBError(results.ErrorMessage(), "Database::SaveCharacterCreate Character Data", query); 
	/* Save Bind Points */
	query = StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, is_home)"
		" VALUES (%u, %u, %u, %f, %f, %f, %f, %i), "
		"(%u, %u, %u, %f, %f, %f, %f, %i)",
		character_id, pp->binds[0].zoneId, 0, pp->binds[0].x, pp->binds[0].y, pp->binds[0].z, pp->binds[0].heading, 0,
		character_id, pp->binds[4].zoneId, 0, pp->binds[4].x, pp->binds[4].y, pp->binds[4].z, pp->binds[4].heading, 1
	); results = QueryDatabase(query); ThrowDBError(results.ErrorMessage(), "Database::SaveCharacterCreate Bind Point", query);

	/* Save Skills */
	int firstquery = 0;
	for (int i = 0; i < MAX_PP_SKILL; i++){
		if (pp->skills[i] > 0){
			if (firstquery != 1){
				firstquery = 1;
				query = StringFormat("REPLACE INTO `character_skills` (id, skill_id, value) VALUES (%u, %u, %u)", character_id, i, pp->skills[i]);
			}
			else{
				query = query + StringFormat(", (%u, %u, %u)", character_id, i, pp->skills[i]);
			}
		}
	}
	results = QueryDatabase(query); ThrowDBError(results.ErrorMessage(), "Database::SaveCharacterCreate Starting Skills", query);

	/* Save Language */
	firstquery = 0;
	for (int i = 0; i < MAX_PP_LANGUAGE; i++){
		if (pp->languages[i] > 0){
			if (firstquery != 1){
				firstquery = 1;
				query = StringFormat("REPLACE INTO `character_languages` (id, lang_id, value) VALUES (%u, %u, %u)", character_id, i, pp->languages[i]);
			}
			else{
				query = query + StringFormat(", (%u, %u, %u)", character_id, i, pp->languages[i]);
			}
		}
	}
	results = QueryDatabase(query); ThrowDBError(results.ErrorMessage(), "Database::SaveCharacterCreate Starting Languages", query);

	return true;
}

/* This only for new Character creation storing */
bool Database::StoreCharacter(uint32 account_id, PlayerProfile_Struct* pp, Inventory* inv) {
	uint32 charid = 0; 
	char zone[50]; 
	float x, y, z; 
	charid = GetCharacterID(pp->name);

	if(!charid) {
		LogFile->write(EQEMuLog::Error, "StoreCharacter: no character id");
		return false;
	}

	const char *zname = GetZoneName(pp->zone_id);
	if(zname == nullptr) {
		/* Zone not in the DB, something to prevent crash... */
		strn0cpy(zone, "qeynos", 49);
		pp->zone_id = 1;
	}
	else{ strn0cpy(zone, zname, 49); }

	x = pp->x;
	y = pp->y;
	z = pp->z;

	/* Saves Player Profile Data */
	SaveCharacterCreate(charid, account_id, pp); 

	/* Insert starting inventory... */
	std::string invquery;
	for (int16 i=EmuConstants::EQUIPMENT_BEGIN; i<=EmuConstants::BANK_BAGS_END;) {
		const ItemInst* newinv = inv->GetItem(i);
		if (newinv) {
			invquery = StringFormat("INSERT INTO `inventory` (charid, slotid, itemid, charges, color) VALUES (%u, %i, %u, %i, %u)",
				charid, i, newinv->GetItem()->ID, newinv->GetCharges(), newinv->GetColor()); 
			
			auto results = QueryDatabase(invquery); 

			if (!results.RowsAffected())
				LogFile->write(EQEMuLog::Error, "StoreCharacter inventory failed. Query '%s' %s", invquery.c_str(), results.ErrorMessage().c_str());
#if EQDEBUG >= 9
			else
				LogFile->write(EQEMuLog::Debug, "StoreCharacter inventory succeeded. Query '%s'", invquery.c_str());
#endif
		}

		if (i == MainCursor) { 
			i = EmuConstants::GENERAL_BAGS_BEGIN; 
			continue;
		}
		else if (i == EmuConstants::CURSOR_BAG_END) { 
			i = EmuConstants::BANK_BEGIN; 
			continue; 
		}
		else if (i == EmuConstants::BANK_END) { 
			i = EmuConstants::BANK_BAGS_BEGIN; 
			continue; 
		} 
		i++;
	}
	return true;
}

uint32 Database::GetCharacterID(const char *name) {
	std::string query = StringFormat("SELECT `id` FROM `character_data` WHERE `name` = '%s'", name);
	auto results = QueryDatabase(query);
	auto row = results.begin();
	if (row[0]){ return atoi(row[0]); }
	return 0; 
}

/*
	This function returns the account_id that owns the character with
	the name "name" or zero if no character with that name was found
	Zero will also be returned if there is a database error.
*/
uint32 Database::GetAccountIDByChar(const char* charname, uint32* oCharID) {
	std::string query = StringFormat("SELECT `account_id`, `id` FROM `character_data` WHERE name='%s'", EscapeString(charname).c_str());

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetAccountIDByChar query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0; 

	auto row = results.begin();

	uint32 accountId = atoi(row[0]);

	if (oCharID)
		*oCharID = atoi(row[1]);

	return accountId;
}

// Retrieve account_id for a given char_id
uint32 Database::GetAccountIDByChar(uint32 char_id) {
	std::string query = StringFormat("SELECT `account_id` FROM `character_data` WHERE `id` = %i LIMIT 1", char_id); 
	auto results = QueryDatabase(query); 
	if (!results.Success()) {
		LogFile->write(EQEMuLog::Error, "Error in GetAccountIDByChar query '%s': %s", query.c_str(), results.ErrorMessage().c_str());
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin(); 
	return atoi(row[0]);
}

uint32 Database::GetAccountIDByName(const char* accname, int16* status, uint32* lsid) {
	if (!isAlphaNumeric(accname))
		return 0;

	std::string query = StringFormat("SELECT `id`, `status`, `lsaccount_id` FROM `account` WHERE `name` = '%s' LIMIT 1", accname);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in GetAccountIDByAcc query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();

	uint32 id = atoi(row[0]);

	if (status)
		*status = atoi(row[1]);

	if (lsid) {
		if (row[2])
			*lsid = atoi(row[2]);
		else
			*lsid = 0;
	}

	return id;
}

void Database::GetAccountName(uint32 accountid, char* name, uint32* oLSAccountID) {
	std::string query = StringFormat("SELECT `name`, `lsaccount_id` FROM `account` WHERE `id` = '%i'", accountid); 
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in GetAccountName query '" << query << "' " << results.ErrorMessage() << std::endl;
		return;
	}

	if (results.RowCount() != 1)
		return;

	auto row = results.begin();

	strcpy(name, row[0]);
	if (row[1] && oLSAccountID) {
		*oLSAccountID = atoi(row[1]);
	}

}

void Database::GetCharName(uint32 char_id, char* name) { 
	std::string query = StringFormat("SELECT `name` FROM `character_data` WHERE id='%i'", char_id);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in GetCharName query '" << query << "' " << results.ErrorMessage() << std::endl;
		return; 
	}

	auto row = results.begin();
	for (auto row = results.begin(); row != results.end(); ++row) {
		strcpy(name, row[0]);
	}
}

static inline void loadbar(unsigned int x, unsigned int n, unsigned int w = 50) {
	if ((x != n) && (x % (n / 100 + 1) != 0)) return; 
	float ratio = x / (float)n;
	int   c = ratio * w; 
	std::cout << std::setw(3) << (int)(ratio * 100) << "% [";
	for (int x = 0; x<c; x++) std::cout << "=";
	for (int x = c; x<w; x++) std::cout << " ";
	std::cout << "]\r" << std::flush;
}

bool Database::CheckDatabaseConversions() {
	unsigned int lengths;
	unsigned int lengths_e;
	std::string squery;
	PlayerProfile_Struct* pp;
	ExtendedProfile_Struct* e_pp;
	uint32 pplen = 0;
	uint32 i;
	int character_id = 0;
	int account_id = 0;
	int number_of_characters = 0;
	int printppdebug = 0; /* Prints Player Profile */
	int runconvert = 0;

	/* Check For Legacy Storage Method */
	std::string rquery = StringFormat("SHOW TABLES LIKE 'character_'");
	auto results = QueryDatabase(rquery);
	if (results.RowCount() == 1){
		runconvert = 1; 
		printf("\n\n::: Legacy Character Data Binary Blob Storage Detected... \n");
		printf("----------------------------------------------------------\n\n");
		printf(" Database currently has character data being stored via \n"); 
		printf("  the legacy character storage method and will proceed with converting...\n\n");
		printf(" It is recommended that you backup your database \n");
		printf("  before continuing the automatic conversion process...\n\n");
		printf("----------------------------------------------------------\n\n");
		std::cout << "Press ENTER to continue....." << std::endl << std::endl;
		std::cin.ignore(1);  
	}

	// runconvert = 0;
	// printppdebug = 1;

	if (runconvert == 1){
		printf("Running character binary blob to database conversion... \n", number_of_characters); 
		/* Get the number of characters */
		rquery = StringFormat("SELECT COUNT(`id`) FROM `character_`");
		results = QueryDatabase(rquery);
		for (auto row = results.begin(); row != results.end(); ++row) {
			number_of_characters = atoi(row[0]);
			printf("Number of Characters in Database: %i \n", number_of_characters);
		}

		/* Check for table `character_data` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_data'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_data` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_data` (									"
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,						"
				"`account_id` int(11) NOT NULL DEFAULT '0',							"
				"`name` varchar(64) NOT NULL DEFAULT '',							"
				"`last_name` varchar(64) NOT NULL DEFAULT '',						"
				"`title` varchar(32) NOT NULL DEFAULT '',							"
				"`suffix` varchar(32) NOT NULL DEFAULT '',							"
				"`zone_id` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`zone_instance` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`y` float NOT NULL DEFAULT '0',									"
				"`x` float NOT NULL DEFAULT '0',									"
				"`z` float NOT NULL DEFAULT '0',									"
				"`heading` float NOT NULL DEFAULT '0',								"
				"`gender` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`race` smallint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`class` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`level` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`deity` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`birthday` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`last_login` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`time_played` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`level2` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`anon` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`gm` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`face` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`hair_color` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`hair_style` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`beard` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`beard_color` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`eye_color_1` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`eye_color_2` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`drakkin_heritage` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`drakkin_tattoo` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`drakkin_details` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ability_time_seconds` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,	"
				"`ability_number` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`ability_time_minutes` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,	"
				"`ability_time_hours` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`exp` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`aa_points_spent` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`aa_exp` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`aa_points` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`group_leadership_exp` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`raid_leadership_exp` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`group_leadership_points` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`raid_leadership_points` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`points` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`cur_hp` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`mana` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`endurance` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`intoxication` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`str` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`sta` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`cha` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`dex` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`int` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`agi` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`wis` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`zone_change_count` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`toxicity` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`hunger_level` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`thirst_level` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ability_up` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`ldon_points_guk` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_mir` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_mmc` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_ruj` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_tak` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_available` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`tribute_time_remaining` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`career_tribute_points` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`tribute_points` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`tribute_active` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`pvp_status` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`pvp_kills` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`pvp_deaths` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`pvp_current_points` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`pvp_career_points` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`pvp_best_kill_streak` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`pvp_worst_death_streak` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`pvp_current_kill_streak` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`pvp2` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`pvp_type` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`show_helm` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`group_auto_consent` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`raid_auto_consent` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`guild_auto_consent` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`leadership_exp_on` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`RestTimer` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`air_remaining` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`autosplit_enabled` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`lfp` tinyint(1) unsigned NOT NULL DEFAULT '0',					"
				"`lfg` tinyint(1) unsigned NOT NULL DEFAULT '0',					"
				"`mailkey` char(16) NOT NULL DEFAULT '',							"
				"`xtargets` tinyint(3) unsigned NOT NULL DEFAULT '5',				"
				"`firstlogon` tinyint(3) NOT NULL DEFAULT '0',						"
				"`e_aa_effects` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`e_percent_to_aa` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`e_expended_aa_spent` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"PRIMARY KEY(`id`),													"
				"UNIQUE KEY `name` (`name`),										"
				"KEY `account_id` (`account_id`)									"
				") ENGINE = InnoDB AUTO_INCREMENT = 1 DEFAULT CHARSET = latin1;		"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_currency` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_currency'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_currency` doesn't exist... creating...");
			rquery = StringFormat(
				" CREATE TABLE `character_currency` (                                  "
				" 	`id` int(11) UNSIGNED NOT NULL DEFAULT 0,                              "
				" 	`platinum` int(11) UNSIGNED NOT NULL DEFAULT 0,                    "
				" 	`gold` int(11) UNSIGNED NOT NULL DEFAULT 0,                        "
				" 	`silver` int(11) UNSIGNED NOT NULL DEFAULT 0,                      "
				" 	`copper` int(11) UNSIGNED NOT NULL DEFAULT 0,                      "
				" 	`platinum_bank` int(11) UNSIGNED NOT NULL DEFAULT 0,               "
				" 	`gold_bank` int(11) UNSIGNED NOT NULL DEFAULT 0,                   "
				" 	`silver_bank` int(11) UNSIGNED NOT NULL DEFAULT 0,                 "
				" 	`copper_bank` int(11) UNSIGNED NOT NULL DEFAULT 0,                 "
				" 	`platinum_cursor` int(11) UNSIGNED NOT NULL DEFAULT 0,             "
				" 	`gold_cursor` int(11) UNSIGNED NOT NULL DEFAULT 0,                 "
				" 	`silver_cursor` int(11) UNSIGNED NOT NULL DEFAULT 0,               "
				" 	`copper_cursor` int(11) UNSIGNED NOT NULL DEFAULT 0,               "
				" 	`radiant_crystals` int(11) UNSIGNED NOT NULL DEFAULT 0,            "
				" 	`career_radiant_crystals` int(11) UNSIGNED NOT NULL DEFAULT 0,     "
				" 	`ebon_crystals` int(11) UNSIGNED NOT NULL DEFAULT 0,               "
				" 	`career_ebon_crystals` int(11) UNSIGNED NOT NULL DEFAULT 0,        "
				" 	PRIMARY KEY (`id`),                                                "
				"   KEY `id` (`id`)                                                    "
				" ) ENGINE=InnoDB DEFAULT CHARSET=latin1;             "
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_alternate_abilities` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_alternate_abilities'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_alternate_abilities` doesn't exist... creating...");
			rquery = StringFormat(
				" CREATE TABLE `character_alternate_abilities` (						"
				" `id` int(11) UNSIGNED NOT NULL DEFAULT 0,									"
				" `slot` smallint(11) UNSIGNED NOT NULL DEFAULT 0,						"
				" `aa_id` smallint(11) UNSIGNED NOT NULL DEFAULT 0,						"
				" `aa_value` smallint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				" PRIMARY KEY(`id`,`slot`),												"
				" KEY `id` (`id`)														"
				" ) ENGINE = InnoDB DEFAULT CHARSET = latin1;		"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_bind` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_bind'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_bind` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_bind` (							   "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,				   "
				"`is_home` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`zone_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`instance_id` mediumint(11) UNSIGNED NOT NULL DEFAULT '0',	   "
				"`x` float NOT NULL DEFAULT '0',							   "
				"`y` float NOT NULL DEFAULT '0',							   "
				"`z` float NOT NULL DEFAULT '0',							   "
				"`heading` float NOT NULL DEFAULT '0',						   "
				"PRIMARY KEY(`id`, `is_home`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;" 
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_languages` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_languages'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_languages` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_languages` (						   "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,				   "
				"`lang_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`value` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"PRIMARY KEY(`id`, `lang_id`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_skills` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_skills'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_skills` doesn't exist... creating...");
			rquery = StringFormat( 
				"CREATE TABLE `character_skills` (							   "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,				   "
				"`skill_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`value` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"PRIMARY KEY(`id`, `skill_id`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
			); 
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_spells` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_spells'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_spells` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_spells` (							   "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,				   "
				"`slot_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`spell_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"PRIMARY KEY(`id`, `slot_id`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		} 
		/* Check for table `character_memmed_spells` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_memmed_spells'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_memmed_spells` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_memmed_spells` (							   "
				"`id` int(11) UNSIGNED NOT NULL DEFAULT 0,				   "
				"`slot_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`spell_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"PRIMARY KEY(`id`, `slot_id`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n"); 
		}
		/* Check for table `character_disciplines` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_disciplines'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_disciplines` doesn't exist... creating...");
			rquery = StringFormat(
				" CREATE TABLE `character_disciplines` (						  "
				" `id` int(11) UNSIGNED NOT NULL DEFAULT 0,				  "
				" `slot_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',			  "
				" `disc_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',			  "
				" PRIMARY KEY(`id`, `slot_id`),									  "
				" KEY `id` (`id`)												  "
				" ) ENGINE = InnoDB DEFAULT CHARSET = latin1;  " 
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_material` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_material'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_material` doesn't exist... creating...");
			rquery = StringFormat( 
				"CREATE TABLE `character_material` ( "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,"
				"`slot` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`blue` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`green` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`red` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`use_tint` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`color` int(11) UNSIGNED NOT NULL DEFAULT '0',"
				"PRIMARY KEY(`id`, `slot`),"
				"KEY `id` (`id`)"
				") ENGINE = InnoDB AUTO_INCREMENT = 1 DEFAULT CHARSET = latin1;"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		} 
		/* Check for table `character_tribute` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_tribute'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_tribute` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_tribute` (							   "
				"`id` int(11) unsigned NOT NULL DEFAULT 0,				   "
				"`tier` tinyint(11) unsigned NOT NULL DEFAULT '0',			   "
				"`tribute` int(11) UNSIGNED NOT NULL DEFAULT '0',			   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_bandolier` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_bandolier'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_bandolier` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_bandolier` (							"
				"`id` int(11) unsigned NOT NULL DEFAULT 0,					"
				"`bandolier_id` tinyint(11) unsigned NOT NULL DEFAULT '0',		"
				"`bandolier_slot` tinyint(11) unsigned NOT NULL DEFAULT '0',	"
				"`item_id` int(11) UNSIGNED NOT NULL DEFAULT '0',				"
				"`icon` int(11) UNSIGNED NOT NULL DEFAULT '0',				"
				"`bandolier_name` varchar(32) NOT NULL DEFAULT '0',				"
				"PRIMARY KEY(`id`,`bandolier_id`, `bandolier_slot`),			"
				"KEY `id` (`id`)												"
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;	"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_potionbelt` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_potionbelt'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_potionbelt` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_potionbelt` (						  "
				"`id` int(11) unsigned NOT NULL DEFAULT 0,				  "
				"`potion_id` tinyint(11) unsigned NOT NULL DEFAULT '0',		  "
				"`item_id` int(11) UNSIGNED NOT NULL DEFAULT '0',			  "
				"`icon` int(11) UNSIGNED NOT NULL DEFAULT '0',				  "
				"PRIMARY KEY(`id`,`potion_id`),								  "
				"KEY `id` (`id`)												  "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_potionbelt` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_inspect_messages'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_inspect_messages` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_inspect_messages` (					  "
				"`id` int(11) unsigned NOT NULL DEFAULT 0,				  "
				"`inspect_message` varchar(255) NOT NULL DEFAULT '',			  "
				"PRIMARY KEY(`id`),											  "
				"KEY `id` (`id`)												  "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}
		/* Check for table `character_leadership_abilities` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_leadership_abilities'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_leadership_abilities` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_leadership_abilities` ("
				"`id` int(11) UNSIGNED NOT NULL DEFAULT 0, "
				"`slot` smallint(11) UNSIGNED NOT NULL DEFAULT 0, "
				"`rank` smallint(11) UNSIGNED NOT NULL DEFAULT 0, "
				"PRIMARY KEY(`id`,`slot`), "
				"KEY `id` (`id`)												  "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1; "
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Table create", rquery);
			printf(" done...\n");
		}

		/* Done */
		printf("Starting conversion...\n\n");
	}

	// Testing account = 11001
	int char_iter_count = 0;
	rquery = StringFormat("SELECT `id` FROM `character_`");
	results = QueryDatabase(rquery);

	uint8 firstlogon = 0;
	uint8 lfg = 0;
	uint8 lfp = 0;
	std::string mailkey;
	uint8 xtargets = 0;
	std::string inspectmessage;

	for (auto row = results.begin(); row != results.end(); ++row) {
		char_iter_count++; 
		squery = StringFormat("SELECT `id`, `profile`, `name`, `level`, `account_id`, `firstlogon`, `lfg`, `lfp`, `mailkey`, `xtargets`, `inspectmessage`, `extprofile` FROM `character_` WHERE `id` = %i", atoi(row[0]));
		auto results2 = QueryDatabase(squery);
		auto row2 = results2.begin();
		pp = (PlayerProfile_Struct*)row2[1];
		e_pp = (ExtendedProfile_Struct*)row2[11];
		character_id = atoi(row[0]);
		account_id = atoi(row2[4]);
		/* Convert some data from the character_ table that is still relevant */
		firstlogon = atoi(row2[5]);
		lfg = atoi(row2[6]);
		lfp = atoi(row2[7]);
		mailkey = row2[8];
		xtargets = atoi(row2[9]);
		inspectmessage = row2[10];

		/* Verify PP Integrity */
		lengths = results2.LengthOfColumn(1);
		if (lengths == sizeof(PlayerProfile_Struct)) { /* If PP is the size it is expected to be */
			memcpy(pp, row2[1], sizeof(PlayerProfile_Struct));
		}
		/* Continue of PP Size does not match (Usually a created character never logged in) */
		else {
			// printf("%s ID: %i profile mismatch, not converting. PP %u - Profile Length %u \n", row2[2] ? row2[2] : "Unknown", character_id, sizeof(PlayerProfile_Struct), lengths);
			std::cout << (row2[2] ? row2[2] : "Unknown") << " ID: " << character_id << " size mismatch. Expected Size: " << sizeof(PlayerProfile_Struct) << " Seen: " << lengths << std::endl;
			continue;
		}

		lengths_e = results2.LengthOfColumn(11);
		if (lengths_e == sizeof(ExtendedProfile_Struct)) {
			memcpy(e_pp, row2[11], sizeof(ExtendedProfile_Struct));
		}
		if (e_pp->expended_aa > 4000000){ e_pp->expended_aa = 0; }

		/* Loading Status on conversion */
		if (runconvert == 1){
			std::cout << "\r" << char_iter_count << "/" << number_of_characters << " " << std::flush;
			loadbar(char_iter_count, number_of_characters, 50);

			/* Run inspect message convert  */
			if (inspectmessage != ""){
				std::string rquery = StringFormat("REPLACE INTO `character_inspect_messages` (id, inspect_message)"
					"VALUES (%u, '%s')",
					character_id,
					EscapeString(inspectmessage).c_str()
					);
				auto results = QueryDatabase(rquery);
				ThrowDBError(results.ErrorMessage(), "Character Inspect Message Convert", rquery);
			}

			/* Run Currency Convert */
			std::string rquery = StringFormat("REPLACE INTO `character_currency` (id, platinum, gold, silver, copper,"
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
				pp->careerEbonCrystals
			);
			auto results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Character Currency Convert", rquery);

			if (pp->tribute_time_remaining < 0 || pp->tribute_time_remaining == 4294967295){ pp->tribute_time_remaining = 0; }

			/* Run Character Data Convert */
			rquery = StringFormat(
				"REPLACE INTO `character_data` ("
				"id,"
				"account_id,"
				"`name`,"
				"last_name,"
				"gender,"
				"race,"
				"class,"
				"`level`,"
				"deity,"
				"birthday,"
				"last_login,"
				"time_played,"
				"pvp_status,"
				"level2,"
				"anon,"
				"gm,"
				"intoxication,"
				"hair_color,"
				"beard_color,"
				"eye_color_1,"
				"eye_color_2,"
				"hair_style,"
				"beard,"
				"ability_time_seconds,"
				"ability_number,"
				"ability_time_minutes,"
				"ability_time_hours,"
				"title,"
				"suffix,"
				"exp,"
				"points,"
				"mana,"
				"cur_hp,"
				"str,"
				"sta,"
				"cha,"
				"dex,"
				"`int`,"
				"agi,"
				"wis,"
				"face,"
				"y,"
				"x,"
				"z,"
				"heading,"
				"pvp2,"
				"pvp_type,"
				"autosplit_enabled,"
				"zone_change_count,"
				"drakkin_heritage,"
				"drakkin_tattoo,"
				"drakkin_details,"
				"toxicity,"
				"hunger_level,"
				"thirst_level,"
				"ability_up,"
				"zone_id,"
				"zone_instance,"
				"leadership_exp_on,"
				"ldon_points_guk,"
				"ldon_points_mir,"
				"ldon_points_mmc,"
				"ldon_points_ruj,"
				"ldon_points_tak,"
				"ldon_points_available,"
				"tribute_time_remaining,"
				"show_helm,"
				"career_tribute_points,"
				"tribute_points,"
				"tribute_active,"
				"endurance,"
				"group_leadership_exp,"
				"raid_leadership_exp,"
				"group_leadership_points,"
				"raid_leadership_points,"
				"air_remaining,"
				"pvp_kills,"
				"pvp_deaths,"
				"pvp_current_points,"
				"pvp_career_points,"
				"pvp_best_kill_streak,"
				"pvp_worst_death_streak,"
				"pvp_current_kill_streak,"
				"aa_points_spent,"
				"aa_exp,"
				"aa_points,"
				"group_auto_consent,"
				"raid_auto_consent,"
				"guild_auto_consent,"
				"RestTimer,"
				"firstlogon,"
				"lfg,"
				"lfp,"
				"mailkey,"
				"xtargets," 
				"e_aa_effects,"
				"e_percent_to_aa,"
				"e_expended_aa_spent"
				")"
				"VALUES ("
				"%u,"		// id														
				"%u,"		// account_id												
				"'%s',"		// `name`					  
				"'%s',"		// last_name					
				"%u,"		// gender					  
				"%u,"		// race						  
				"%u,"		// class							
				"%u,"		// `level`					  
				"%u,"		// deity							
				"%u,"		// birthday					  
				"%u,"		// last_login				  
				"%u,"		// time_played				  
				"%u,"		// pvp_status				  
				"%u,"		// level2					  
				"%u,"		// anon						  
				"%u,"		// gm						  
				"%u,"		// intoxication				  
				"%u,"		// hair_color				  
				"%u,"		// beard_color				  
				"%u,"		// eye_color_1				  
				"%u,"		// eye_color_2				  
				"%u,"		// hair_style				  
				"%u,"		// beard							
				"%u,"		// ability_time_seconds		  
				"%u,"		// ability_number			  
				"%u,"		// ability_time_minutes		  
				"%u,"		// ability_time_hours		  
				"'%s',"		// title						
				"'%s',"		// suffix					  
				"%u,"		// exp						  
				"%u,"		// points					  
				"%u,"		// mana						  
				"%u,"		// cur_hp					  
				"%u,"		// str						  
				"%u,"		// sta						  
				"%u,"		// cha						  
				"%u,"		// dex						  
				"%u,"		// `int`							
				"%u,"		// agi						  
				"%u,"		// wis						  
				"%u,"		// face						  
				"%f,"		// y								
				"%f,"		// x								
				"%f,"		// z								
				"%f,"		// heading					  
				"%u,"		// pvp2						  
				"%u,"		// pvp_type					  
				"%u,"		// autosplit_enabled				
				"%u,"		// zone_change_count				
				"%u,"		// drakkin_heritage			  
				"%u,"		// drakkin_tattoo			  
				"%u,"		// drakkin_details			  
				"%i,"		// toxicity	 				  
				"%u,"		// hunger_level				  
				"%u,"		// thirst_level				  
				"%u,"		// ability_up				  
				"%u,"		// zone_id					  
				"%u,"		// zone_instance					
				"%u,"		// leadership_exp_on				
				"%u,"		// ldon_points_guk			  
				"%u,"		// ldon_points_mir			  
				"%u,"		// ldon_points_mmc			  
				"%u,"		// ldon_points_ruj			  
				"%u,"		// ldon_points_tak			  
				"%u,"		// ldon_points_available			
				"%u,"		// tribute_time_remaining	  
				"%u,"		// show_helm						
				"%u,"		// career_tribute_points			
				"%u,"		// tribute_points			  
				"%u,"		// tribute_active			  
				"%u,"		// endurance						
				"%u,"		// group_leadership_exp		  
				"%u,"		// raid_leadership_exp		  
				"%u,"		// group_leadership_points	  
				"%u,"		// raid_leadership_points	  
				"%u,"		// air_remaining					
				"%u,"		// pvp_kills						
				"%u,"		// pvp_deaths				  
				"%u,"		// pvp_current_points		  
				"%u,"		// pvp_career_points				
				"%u,"		// pvp_best_kill_streak		  
				"%u,"		// pvp_worst_death_streak	  
				"%u,"		// pvp_current_kill_streak	  
				"%u,"		// aa_points_spent			  
				"%u,"		// aa_exp					  
				"%u,"		// aa_points						
				"%u,"		// group_auto_consent		  
				"%u,"		// raid_auto_consent				
				"%u,"		// guild_auto_consent		  
				"%u," 		// RestTimer
				"%u,"		// First Logon - References online status for EVENT_CONNECT/EVENT_DISCONNECt
				"%u,"		// Looking for Group
				"%u,"		// Looking for P?
				"'%s',"		// Mailkey
				"%u,"		// X Targets
				"%u,"		// AA Effects
				"%u,"		// Percent to AA
				"%u"		// e_expended_aa_spent
				")",
				character_id,
				account_id,
				EscapeString(pp->name).c_str(),
				EscapeString(pp->last_name).c_str(),
				pp->gender,
				pp->race,
				pp->class_,
				pp->level,
				pp->deity,
				pp->birthday,
				pp->lastlogin,
				pp->timePlayedMin,
				pp->pvp,
				pp->level2,
				pp->anon,
				pp->gm,
				pp->intoxication,
				pp->haircolor,
				pp->beardcolor,
				pp->eyecolor1,
				pp->eyecolor2,
				pp->hairstyle,
				pp->beard,
				pp->ability_time_seconds,
				pp->ability_number,
				pp->ability_time_minutes,
				pp->ability_time_hours,
				EscapeString(pp->title).c_str(),
				EscapeString(pp->suffix).c_str(),
				pp->exp,
				pp->points,
				pp->mana,
				pp->cur_hp,
				pp->STR,
				pp->STA,
				pp->CHA,
				pp->DEX,
				pp->INT,
				pp->AGI,
				pp->WIS,
				pp->face,
				pp->y,
				pp->x,
				pp->z,
				pp->heading,
				pp->pvp2,
				pp->pvptype,
				pp->autosplit,
				pp->zone_change_count,
				pp->drakkin_heritage,
				pp->drakkin_tattoo,
				pp->drakkin_details,
				pp->toxicity,
				pp->hunger_level,
				pp->thirst_level,
				pp->ability_up,
				pp->zone_id,
				pp->zoneInstance,
				pp->leadAAActive == 0 ? 0 : 1,
				pp->ldon_points_guk,
				pp->ldon_points_mir,
				pp->ldon_points_mmc,
				pp->ldon_points_ruj,
				pp->ldon_points_tak,
				pp->ldon_points_available,
				pp->tribute_time_remaining,
				pp->showhelm,
				pp->career_tribute_points,
				pp->tribute_points,
				pp->tribute_active,
				pp->endurance,
				pp->group_leadership_exp,
				pp->raid_leadership_exp,
				pp->group_leadership_points,
				pp->raid_leadership_points,
				pp->air_remaining,
				pp->PVPKills,
				pp->PVPDeaths,
				pp->PVPCurrentPoints,
				pp->PVPCareerPoints,
				pp->PVPBestKillStreak,
				pp->PVPWorstDeathStreak,
				pp->PVPCurrentKillStreak,
				pp->aapoints_spent,
				pp->expAA,
				pp->aapoints,
				pp->groupAutoconsent,
				pp->raidAutoconsent,
				pp->guildAutoconsent,
				pp->RestTimer,
				firstlogon,
				lfg,
				lfp,
				mailkey.c_str(),
				xtargets,
				e_pp->aa_effects,
				e_pp->perAA,
				e_pp->expended_aa
			);
			results = QueryDatabase(rquery);
			ThrowDBError(results.ErrorMessage(), "Character Data Convert", rquery);
			

			/*
				We set a first entry variable because we need the first initial piece of the query to be declared
				This is to speed up the INSERTS and trim down the amount of individual sends during the process.
				The speed difference is dramatic
			*/
			/* Run AA Convert */
			int first_entry = 0; rquery = "";
			for (i = 0; i < MAX_PP_AA_ARRAY; i++){
				if (pp->aa_array[i].AA > 0 && pp->aa_array[i].value > 0){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_alternate_abilities` (id, slot, aa_id, aa_value)"
							" VALUES (%u, %u, %u, %u)", character_id, i, pp->aa_array[i].AA, pp->aa_array[i].value);
						first_entry = 1;
					} else {
						rquery = rquery + StringFormat(", (%u, %u, %u, %u)", character_id, i, pp->aa_array[i].AA, pp->aa_array[i].value);
					}
				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "AA Convert", rquery); } 
			
			/* Run Bind Home Convert */
			if(pp->binds[4].zoneId < 999 && !_ISNAN_(pp->binds[4].x) && !_ISNAN_(pp->binds[4].y) && !_ISNAN_(pp->binds[4].z) && !_ISNAN_(pp->binds[4].heading)) {
				rquery = StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, is_home)"
					" VALUES (%u, %u, %u, %f, %f, %f, %f, 1)",
					character_id, pp->binds[4].zoneId, 0, pp->binds[4].x, pp->binds[4].y, pp->binds[4].z, pp->binds[4].heading);
				if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Bind Home Convert", rquery); }  
			}

			/* Run Bind Convert */
			if(pp->binds[0].zoneId < 999 && !_ISNAN_(pp->binds[0].x) && !_ISNAN_(pp->binds[0].y) && !_ISNAN_(pp->binds[0].z) && !_ISNAN_(pp->binds[0].heading)) {
				rquery = StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, is_home)"
					" VALUES (%u, %u, %u, %f, %f, %f, %f, 0)",
					character_id, pp->binds[0].zoneId, 0, pp->binds[0].x, pp->binds[0].y, pp->binds[0].z, pp->binds[0].heading);
				if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Bind Convert", rquery); }  
			}
			/* Run Language Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < MAX_PP_LANGUAGE; i++){
				if (pp->languages[i] > 0){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_languages` (id, lang_id, value) VALUES (%u, %u, %u)", character_id, i, pp->languages[i]);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->languages[i]);
				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Language Convert", rquery);  } 
			/* Run Skill Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < MAX_PP_SKILL; i++){
				if (pp->skills[i] > 0){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_skills` (id, skill_id, value) VALUES (%u, %u, %u)", character_id, i, pp->skills[i]);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->skills[i]);
				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Skills Convert Convert", rquery);  } 
			/* Run Spell Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < MAX_PP_REF_SPELLBOOK; i++){
				if (pp->spell_book[i] > 0 && pp->spell_book[i] != 4294967295 && pp->spell_book[i] < 40000 && pp->spell_book[i] != 1){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_spells` (id, slot_id, spell_id) VALUES (%u, %u, %u)", character_id, i, pp->spell_book[i]);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->spell_book[i]);
				}
			}
			// std::cout << rquery << "\n";
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Spell Convert", rquery);  }  
			/* Run Max Memmed Spell Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < MAX_PP_REF_MEMSPELL; i++){
				if (pp->mem_spells[i] > 0 && pp->mem_spells[i] != 65535 && pp->mem_spells[i] != 4294967295){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_memmed_spells` (id, slot_id, spell_id) VALUES (%u, %u, %u)", character_id, i, pp->mem_spells[i]);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->mem_spells[i]);
				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Memmed Spells Convert", rquery);  }  
			/* Run Discipline Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < MAX_PP_DISCIPLINES; i++){
				if(pp->disciplines.values[i] > 0 && pp->disciplines.values[i] < 60000){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_disciplines` (id, slot_id, disc_id) VALUES (%u, %u, %u)", character_id, i, pp->disciplines.values[i]);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->disciplines.values[i]);
				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Discipline Convert", rquery);  } 
			/* Run Material Color Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < _MaterialCount; i++){
				if (pp->item_tint[i].color > 0){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_material` (id, slot, blue, green, red, use_tint, color) VALUES (%u, %u, %u, %u, %u, %u, %u)", character_id, i, pp->item_tint[i].rgb.blue, pp->item_tint[i].rgb.green, pp->item_tint[i].rgb.red, pp->item_tint[i].rgb.use_tint, pp->item_tint[i].color);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%u, %u, %u, %u, %u, %u, %u)", character_id, i, pp->item_tint[i].rgb.blue, pp->item_tint[i].rgb.green, pp->item_tint[i].rgb.red, pp->item_tint[i].rgb.use_tint, pp->item_tint[i].color);
				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Material Convert", rquery);  } 
			/* Run Tribute Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < EmuConstants::TRIBUTE_SIZE; i++){
				if (pp->tributes[i].tribute > 0 && pp->tributes[i].tribute != 4294967295){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_tribute` (id, tier, tribute) VALUES (%u, %u, %u)", character_id, pp->tributes[i].tier, pp->tributes[i].tribute);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, pp->tributes[i].tier, pp->tributes[i].tribute);
				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Tribute Convert", rquery);  }
			/* Run Bandolier Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < EmuConstants::BANDOLIERS_COUNT; i++){
				if(strlen(pp->bandoliers[i].name) < 32) {
					for (int si = 0; si < EmuConstants::BANDOLIER_SIZE; si++){
						if (pp->bandoliers[i].items[si].item_id > 0){
							if (first_entry != 1) {
								rquery = StringFormat("REPLACE INTO `character_bandolier` (id, bandolier_id, bandolier_slot, item_id, icon, bandolier_name) VALUES (%i, %u, %i, %u, %u, '%s')", character_id, i, si, pp->bandoliers[i].items[si].item_id, pp->bandoliers[i].items[si].icon, pp->bandoliers[i].name);
								first_entry = 1;
							}
							rquery = rquery + StringFormat(", (%i, %u, %i, %u, %u, '%s')", character_id, i, si, pp->bandoliers[i].items[si].item_id, pp->bandoliers[i].items[si].icon, pp->bandoliers[i].name);
						}
					}
				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Bandolier Convert", rquery);  } 
			/* Run Potion Belt Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < EmuConstants::POTION_BELT_SIZE; i++){
				if (pp->potionbelt.items[i].item_id > 0){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_potionbelt` (id, potion_id, item_id, icon) VALUES (%i, %u, %u, %u)", character_id, i, pp->potionbelt.items[i].item_id, pp->potionbelt.items[i].icon);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%i, %u, %u, %u)", character_id, i, pp->potionbelt.items[i].item_id, pp->potionbelt.items[i].icon);

				}
			}
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Potion Belt Convert", rquery);  } 
			/* Run Leadership AA Convert */
			first_entry = 0; rquery = "";
			for (i = 0; i < MAX_LEADERSHIP_AA_ARRAY; i++){
				if(pp->leader_abilities.ranks[i] > 0 && pp->leader_abilities.ranks[i] < 6){
					if (first_entry != 1){
						rquery = StringFormat("REPLACE INTO `character_leadership_abilities` (id, slot, rank) VALUES (%i, %u, %u)", character_id, i, pp->leader_abilities.ranks[i]);
						first_entry = 1;
					}
					rquery = rquery + StringFormat(", (%i, %u, %u)", character_id, i, pp->leader_abilities.ranks[i]);
				}
			} 
			if (rquery != ""){ results = QueryDatabase(rquery); ThrowDBError(results.ErrorMessage(), "Character Leadership AA Convert", rquery);  }
		}
	}
	if (runconvert == 1){
		std::string rquery = StringFormat("RENAME TABLE `character_` TO `character_old`"); QueryDatabase(rquery);
		printf("\n\nRenaming `character_` table to `character_old`, this is a LARGE table so when you don't need it anymore, I would suggest deleting it yourself...\n"); 
		printf("\n\nCharacter blob conversion complete, continuing world bootup...\n"); 
	}

	return true;
}

bool Database::LoadVariables() {
	char *query = nullptr;

	auto results = QueryDatabase(query, LoadVariables_MQ(&query));

	if (!results.Success())
	{
		std::cerr << "Error in LoadVariables query '" << query << "' " << results.ErrorMessage() << std::endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return LoadVariables_result(std::move(results));
}

uint32 Database::LoadVariables_MQ(char** query)
{
	return MakeAnyLenString(query, "SELECT varname, value, unix_timestamp() FROM variables where unix_timestamp(ts) >= %d", varcache_lastupdate);
}

// always returns true? not sure about this.
bool Database::LoadVariables_result(MySQLRequestResult results) {
	uint32 i = 0;
	LockMutex lock(&Mvarcache);

	if (results.RowCount() == 0)
		return true;

	if (!varcache_array) {
		varcache_max = results.RowCount();
		varcache_array = new VarCache_Struct*[varcache_max];
		for (i=0; i<varcache_max; i++)
			varcache_array[i] = 0;
	}
	else {
		uint32 tmpnewmax = varcache_max + results.RowCount();
		VarCache_Struct** tmp = new VarCache_Struct*[tmpnewmax];
		for (i=0; i<tmpnewmax; i++)
			tmp[i] = 0;
		for (i=0; i<varcache_max; i++)
			tmp[i] = varcache_array[i];
		VarCache_Struct** tmpdel = varcache_array;
		varcache_array = tmp;
		varcache_max = tmpnewmax;
		delete [] tmpdel;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
	{
		varcache_lastupdate = atoi(row[2]);
		for (i=0; i<varcache_max; i++) {
			if (varcache_array[i]) {
				if (strcasecmp(varcache_array[i]->varname, row[0]) == 0) {
					delete varcache_array[i];
					varcache_array[i] = (VarCache_Struct*) new uint8[sizeof(VarCache_Struct) + strlen(row[1]) + 1];
					strn0cpy(varcache_array[i]->varname, row[0], sizeof(varcache_array[i]->varname));
					strcpy(varcache_array[i]->value, row[1]);
					break;
				}
			}
			else {
				varcache_array[i] = (VarCache_Struct*) new uint8[sizeof(VarCache_Struct) + strlen(row[1]) + 1];
				strcpy(varcache_array[i]->varname, row[0]);
				strcpy(varcache_array[i]->value, row[1]);
				break;
			}
		}
	}

	uint32 max_used = 0;
	for (i=0; i<varcache_max; i++) {
		if (varcache_array[i]) {
			if (i > max_used)
				max_used = i;
		}
	}

	varcache_max = max_used + 1;

	return true;
}

// Gets variable from 'variables' table
bool Database::GetVariable(const char* varname, char* varvalue, uint16 varvalue_len) {
	varvalue[0] = '\0';

	LockMutex lock(&Mvarcache);
	if (strlen(varname) <= 1)
		return false;
	for (uint32 i=0; i<varcache_max; i++) {

		if (varcache_array[i]) {
			if (strcasecmp(varcache_array[i]->varname, varname) == 0) {
				snprintf(varvalue, varvalue_len, "%s", varcache_array[i]->value);
				varvalue[varvalue_len-1] = 0;
				return true;
			}
		}
		else
			return false;
	}
	return false;
}

bool Database::SetVariable(const char* varname_in, const char* varvalue_in) {
	
	char *varname,*varvalue;

	varname=(char *)malloc(strlen(varname_in)*2+1);
	varvalue=(char *)malloc(strlen(varvalue_in)*2+1);
	DoEscapeString(varname, varname_in, strlen(varname_in));
	DoEscapeString(varvalue, varvalue_in, strlen(varvalue_in));

	std::string query = StringFormat("Update variables set value='%s' WHERE varname like '%s'", varvalue, varname);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in SetVariable query '" << query << "' " << results.ErrorMessage() << std::endl;
		free(varname);
		free(varvalue);
		return false;
	}

	if (results.RowsAffected() == 1)
	{
		LoadVariables(); // refresh cache
		free(varname);
		free(varvalue);
		return true;
	}

	query = StringFormat("Insert Into variables (varname, value) values ('%s', '%s')", varname, varvalue);
	results = QueryDatabase(query);
	free(varname);
	free(varvalue);

	if (results.RowsAffected() != 1)
		return false;
	
	LoadVariables(); // refresh cache
	return true;
}

uint32 Database::GetMiniLoginAccount(char* ip){

	std::string query = StringFormat("SELECT id FROM account WHERE minilogin_ip='%s'", ip);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetMiniLoginAccount query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	auto row = results.begin();

	return atoi(row[0]);
}

// Get zone starting points from DB
bool Database::GetSafePoints(const char* short_name, uint32 version, float* safe_x, float* safe_y, float* safe_z, int16* minstatus, uint8* minlevel, char *flag_needed) {
	
	std::string query = StringFormat("SELECT safe_x, safe_y, safe_z, min_status, min_level, flag_needed FROM zone "
		" WHERE short_name='%s' AND (version=%i OR version=0) ORDER BY version DESC", short_name, version);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetSafePoint query '" << query << "' " << results.ErrorMessage() << std::endl;
		std::cerr << "If it errors, run the following querys:\n";
		std::cerr << "ALTER TABLE `zone` CHANGE `minium_level` `min_level` TINYINT(3)  UNSIGNED DEFAULT \"0\" NOT NULL;\n";
		std::cerr << "ALTER TABLE `zone` CHANGE `minium_status` `min_status` TINYINT(3)  UNSIGNED DEFAULT \"0\" NOT NULL;\n";
		std::cerr << "ALTER TABLE `zone` ADD flag_needed VARCHAR(128) NOT NULL DEFAULT '';\n";
		return false;
	}

	if (results.RowCount() == 0)
		return false;

	auto row = results.begin();

	if (safe_x != nullptr)
		*safe_x = atof(row[0]);
	if (safe_y != nullptr)
		*safe_y = atof(row[1]);
	if (safe_z != nullptr)
		*safe_z = atof(row[2]);
	if (minstatus != nullptr)
		*minstatus = atoi(row[3]);
	if (minlevel != nullptr)
		*minlevel = atoi(row[4]);
	if (flag_needed != nullptr)
		strcpy(flag_needed, row[5]);

	return true;
}

bool Database::GetZoneLongName(const char* short_name, char** long_name, char* file_name, float* safe_x, float* safe_y, float* safe_z, uint32* graveyard_id, uint32* maxclients) {
	
	std::string query = StringFormat("SELECT long_name, file_name, safe_x, safe_y, safe_z, graveyard_id, maxclients FROM zone WHERE short_name='%s' AND version=0", short_name);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in GetZoneLongName query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	if (results.RowCount() == 0)
		return false;

	auto row = results.begin();

	if (long_name != nullptr)
		*long_name = strcpy(new char[strlen(row[0])+1], row[0]);

	if (file_name != nullptr) {
		if (row[1] == nullptr)
			strcpy(file_name, short_name);
		else
			strcpy(file_name, row[1]);
	}

	if (safe_x != nullptr)
		*safe_x = atof(row[2]);
	if (safe_y != nullptr)
		*safe_y = atof(row[3]);
	if (safe_z != nullptr)
		*safe_z = atof(row[4]);
	if (graveyard_id != nullptr)
		*graveyard_id = atoi(row[5]);
	if (maxclients != nullptr)
		*maxclients = atoi(row[6]);

	return true;
}

uint32 Database::GetZoneGraveyardID(uint32 zone_id, uint32 version) {

	std::string query = StringFormat("SELECT graveyard_id FROM zone WHERE zoneidnumber='%u' AND (version=%i OR version=0) ORDER BY version DESC", zone_id, version);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetZoneGraveyardID query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

bool Database::GetZoneGraveyard(const uint32 graveyard_id, uint32* graveyard_zoneid, float* graveyard_x, float* graveyard_y, float* graveyard_z, float* graveyard_heading) {
	
	std::string query = StringFormat("SELECT zone_id, x, y, z, heading FROM graveyard WHERE id=%i", graveyard_id);
	auto results = QueryDatabase(query);

	if (!results.Success()){
		std::cerr << "Error in GetZoneGraveyard query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	if (results.RowCount() != 1)
		return false;

	auto row = results.begin();

	if(graveyard_zoneid != nullptr)
		*graveyard_zoneid = atoi(row[0]);
	if(graveyard_x != nullptr)
		*graveyard_x = atof(row[1]);
	if(graveyard_y != nullptr)
		*graveyard_y = atof(row[2]);
	if(graveyard_z != nullptr)
		*graveyard_z = atof(row[3]);
	if(graveyard_heading != nullptr)
		*graveyard_heading = atof(row[4]);

	return true;
}

bool Database::LoadZoneNames() {
	std::string query("SELECT zoneidnumber, short_name FROM zone");

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in LoadZoneNames query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	for (auto row= results.begin();row != results.end();++row)
	{
		uint32 zoneid = atoi(row[0]);
		std::string zonename = row[1];
		zonename_array.insert(std::pair<uint32,std::string>(zoneid,zonename));
	}

	return true;
}

uint32 Database::GetZoneID(const char* zonename) {

	if (zonename == nullptr)
		return 0;

	for (auto iter = zonename_array.begin(); iter != zonename_array.end(); ++iter)
		if (strcasecmp(iter->second.c_str(), zonename) == 0)
			return iter->first;

	return 0;
}

const char* Database::GetZoneName(uint32 zoneID, bool ErrorUnknown) {
	auto iter = zonename_array.find(zoneID);

	if (iter != zonename_array.end())
		return iter->second.c_str();

	if (ErrorUnknown)
		return "UNKNOWN";

	return 0;
}

uint8 Database::GetPEQZone(uint32 zoneID, uint32 version){
	
	std::string query = StringFormat("SELECT peqzone from zone where zoneidnumber='%i' AND (version=%i OR version=0) ORDER BY version DESC", zoneID, version);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetPEQZone query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

bool Database::CheckNameFilter(const char* name, bool surname)
{
	std::string str_name = name;

	if(surname)
	{
		// the minimum 4 is enforced by the client too
		if(!name || strlen(name) < 3)
		{
			return false;
		}
	}
	else
	{
		// the minimum 4 is enforced by the client too
		if(!name || strlen(name) < 4 || strlen(name) > 15)
		{
			return false;
		}
	}

	for (size_t i = 0; i < str_name.size(); i++)
	{
		if(!isalpha(str_name[i]))
		{
			return false;
		}
	}

	for(size_t x = 0; x < str_name.size(); ++x)
	{
		str_name[x] = tolower(str_name[x]);
	}

	char c = '\0';
	uint8 num_c = 0;
	for(size_t x = 0; x < str_name.size(); ++x)
	{
		if(str_name[x] == c)
		{
			num_c++;
		}
		else
		{
			num_c = 1;
			c = str_name[x];
		}
		if(num_c > 2)
		{
			return false;
		}
	}

	
	std::string query("SELECT name FROM name_filter");
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in CheckNameFilter query '" << query << "' " << results.ErrorMessage() << std::endl;
		// false through to true? shouldn't it be falls through to false?
		return true;
	}

	for (auto row = results.begin();row != results.end();++row)
	{
		std::string current_row = row[0];

		for(size_t x = 0; x < current_row.size(); ++x)
			current_row[x] = tolower(current_row[x]);

		if(str_name.find(current_row) != std::string::npos)
			return false;
	}

	return true;
}

bool Database::AddToNameFilter(const char* name) {
	
	std::string query = StringFormat("INSERT INTO name_filter (name) values ('%s')", name);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in AddToNameFilter query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	if (results.RowsAffected() == 0)
		return false;

	return true;
}

uint32 Database::GetAccountIDFromLSID(uint32 iLSID, char* oAccountName, int16* oStatus) {
	uint32 account_id = 0;
	std::string query = StringFormat("SELECT id, name, status FROM account WHERE lsaccount_id=%i", iLSID);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetAccountIDFromLSID query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	for (auto row = results.begin(); row != results.end(); ++row) {
		account_id = atoi(row[0]);

		if (oAccountName)
			strcpy(oAccountName, row[1]);
		if (oStatus)
			*oStatus = atoi(row[2]);
	}

	return account_id;
}

void Database::GetAccountFromID(uint32 id, char* oAccountName, int16* oStatus) {
	
	std::string query = StringFormat("SELECT name, status FROM account WHERE id=%i", id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetAccountFromID query '" << query << "' " << results.ErrorMessage() << std::endl;
		return;
	}

	if (results.RowCount() != 1)
		return;

	auto row = results.begin();

	if (oAccountName)
		strcpy(oAccountName, row[0]);
	if (oStatus)
		*oStatus = atoi(row[1]);
}

void Database::ClearMerchantTemp(){

	std::string query("delete from merchantlist_temp");
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cerr << "Error in ClearMerchantTemp query '" << query << "' " << results.ErrorMessage() << std::endl;
}

bool Database::UpdateName(const char* oldname, const char* newname) { 
	std::cout << "Renaming " << oldname << " to " << newname << "..." << std::endl; 
	std::string query = StringFormat("UPDATE `character_data` SET `name` = '%s' WHERE `name` = '%s';", newname, oldname);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowsAffected() == 0)
		return false;

	return true;
}

// If the name is used or an error occurs, it returns false, otherwise it returns true
bool Database::CheckUsedName(const char* name) {
	std::string query = StringFormat("SELECT `id` FROM `character_data` WHERE `name` = '%s'", name);
	auto results = QueryDatabase(query); 
	if (!results.Success()) {
		std::cerr << "Error in CheckUsedName query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	if (results.RowCount() > 0)
		return false;

	return true;
}

uint8 Database::GetServerType() {
	std::string query("SELECT `value` FROM `variables` WHERE `varname` = 'ServerType' LIMIT 1");
	auto results = QueryDatabase(query); 
	if (!results.Success()) {
		std::cerr << "Error in GetServerType query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

bool Database::MoveCharacterToZone(const char* charname, const char* zonename, uint32 zoneid) {
	if(zonename == nullptr || strlen(zonename) == 0)
		return false;

	std::string query = StringFormat("UPDATE `character_data` SET `zone_id` = %i, `x` = -1, `y` = -1, `z` = -1 WHERE `name` = '%s'", zoneid, charname);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in MoveCharacterToZone(name) query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	if (results.RowsAffected() == 0)
		return false;

	return true;
}

bool Database::MoveCharacterToZone(const char* charname, const char* zonename) {
	return MoveCharacterToZone(charname, zonename, GetZoneID(zonename));
}

bool Database::MoveCharacterToZone(uint32 iCharID, const char* iZonename) { 
	std::string query = StringFormat("UPDATE `character_data` SET `zone_id` = %i, `x` = -1, `y` = -1, `z` = -1 WHERE `id` = %i", iZonename, GetZoneID(iZonename), iCharID);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in MoveCharacterToZone(id) query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	return results.RowsAffected() != 0;
}

bool Database::SetHackerFlag(const char* accountname, const char* charactername, const char* hacked) { 
	std::string query = StringFormat("INSERT INTO `hackers` (account, name, hacked) values('%s','%s','%s')", accountname, charactername, hacked);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in SetHackerFlag query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	return results.RowsAffected() != 0;
}

bool Database::SetMQDetectionFlag(const char* accountname, const char* charactername, const char* hacked, const char* zone) { 
	//Utilize the "hacker" table, but also give zone information.
	std::string query = StringFormat("INSERT INTO hackers(account,name,hacked,zone) values('%s','%s','%s','%s')", accountname, charactername, hacked, zone);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in SetMQDetectionFlag query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	return results.RowsAffected() != 0;
}

uint8 Database::GetRaceSkill(uint8 skillid, uint8 in_race)
{
	uint16 race_cap = 0;
	
	//Check for a racial cap!
	std::string query = StringFormat("SELECT skillcap from race_skillcaps where skill = %i && race = %i", skillid, in_race);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint8 Database::GetSkillCap(uint8 skillid, uint8 in_race, uint8 in_class, uint16 in_level)
{
	uint8 skill_level = 0, skill_formula = 0;
	uint16 base_cap = 0, skill_cap = 0, skill_cap2 = 0, skill_cap3 = 0;
	

	//Fetch the data from DB.
	std::string query = StringFormat("SELECT level, formula, pre50cap, post50cap, post60cap from skillcaps where skill = %i && class = %i", skillid, in_class);
	auto results = QueryDatabase(query);

	if (results.Success() && results.RowsAffected() != 0)
	{
		auto row = results.begin();
		skill_level = atoi(row[0]);
		skill_formula = atoi(row[1]);
		skill_cap = atoi(row[2]);
		if (atoi(row[3]) > skill_cap)
			skill_cap2 = (atoi(row[3])-skill_cap)/10; //Split the post-50 skill cap into difference between pre-50 cap and post-50 cap / 10 to determine amount of points per level.
		skill_cap3 = atoi(row[4]);
	}

	int race_skill = GetRaceSkill(skillid,in_race);

	if (race_skill > 0 && (race_skill > skill_cap || skill_cap == 0 || in_level < skill_level))
		return race_skill;

	if (skill_cap == 0) //Can't train this skill at all.
		return 255; //Untrainable

	if (in_level < skill_level)
		return 254; //Untrained

	//Determine pre-51 level-based cap
	if (skill_formula > 0)
		base_cap = in_level*skill_formula+skill_formula;
	if (base_cap > skill_cap || skill_formula == 0)
		base_cap = skill_cap;

	//If post 50, add post 50 cap to base cap.
	if (in_level > 50 && skill_cap2 > 0)
		base_cap += skill_cap2*(in_level-50);

	//No cap should ever go above its post50cap
	if (skill_cap3 > 0 && base_cap > skill_cap3)
		base_cap = skill_cap3;

	//Base cap is now the max value at the person's level, return it!
	return base_cap;
}

uint32 Database::GetCharacterInfo(const char* iName, uint32* oAccID, uint32* oZoneID, uint32* oInstanceID, float* oX, float* oY, float* oZ) { 
	std::string query = StringFormat("SELECT `id`, `account_id`, `zone_id`, `zone_instance`, `x`, `y`, `z` FROM `character_data` WHERE `name` = '%s'", iName);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		std::cerr << "Error in GetCharacterInfo query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();
	uint32 charid = atoi(row[0]);
	if (oAccID){ *oAccID = atoi(row[1]); }
	if (oZoneID){ *oZoneID = atoi(row[2]); }
	if (oInstanceID){ *oInstanceID = atoi(row[3]); }
	if (oX){ *oX = atof(row[4]); }
	if (oY){ *oY = atof(row[5]); }
	if (oZ){ *oZ = atof(row[6]); }

	return charid;
}

bool Database::UpdateLiveChar(char* charname,uint32 lsaccount_id) {

	std::string query = StringFormat("UPDATE account SET charname='%s' WHERE id=%i;",charname, lsaccount_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in UpdateLiveChar query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	return true;
}

bool Database::GetLiveChar(uint32 account_id, char* cname) {

	std::string query = StringFormat("SELECT charname FROM account WHERE id=%i", account_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetLiveChar query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	if (results.RowCount() != 1)
		return false;

	auto row = results.begin();
	strcpy(cname,row[0]);

	return true;
}

void Database::SetLFP(uint32 CharID, bool LFP) { 
	std::string query = StringFormat("UPDATE `character_data` SET `lfp` = %i WHERE `id` = %i",LFP, CharID);
	auto results = QueryDatabase(query); 
	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, results.ErrorMessage().c_str());
}

void Database::SetLoginFlags(uint32 CharID, bool LFP, bool LFG, uint8 firstlogon) { 
	std::string query = StringFormat("update `character_data` SET `lfp` = %i, `lfg` = %i, `firstlogon` = %i WHERE `id` = %i",LFP, LFG, firstlogon, CharID);
	auto results = QueryDatabase(query); 
	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, results.ErrorMessage().c_str());
}

void Database::SetLFG(uint32 CharID, bool LFG) { 
	std::string query = StringFormat("update `character_data` SET `lfg` = %i WHERE `id` = %i",LFG, CharID);
	auto results = QueryDatabase(query); 
	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, results.ErrorMessage().c_str());
}

void Database::SetFirstLogon(uint32 CharID, uint8 firstlogon) { 
	std::string query = StringFormat( "UPDATE `character_data` SET `firstlogon` = %i WHERE `id` = %i",firstlogon, CharID);
	auto results = QueryDatabase(query); 
	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error updating firstlogon for character %i : %s", CharID, results.ErrorMessage().c_str());
}

void Database::AddReport(std::string who, std::string against, std::string lines) { 
	char *escape_str = new char[lines.size()*2+1];
	DoEscapeString(escape_str, lines.c_str(), lines.size());

	std::string query = StringFormat("INSERT INTO reports (name, reported, reported_text) VALUES('%s', '%s', '%s')", who.c_str(), against.c_str(), escape_str);
	auto results = QueryDatabase(query);
	safe_delete_array(escape_str);

	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error adding a report for %s: %s", who.c_str(), results.ErrorMessage().c_str());
}

void Database::SetGroupID(const char* name, uint32 id, uint32 charid, uint32 ismerc) {
	std::string query;
	if (id == 0) {
		// removing from group
		query = StringFormat("delete from group_id where charid=%i and name='%s' and ismerc=%i",charid, name, ismerc);
		auto results = QueryDatabase(query);

		if (!results.Success())
			LogFile->write(EQEMuLog::Error, "Error deleting character from group id: %s", results.ErrorMessage().c_str());

		return;
	}

	/* Add to the Group */
	query = StringFormat("REPLACE INTO  `group_id` SET `charid` = %i, `groupid` = %i, `name` = '%s', `ismerc` = '%i'", charid, id, name, ismerc);
	auto results = QueryDatabase(query);

	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error adding character to group id: %s", results.ErrorMessage().c_str());
}

void Database::ClearAllGroups(void)
{
	std::string query("delete from group_id");
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to clear groups: " << results.ErrorMessage() << std::endl;

	return;
}

void Database::ClearGroup(uint32 gid) {
	ClearGroupLeader(gid);
	
	if(gid == 0)
	{
		//clear all groups
		ClearAllGroups();
		return;
	}

	//clear a specific group
	std::string query = StringFormat("delete from group_id where groupid = %lu", (unsigned long)gid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to clear groups: " << results.ErrorMessage() << std::endl;
}

uint32 Database::GetGroupID(const char* name){

	std::string query = StringFormat("SELECT groupid from group_id where name='%s'", name);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		LogFile->write(EQEMuLog::Error, "Error getting group id: %s", results.ErrorMessage().c_str());
		return 0;
	}

	if (results.RowCount() == 0)
	{
		LogFile->write(EQEMuLog::Debug, "Character not in a group: %s", name);
		return 0;
	}

	auto row = results.begin();

	return atoi(row[0]);
}

/* Is this really getting used properly... A half implementation ? Akkadius */
char* Database::GetGroupLeaderForLogin(const char* name, char* leaderbuf){ 
	leaderbuf = "";
	std::string query = StringFormat("SELECT `groupid` FROM `group_id` WHERE `name = '%s'", name);
	auto results = QueryDatabase(query);
	auto row = results.begin(); uint32 group_id = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (row[0]){ group_id = atoi(row[0]); }
	}

	if (group_id > 0){
		query = StringFormat("SELECT `leadername` FROM `group_leader` WHERE `gid` = '%u' AND `groupid` = %u LIMIT 1", group_id);
		results = QueryDatabase(query);
		for (auto row = results.begin(); row != results.end(); ++row) {
			if (row[0]){ strcpy(leaderbuf, row[0]); }
		}
	}

	return leaderbuf;
}

void Database::SetGroupLeaderName(uint32 gid, const char* name) { 
	std::string query = StringFormat("REPLACE INTO `group_leaders` SET `gid` = %lu, `leadername` = '%s'",(unsigned long)gid,name);
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to set group leader: " << results.ErrorMessage() << std::endl;
}

char *Database::GetGroupLeadershipInfo(uint32 gid, char* leaderbuf, char* maintank, char* assist, char* puller, char *marknpc, GroupLeadershipAA_Struct* GLAA){ 
	std::string query = StringFormat("SELECT `leadername`, `maintank`, `assist`, `puller`, `marknpc`, `leadershipaa` FROM `group_leaders` WHERE `gid` = %lu",(unsigned long)gid);
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() == 0) {
		if(leaderbuf)
			strcpy(leaderbuf, "UNKNOWN");

		if(maintank)
			maintank[0] = '\0';

		if(assist)
			assist[0] = '\0';

		if(puller)
			puller[0] = '\0';

		if(marknpc)
			marknpc[0] = '\0';

		return leaderbuf;
	}

	auto row = results.begin();

	if(leaderbuf)
		strcpy(leaderbuf, row[0]);

	if(maintank)
		strcpy(maintank, row[1]);

	if(assist)
		strcpy(assist, row[2]);

	if(puller)
		strcpy(puller, row[3]);

	if(marknpc)
		strcpy(marknpc, row[4]);

	if(GLAA && results.LengthOfColumn(5) == sizeof(GroupLeadershipAA_Struct))
		memcpy(GLAA, row[5], sizeof(GroupLeadershipAA_Struct));

	return leaderbuf;
}

// Clearing all group leaders
void Database::ClearAllGroupLeaders(void) {
	std::string query("DELETE from group_leaders");
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to clear group leaders: " << results.ErrorMessage() << std::endl;

	return;
}

void Database::ClearGroupLeader(uint32 gid) {
	
	if(gid == 0)
	{
		ClearAllGroupLeaders();
		return;
	}

	std::string query = StringFormat("DELETE from group_leaders where gid = %lu", (unsigned long)gid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to clear group leader: " << results.ErrorMessage() << std::endl;
}

uint8 Database::GetAgreementFlag(uint32 acctid) {

	std::string query = StringFormat("SELECT rulesflag FROM account WHERE id=%i",acctid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

void Database::SetAgreementFlag(uint32 acctid) {
	std::string query = StringFormat("UPDATE account SET rulesflag=1 where id=%i", acctid);
	QueryDatabase(query);
}

void Database::ClearRaid(uint32 rid) {
	if(rid == 0)
	{
		//clear all raids
		ClearAllRaids();
		return;
	}

	//clear a specific group
	std::string query = StringFormat("delete from raid_members where raidid = %lu", (unsigned long)rid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to clear raids: " << results.ErrorMessage() << std::endl;
}

void Database::ClearAllRaids(void) {

	std::string query("delete from raid_members");
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to clear raids: " << results.ErrorMessage() << std::endl;
}

void Database::ClearAllRaidDetails(void)
{

	std::string query("delete from raid_details");
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to clear raid details: " << results.ErrorMessage() << std::endl;
}

void Database::ClearRaidDetails(uint32 rid) {
	
	if(rid == 0)
	{
		//clear all raids
		ClearAllRaidDetails();
		return;
	}

	//clear a specific group
	std::string query = StringFormat("delete from raid_details where raidid = %lu", (unsigned long)rid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to clear raid details: " << results.ErrorMessage() << std::endl;
}

// returns 0 on error or no raid for that character, or
// the raid id that the character is a member of.
uint32 Database::GetRaidID(const char* name){

	std::string query = StringFormat("SELECT raidid from raid_members where name='%s'", name);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cout << "Unable to get raid id: " << results.ErrorMessage() << std::endl;
		return 0;
	}

	auto row = results.begin();

	if (row == results.end())
	{
		std::cout << "Unable to get raid id, char not found!" << std::endl;
		return 0;
	}

	if (row[0]) // would it ever be possible to have a null here?
		return atoi(row[0]);

	return 0;
}

const char* Database::GetRaidLeaderName(uint32 rid)
{
	// Would be a good idea to fix this to be a passed in variable and
	// make the caller responsible. static local variables like this are
	// not guaranteed to be thread safe (nor is the internal guard
	// variable). C++0x standard states this should be thread safe
	// but may not be fully supported in some compilers.
	static char name[128];
	
	std::string query = StringFormat("SELECT name FROM raid_members WHERE raidid=%u AND israidleader=1",rid);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cout << "Unable to get raid id: " << results.ErrorMessage() << std::endl;
		return "UNKNOWN";
	}

	auto row = results.begin();

	if (row == results.end())
	{
		std::cout << "Unable to get raid id, char not found!" << std::endl;
		return "UNKNOWN";
	}

	memset(name, 0, sizeof(name));
	strcpy(name, row[0]);

	return name;
}

bool Database::VerifyInstanceAlive(uint16 instance_id, uint32 char_id)
{
	//we are not saved to this instance so set our instance to 0
	if(!GlobalInstance(instance_id) && !CharacterInInstanceGroup(instance_id, char_id))
		return false;

	if(CheckInstanceExpired(instance_id))
	{
		DeleteInstance(instance_id);
		return false;
	}

	return true;
}

bool Database::VerifyZoneInstance(uint32 zone_id, uint16 instance_id)
{

	std::string query = StringFormat("SELECT id FROM instance_list where id=%u AND zone=%u",instance_id, zone_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	return true;
}

bool Database::CharacterInInstanceGroup(uint16 instance_id, uint32 char_id)
{

	std::string query = StringFormat("SELECT charid FROM instance_list_player where id=%u AND charid=%u",instance_id, char_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() != 1)
		return false;

	return true;
}

void Database::DeleteInstance(uint16 instance_id)
{

	std::string query = StringFormat("DELETE FROM instance_list WHERE id=%u", instance_id);
	QueryDatabase(query);

	query = StringFormat("DELETE FROM instance_list_player WHERE id=%u", instance_id);
	QueryDatabase(query);

	query = StringFormat("DELETE FROM respawn_times WHERE instance_id=%u", instance_id);
	QueryDatabase(query);

	query = StringFormat("DELETE FROM spawn_condition_values WHERE instance_id=%u", instance_id);
	QueryDatabase(query);

	BuryCorpsesInInstance(instance_id);
}

bool Database::CheckInstanceExpired(uint16 instance_id)
{
	
	int32 start_time = 0;
	int32 duration = 0;
	uint32 never_expires = 0;

	std::string query = StringFormat("SELECT start_time, duration, never_expires FROM instance_list WHERE id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return true;

	if (results.RowCount() == 0)
		return true;

	auto row = results.begin();

	start_time = atoi(row[0]);
	duration = atoi(row[1]);
	never_expires = atoi(row[2]);

	if(never_expires == 1)
		return false;

	timeval tv;
	gettimeofday(&tv, nullptr);

	if((start_time + duration) <= tv.tv_sec)
		return true;

	return false;
}

uint32 Database::ZoneIDFromInstanceID(uint16 instance_id)
{

	std::string query = StringFormat("SELECT zone FROM instance_list where id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

uint32 Database::VersionFromInstanceID(uint16 instance_id)
{

	std::string query = StringFormat("SELECT version FROM instance_list where id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

uint32 Database::GetTimeRemainingInstance(uint16 instance_id, bool &is_perma)
{
	uint32 start_time = 0;
	uint32 duration = 0;
	uint32 never_expires = 0;

	std::string query = StringFormat("SELECT start_time, duration, never_expires FROM instance_list WHERE id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		is_perma = false;
		return 0;
	}

	if (results.RowCount() == 0)
	{
		is_perma = false;
		return 0;
	}

	auto row = results.begin();

	start_time = atoi(row[0]);
	duration = atoi(row[1]);
	never_expires = atoi(row[2]);

	if(never_expires == 1)
	{
		is_perma = true;
		return 0;
	}

	is_perma = false;

	timeval tv;
	gettimeofday(&tv, nullptr);
	return ((start_time + duration) - tv.tv_sec);
}

bool Database::GetUnusedInstanceID(uint16 &instance_id)
{
	uint32 count = RuleI(Zone, ReservedInstances);
	uint32 max = 65535;

	std::string query = StringFormat("SELECT IFNULL(MAX(id),%u)+1 FROM instance_list  WHERE id > %u", count, count);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		instance_id = 0;
		return false;
	}

	if (results.RowCount() == 0)
	{
		instance_id = 0;
		return false;
	}

	auto row = results.begin();

	if (atoi(row[0]) <= max)
	{
		instance_id = atoi(row[0]);
		return true;
	}

	query = StringFormat("SELECT id FROM instance_list where id > %u ORDER BY id", count);
	results = QueryDatabase(query);

	if (!results.Success())
	{
		instance_id = 0;
		return false;
	}

	if (results.RowCount() == 0)
	{
		instance_id = 0;
		return false;
	}

	count++;
	for (auto row = results.begin();row != results.end();++row)
	{
		if(count < atoi(row[0]))
		{
			instance_id = count;
			return true;
		}

		if(count > max)
		{
			instance_id = 0;
			return false;
		}

		count++;
	}

	instance_id = count;
	return true;
}

//perhaps purge any expireds too
bool Database::CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration)
{

	std::string query = StringFormat("INSERT INTO instance_list (id, zone, version, start_time, duration)"
		" values(%lu, %lu, %lu, UNIX_TIMESTAMP(), %lu)", 
		(unsigned long)instance_id, (unsigned long)zone_id, (unsigned long)version, (unsigned long)duration);
	auto results = QueryDatabase(query);

	return results.Success();
}

void Database::PurgeExpiredInstances()
{

	std::string query("SELECT id FROM instance_list where (start_time+duration) <= UNIX_TIMESTAMP() and never_expires = 0");
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	if (results.RowCount() == 0)
		return;

	for (auto row = results.begin();row != results.end();++row)
		DeleteInstance(atoi(row[0]));
}

bool Database::AddClientToInstance(uint16 instance_id, uint32 char_id)
{
	std::string query = StringFormat("INSERT INTO instance_list_player(id, charid) values(%lu, %lu)", 
		(unsigned long)instance_id, (unsigned long)char_id);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool Database::RemoveClientFromInstance(uint16 instance_id, uint32 char_id)
{
	
	std::string query = StringFormat("DELETE FROM instance_list_player WHERE id=%lu AND charid=%lu",
		(unsigned long)instance_id, (unsigned long)char_id);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool Database::RemoveClientsFromInstance(uint16 instance_id)
{
	std::string query = StringFormat("DELETE FROM instance_list_player WHERE id=%lu", (unsigned long)instance_id);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool Database::CheckInstanceExists(uint16 instance_id)
{

	std::string query = StringFormat("SELECT * FROM instance_list where id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	return true;
}

void Database::BuryCorpsesInInstance(uint16 instance_id)
{

	std::string query = StringFormat("UPDATE player_corpses SET IsBurried=1, instanceid=0 WHERE instanceid=%u", instance_id);
	auto results = QueryDatabase(query);
}

uint16 Database::GetInstanceVersion(uint16 instance_id)
{
	if(instance_id == 0)
		return 0;

	std::string query = StringFormat("SELECT version FROM instance_list where id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint16 Database::GetInstanceID(const char* zone, uint32 charid, int16 version)
{

	std::string query = StringFormat("SELECT instance_list.id FROM instance_list, instance_list_player "
		"WHERE instance_list.zone=%u AND instance_list.version=%u AND instance_list.id=instance_list_player.id AND "
		"instance_list_player.charid=%u LIMIT 1;", GetZoneID(zone), version, charid, charid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint16 Database::GetInstanceID(uint32 zone, uint32 charid, int16 version)
{
	if(!zone)
		return 0;

	std::string query = StringFormat("SELECT instance_list.id FROM instance_list, instance_list_player "
		"WHERE instance_list.zone=%u AND instance_list.version=%u AND instance_list.id=instance_list_player.id AND "
		"instance_list_player.charid=%u LIMIT 1;", zone, version, charid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

void Database::GetCharactersInInstance(uint16 instance_id, std::list<uint32> &charid_list) {

	std::string query = StringFormat("SELECT charid FROM instance_list_player WHERE id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		LogFile->write(EQEMuLog::Error, "Error in GetCharactersInInstace query '%s': %s", query.c_str(), results.ErrorMessage().c_str());
		return;
	}

	for(auto row=results.begin();row != results.end();++row)
		charid_list.push_back(atoi(row[0]));
}

void Database::AssignGroupToInstance(uint32 gid, uint32 instance_id)
{
	
	uint32 zone_id = ZoneIDFromInstanceID(instance_id);
	uint16 version = VersionFromInstanceID(instance_id);

	std::string query = StringFormat("SELECT charid FROM group_id WHERE groupid=%u", gid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row=results.begin();row != results.end();++row)
	{
		uint32 charid = atoi(row[0]);
		if(GetInstanceID(zone_id, charid, version) == 0)
			AddClientToInstance(instance_id, charid);
	}
}

void Database::AssignRaidToInstance(uint32 rid, uint32 instance_id)
{
	
	uint32 zone_id = ZoneIDFromInstanceID(instance_id);
	uint16 version = VersionFromInstanceID(instance_id);

	std::string query = StringFormat("SELECT charid FROM raid_members WHERE raidid=%u", rid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row=results.begin();row!=results.end();++row)
	{
		uint32 charid = atoi(row[0]);
		if(GetInstanceID(zone_id, charid, version) == 0)
			AddClientToInstance(instance_id, charid);
	}
}

void Database::FlagInstanceByGroupLeader(uint32 zone, int16 version, uint32 charid, uint32 gid)
{
	uint16 id = GetInstanceID(zone, charid, version);
	if(id != 0)
		return;

	char ln[128];
	memset(ln, 0, 128);
	strcpy(ln, GetGroupLeadershipInfo(gid, ln));
	uint32 l_charid = GetCharacterID((const char*)ln);
	uint16 l_id = GetInstanceID(zone, l_charid, version);

	if(l_id == 0)
		return;

	AddClientToInstance(l_id, charid);
}

void Database::FlagInstanceByRaidLeader(uint32 zone, int16 version, uint32 charid, uint32 rid)
{
	uint16 id = GetInstanceID(zone, charid, version);
	if(id != 0)
		return;

	uint32 l_charid = GetCharacterID(GetRaidLeaderName(rid));
	uint16 l_id = GetInstanceID(zone, l_charid, version);

	if(l_id == 0)
		return;

	AddClientToInstance(l_id, charid);
}

void Database::SetInstanceDuration(uint16 instance_id, uint32 new_duration)
{

	std::string query = StringFormat("UPDATE `instance_list` SET start_time=UNIX_TIMESTAMP(), "
		"duration=%u WHERE id=%u", new_duration, instance_id);
	auto results = QueryDatabase(query);
}

bool Database::GlobalInstance(uint16 instance_id)
{
	
	std::string query = StringFormat("SELECT is_global from instance_list where id=%u LIMIT 1", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	auto row = results.begin();

	return (atoi(row[0]) == 1) ? true : false;
}

void Database::UpdateAdventureStatsEntry(uint32 char_id, uint8 theme, bool win)
{
	
	std::string field;

	switch(theme)
	{
		case 1:
		{
			field = "guk_";
			break;
		}
		case 2:
		{
			field = "mir_";
			break;
		}
		case 3:
		{
			field = "mmc_";
			break;
		}
		case 4:
		{
			field = "ruj_";
			break;
		}
		case 5:
		{
			field = "tak_";
			break;
		}
		default:
		{
			return;
		}
	}

	if (win)
		field += "wins";
	else
		field += "losses";

	std::string query = StringFormat("UPDATE `adventure_stats` SET %s=%s+1 WHERE player_id=%u",field.c_str(), field.c_str(), char_id);
	auto results = QueryDatabase(query);

	if (results.RowsAffected() != 0)
		return;

	query = StringFormat("INSERT INTO `adventure_stats` SET %s=1, player_id=%u", field.c_str(), char_id);
	QueryDatabase(query);
}

bool Database::GetAdventureStats(uint32 char_id, uint32 &guk_w, uint32 &mir_w, uint32 &mmc_w, uint32 &ruj_w,
								uint32 &tak_w, uint32 &guk_l, uint32 &mir_l, uint32 &mmc_l, uint32 &ruj_l, uint32 &tak_l)
{

	std::string query = StringFormat("SELECT `guk_wins`, `mir_wins`, `mmc_wins`, `ruj_wins`, `tak_wins`, `guk_losses`, "
		"`mir_losses`, `mmc_losses`, `ruj_losses`, `tak_losses` FROM `adventure_stats` WHERE player_id=%u", char_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	auto row = results.begin();

	guk_w = atoi(row[0]);
	mir_w = atoi(row[1]);
	mmc_w = atoi(row[2]);
	ruj_w = atoi(row[3]);
	tak_w = atoi(row[4]);
	guk_l = atoi(row[5]);
	mir_l = atoi(row[6]);
	mmc_l = atoi(row[7]);
	ruj_l = atoi(row[8]);
	tak_l = atoi(row[9]);

	return true;
}

uint32 Database::GetGuildIDByCharID(uint32 char_id) 
{

	std::string query = StringFormat("SELECT guild_id FROM guild_members WHERE char_id='%i'", char_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetGuildIDByChar query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}
