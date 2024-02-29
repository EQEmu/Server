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

#include "../common/repositories/account_repository.h"
#include "../common/repositories/character_bind_repository.h"
#include "../common/repositories/character_data_repository.h"
#include "../common/repositories/character_languages_repository.h"
#include "../common/repositories/character_leadership_abilities_repository.h"
#include "../common/repositories/character_skills_repository.h"

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
#include "repositories/merchantlist_temp_repository.h"

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

	auto id = Strings::ToUnsignedInt(row[0]);

	if (oStatus) {
		*oStatus = Strings::ToInt(row[1]);
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
	int16 status = Strings::ToInt(row[0]);
	int32 date_diff = 0;

	if (row[1]) {
		date_diff = Strings::ToInt(row[1]);
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
		if (row[0] && Strings::ToInt(row[0]) > 0){
			LogInfo("Account: [{}] tried to request name: [{}], but it is already taken", account_id, name);
			return false;
		}
	}

	query = StringFormat("INSERT INTO `character_data` SET `account_id` = %i, `name` = '%s'", account_id, name);
	results = QueryDatabase(query);
	if (!results.Success() || !results.ErrorMessage().empty()){ return false; }

	// Put character into the default guild if rule is being used.
	int guild_id = RuleI(Character, DefaultGuild);

	if (guild_id != 0) {
		int character_id=results.LastInsertedID();
		if (character_id > -1) {
			query = StringFormat("INSERT INTO `guild_members` SET `char_id` = %i, `guild_id` = '%i'", character_id, guild_id);
			results = QueryDatabase(query);
			if (!results.Success() || !results.ErrorMessage().empty()){
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
		character_id = Strings::ToUnsignedInt(row[0]);
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

bool Database::SaveCharacterCreate(uint32 character_id, uint32 account_id, PlayerProfile_Struct *pp)
{
	auto c = CharacterDataRepository::NewEntity();

	c.id                      = character_id;
	c.account_id              = account_id;
	c.name                    = pp->name;
	c.last_name               = pp->last_name;
	c.gender                  = pp->gender;
	c.race                    = pp->race;
	c.class_                  = pp->class_;
	c.level                   = pp->level;
	c.deity                   = pp->deity;
	c.birthday                = pp->birthday;
	c.last_login              = pp->lastlogin;
	c.time_played             = pp->timePlayedMin;
	c.pvp_status              = pp->pvp;
	c.level2                  = pp->level2;
	c.anon                    = pp->anon;
	c.gm                      = pp->gm;
	c.intoxication            = pp->intoxication;
	c.hair_color              = pp->haircolor;
	c.beard_color             = pp->beardcolor;
	c.eye_color_1             = pp->eyecolor1;
	c.eye_color_2             = pp->eyecolor2;
	c.hair_style              = pp->hairstyle;
	c.beard                   = pp->beard;
	c.ability_time_seconds    = pp->ability_time_seconds;
	c.ability_number          = pp->ability_number;
	c.ability_time_minutes    = pp->ability_time_minutes;
	c.ability_time_hours      = pp->ability_time_hours;
	c.title                   = pp->title;
	c.suffix                  = pp->suffix;
	c.exp                     = pp->exp;
	c.points                  = pp->points;
	c.mana                    = pp->mana;
	c.cur_hp                  = pp->cur_hp;
	c.str                     = pp->STR;
	c.sta                     = pp->STA;
	c.cha                     = pp->CHA;
	c.dex                     = pp->DEX;
	c.int_                    = pp->INT;
	c.agi                     = pp->AGI;
	c.wis                     = pp->WIS;
	c.face                    = pp->face;
	c.y                       = pp->y;
	c.x                       = pp->x;
	c.z                       = pp->z;
	c.heading                 = pp->heading;
	c.pvp2                    = pp->pvp2;
	c.pvp_type                = pp->pvptype;
	c.autosplit_enabled       = pp->autosplit;
	c.zone_change_count       = pp->zone_change_count;
	c.drakkin_heritage        = pp->drakkin_heritage;
	c.drakkin_tattoo          = pp->drakkin_tattoo;
	c.drakkin_details         = pp->drakkin_details;
	c.toxicity                = pp->toxicity;
	c.hunger_level            = pp->hunger_level;
	c.thirst_level            = pp->thirst_level;
	c.ability_up              = pp->ability_up;
	c.zone_id                 = pp->zone_id;
	c.zone_instance           = pp->zoneInstance;
	c.leadership_exp_on       = pp->leadAAActive;
	c.ldon_points_guk         = pp->ldon_points_guk;
	c.ldon_points_mir         = pp->ldon_points_mir;
	c.ldon_points_mmc         = pp->ldon_points_mmc;
	c.ldon_points_ruj         = pp->ldon_points_ruj;
	c.ldon_points_tak         = pp->ldon_points_tak;
	c.ldon_points_available   = pp->ldon_points_available;
	c.tribute_time_remaining  = pp->tribute_time_remaining;
	c.show_helm               = pp->showhelm;
	c.career_tribute_points   = pp->career_tribute_points;
	c.tribute_points          = pp->tribute_points;
	c.tribute_active          = pp->tribute_active;
	c.endurance               = pp->endurance;
	c.group_leadership_exp    = pp->group_leadership_exp;
	c.raid_leadership_exp     = pp->raid_leadership_exp;
	c.group_leadership_points = pp->group_leadership_points;
	c.raid_leadership_points  = pp->raid_leadership_points;
	c.air_remaining           = pp->air_remaining;
	c.pvp_kills               = pp->PVPKills;
	c.pvp_deaths              = pp->PVPDeaths;
	c.pvp_current_points      = pp->PVPCurrentPoints;
	c.pvp_career_points       = pp->PVPCareerPoints;
	c.pvp_best_kill_streak    = pp->PVPBestKillStreak;
	c.pvp_worst_death_streak  = pp->PVPWorstDeathStreak;
	c.pvp_current_kill_streak = pp->PVPCurrentKillStreak;
	c.aa_points_spent         = pp->aapoints_spent;
	c.aa_exp                  = pp->expAA;
	c.aa_points               = pp->aapoints;
	c.group_auto_consent      = pp->groupAutoconsent;
	c.raid_auto_consent       = pp->raidAutoconsent;
	c.guild_auto_consent      = pp->guildAutoconsent;
	c.RestTimer               = pp->RestTimer;


	if (RuleB(Custom, MulticlassingEnabled)) {
		std::string insertQuery = StringFormat("REPLACE INTO data_buckets (`key`, `value`,`character_id`) VALUES ('GestaltClasses', '%d', %d)", pp->classes, character_id);
    	auto results = QueryDatabase(insertQuery);
	}	

	CharacterDataRepository::ReplaceOne(*this, c);

	std::vector<CharacterBindRepository::CharacterBind> character_binds;

	character_binds.reserve(5);

	auto b = CharacterBindRepository::NewEntity();

	b.id = character_id;

	for (uint8 slot_id = 0; slot_id < 5; slot_id++) {
		b.zone_id     = pp->binds[slot_id].zone_id;
		b.x           = pp->binds[slot_id].x;
		b.y           = pp->binds[slot_id].y;
		b.z           = pp->binds[slot_id].z;
		b.heading     = pp->binds[slot_id].heading;
		b.slot        = slot_id;

		character_binds.emplace_back(b);
	}

	CharacterBindRepository::ReplaceMany(*this, character_binds);

	if (RuleB(Character, GrantHoTTOnCreate)) {
		CharacterLeadershipAbilitiesRepository::InsertOne(
			*this,
			CharacterLeadershipAbilitiesRepository::CharacterLeadershipAbilities{
				.id = character_id,
				.slot = LeadershipAbilitySlot::HealthOfTargetsTarget,
				.rank_ = 1
			}
		);
	}

	std::vector<CharacterSkillsRepository::CharacterSkills> character_skills;

	character_skills.reserve(MAX_PP_SKILL);

	for (uint16 slot_id = 0; slot_id < MAX_PP_SKILL; slot_id++) {
		character_skills.emplace_back(
			CharacterSkillsRepository::CharacterSkills{
				.id = character_id,
				.skill_id = slot_id,
				.value = static_cast<uint16_t>(pp->skills[slot_id])
			}
		);
	}

	CharacterSkillsRepository::ReplaceMany(*this, character_skills);

	std::vector<CharacterLanguagesRepository::CharacterLanguages> character_languages;

	character_languages.reserve(MAX_PP_LANGUAGE);

	for (uint16 slot_id = 0; slot_id < MAX_PP_LANGUAGE; slot_id++) {
		character_languages.emplace_back(
			CharacterLanguagesRepository::CharacterLanguages{
				.id = character_id,
				.lang_id = slot_id,
				.value = static_cast<uint16_t>(pp->languages[slot_id])
			}
		);
	}

	CharacterLanguagesRepository::ReplaceMany(*this, character_languages);

	return true;
}

uint32 Database::GetCharacterID(const std::string& name)
{
	const auto& l = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto e = l.front();

	return e.id;
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

	uint32 accountId = Strings::ToUnsignedInt(row[0]);

	if (oCharID)
		*oCharID = Strings::ToUnsignedInt(row[1]);

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
	return Strings::ToUnsignedInt(row[0]);
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
	auto account_id = Strings::ToUnsignedInt(row[0]);

	if (status) {
		*status = static_cast<int16>(Strings::ToInt(row[1]));
	}

	if (lsid) {
		*lsid = row[2] ? Strings::ToUnsignedInt(row[2]) : 0;
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
		*oLSAccountID = Strings::ToUnsignedInt(row[1]);
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
		varcache.last_update = Strings::ToUnsignedInt(row[2]); // ahh should we be comparing if this is newer?
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

bool Database::SetVariable(const std::string& varname, const std::string &varvalue)
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
		*graveyard_zoneid = Strings::ToUnsignedInt(row[0]);
	if(graveyard_x != nullptr)
		*graveyard_x = Strings::ToFloat(row[1]);
	if(graveyard_y != nullptr)
		*graveyard_y = Strings::ToFloat(row[2]);
	if(graveyard_z != nullptr)
		*graveyard_z = Strings::ToFloat(row[3]);
	if(graveyard_heading != nullptr)
		*graveyard_heading = Strings::ToFloat(row[4]);

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
		account_id = Strings::ToUnsignedInt(row[0]);

		if (in_account_name) {
			strcpy(in_account_name, row[1]);
		}
		if (in_status) {
			*in_status = Strings::ToInt(row[2]);
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
		*oStatus = Strings::ToInt(row[1]);
}

void Database::ClearMerchantTemp()
{
	MerchantlistTempRepository::ClearTemporaryMerchantLists(*this);
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
	return Strings::ToUnsignedInt(row[0]);
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
	return Strings::ToUnsignedInt(row[0]);
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
		skill_level = Strings::ToUnsignedInt(row[0]);
		skill_formula = Strings::ToUnsignedInt(row[1]);
		skill_cap = Strings::ToUnsignedInt(row[2]);
		if (Strings::ToUnsignedInt(row[3]) > skill_cap)
			skill_cap2 = (Strings::ToUnsignedInt(row[3])-skill_cap)/10; //Split the post-50 skill cap into difference between pre-50 cap and post-50 cap / 10 to determine amount of points per level.
		skill_cap3 = Strings::ToUnsignedInt(row[4]);
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
	auto character_id = Strings::ToUnsignedInt(row[0]);
	*account_id = Strings::ToUnsignedInt(row[1]);
	*zone_id = Strings::ToUnsignedInt(row[2]);
	*instance_id = Strings::ToUnsignedInt(row[3]);

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

	return Strings::ToUnsignedInt(row[0]);
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
		group_id = Strings::ToUnsignedInt(row[0]);
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
		*mentor_percent = Strings::ToInt(row[6]);

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

uint8 Database::GetAgreementFlag(uint32 account_id)
{
	const auto& e = AccountRepository::FindOne(*this, account_id);
	if (!e.id) {
		return 0;
	}

	return e.rulesflag;
}

void Database::SetAgreementFlag(uint32 account_id) {
	auto e = AccountRepository::FindOne(*this, account_id);
	e.rulesflag = 1;
	AccountRepository::UpdateOne(*this, e);
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
		return Strings::ToUnsignedInt(row[0]);

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
		*mentor_percent = Strings::ToInt(row[5]);

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

	as->success.guk = Strings::ToUnsignedInt(row[0]);
	as->success.mir = Strings::ToUnsignedInt(row[1]);
	as->success.mmc = Strings::ToUnsignedInt(row[2]);
	as->success.ruj = Strings::ToUnsignedInt(row[3]);
	as->success.tak = Strings::ToUnsignedInt(row[4]);
	as->failure.guk = Strings::ToUnsignedInt(row[5]);
	as->failure.mir = Strings::ToUnsignedInt(row[6]);
	as->failure.mmc = Strings::ToUnsignedInt(row[7]);
	as->failure.ruj = Strings::ToUnsignedInt(row[8]);
	as->failure.tak = Strings::ToUnsignedInt(row[9]);
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
	return Strings::ToUnsignedInt(row[0]);
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
	return Strings::ToUnsignedInt(row[0]);
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
	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto row = results.begin();
	return Strings::ToUnsignedInt(row[0]);
}

int Database::CountInvSnapshots() {
	std::string query = StringFormat("SELECT COUNT(*) FROM (SELECT * FROM `inventory_snapshots` a GROUP BY `charid`, `time_index`) b");
	auto results = QueryDatabase(query);

	if (!results.Success())
		return -1;

	auto row = results.begin();

	int64 count = Strings::ToBigInt(row[0]);
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
	TimeOfDay_Struct t{};
	std::string      query = StringFormat("SELECT minute,hour,day,month,year,realtime FROM eqtime limit 1");
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() == 0) {
		LogInfo("Loading EQ time of day failed. Using defaults");
		t.minute = 0;
		t.hour   = 9;
		t.day    = 1;
		t.month  = 1;
		t.year   = 3100;
		realtime = time(nullptr);
		return t;
	}

	auto row = results.begin();

	uint8  hour      = Strings::ToUnsignedInt(row[1]);
	time_t realtime_ = Strings::ToBigInt(row[5]);
	if (RuleI(World, BootHour) > 0 && RuleI(World, BootHour) <= 24) {
		hour      = RuleI(World, BootHour);
		realtime_ = time(nullptr);
	}

	t.minute = Strings::ToUnsignedInt(row[0]);
	t.hour   = hour;
	t.day    = Strings::ToUnsignedInt(row[2]);
	t.month  = Strings::ToUnsignedInt(row[3]);
	t.year   = Strings::ToUnsignedInt(row[4]);
	realtime = realtime_;

	LogEqTime("Setting hour to [{}]", hour);

	return t;
}

bool Database::SaveTime(int8 minute, int8 hour, int8 day, int8 month, int16 year)
{
	std::string query = StringFormat("UPDATE eqtime set minute = %d, hour = %d, day = %d, month = %d, year = %d, realtime = %d limit 1", minute, hour, day, month, year, time(nullptr));
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
	return Strings::ToInt(row[0]);
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
		exemption_id = Strings::ToUnsignedInt(row[0]);
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
		return Strings::ToInt(row[0]);;
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

	std::vector<std::string> tables_to_zero_id = {
		"keyring",
		"data_buckets",
	};

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

				if (column == "id" && Strings::Contains(tables_to_zero_id, table_name)) {
					value = "0";
				}

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
			LogMySQLQuery("Table [{}] does not exist. Downloading and installing...", table_name);

			// http get request
			httplib::Client cli(
				fmt::format(
					"{}://{}",
					request_uri.get_scheme(),
					request_uri.get_host()
				)
			);

			cli.set_connection_timeout(0, 60000000); // 60 sec
			cli.set_read_timeout(60, 0); // 60 seconds
			cli.set_write_timeout(60, 0); // 60 seconds

			int sourced_queries = 0;

			if (auto res = cli.Get(request_uri.get_path())) {
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

void Database::SourceSqlFromUrl(std::string url)
{
	try {
		uri request_uri(url);

		LogHTTPDetail(
			"parsing url [{}] path [{}] host [{}] query_string [{}] protocol [{}] port [{}]",
			url,
			request_uri.get_path(),
			request_uri.get_host(),
			request_uri.get_query(),
			request_uri.get_scheme(),
			request_uri.get_port()
		);

		LogInfo("Downloading and installing from [{}]", url);

		// http get request
		httplib::Client cli(
			fmt::format(
				"{}://{}",
				request_uri.get_scheme(),
				request_uri.get_host()
			)
		);

		cli.set_connection_timeout(0, 60000000); // 60 sec
		cli.set_read_timeout(60, 0); // 60 seconds
		cli.set_write_timeout(60, 0); // 60 seconds

		if (auto res = cli.Get(request_uri.get_path())) {
			if (res->status == 200) {
				auto results = QueryDatabaseMulti(res->body);
				if (!results.ErrorMessage().empty()) {
					LogError("Error sourcing SQL [{}]", results.ErrorMessage());
					return;
				}
			}
			if (res->status == 404) {
				LogError("Error retrieving URL [{}]", url);
			}
		}
		else {
			LogError("Error retrieving URL [{}]", url);
		}

	}
	catch (std::invalid_argument iae) {
		LogError("URI parser error [{}]", iae.what());
	}
}
