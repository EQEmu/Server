/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/global_define.h"
#include "../common/rulesys.h"

#include <ctype.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <algorithm>
#include <mysqld_error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include "extprofile.h"
#include "strings.h"
#include "database_schema.h"
#include "http/httplib.h"
#include "http/uri.h"

#include "repositories/zone_repository.h"
#include "zone_store.h"

extern Client client;

Database::Database () {
}

/*
Establish a connection to a mysql database with the supplied parameters
*/

Database::Database(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
{
	Connect(host, user, passwd, database, port);
}

bool Database::Connect(const char* host, const char* user, const char* passwd, const char* database, uint32 port, std::string connection_label) {
	uint32 errnum= 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(host, user, passwd, database, port, &errnum, errbuf)) {
		LogError("Connection [{}] Failed to connect to database Error [{}]", connection_label, errbuf);
		return false;
	}
	else {
		LogInfo("Connected to database [{}] [{}] @ [{}:{}]", connection_label, database, host,port);
		return true;
	}
}

/*
	Close the connection to the database
*/

Database::~Database()
{
}

/*
	Check if there is an account with name "name" and password "password"
	Return the account id or zero if no account matches.
	Zero will also be returned if there is a database error.
*/
uint32 Database::CheckLogin(const char* name, const char* password, const char *loginserver, int16* oStatus) {

	if (strlen(name) >= 50 || strlen(password) >= 50)
		return(0);

	char temporary_username[100];
	char temporary_password[100];

	DoEscapeString(temporary_username, name, strlen(name));
	DoEscapeString(temporary_password, password, strlen(password));

	std::string query = fmt::format(
		"SELECT id, status FROM account WHERE `name` = '{}' AND ls_id = '{}' AND password is NOT NULL "
		"AND length(password) > 0 AND (password = '{}' OR password = MD5('{}'))",
		temporary_username,
		Strings::Escape(loginserver),
		temporary_password,
		temporary_password
	);

	auto results = QueryDatabase(query);

	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto row = results.begin();

	auto id = std::stoul(row[0]);

	if (oStatus) {
		*oStatus = std::stoi(row[1]);
	}

	return id;
}

//Get Banned IP Address List - Only return false if the incoming connection's IP address is not present in the banned_ips table.
bool Database::CheckBannedIPs(std::string login_ip)
{
	auto query = fmt::format(
		"SELECT ip_address FROM banned_ips WHERE ip_address = '{}'",
		login_ip
	);
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() != 0) {
		return true;
	}

	return false;
}

bool Database::AddBannedIP(std::string banned_ip, std::string notes) {
	auto query = fmt::format(
		"INSERT into banned_ips SET ip_address = '{}', notes = '{}'",
		Strings::Escape(banned_ip),
		Strings::Escape(notes)
	);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	return true;
}

 bool Database::CheckGMIPs(std::string login_ip, uint32 account_id) {
	auto query = fmt::format(
		"SELECT * FROM `gm_ips` WHERE `ip_address` = '{}' AND `account_id` = {}",
		login_ip,
		account_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	if (results.RowCount() == 1) {
		return true;
	}

	return false;
}

void Database::LoginIP(uint32 account_id, std::string login_ip) {
	auto query = fmt::format(
		"INSERT INTO account_ip SET accid = {}, ip = '{}' ON DUPLICATE KEY UPDATE count=count+1, lastused=now()",
		account_id,
		login_ip
	);
	QueryDatabase(query);
}

int16 Database::CheckStatus(uint32 account_id)
{
	auto query = fmt::format(
	    "SELECT `status`, TIMESTAMPDIFF(SECOND, NOW(), `suspendeduntil`) FROM `account` WHERE `id` = {}",
	    account_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() != 1) {
		return 0;
	}

	auto row = results.begin();
	int16 status = std::stoi(row[0]);
	int32 date_diff = 0;

	if (row[1]) {
		date_diff = std::stoi(row[1]);
	}

	if (date_diff > 0) {
		return -1;
	}

	return status;
}

/**
 * @param name
 * @param password
 * @param status
 * @param loginserver
 * @param lsaccount_id
 * @return
 */
uint32 Database::CreateAccount(
	const char *name,
	const char *password,
	int16 status,
	const char *loginserver,
	uint32 lsaccount_id
)
{
	std::string query;

	if (password) {
		query = StringFormat(
			"INSERT INTO account SET name='%s', password='%s', status=%i, ls_id='%s', lsaccount_id=%i, time_creation=UNIX_TIMESTAMP();",
			name,
			password,
			status,
			loginserver,
			lsaccount_id
		);
	}
	else {
		query = StringFormat(
			"INSERT INTO account SET name='%s', status=%i, ls_id='%s', lsaccount_id=%i, time_creation=UNIX_TIMESTAMP();",
			name,
			status,
			loginserver,
			lsaccount_id
		);
	}

	LogInfo("Account Attempting to be created: [{0}:{1}] status: {2}", loginserver, name, status);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return 0;
	}

	if (results.LastInsertedID() == 0) {
		return 0;
	}

	return results.LastInsertedID();
}

bool Database::DeleteAccount(const char* name, const char *loginserver) {
	std::string query = StringFormat("DELETE FROM account WHERE `name`='%s' AND ls_id='%s'", name, loginserver);
	LogInfo("Account Attempting to be deleted:'[{}]:[{}]'", loginserver, name);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return results.RowsAffected() == 1;
}

bool Database::SetLocalPassword(uint32 accid, const char* password) {
	std::string query = StringFormat("UPDATE account SET password=MD5('%s') where id=%i;", Strings::Escape(password).c_str(), accid);

	auto results = QueryDatabase(query);

	if (!results.Success()) {
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

/**
 * @param account_name
 * @param status
 * @return
 */
bool Database::SetAccountStatus(const std::string& account_name, int16 status)
{
	LogInfo("Account [{}] is attempting to be set to status [{}]", account_name, status);

	std::string query = fmt::format(
		"UPDATE account SET status = {} WHERE name = '{}'",
		status,
		account_name
	);

	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	if (results.RowsAffected() == 0) {
		LogWarning("Account [{}] does not exist!", account_name);
		return false;
	}

	return true;
}

/* This initially creates the character during character create */
bool Database::ReserveName(uint32 account_id, char* name) {
	std::string query = StringFormat("SELECT `account_id`, `name` FROM `character_data` WHERE `name` = '%s'", name);
	auto results = QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (row[0] && atoi(row[0]) > 0){
			LogInfo("Account: [{}] tried to request name: [{}], but it is already taken", account_id, name);
			return false;
		}
	}

	query = StringFormat("INSERT INTO `character_data` SET `account_id` = %i, `name` = '%s'", account_id, name);
	results = QueryDatabase(query);
	if (!results.Success() || results.ErrorMessage() != ""){ return false; }

	// Put character into the default guild if rule is being used.
	int guild_id = RuleI(Character, DefaultGuild);

	if (guild_id != 0) {
		int character_id=results.LastInsertedID();
		if (character_id > -1) {
			query = StringFormat("INSERT INTO `guild_members` SET `char_id` = %i, `guild_id` = '%i'", character_id, guild_id);
			results = QueryDatabase(query);
			if (!results.Success() || results.ErrorMessage() != ""){
				LogInfo("Could not put character [{}] into default Guild", name);
			}
		}
	}

	return true;
}

/**
 * @param character_name
 * @return
 */
bool Database::DeleteCharacter(char *character_name)
{
	uint32 character_id = 0;
	if (!character_name || !strlen(character_name)) {
		LogInfo("DeleteCharacter: request to delete without a name (empty char slot)");
		return false;
	}

	std::string query   = StringFormat("SELECT `id` from `character_data` WHERE `name` = '%s'", character_name);
	auto        results = QueryDatabase(query);
	for (auto   row     = results.begin(); row != results.end(); ++row) {
		character_id = atoi(row[0]);
	}

	if (character_id <= 0) {
		LogError("Invalid Character ID [{}]", character_name);
		return false;
	}

	std::string delete_type = "hard-deleted";
	if (RuleB(Character, SoftDeletes)) {
		delete_type = "soft-deleted";
		query       = fmt::format(
			SQL(
				UPDATE
					character_data
				SET
				name       = SUBSTRING(CONCAT(name, '-deleted-', UNIX_TIMESTAMP()), 1, 64),
				deleted_at = NOW()
					WHERE
					id     = '{}'
			),
			character_id
		);

		QueryDatabase(query);

		if (RuleB(Bots, Enabled)) {
			query = fmt::format(
				SQL(
					UPDATE
					bot_data
						SET
					name = SUBSTRING(CONCAT(name, '-deleted-', UNIX_TIMESTAMP()), 1, 64)
					WHERE
					owner_id = '{}'
				),
				character_id
			);
			QueryDatabase(query);
			LogInfo(
				"[DeleteCharacter] character_name [{}] ({}) bots are being [{}]",
				character_name,
				character_id,
				delete_type
			);
		}

		return true;
	}

	for (const auto &iter: DatabaseSchema::GetCharacterTables()) {
		std::string table_name               = iter.first;
		std::string character_id_column_name = iter.second;

		QueryDatabase(fmt::format("DELETE FROM {} WHERE {} = {}", table_name, character_id_column_name, character_id));
	}

	LogInfo("character_name [{}] ({}) is being [{}]", character_name, character_id, delete_type);

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
		Strings::Escape(pp->name).c_str(),	  // " `name`,                    "
		Strings::Escape(pp->last_name).c_str(), // " last_name,              "
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
		Strings::Escape(pp->title).c_str(),  // " title,                     "
		Strings::Escape(pp->suffix).c_str(), // " suffix,                    "
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

	/* Save Bind Points */
	query = StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, slot)"
		" VALUES (%u, %u, %u, %f, %f, %f, %f, %i), "
		"(%u, %u, %u, %f, %f, %f, %f, %i), "
		"(%u, %u, %u, %f, %f, %f, %f, %i), "
		"(%u, %u, %u, %f, %f, %f, %f, %i), "
		"(%u, %u, %u, %f, %f, %f, %f, %i)",
		character_id, pp->binds[0].zone_id, 0, pp->binds[0].x, pp->binds[0].y, pp->binds[0].z, pp->binds[0].heading, 0,
		character_id, pp->binds[1].zone_id, 0, pp->binds[1].x, pp->binds[1].y, pp->binds[1].z, pp->binds[1].heading, 1,
		character_id, pp->binds[2].zone_id, 0, pp->binds[2].x, pp->binds[2].y, pp->binds[2].z, pp->binds[2].heading, 2,
		character_id, pp->binds[3].zone_id, 0, pp->binds[3].x, pp->binds[3].y, pp->binds[3].z, pp->binds[3].heading, 3,
		character_id, pp->binds[4].zone_id, 0, pp->binds[4].x, pp->binds[4].y, pp->binds[4].z, pp->binds[4].heading, 4
	); results = QueryDatabase(query);

        /* HoTT Ability */
        if(RuleB(Character, GrantHoTTOnCreate))
        {
                query = StringFormat("INSERT INTO `character_leadership_abilities` (id, slot, `rank`) VALUES (%u, %i, %i)", character_id, 14, 1);
                results = QueryDatabase(query);
        }

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
	results = QueryDatabase(query);

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
	results = QueryDatabase(query);

	return true;
}

uint32 Database::GetCharacterID(const char *name) {
	std::string query = StringFormat("SELECT `id` FROM `character_data` WHERE `name` = '%s'", name);
	auto results = QueryDatabase(query);
	auto row = results.begin();
	if (results.RowCount() == 1)
	{
		return atoi(row[0]);
	}
	return 0;
}

/*
	This function returns the account_id that owns the character with
	the name "name" or zero if no character with that name was found
	Zero will also be returned if there is a database error.
*/
uint32 Database::GetAccountIDByChar(const char* charname, uint32* oCharID) {
	std::string query = StringFormat("SELECT `account_id`, `id` FROM `character_data` WHERE name='%s'", Strings::Escape(charname).c_str());

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
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
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint32 Database::GetAccountIDByName(std::string account_name, std::string loginserver, int16* status, uint32* lsid) {
	if (!isAlphaNumeric(account_name.c_str())) {
		return 0;
	}

	auto query = fmt::format(
		"SELECT `id`, `status`, `lsaccount_id` FROM `account` WHERE `name` = '{}' AND `ls_id` = '{}' LIMIT 1",
		Strings::Escape(account_name),
		Strings::Escape(loginserver)
	);
	auto results = QueryDatabase(query);

	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto row = results.begin();
	auto account_id = std::stoul(row[0]);

	if (status) {
		*status = static_cast<int16>(std::stoi(row[1]));
	}

	if (lsid) {
		*lsid = row[2] ? std::stoul(row[2]) : 0;
	}

	return account_id;
}

void Database::GetAccountName(uint32 accountid, char* name, uint32* oLSAccountID) {
	std::string query = StringFormat("SELECT `name`, `lsaccount_id` FROM `account` WHERE `id` = '%i'", accountid);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
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
		return;
	}

	auto row = results.begin();
	for (auto row = results.begin(); row != results.end(); ++row) {
		strcpy(name, row[0]);
	}
}

std::string Database::GetCharNameByID(uint32 char_id) {
	std::string query = fmt::format("SELECT `name` FROM `character_data` WHERE id = {}", char_id);
	auto results = QueryDatabase(query);
	std::string res;

	if (!results.Success()) {
		return res;
	}

	if (results.RowCount() == 0) {
		return res;
	}

	auto row = results.begin();
	res = row[0];
	return res;
}

std::string Database::GetNPCNameByID(uint32 npc_id) {
	std::string query = fmt::format("SELECT `name` FROM `npc_types` WHERE id = {}", npc_id);
	auto results = QueryDatabase(query);
	std::string res;

	if (!results.Success()) {
		return res;
	}

	if (results.RowCount() == 0) {
		return res;
	}

	auto row = results.begin();
	res = row[0];
	return res;
}

std::string Database::GetCleanNPCNameByID(uint32 npc_id) {
	std::string query = fmt::format("SELECT `name` FROM `npc_types` WHERE id = {}", npc_id);
	auto results = QueryDatabase(query);
	std::string res;
	std::string mob_name;

	if (!results.Success()) {
		return res;
	}

	if (results.RowCount() == 0) {
		return res;
	}

	auto row = results.begin();
	mob_name = row[0];
	CleanMobName(mob_name.begin(), mob_name.end(), std::back_inserter(res));
	return res;
}

bool Database::LoadVariables() {
	auto results = QueryDatabase(StringFormat("SELECT varname, value, unix_timestamp() FROM variables where unix_timestamp(ts) >= %d", varcache.last_update));

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return true;

	LockMutex lock(&Mvarcache);

	std::string key, value;
	for (auto row = results.begin(); row != results.end(); ++row) {
		varcache.last_update = atoi(row[2]); // ahh should we be comparing if this is newer?
		key = row[0];
		value = row[1];
		std::transform(std::begin(key), std::end(key), std::begin(key), ::tolower); // keys are lower case, DB doesn't have to be
		varcache.Add(key, value);
	}

	LogInfo("Loaded [{}] variable(s)", Strings::Commify(std::to_string(results.RowCount())));

	return true;
}

// Gets variable from 'variables' table
bool Database::GetVariable(std::string varname, std::string &varvalue)
{
	varvalue.clear();

	LockMutex lock(&Mvarcache);

	if (varname.empty())
		return false;

	std::transform(std::begin(varname), std::end(varname), std::begin(varname), ::tolower); // all keys are lower case
	auto tmp = varcache.Get(varname);
	if (tmp) {
		varvalue = *tmp;
		return true;
	}
	return false;
}

bool Database::SetVariable(const std::string varname, const std::string &varvalue)
{
	std::string escaped_name = Strings::Escape(varname);
	std::string escaped_value = Strings::Escape(varvalue);
	std::string query = StringFormat("Update variables set value='%s' WHERE varname like '%s'", escaped_value.c_str(), escaped_name.c_str());
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowsAffected() == 1)
	{
		LoadVariables(); // refresh cache
		return true;
	}

	query = StringFormat("Insert Into variables (varname, value) values ('%s', '%s')", escaped_name.c_str(), escaped_value.c_str());
	results = QueryDatabase(query);

	if (results.RowsAffected() != 1)
		return false;

	LoadVariables(); // refresh cache
	return true;
}

void Database::SetAccountCRCField(uint32 account_id, std::string field_name, uint64 checksum)
{
	QueryDatabase(
		fmt::format(
			"UPDATE `account` SET `{}` = '{}' WHERE `id` = {}",
			field_name,
			checksum,
			account_id
		)
	);
}

bool Database::GetZoneGraveyard(const uint32 graveyard_id, uint32* graveyard_zoneid, float* graveyard_x, float* graveyard_y, float* graveyard_z, float* graveyard_heading) {

	std::string query = StringFormat("SELECT zone_id, x, y, z, heading FROM graveyard WHERE id=%i", graveyard_id);
	auto results = QueryDatabase(query);

	if (!results.Success()){
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

uint8 Database::GetPEQZone(uint32 zone_id, uint32 version){

	auto z = GetZoneVersionWithFallback(zone_id, version);

	return z ? z->peqzone : 0;
}

bool Database::CheckNameFilter(std::string name, bool surname)
{
	name = Strings::ToLower(name);

	// the minimum 4 is enforced by the client too
	if (name.empty() || name.size() < 4) {
		return false;
	}

	// Given name length is enforced by the client too
	if (!surname && name.size() > 15) {
		return false;
	}

	for (size_t i = 0; i < name.size(); i++) {
		if (!isalpha(name[i])) {
			return false;
		}
	}

	char c = '\0';
	uint8 num_c = 0;
	for (size_t x = 0; x < name.size(); ++x) {
		if (name[x] == c) {
			num_c++;
		} else {
			num_c = 1;
			c = name[x];
		}

		if (num_c > 2) {
			return false;
		}
	}

	std::string query = "SELECT name FROM name_filter";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return true;
	}

	for (auto row : results) {
		std::string current_row = Strings::ToLower(row[0]);
		if (name.find(current_row) != std::string::npos) {
			return false;
		}
	}

	return true;
}

bool Database::AddToNameFilter(std::string name) {
	auto query = fmt::format(
		"INSERT INTO name_filter (name) values ('{}')",
		name
	);
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowsAffected()) {
		return false;
	}

	return true;
}

/**
 * @param in_loginserver_id
 * @param in_loginserver_account_id
 * @param in_account_name
 * @param in_status
 * @return
 */
uint32 Database::GetAccountIDFromLSID(
	const std::string &in_loginserver_id,
	uint32 in_loginserver_account_id,
	char *in_account_name,
	int16 *in_status
)
{
	uint32 account_id = 0;
	auto query = fmt::format(
		"SELECT id, `name`, status FROM account WHERE lsaccount_id = {0} AND ls_id = '{1}'",
		in_loginserver_account_id,
		in_loginserver_id
	);

	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return 0;
	}

	if (results.RowCount() != 1) {
		return 0;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		account_id = std::stoi(row[0]);

		if (in_account_name) {
			strcpy(in_account_name, row[1]);
		}
		if (in_status) {
			*in_status = std::stoi(row[2]);
		}
	}

	return account_id;
}

void Database::GetAccountFromID(uint32 id, char* oAccountName, int16* oStatus) {

	std::string query = StringFormat("SELECT `name`, status FROM account WHERE id=%i", id);
	auto results = QueryDatabase(query);

	if (!results.Success()){
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
	QueryDatabase("DELETE FROM merchantlist_temp");
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
bool Database::CheckUsedName(std::string name) {
	auto query = fmt::format(
		"SELECT `id` FROM `character_data` WHERE `name` = '{}'",
		name
	);
	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount()) {
		return false;
	}

	return true;
}

uint8 Database::GetServerType() {
	std::string query("SELECT `value` FROM `variables` WHERE `varname` = 'ServerType' LIMIT 1");
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

bool Database::MoveCharacterToZone(uint32 character_id, uint32 zone_id)
{
	std::string query = StringFormat(
		"UPDATE `character_data` SET `zone_id` = %i, `x` = -1, `y` = -1, `z` = -1 WHERE `id` = %i",
		zone_id,
		character_id
	);

	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	return results.RowsAffected() != 0;
}

bool Database::MoveCharacterToZone(const char *charname, uint32 zone_id)
{
	std::string query = StringFormat(
		"UPDATE `character_data` SET `zone_id` = %i, `x` = -1, `y` = -1, `z` = -1 WHERE `name` = '%s'",
		zone_id,
		charname
	);

	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	return results.RowsAffected() != 0;
}

bool Database::SetHackerFlag(const char* accountname, const char* charactername, const char* hacked) {
	std::string query = StringFormat("INSERT INTO `hackers` (account, name, hacked) values('%s','%s','%s')", accountname, charactername, hacked);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
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
		return false;
	}

	return results.RowsAffected() != 0;
}

bool Database::SetMQDetectionFlag(const char* accountname, const char* charactername, const std::string &hacked, const char* zone) {
	//Utilize the "hacker" table, but also give zone information.
	auto query = fmt::format("INSERT INTO hackers(account, name, hacked, zone) values('{}', '{}', '{}', '{}')",
				 accountname, charactername, hacked, zone);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
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

uint32 Database::GetCharacterInfo(std::string character_name, uint32 *account_id, uint32 *zone_id, uint32 *instance_id)
{
	auto query = fmt::format(
		"SELECT `id`, `account_id`, `zone_id`, `zone_instance` FROM `character_data` WHERE `name` = '{}'",
		Strings::Escape(character_name)
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto row = results.begin();
	auto character_id = std::stoul(row[0]);
	*account_id = std::stoul(row[1]);
	*zone_id = std::stoul(row[2]);
	*instance_id = std::stoul(row[3]);

	return character_id;
}

bool Database::UpdateLiveChar(char* charname, uint32 account_id) {

	std::string query = StringFormat("UPDATE account SET charname='%s' WHERE id=%i;", charname, account_id);
	auto results = QueryDatabase(query);

	if (!results.Success()){
		return false;
	}

	return true;
}

bool Database::GetLiveChar(uint32 account_id, char* cname) {

	std::string query = StringFormat("SELECT charname FROM account WHERE id=%i", account_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
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
	QueryDatabase(query);
}

void Database::SetLoginFlags(uint32 CharID, bool LFP, bool LFG, uint8 firstlogon) {
	std::string query = StringFormat("update `character_data` SET `lfp` = %i, `lfg` = %i, `firstlogon` = %i WHERE `id` = %i",LFP, LFG, firstlogon, CharID);
	QueryDatabase(query);
}

void Database::SetLFG(uint32 CharID, bool LFG) {
	std::string query = StringFormat("update `character_data` SET `lfg` = %i WHERE `id` = %i",LFG, CharID);
	QueryDatabase(query);
}

void Database::SetFirstLogon(uint32 CharID, uint8 firstlogon) {
	std::string query = StringFormat( "UPDATE `character_data` SET `firstlogon` = %i WHERE `id` = %i",firstlogon, CharID);
	QueryDatabase(query);
}

void Database::AddReport(std::string who, std::string against, std::string lines)
{
	auto escape_str = new char[lines.size() * 2 + 1];
	DoEscapeString(escape_str, lines.c_str(), lines.size());

	std::string query = StringFormat("INSERT INTO reports (name, reported, reported_text) VALUES('%s', '%s', '%s')", Strings::Escape(who).c_str(), Strings::Escape(against).c_str(), escape_str);
	QueryDatabase(query);
	safe_delete_array(escape_str);
}

void Database::SetGroupID(const char* name, uint32 id, uint32 charid, uint32 ismerc) {
	std::string query;
	if (id == 0) {
		// removing from group
		query = StringFormat("delete from group_id where charid=%i and name='%s' and ismerc=%i",charid, name, ismerc);
		auto results = QueryDatabase(query);

		if (!results.Success())
			LogError("Error deleting character from group id: {}", results.ErrorMessage().c_str());

		return;
	}

	/* Add to the Group */
	query = StringFormat("REPLACE INTO  `group_id` SET `charid` = %i, `groupid` = %i, `name` = '%s', `ismerc` = '%i'", charid, id, name, ismerc);
	QueryDatabase(query);
}

void Database::ClearAllGroups(void)
{
	std::string query("DELETE FROM `group_id`");
	QueryDatabase(query);
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
	QueryDatabase(query);
}

uint32 Database::GetGroupID(const char* name){
	std::string query = StringFormat("SELECT groupid from group_id where name='%s'", name);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return 0;
	}

	if (results.RowCount() == 0)
	{
		// Commenting this out until logging levels can prevent this from going to console
		//LogDebug(, "Character not in a group: [{}]", name);
		return 0;
	}

	auto row = results.begin();

	return atoi(row[0]);
}

std::string Database::GetGroupLeaderForLogin(std::string character_name) {
	uint32 group_id = 0;

	auto query = fmt::format(
		"SELECT `groupid` FROM `group_id` WHERE `name` = '{}'",
		character_name
	);
	auto results = QueryDatabase(query);

	if (results.Success() && results.RowCount()) {
		auto row = results.begin();
		group_id = std::stoul(row[0]);
	}

	if (!group_id) {
		return std::string();
	}

	query = fmt::format(
		"SELECT `leadername` FROM `group_leaders` WHERE `gid` = {} LIMIT 1",
		group_id
	);
	results = QueryDatabase(query);

	if (results.Success() && results.RowCount()) {
		auto row = results.begin();
		return row[0];
	}

	return std::string();
}

void Database::SetGroupLeaderName(uint32 gid, const char* name) {
	std::string query = StringFormat("UPDATE group_leaders SET leadername = '%s' WHERE gid = %u", Strings::Escape(name).c_str(), gid);
	auto result = QueryDatabase(query);

	if(result.RowsAffected() != 0) {
		return;
	}

	query = StringFormat("REPLACE INTO group_leaders(gid, leadername, marknpc, leadershipaa, maintank, assist, puller, mentoree, mentor_percent) VALUES(%u, '%s', '', '', '', '', '', '', '0')",
						 gid, Strings::Escape(name).c_str());
	result = QueryDatabase(query);

	if(!result.Success()) {
		LogDebug("Error in Database::SetGroupLeaderName: [{}]", result.ErrorMessage().c_str());
	}
}

char *Database::GetGroupLeadershipInfo(uint32 gid, char* leaderbuf, char* maintank, char* assist, char* puller, char *marknpc, char *mentoree, int *mentor_percent, GroupLeadershipAA_Struct* GLAA)
{
	std::string query = StringFormat("SELECT `leadername`, `maintank`, `assist`, `puller`, `marknpc`, `mentoree`, `mentor_percent`, `leadershipaa` FROM `group_leaders` WHERE `gid` = %lu",(unsigned long)gid);
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

		if (mentoree)
			mentoree[0] = '\0';

		if (mentor_percent)
			*mentor_percent = 0;

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

	if (mentoree)
		strcpy(mentoree, row[5]);

	if (mentor_percent)
		*mentor_percent = atoi(row[6]);

	if(GLAA && results.LengthOfColumn(7) == sizeof(GroupLeadershipAA_Struct))
		memcpy(GLAA, row[7], sizeof(GroupLeadershipAA_Struct));

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

void Database::PurgeAllDeletedDataBuckets() {
	std::string query = StringFormat(
			"DELETE FROM `data_buckets` WHERE (`expires` < %lld AND `expires` > 0)",
			(long long) std::time(nullptr)
	);

	QueryDatabase(query);
}

// returns 0 on error or no raid for that character, or
// the raid id that the character is a member of.
uint32 Database::GetRaidID(const char* name)
{
	std::string query = StringFormat("SELECT `raidid` FROM `raid_members` WHERE `name` = '%s'", name);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return 0;
	}

	auto row = results.begin();
	if (row == results.end()) {
		return 0;
	}

	if (row[0]) // would it ever be possible to have a null here?
		return atoi(row[0]);

	return 0;
}

const char* Database::GetRaidLeaderName(uint32 raid_id)
{
	// Would be a good idea to fix this to be a passed in variable and
	// make the caller responsible. static local variables like this are
	// not guaranteed to be thread safe (nor is the internal guard
	// variable). C++0x standard states this should be thread safe
	// but may not be fully supported in some compilers.
	static char name[128];

	std::string query = StringFormat("SELECT `name` FROM `raid_members` WHERE `raidid` = %u AND `israidleader` = 1", raid_id);
	auto results = QueryDatabase(query);

	if (!results.Success()) {
		LogDebug("Unable to get Raid Leader Name for Raid ID: [{}]", raid_id);
		return "UNKNOWN";
	}

	auto row = results.begin();

	if (row == results.end()) {
		return "UNKNOWN";
	}

	memset(name, 0, sizeof(name));
	strcpy(name, row[0]);

	return name;
}

// maintank, assist, puller, marknpc currently unused
void Database::GetGroupLeadershipInfo(uint32 gid, uint32 rid, char *maintank,
		char *assist, char *puller, char *marknpc, char *mentoree, int *mentor_percent, GroupLeadershipAA_Struct *GLAA)
{
	std::string query = StringFormat(
			"SELECT maintank, assist, puller, marknpc, mentoree, mentor_percent, leadershipaa FROM raid_leaders WHERE gid = %lu AND rid = %lu",
			(unsigned long)gid, (unsigned long)rid);
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() == 0) {
		if (maintank)
			maintank[0] = '\0';

		if (assist)
			assist[0] = '\0';

		if (puller)
			puller[0] = '\0';

		if (marknpc)
			marknpc[0] = '\0';

		if (mentoree)
			mentoree[0] = '\0';

		if (mentor_percent)
			*mentor_percent = 0;

		return;
	}

	auto row = results.begin();

	if (maintank)
		strcpy(maintank, row[0]);

	if (assist)
		strcpy(assist, row[1]);

	if (puller)
		strcpy(puller, row[2]);

	if (marknpc)
		strcpy(marknpc, row[3]);

	if (mentoree)
		strcpy(mentoree, row[4]);

	if (mentor_percent)
		*mentor_percent = atoi(row[5]);

	if (GLAA && results.LengthOfColumn(6) == sizeof(GroupLeadershipAA_Struct))
		memcpy(GLAA, row[6], sizeof(GroupLeadershipAA_Struct));

	return;
}

// maintank, assist, puller, marknpc currently unused
void Database::GetRaidLeadershipInfo(uint32 rid, char *maintank,
		char *assist, char *puller, char *marknpc, RaidLeadershipAA_Struct *RLAA)
{
	std::string query = StringFormat(
			"SELECT maintank, assist, puller, marknpc, leadershipaa FROM raid_leaders WHERE gid = %lu AND rid = %lu",
			(unsigned long)0xFFFFFFFF, (unsigned long)rid);
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() == 0) {
		if (maintank)
			maintank[0] = '\0';

		if (assist)
			assist[0] = '\0';

		if (puller)
			puller[0] = '\0';

		if (marknpc)
			marknpc[0] = '\0';

		return;
	}

	auto row = results.begin();

	if (maintank)
		strcpy(maintank, row[0]);

	if (assist)
		strcpy(assist, row[1]);

	if (puller)
		strcpy(puller, row[2]);

	if (marknpc)
		strcpy(marknpc, row[3]);

	if (RLAA && results.LengthOfColumn(4) == sizeof(RaidLeadershipAA_Struct))
		memcpy(RLAA, row[4], sizeof(RaidLeadershipAA_Struct));

	return;
}

void Database::SetRaidGroupLeaderInfo(uint32 gid, uint32 rid)
{
	std::string query = StringFormat("UPDATE raid_leaders SET leadershipaa = '' WHERE gid = %lu AND rid = %lu",
			(unsigned long)gid, (unsigned long)rid);
	auto results = QueryDatabase(query);

	if (results.RowsAffected() != 0)
		return;

	query = StringFormat("REPLACE INTO raid_leaders(gid, rid, marknpc, leadershipaa, maintank, assist, puller, mentoree, mentor_percent) VALUES(%lu, %lu, '', '', '', '', '', '', 0)",
			(unsigned long)gid, (unsigned long)rid);
	results = QueryDatabase(query);

	return;
}

// Clearing all raid leaders
void Database::ClearAllRaidLeaders(void)
{
	std::string query("DELETE from raid_leaders");
	QueryDatabase(query);
	return;
}

void Database::ClearRaidLeader(uint32 gid, uint32 rid)
{
	if (rid == 0) {
		ClearAllRaidLeaders();
		return;
	}

	std::string query = StringFormat("DELETE from raid_leaders where gid = %lu and rid = %lu", (unsigned long)gid, (unsigned long)rid);
	QueryDatabase(query);
}

void Database::UpdateAdventureStatsEntry(uint32 char_id, uint8 theme, bool win, bool remove)
{
	std::string field;
	switch(theme) {
		case LDoNThemes::GUK: {
			field = "guk_";
			break;
		}
		case LDoNThemes::MIR: {
			field = "mir_";
			break;
		}
		case LDoNThemes::MMC: {
			field = "mmc_";
			break;
		}
		case LDoNThemes::RUJ: {
			field = "ruj_";
			break;
		}
		case LDoNThemes::TAK: {
			field = "tak_";
			break;
		}
		default: {
			return;
		}
	}

	field += win ? "wins" : "losses";
	std::string field_operation = remove ? "-" : "+";

	std::string query = fmt::format(
		"UPDATE `adventure_stats` SET {} = {} {} 1 WHERE player_id = {}",
		field,
		field,
		field_operation,
		char_id
	);
	auto results = QueryDatabase(query);

	if (results.RowsAffected() != 0) {
		return;
	}

	if (!remove) {
		query = fmt::format(
			"INSERT INTO `adventure_stats` SET {} = 1, player_id = {}",
			field,
			char_id
		);
		QueryDatabase(query);
	}
}

bool Database::GetAdventureStats(uint32 char_id, AdventureStats_Struct *as)
{
	std::string query = fmt::format(
		"SELECT "
		"`guk_wins`, "
		"`mir_wins`, "
		"`mmc_wins`, "
		"`ruj_wins`, "
		"`tak_wins`, "
		"`guk_losses`, "
		"`mir_losses`, "
		"`mmc_losses`, "
		"`ruj_losses`, "
		"`tak_losses` "
		"FROM "
		"`adventure_stats` "
		"WHERE "
		"player_id = {}",
		char_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	auto row = results.begin();

	as->success.guk = atoi(row[0]);
	as->success.mir = atoi(row[1]);
	as->success.mmc = atoi(row[2]);
	as->success.ruj = atoi(row[3]);
	as->success.tak = atoi(row[4]);
	as->failure.guk = atoi(row[5]);
	as->failure.mir = atoi(row[6]);
	as->failure.mmc = atoi(row[7]);
	as->failure.ruj = atoi(row[8]);
	as->failure.tak = atoi(row[9]);
	as->failure.total = as->failure.guk + as->failure.mir + as->failure.mmc + as->failure.ruj + as->failure.tak;
	as->success.total = as->success.guk + as->success.mir + as->success.mmc + as->success.ruj + as->success.tak;

	return true;
}

uint32 Database::GetGuildIDByCharID(uint32 character_id)
{
	std::string query = StringFormat("SELECT guild_id FROM guild_members WHERE char_id='%i'", character_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint32 Database::GetGroupIDByCharID(uint32 character_id)
{
	std::string query = fmt::format(
		SQL(
			SELECT groupid
			FROM group_id
			WHERE charid = '{}'
		),
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint32 Database::GetRaidIDByCharID(uint32 character_id) {
	std::string query = fmt::format(
		SQL(
			SELECT raidid
			FROM raid_members
			WHERE charid = '{}'
		),
		character_id
	);
	auto results = QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		return atoi(row[0]);
	}
	return 0;
}

int Database::CountInvSnapshots() {
	std::string query = StringFormat("SELECT COUNT(*) FROM (SELECT * FROM `inventory_snapshots` a GROUP BY `charid`, `time_index`) b");
	auto results = QueryDatabase(query);

	if (!results.Success())
		return -1;

	auto row = results.begin();

	int64 count = atoll(row[0]);
	if (count > 2147483647)
		return -2;
	if (count < 0)
		return -3;

	return count;
}

void Database::ClearInvSnapshots(bool from_now) {
	uint32 del_time = time(nullptr);
	if (!from_now) { del_time -= RuleI(Character, InvSnapshotHistoryD) * 86400; }

	std::string query = StringFormat("DELETE FROM inventory_snapshots WHERE time_index <= %lu", (unsigned long)del_time);
	QueryDatabase(query);
}

struct TimeOfDay_Struct Database::LoadTime(time_t &realtime)
{

	TimeOfDay_Struct eqTime;
	std::string query = StringFormat("SELECT minute,hour,day,month,year,realtime FROM eqtime limit 1");
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() == 0){
		LogInfo("Loading EQ time of day failed. Using defaults");
		eqTime.minute = 0;
		eqTime.hour = 9;
		eqTime.day = 1;
		eqTime.month = 1;
		eqTime.year = 3100;
		realtime = time(0);
	}
	else{
		auto row = results.begin();

		eqTime.minute = atoi(row[0]);
		eqTime.hour = atoi(row[1]);
		eqTime.day = atoi(row[2]);
		eqTime.month = atoi(row[3]);
		eqTime.year = atoi(row[4]);
		realtime = atoi(row[5]);
	}

	return eqTime;
}

bool Database::SaveTime(int8 minute, int8 hour, int8 day, int8 month, int16 year)
{
	std::string query = StringFormat("UPDATE eqtime set minute = %d, hour = %d, day = %d, month = %d, year = %d, realtime = %d limit 1", minute, hour, day, month, year, time(0));
	auto results = QueryDatabase(query);

	return results.Success();

}

int Database::GetIPExemption(std::string account_ip) {
	auto query = fmt::format(
		"SELECT `exemption_amount` FROM `ip_exemptions` WHERE `exemption_ip` = '{}'",
		account_ip
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return RuleI(World, MaxClientsPerIP);
	}

	auto row = results.begin();
	return std::stoi(row[0]);
}

void Database::SetIPExemption(std::string account_ip, int exemption_amount) {
	auto query = fmt::format(
		"SELECT `exemption_id` FROM `ip_exemptions` WHERE `exemption_ip` = '{}'",
		account_ip
	);

	uint32 exemption_id = 0;

	auto results = QueryDatabase(query);
	if (results.Success() && results.RowCount()) {
		auto row = results.begin();
		exemption_id = std::stoul(row[0]);
	}

	query = fmt::format(
		"INSERT INTO `ip_exemptions` (`exemption_ip`, `exemption_amount`) VALUES ('{}', {})",
		account_ip,
		exemption_amount
	);

	if (exemption_id) {
		query = fmt::format(
			"UPDATE `ip_exemptions` SET `exemption_amount` = {} WHERE `exemption_ip` = '{}'",
			exemption_amount,
			account_ip
		);
	}

	QueryDatabase(query);
}

int Database::GetInstanceID(uint32 char_id, uint32 zone_id) {
	std::string query = StringFormat("SELECT instance_list.id FROM instance_list INNER JOIN instance_list_player ON instance_list.id = instance_list_player.id WHERE instance_list.zone = '%i' AND instance_list_player.charid = '%i'", zone_id, char_id);
	auto results = QueryDatabase(query);

	if (results.Success() && results.RowCount() > 0) {
		auto row = results.begin();
		return atoi(row[0]);;
	}

	return 0;
}

/**
 * @param source_character_name
 * @param destination_character_name
 * @param destination_account_name
 * @return
 */
bool Database::CopyCharacter(
	const std::string& source_character_name,
	const std::string& destination_character_name,
	const std::string& destination_account_name
)
{
	auto results = QueryDatabase(
		fmt::format(
			"SELECT id FROM character_data WHERE name = '{}' and deleted_at is NULL LIMIT 1",
			source_character_name
		)
	);

	if (results.RowCount() == 0) {
		LogError("No character found with name [{}]", source_character_name);
		return false;
	}

	auto        row                 = results.begin();
	std::string source_character_id = row[0];

	results = QueryDatabase(
		fmt::format(
			"SELECT id FROM account WHERE name = '{}' LIMIT 1",
			destination_account_name
		)
	);

	if (results.RowCount() == 0) {
		LogError("No account found with name [{}]", destination_account_name);
		return false;
	}

	row = results.begin();
	std::string source_account_id = row[0];

	/**
	 * Fresh ID
	 */
	results = QueryDatabase("SELECT (MAX(id) + 1) as new_id from character_data");
	row     = results.begin();
	std::string new_character_id = row[0];

	TransactionBegin();
	for (const auto &iter : DatabaseSchema::GetCharacterTables()) {
		std::string table_name               = iter.first;
		std::string character_id_column_name = iter.second;

		/**
		 * Columns
		 */
		results = QueryDatabase(fmt::format("SHOW COLUMNS FROM {}", table_name));
		std::vector<std::string> columns      = {};
		int                      column_count = 0;
		for (row = results.begin(); row != results.end(); ++row) {
			columns.emplace_back(row[0]);
			column_count++;
		}

		results = QueryDatabase(
			fmt::format(
				"SELECT {} FROM {} WHERE {} = {}",
				Strings::Implode(",", Strings::Wrap(columns, "`")),
				table_name,
				character_id_column_name,
				source_character_id
			)
		);

		std::vector<std::vector<std::string>> new_rows;
		for (row = results.begin(); row != results.end(); ++row) {
			std::vector<std::string> new_values   = {};
			for (int                 column_index = 0; column_index < column_count; column_index++) {
				std::string column = columns[column_index];
				std::string value  = row[column_index] ? row[column_index] : "null";

				if (column == character_id_column_name) {
					value = new_character_id;
				}

				if (column == "name" && table_name == "character_data") {
					value = destination_character_name;
				}

				if (column == "account_id" && table_name == "character_data") {
					value = source_account_id;
				}

				new_values.emplace_back(value);
			}

			new_rows.emplace_back(new_values);
		}

		std::string              insert_values;
		std::vector<std::string> insert_rows;

		for (auto &r: new_rows) {
			std::string insert_row = "(" + Strings::Implode(",", Strings::Wrap(r, "'")) + ")";
			insert_rows.emplace_back(insert_row);
		}

		if (!insert_rows.empty()) {
			QueryDatabase(
				fmt::format(
					"DELETE FROM {} WHERE {} = {}",
					table_name,
					character_id_column_name,
					new_character_id
				)
			);

			auto insert = QueryDatabase(
				fmt::format(
					"INSERT INTO {} ({}) VALUES {}",
					table_name,
					Strings::Implode(",", Strings::Wrap(columns, "`")),
					Strings::Implode(",", insert_rows)
				)
			);

			if (!insert.ErrorMessage().empty()) {
				TransactionRollback();
				return false;
				break;
			}
		}
	}

	TransactionCommit();

	return true;
}

void Database::SourceDatabaseTableFromUrl(std::string table_name, std::string url)
{
	try {
		uri request_uri(url);

		LogHTTP(
			"parsing url [{}] path [{}] host [{}] query_string [{}] protocol [{}] port [{}]",
			url,
			request_uri.get_path(),
			request_uri.get_host(),
			request_uri.get_query(),
			request_uri.get_scheme(),
			request_uri.get_port()
		);

		if (!DoesTableExist(table_name)) {
			LogMySQLQuery("Table [{}] does not exist. Downloading from Github and installing...", table_name);

			// http get request
			httplib::Client cli(
				fmt::format(
					"{}://{}",
					request_uri.get_scheme(),
					request_uri.get_host()
				).c_str()
			);

			cli.set_connection_timeout(0, 60000000); // 60 sec
			cli.set_read_timeout(60, 0); // 60 seconds
			cli.set_write_timeout(60, 0); // 60 seconds

			int sourced_queries = 0;

			if (auto res = cli.Get(request_uri.get_path().c_str())) {
				if (res->status == 200) {
					for (auto &s: Strings::Split(res->body, ';')) {
						if (!Strings::Trim(s).empty()) {
							auto results = QueryDatabase(s);
							if (!results.ErrorMessage().empty()) {
								LogError("Error sourcing SQL [{}]", results.ErrorMessage());
								return;
							}
							sourced_queries++;
						}
					}
				}
			}
			else {
				LogError("Error retrieving URL [{}]", url);
			}

			LogMySQLQuery(
				"Table [{}] installed. Sourced [{}] queries",
				table_name,
				sourced_queries
			);
		}

	}
	catch (std::invalid_argument iae) {
		LogError("URI parser error [{}]", iae.what());
	}
}

uint8 Database::GetMinStatus(uint32 zone_id, uint32 instance_version)
{
	auto zones = ZoneRepository::GetWhere(
		*this,
		fmt::format(
			"zoneidnumber = {} AND (version = {} OR version = 0) ORDER BY version DESC LIMIT 1",
			zone_id,
			instance_version
		)
	);

	return !zones.empty() ? zones[0].min_status : 0;
}
