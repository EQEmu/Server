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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errmsg.h>
#include <mysqld_error.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
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
//#include "MiscFunctions.h"
#include "StringUtil.h"
#include "extprofile.h"
extern Client client;

/*
This is the amount of time in seconds the client has to enter the zone
server after the world server, or inbetween zones when that is finished
*/

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

bool Database::Connect(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
{
	uint32 errnum= 0;
	std::string errbuf;
	if (!Open(host, user, passwd, database, port, &errnum, &errbuf))
	{
		LogFile->write(EQEMuLog::Error, "Failed to connect to database: Error: %s", errbuf.c_str());
		HandleMysqlError(errnum);

		return false;
	}
	else
	{
		LogFile->write(EQEMuLog::Status, "Using database '%s' at %s:%d",database,host,port);
		return true;
	}
}

void Database::DBInitVars() {

	max_zonename = 0;
	zonename_array = 0;
	varcache_array = 0;
	varcache_max = 0;
	varcache_lastupdate = 0;
}



void Database::HandleMysqlError(uint32 errnum) {
/*	switch(errnum) {
		case 0:
			break;
		case 1045: // Access Denied
		case 2001: {
			AddEQEMuError(EQEMuError_Mysql_1405, true);
			break;
		}
		case 2003: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2003, true);
			break;
		}
		case 2005: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2005, true);
			break;
		}
		case 2007: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2007, true);
			break;
		}
	}*/
}

/*

Close the connection to the database
*/
Database::~Database()
{
	unsigned int x;
	if (zonename_array) {
		for (x=0; x<=max_zonename; x++) {
			if (zonename_array[x])
				safe_delete_array(zonename_array[x]);
		}
		safe_delete_array(zonename_array);
	}
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
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(strlen(name) >= 50 || strlen(password) >= 50)
		return(0);

	std::string tmpUserName;
	std::string tmpPassWord;
	
	DoEscapeString(tmpUserName, name, strlen(name));
	DoEscapeString(tmpPassWord, password, strlen(password));

	StringFormat(query,"SELECT id, status FROM account WHERE name='%s' AND password is not null "
						"and length(password) > 0 and (password='%s' or password=MD5('%s'))",
						tmpUserName.c_str(), tmpPassWord.c_str(), tmpPassWord.c_str());
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			uint32 id = atoi(row[0]);
			if (oStatus)
				*oStatus = atoi(row[1]);
			mysql_free_result(result);
			return id;
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
		std::cerr << "Error in CheckLogin query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	return 0;
}


//Lieka: Get Banned IP Address List - Only return false if the incoming connection's IP address is not present in the banned_ips table.
bool Database::CheckBannedIPs(const char* loginIP)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	//cout << "Checking against Banned IPs table."<< endl; //Lieka: Debugging
	StringFormat(query,"SELECT ip_address FROM Banned_IPs WHERE ip_address='%s'", loginIP);

	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) != 0)
		{
			//cout << loginIP << " was present in the banned IPs table" << endl; //Lieka: Debugging
			mysql_free_result(result);
			return true;
		}
		else
		{
			//cout << loginIP << " was not present in the banned IPs table." << endl; //Lieka: Debugging
			mysql_free_result(result);
			return false;
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error in CheckBannedIPs query '" << query << "' " << errbuf << std::endl;
		return true;
	}
	return true;
}

bool Database::AddBannedIP(char* bannedIP, const char* notes)
{
	std::string errbuf;
	std::string query;

	StringFormat(query,"INSERT into Banned_IPs SET ip_address='%s', notes='%s'", bannedIP, notes);
	if (!RunQuery(query, &errbuf)) {
		std::cerr << "Error in ReserveName query '" << query << "' " << errbuf << std::endl;
		return false;
	}
	return true;
}
 //End Lieka Edit

 bool Database::CheckGMIPs(const char* ip_address, uint32 account_id) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;

	StringFormat(query, "SELECT * FROM `gm_ips` WHERE `ip_address` = '%s' AND `account_id` = %i", ip_address, account_id);

	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			mysql_free_result(result);
			return true;
		} else {
			mysql_free_result(result);
			return false;
		}
		mysql_free_result(result);

	} else {
		return false;
	}

	return false;
}

bool Database::AddGMIP(char* ip_address, char* name) {
	std::string errbuf;
	std::string query;

	StringFormat(query, "INSERT into `gm_ips` SET `ip_address` = '%s', `name` = '%s'", ip_address, name);

	if (!RunQuery(query, &errbuf)) {
		return false;
	}
	return true;
}

void Database::LoginIP(uint32 AccountID, const char* LoginIP)
{
	std::string errbuf;
	std::string query;

	StringFormat(query,"INSERT INTO account_ip SET accid=%i, ip='%s' ON DUPLICATE KEY UPDATE count=count+1, lastused=now()", AccountID, LoginIP);
	if (!RunQuery(query, &errbuf)) {
		std::cerr << "Error in Log IP query '" << query << "' " << errbuf << std::endl;
	}
}

int16 Database::CheckStatus(uint32 account_id)
{
	std::string errbuf;
	std::string query ;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query, "SELECT `status`, UNIX_TIMESTAMP(`suspendeduntil`) as `suspendeduntil`, UNIX_TIMESTAMP() as `current`"
							" FROM `account` WHERE `id` = %i", account_id);

	if (RunQuery(query, &errbuf, &result))
	{

		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);

			int16 status = atoi(row[0]);

			int32 suspendeduntil = atoi(row[1]);

			int32 current = atoi(row[2]);

			mysql_free_result(result);

			if(suspendeduntil > current)
				return -1;

			return status;
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
		std::cerr << "Error in CheckStatus query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	return 0;
}

uint32 Database::CreateAccount(const char* name, const char* password, int16 status, uint32 lsaccount_id) {
	std::string errbuf;
	std::string query;
	uint32 querylen;
	uint32 last_insert_id;

	if (password)
	{
		StringFormat(query,"INSERT INTO account SET name='%s', password='%s', status=%i, lsaccount_id=%i, time_creation=UNIX_TIMESTAMP();",name,password,status, lsaccount_id);
	}
	else
	{
		StringFormat(query, "INSERT INTO account SET name='%s', status=%i, lsaccount_id=%i, time_creation=UNIX_TIMESTAMP();",name, status, lsaccount_id);
	}

	std::cerr << "Account Attempting to be created:" << name << " " << (int16) status << std::endl;
	if (!RunQuery(query, &errbuf, nullptr, nullptr, &last_insert_id)) {
		std::cerr << "Error in CreateAccount query '" << query << "' " << errbuf << std::endl;
		return 0;
	}

	if (last_insert_id == 0) {
		std::cerr << "Error in CreateAccount query '" << query << "' " << errbuf << std::endl;
		return 0;
	}

	return last_insert_id;
}

bool Database::DeleteAccount(const char* name) {
	std::string errbuf;
	std::string query;
	uint32 affected_rows = 0;

	std::cerr << "Account Attempting to be deleted:" << name << std::endl;
	StringFormat(query,"DELETE FROM account WHERE name='%s';",name);

	if (RunQuery(query, &errbuf, nullptr, &affected_rows)) {
		if (affected_rows == 1) {
			return true;
		}
	}
	else {

		std::cerr << "Error in DeleteAccount query '" << query << "' " << errbuf << std::endl;
	}

	return false;
}

bool Database::SetLocalPassword(uint32 accid, const char* password) {
	std::string errbuf;
	std::string query;

	StringFormat(query,"UPDATE account SET password=MD5('%s') where id=%i;", password, accid);

	if (!RunQuery(query, &errbuf)) {
		std::cerr << "Error in SetLocalPassword query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	return true;
}

bool Database::SetAccountStatus(const char* name, int16 status) {
	std::string errbuf;
	std::string query;
	uint32	affected_rows = 0;

	std::cout << "Account being GM Flagged:" << name << ", Level: " << (int16) status << std::endl;
	StringFormat(query,"UPDATE account SET status=%i WHERE name='%s';", status, name);
	if (!RunQuery(query, &errbuf, nullptr, &affected_rows)) {
		return false;
	}

	if (affected_rows == 0) {
		std::cout << "Account: " << name << " does not exist, therefore it cannot be flagged\n";
		return false;
	}

	return true;
}

bool Database::ReserveName(uint32 account_id, char* name)
{
	std::string errbuf;
	std::string query;

	StringFormat(query,"INSERT into character_ SET account_id=%i, name='%s', profile=NULL", account_id, name);
	if (!RunQuery(query, &errbuf)) {
		std::cerr << "Error in ReserveName query '" << query << "' " << errbuf << std::endl;
		return false;
	}
	return true;
}

/*
Delete the character with the name "name"
returns false on failure, true otherwise
*/
bool Database::DeleteCharacter(char *name)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int charid, matches;
	uint32 affected_rows;

	if(!name ||	!strlen(name))
	{
		printf("DeleteCharacter: request to delete without a name (empty char slot)\n");
		return false;
	}

// get id from character_ before deleting record so we can clean up inventory and qglobal

#if DEBUG >= 5
	printf("DeleteCharacter: Attempting to delete '%s'\n", name);
#endif
	StringFormat(query,"SELECT id from character_ WHERE name='%s'", name);
	
	RunQuery(query, &errbuf, &result);
	matches = mysql_num_rows(result);
	if(matches == 1)
	{
		row = mysql_fetch_row(result);
		charid = atoi(row[0]);
#if DEBUG >= 5
		printf("DeleteCharacter: found '%s' with char id: %d\n", name, charid);
#endif
	}
	else
	{
		printf("DeleteCharacter: error: got %d rows matching '%s'\n", matches, name);
		if(result)
		{
			mysql_free_result(result);
			result = nullptr;
		}
		return false;
	}

	if(result)
	{
		mysql_free_result(result);
		result = nullptr;
	}

#if DEBUG >= 5
	printf("DeleteCharacter: deleting '%s' (id %d): ", name, charid);
	printf(" quest_globals");
#endif
	StringFormat(query,"DELETE from quest_globals WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" character_tasks");
#endif
	StringFormat(query,"DELETE from character_tasks WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" character_activities");
#endif
	StringFormat(query,"DELETE from character_activities WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" character_enabledtasks");
#endif
	StringFormat(query,"DELETE from character_enabledtasks WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" completed_tasks");
#endif
	StringFormat(query,"DELETE from completed_tasks WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" friends");
#endif
	StringFormat(query, "DELETE from friends WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" mail");
#endif
	StringFormat(query, "DELETE from mail WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" ptimers");
#endif
	StringFormat(query, "DELETE from timers WHERE char_id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" inventory");
#endif
	StringFormat(query, "DELETE from inventory WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" guild_members");
#endif
#ifdef BOTS
	StringFormat(query,"DELETE FROM guild_members WHERE char_id='%d' AND GetMobTypeById(%i) = 'C'", charid);
#else
	StringFormat(query,"DELETE FROM guild_members WHERE char_id='%d'", charid);
#endif
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" recipes");
#endif
	StringFormat(query,"DELETE FROM char_recipe_list WHERE char_id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" adventure_stats");
#endif
	StringFormat(query,"DELETE FROM adventure_stats WHERE player_id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" zone_flags");
#endif
	StringFormat(query,"DELETE FROM zone_flags WHERE charID='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" titles");
#endif
	StringFormat(query, "DELETE FROM titles WHERE char_id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" titlesets");
#endif
	StringFormat(query,"DELETE FROM player_titlesets WHERE char_id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" keyring");
#endif
	StringFormat(query, "DELETE FROM keyring WHERE char_id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" factions");
#endif
	StringFormat(query,"DELETE FROM faction_values WHERE char_id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" instances");
#endif
	StringFormat(query,"DELETE FROM instance_lockout_player WHERE charid='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf(" _character");
#endif
	StringFormat(query,"DELETE from character_ WHERE id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);
	if(affected_rows != 1)	// here we have to have a match or it's an error
	{
		LogFile->write(EQEMuLog::Error, "DeleteCharacter: error: delete operation affected %d rows\n", affected_rows);
		return false;
	}

#if DEBUG >= 5
	printf(" alternate currency");
#endif
	StringFormat(query,"DELETE FROM character_alt_currency WHERE char_id='%d'", charid);
	RunQuery(query, &errbuf, nullptr, &affected_rows);

#if DEBUG >= 5
	printf("\n");
#endif
	printf("DeleteCharacter: successfully deleted '%s' (id %d)\n", name, charid);

	return true;
}

// Store new character information into the character_ and inventory tables
bool Database::StoreCharacter(uint32 account_id, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext)
{
	_CP(Database_StoreCharacter);
	std::string errbuf;
	std::string query;
	uint32 affected_rows = 0;
	int i;
	uint32 charid = 0;
	MYSQL_RES *result;
	MYSQL_ROW row = 0;

	char zone[50];
	float x, y, z;


	// get the char id (used in inventory inserts below)
	StringFormat(query,"SELECT id FROM character_ where name='%s'", pp->name);

	if(!RunQuery(query, &errbuf, &result)) {
		LogFile->write(EQEMuLog::Error, "Error in char store id query: %s: %s", query.c_str(), errbuf.c_str());
		return false;
	}

	if(mysql_num_rows(result) == 1)
	{
		row = mysql_fetch_row(result);
		if(row[0])
			charid = atoi(row[0]);
	}

	if(!charid)
	{
		LogFile->write(EQEMuLog::Error, "StoreCharacter: no character id");
		return false;
	}

	const char *zname = GetZoneName(pp->zone_id);
	if(zname == nullptr) {
		//zone not in the DB, something to prevent crash...
		strn0cpy(zone, "qeynos", 49);
		pp->zone_id = 1;
	} else
		strn0cpy(zone, zname, 49);
	x=pp->x;
	y=pp->y;
	z=pp->z;

	std::string playerProfileBuffer;
	std::string extendedProfileBuffer;

	// construct the character_ query
	StringFormat(query, "UPDATE character_ SET timelaston=0, "
						"zonename=\'%s\', x=%f, y=%f, z=%f, profile=\'",
						zone, x, y, z);
	
	DoEscapeString(playerProfileBuffer, (char*)pp, sizeof(PlayerProfile_Struct));

	query.append(playerProfileBuffer);
	
	query.append("\', extprofile=\'");
	
	DoEscapeString(extendedProfileBuffer, (char*)ext, sizeof(ExtendedProfile_Struct));
	
	query.append(extendedProfileBuffer);

	std::string ending;
	
	StringFormat(ending, "\' WHERE account_id=%d AND name='%s'",account_id, pp->name);
	
	query.append(ending);
	
	RunQuery(query, &errbuf, 0, &affected_rows);

	if(!affected_rows)
	{
		LogFile->write(EQEMuLog::Error, "StoreCharacter query '%s' %s", query.c_str(), errbuf.c_str());
		return false;
	}

	affected_rows = 0;


	// Doodman: Is this even used?
	// now the inventory

	for (i=0; i<=2270;)
	{
		const ItemInst* newinv = inv->GetItem((int16)i);
		if (newinv)
		{
			StringFormat(query,"INSERT INTO inventory SET "
								"charid=%0u, slotid=%0d, itemid=%0u, charges=%0d, color=%0u",
								charid, i, newinv->GetItem()->ID,
								newinv->GetCharges(), newinv->GetColor());
			
			RunQuery(query, &errbuf, 0, &affected_rows);
			if(!affected_rows)
			{
				LogFile->write(EQEMuLog::Error, "StoreCharacter inventory failed. Query '%s' %s", query.c_str(), errbuf.c_str());
			}
#if EQDEBUG >= 9
			else
			{
				LogFile->write(EQEMuLog::Debug, "StoreCharacter inventory succeeded. Query '%s' %s", query.c_str(), errbuf.c_str());
			}
#endif
		}

		if(i==30){ //end of standard inventory/cursor, jump to internals of bags/cursor
			i = 251;
			continue;
		} else if(i==340){ //end of internals of bags/cursor, jump to bank slots
			i = 2000;
			continue;
		} else if(i==2023){ //end of bank slots, jump to internals of bank bags
			i = 2031;
			continue;
		}

		i++;
	}

	return true;
}

//0=failure, otherwise returns the char ID for the given char name.
uint32 Database::GetCharacterID(const char *name) {
	uint32 cid = 0;
	if(GetAccountIDByChar(name, &cid) == 0)
		return(0);
	return(cid);
}

/*
This function returns the account_id that owns the character with
the name "name" or zero if no character with that name was found
Zero will also be returned if there is a database error.
*/
uint32 Database::GetAccountIDByChar(const char* charname, uint32* oCharID) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT account_id, id FROM character_ WHERE name='%s'", charname);
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			uint32 tmp = atoi(row[0]); // copy to temp var because gotta free the result before exitting this function
			if (oCharID)
				*oCharID = atoi(row[1]);
			mysql_free_result(result);
			return tmp;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetAccountIDByChar query '" << query << "' " << errbuf << std::endl;
	}

	return 0;
}

// Retrieve account_id for a given char_id
uint32 Database::GetAccountIDByChar(uint32 char_id) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 ret = 0;

	StringFormat(query,"SELECT account_id FROM character_ WHERE id=%i", char_id);
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			ret = atoi(row[0]); // copy to temp var because gotta free the result before exitting this function
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in GetAccountIDByChar query '%s': %s", query.c_str(), errbuf.c_str());
	}

	return ret;
}

uint32 Database::GetAccountIDByName(const char* accname, int16* status, uint32* lsid) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;


	for (unsigned int i=0; i<strlen(accname); i++) {
		if ((accname[i] < 'a' || accname[i] > 'z') &&
			(accname[i] < 'A' || accname[i] > 'Z') &&
			(accname[i] < '0' || accname[i] > '9'))
			return 0;
	}

	StringFormat(query,"SELECT id, status, lsaccount_id FROM account WHERE name='%s'", accname);
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			uint32 tmp = atoi(row[0]); // copy to temp var because gotta free the result before exitting this function
			if (status)
				*status = atoi(row[1]);
			if (lsid) {
				if (row[2])
					*lsid = atoi(row[2]);
				else
					*lsid = 0;
			}
			mysql_free_result(result);
			return tmp;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetAccountIDByAcc query '" << query << "' " << errbuf << std::endl;
	}

	return 0;
}

void Database::GetAccountName(uint32 accountid, char* name, uint32* oLSAccountID) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT name, lsaccount_id FROM account WHERE id='%i'", accountid);
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);

			strcpy(name, row[0]);
			if (row[1] && oLSAccountID) {
				*oLSAccountID = atoi(row[1]);
			}
		}

		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetAccountName query '" << query << "' " << errbuf << std::endl;
	}
}

void Database::GetCharName(uint32 char_id, char* name) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT name FROM character_ WHERE id='%i'", char_id);
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);

			strcpy(name, row[0]);
		}

		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetCharName query '" << query << "' " << errbuf << std::endl;
	}

}

void Database::LoadVariables_MQ(std::string& query) {
// the read of this single variable should be atomic... this was causing strange problems
//	LockMutex lock(&Mvarcache);
	StringFormat(query,"SELECT varname, value, unix_timestamp() "
						"FROM variables where unix_timestamp(ts) >= %d", 
						varcache_lastupdate);
}

bool Database::LoadVariables() {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	
	LoadVariables_MQ(query);

	if (RunQuery(query, &errbuf, &result)) {
		bool ret = LoadVariables_result(result);
		mysql_free_result(result);
		return ret;
	}
	else {
		std::cerr << "Error in LoadVariables query '" << query << "' " << errbuf << std::endl;
	}
	return false;
}


bool Database::LoadVariables_result(MYSQL_RES* result) {
	uint32 i;
	MYSQL_ROW row;
	LockMutex lock(&Mvarcache);
	if (mysql_num_rows(result) > 0) {
		if (!varcache_array) {
			varcache_max = mysql_num_rows(result);
			varcache_array = new VarCache_Struct*[varcache_max];
			for (i=0; i<varcache_max; i++)
				varcache_array[i] = 0;
		}
		else {
			uint32 tmpnewmax = varcache_max + mysql_num_rows(result);
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
		while ((row = mysql_fetch_row(result))) {
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
		max_used++;
		varcache_max = max_used;
	}
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
	std::string errbuf;
	std::string query;
	uint32 affected_rows = 0;

	std::string varname, varvalue;
	DoEscapeString(varname, varname_in, strlen(varname_in));
	DoEscapeString(varvalue, varvalue_in, strlen(varvalue_in));

	StringFormat(query,"Update variables set value='%s' WHERE varname like '%s'", varvalue.c_str(), varname.c_str());

	if (RunQuery(query, &errbuf, 0, &affected_rows)) {
		if (affected_rows == 1) {
			LoadVariables(); // refresh cache
			return true;
		}
		else {
			StringFormat(query,"Insert Into variables (varname, value) values ('%s', '%s')", varname.c_str(), varvalue.c_str());
			if (RunQuery(query, &errbuf, 0, &affected_rows)) {
				if (affected_rows == 1) {
					LoadVariables(); // refresh cache
					return true;
				}
			}
		}
	}
	else {
		std::cerr << "Error in SetVariable query '" << query << "' " << errbuf << std::endl;
	}
	return false;
}

uint32 Database::GetMiniLoginAccount(char* ip){
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 retid = 0;

	StringFormat(query,"SELECT id FROM account WHERE minilogin_ip='%s'", ip);
	if (RunQuery(query, &errbuf, &result)) {
		if ((row = mysql_fetch_row(result)))
			retid = atoi(row[0]);
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error in GetMiniLoginAccount query '" << query << "' " << errbuf << std::endl;
	}
	return retid;
}

// Pyro: Get zone starting points from DB
bool Database::GetSafePoints(const char* short_name, uint32 version, float* safe_x, float* safe_y, float* safe_z, int16* minstatus, uint8* minlevel, char *flag_needed) {
	std::string errbuf;
	std::string query;
	//	int buf_len = 256;
	//	int chars = -1;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT safe_x, safe_y, safe_z, min_status, min_level, "
						"flag_needed FROM zone "
						"WHERE short_name='%s' AND (version=%i OR version=0) "
						"ORDER BY version DESC", short_name, version);

	if (RunQuery(query, &errbuf, &result) ){
		if (mysql_num_rows(result) > 0) {
			row = mysql_fetch_row(result);
			if (safe_x != 0)
				*safe_x = atof(row[0]);
			if (safe_y != 0)
				*safe_y = atof(row[1]);
			if (safe_z != 0)
				*safe_z = atof(row[2]);
			if (minstatus != 0)
				*minstatus = atoi(row[3]);
			if (minlevel != 0)
				*minlevel = atoi(row[4]);
			if (flag_needed != nullptr)
				strcpy(flag_needed, row[5]);
			mysql_free_result(result);
			return true;
		}

		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error in GetSafePoint query '" << query << "' " << errbuf << std::endl;
		std::cerr << "If it errors, run the following querys:\n";
		std::cerr << "ALTER TABLE `zone` CHANGE `minium_level` `min_level` TINYINT(3)  UNSIGNED DEFAULT \"0\" NOT NULL;\n";
		std::cerr << "ALTER TABLE `zone` CHANGE `minium_status` `min_status` TINYINT(3)  UNSIGNED DEFAULT \"0\" NOT NULL;\n";
		std::cerr << "ALTER TABLE `zone` ADD flag_needed VARCHAR(128) NOT NULL DEFAULT '';\n";
	}
	return false;
}


bool Database::GetZoneLongName(const char* short_name, char** long_name, char* file_name, float* safe_x, float* safe_y, float* safe_z, uint32* graveyard_id, uint32* maxclients) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query, "SELECT long_name, file_name, safe_x, safe_y, safe_z, graveyard_id, maxclients "
						"FROM zone WHERE short_name='%s' AND version=0", short_name);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) > 0) {
			row = mysql_fetch_row(result);
			if (long_name != 0) {
				*long_name = strcpy(new char[strlen(row[0])+1], row[0]);
			}
			if (file_name != 0) {
				if (row[1] == 0)
					strcpy(file_name, short_name);
				else
					strcpy(file_name, row[1]);
			}
			if (safe_x != 0)
				*safe_x = atof(row[2]);
			if (safe_y != 0)
				*safe_y = atof(row[3]);
			if (safe_z != 0)
				*safe_z = atof(row[4]);
			if (graveyard_id != 0)
				*graveyard_id = atoi(row[5]);
			if (maxclients)
				*maxclients = atoi(row[6]);
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error in GetZoneLongName query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	return false;
}
uint32 Database::GetZoneGraveyardID(uint32 zone_id, uint32 version) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 GraveyardID = 0;

	StringFormat(query,"SELECT graveyard_id FROM zone WHERE zoneidnumber='%u' "
						"AND (version=%i OR version=0) ORDER BY version DESC", zone_id, version);
	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) > 0) {
			row = mysql_fetch_row(result);
			GraveyardID = atoi(row[0]);
		}
		mysql_free_result(result);
		return GraveyardID;
	}
	else
	{
		std::cerr << "Error in GetZoneGraveyardID query '" << query << "' " << errbuf << std::endl;
	}
	return GraveyardID;
}

bool Database::GetZoneGraveyard(const uint32 graveyard_id, uint32* graveyard_zoneid, float* graveyard_x, float* graveyard_y, float* graveyard_z, float* graveyard_heading) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT zone_id, x, y, z, heading FROM graveyard WHERE id=%i", graveyard_id);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if(graveyard_zoneid != 0)
				*graveyard_zoneid = atoi(row[0]);
			if(graveyard_x != 0)
				*graveyard_x = atof(row[1]);
			if(graveyard_y != 0)
				*graveyard_y = atof(row[2]);
			if(graveyard_z != 0)
				*graveyard_z = atof(row[3]);
			if(graveyard_heading != 0)
				*graveyard_heading = atof(row[4]);
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error in GetZoneGraveyard query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	return false;
}

bool Database::LoadZoneNames() {
	std::string errbuf;
	MYSQL_RES *result;
	MYSQL_ROW row;

	std::string query = "SELECT MAX(zoneidnumber) FROM zone";

	if (RunQuery(query, &errbuf, &result)) {
		row = mysql_fetch_row(result);
		if (row && row[0])
		{
			max_zonename = atoi(row[0]);
			zonename_array = new char*[max_zonename+1];
			for(unsigned int i=0; i<max_zonename; i++) {
				zonename_array[i] = 0;
			}
			mysql_free_result(result);


			query = "SELECT zoneidnumber, short_name FROM zone";
			if (RunQuery(query, &errbuf, &result)) {
				while((row = mysql_fetch_row(result))) {
					zonename_array[atoi(row[0])] = new char[strlen(row[1]) + 1];
					strcpy(zonename_array[atoi(row[0])], row[1]);
					Sleep(0);
				}
				mysql_free_result(result);
			}
			else {
				std::cerr << "Error in LoadZoneNames query '" << query << "' " << errbuf << std::endl;
				return false;
			}
		}
		else {
			mysql_free_result(result);
		}
	}
	else {
		std::cerr << "Error in LoadZoneNames query '" << query << "' " << errbuf << std::endl;
		return false;
	}
	return true;
}

uint32 Database::GetZoneID(const char* zonename) {
	if (zonename_array == 0)
		return 0;
	if (zonename == 0)
		return 0;
	for (unsigned int i=0; i<=max_zonename; i++) {
		if (zonename_array[i] != 0 && strcasecmp(zonename_array[i], zonename) == 0) {
			return i;
		}
	}
	return 0;
}

const char* Database::GetZoneName(uint32 zoneID, bool ErrorUnknown) {
	if (zonename_array == 0) {
		if (ErrorUnknown)
			return "UNKNOWN";
		else
			return 0;
	}

	if (zoneID <= max_zonename) {
		if (zonename_array[zoneID])
			return zonename_array[zoneID];
		else {
			if (ErrorUnknown)
				return "UNKNOWN";
			else
				return 0;
		}
	}
	else {
		if (ErrorUnknown)
			return "UNKNOWN";
		else
			return 0;
	}
	return 0;
}

uint8 Database::GetPEQZone(uint32 zoneID, uint32 version){
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int peqzone = 0;

	StringFormat(query,"SELECT peqzone from zone where zoneidnumber='%i' "
						"AND (version=%i OR version=0) ORDER BY version DESC", zoneID, version);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) > 0)
		{
			row = mysql_fetch_row(result);
			peqzone = atoi(row[0]);
		}
		mysql_free_result(result);
		return peqzone;
	}
	else
	{
		std::cerr << "Error in GetPEQZone query '" << query << "' " << errbuf << std::endl;
	}
	return peqzone;
}

bool Database::CheckNameFilter(const char* name, bool surname)
{
	std::string str_name = name;
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

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
		if(!name || strlen(name) < 4 || strlen(name) > 64)
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

	query ="SELECT name FROM name_filter";

	if (RunQuery(query, &errbuf, &result)) {
		while((row = mysql_fetch_row(result)))
		{
			std::string current_row = row[0];
			for(size_t x = 0; x < current_row.size(); ++x)
			{
				current_row[x] = tolower(current_row[x]);
			}

			if(str_name.find(current_row) != std::string::npos)
			{
				return false;
			}
		}

		mysql_free_result(result);
		return true;
	}
	else
	{
		std::cerr << "Error in CheckNameFilter query '" << query << "' " << errbuf << std::endl;
	}

	return true;
}

bool Database::AddToNameFilter(const char* name) {
	std::string errbuf;
	std::string query;
	uint32 affected_rows = 0;

	StringFormat(query,"INSERT INTO name_filter (name) values ('%s')", name);
	if (!RunQuery(query, &errbuf, 0, &affected_rows)) {
		std::cerr << "Error in AddToNameFilter query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	if (affected_rows == 0) {
		return false;
	}

	return true;
}

uint32 Database::GetAccountIDFromLSID(uint32 iLSID, char* oAccountName, int16* oStatus) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT id, name, status FROM account WHERE lsaccount_id=%i", iLSID);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			uint32 account_id = atoi(row[0]);
			if (oAccountName)
				strcpy(oAccountName, row[1]);
			if (oStatus)
				*oStatus = atoi(row[2]);
			mysql_free_result(result);
			return account_id;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetAccountIDFromLSID query '" << query << "' " << errbuf << std::endl;
		return 0;
	}

	return 0;
}

void Database::GetAccountFromID(uint32 id, char* oAccountName, int16* oStatus) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query, "SELECT name, status FROM account WHERE id=%i", id);
	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if (oAccountName)
				strcpy(oAccountName, row[0]);
			if (oStatus)
				*oStatus = atoi(row[1]);
		}
		mysql_free_result(result);
	}
	else
		std::cerr << "Error in GetAccountFromID query '" << query << "' " << errbuf << std::endl;
}

void Database::ClearMerchantTemp(){
	std::string errbuf;

	std::string query = "delete from merchantlist_temp";

	if (!RunQuery(query, &errbuf)) {
		std::cerr << "Error in ClearMerchantTemp query '" << query << "' " << errbuf << std::endl;
	}
}

bool Database::UpdateName(const char* oldname, const char* newname) {
	std::string errbuf;
	std::string query;
	uint32	affected_rows = 0;

	std::cout << "Renaming " << oldname << " to " << newname << "..." << std::endl;

	StringFormat(query,"UPDATE character_ SET name='%s' WHERE name='%s';", newname, oldname);

	if (!RunQuery(query, &errbuf, nullptr, &affected_rows)) {
		return false;
	}

	if (affected_rows == 0)
	{
		return false;
	}

	return true;
}

// If the name is used or an error occurs, it returns false, otherwise it returns true
bool Database::CheckUsedName(const char* name)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;

	//if (strlen(name) > 15)
	//	return false;

	StringFormat(query,"SELECT id FROM character_ where name='%s'", name);

	if (!RunQuery(query, &errbuf, &result)) {
		std::cerr << "Error in CheckUsedName query '" << query << "' " << errbuf << std::endl;
		return false;
	}
	else { // It was a valid Query, so lets do our counts!
		uint32 tmp = mysql_num_rows(result);
		mysql_free_result(result);
		if (tmp > 0) // There is a Name! No change (Return False)
			return false;
		else // Everything is okay, so we go and do this.
			return true;
	}
}

uint8 Database::GetServerType()
{
	std::string errbuf;
	
	MYSQL_RES *result;
	MYSQL_ROW row;

	std::string query ="SELECT value FROM variables WHERE varname='ServerType'";

	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			uint8 ServerType = atoi(row[0]);
			mysql_free_result(result);
			return ServerType;
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
		std::cerr << "Error in GetServerType query '" << query << "' " << errbuf << std::endl;
		return false;
	}
	return 0;

}

bool Database::MoveCharacterToZone(const char* charname, const char* zonename,uint32 zoneid) {
	std::string errbuf;
	std::string query;
	uint32	affected_rows = 0;

	if(zonename == nullptr || strlen(zonename) == 0)
		return(false);

	StringFormat(query,"UPDATE character_ SET zonename = '%s',zoneid=%i,x=-1, y=-1, z=-1 WHERE name='%s'", zonename,zoneid, charname);
	if (!RunQuery(query, &errbuf, 0,&affected_rows)) {
		std::cerr << "Error in MoveCharacterToZone(name) query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	if (affected_rows == 0)
		return false;

	return true;
}

bool Database::MoveCharacterToZone(const char* charname, const char* zonename) {
	return MoveCharacterToZone(charname, zonename, GetZoneID(zonename));
}

bool Database::MoveCharacterToZone(uint32 iCharID, const char* iZonename) {
	std::string errbuf;
	std::string query;
	uint32	affected_rows = 0;

	StringFormat(query,"UPDATE character_ SET zonename = '%s', zoneid=%i, x=-1, y=-1, z=-1 WHERE id=%i", iZonename, GetZoneID(iZonename), iCharID);
	if (!RunQuery(query, &errbuf, 0,&affected_rows)) {
		std::cerr << "Error in MoveCharacterToZone(id) query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	if (affected_rows == 0)
		return false;

	return true;
}

uint8 Database::CopyCharacter(const char* oldname, const char* newname, uint32 acctid) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	PlayerProfile_Struct* pp;
	ExtendedProfile_Struct* ext;

	StringFormat(query,"SELECT profile, guild, guildrank, extprofile FROM character_ WHERE name='%s'", oldname);
	if (RunQuery(query, &errbuf, &result)) {

		row = mysql_fetch_row(result);

		pp = (PlayerProfile_Struct*)row[0];
		strcpy(pp->name, newname);

		ext = (ExtendedProfile_Struct*)row[3];

		mysql_free_result(result);
	}

	else {
		std::cerr << "Error in CopyCharacter read query '" << query << "' " << errbuf << std::endl;
		return 0;
	}

	uint32 affected_rows = 0;
	std::string playerProfileBuffer;
	std::string extendedProfileBuffer;

	StringFormat(query, "INSERT INTO character_ SET zonename=\'%s\', x = %f, y = %f, z = %f, profile=\'", 
						GetZoneName(pp->zone_id), pp->x, pp->y, pp->z);
	
	DoEscapeString(playerProfileBuffer, (char*) pp, sizeof(PlayerProfile_Struct));
	query.append(playerProfileBuffer);

	query.append("\', extprofile=\'");

	DoEscapeString(extendedProfileBuffer, (char*) ext, sizeof(ExtendedProfile_Struct));
	query.append(extendedProfileBuffer);
	
	std::string queryEnding;	
	StringFormat(queryEnding,"\', account_id=%d, name='%s'", acctid, newname);

	query.append(queryEnding);

	if (!RunQuery(query, &errbuf, nullptr, &affected_rows)) {
		std::cerr << "Error in CopyCharacter query '" << query << "' " << errbuf << std::endl;
		return 0;
	}

	// @merth: Need to copy inventory as well (and shared bank?)
	if (affected_rows == 0) {
		return 0;
	}

	return 1;
}

bool Database::SetHackerFlag(const char* accountname, const char* charactername, const char* hacked) {
	std::string errbuf;
	std::string query;
	uint32	affected_rows = 0;

	StringFormat(query,"INSERT INTO hackers(account,name,hacked) values('%s','%s','%s')", accountname, charactername, hacked);

	if (!RunQuery(query, &errbuf, 0,&affected_rows)) {
		std::cerr << "Error in SetHackerFlag query '" << query << "' " << errbuf << std::endl;
		return false;
	}

	if (affected_rows == 0)
	{
		return false;
	}

	return true;
}

bool Database::SetMQDetectionFlag(const char* accountname, const char* charactername, const char* hacked, const char* zone) { //Utilize the "hacker" table, but also give zone information.

	std::string errbuf;
	std::string query;
	uint32	affected_rows = 0;

	StringFormat(query,"INSERT INTO hackers(account,name,hacked,zone) values('%s','%s','%s','%s')", accountname, charactername, hacked, zone);

	if (!RunQuery(query, &errbuf, 0,&affected_rows)) {
		std::cerr << "Error in SetMQDetectionFlag query '" << query << "' " << errbuf << std::endl;
		return false;
	}


	if (affected_rows == 0)
	{
		return false;
	}

	return true;
}

uint8 Database::GetRaceSkill(uint8 skillid, uint8 in_race)
{
	uint16 race_cap = 0;
	std::string errbuf;
	std::string query;
	uint32	affected_rows = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	//Check for a racial cap!
	StringFormat(query,"SELECT skillcap from race_skillcaps where skill = %i && race = %i", skillid, in_race);
	if (RunQuery(query, &errbuf, &result, &affected_rows))
	{
		if (affected_rows != 0)
		{
			row = mysql_fetch_row(result);
			race_cap = atoi(row[0]);
		}
		mysql_free_result(result);
	}

	return race_cap;
}

uint8 Database::GetSkillCap(uint8 skillid, uint8 in_race, uint8 in_class, uint16 in_level)
{
	uint8 skill_level = 0, skill_formula = 0;
	uint16 base_cap = 0, skill_cap = 0, skill_cap2 = 0, skill_cap3 = 0;
	std::string errbuf;
	std::string query;
	uint32	affected_rows = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	//Fetch the data from DB.
	StringFormat(query,"SELECT level, formula, pre50cap, post50cap, post60cap from skillcaps where skill = %i && class = %i", skillid, in_class);
	if (RunQuery(query, &errbuf, &result, &affected_rows))
	{
		if (affected_rows != 0)
		{
			row = mysql_fetch_row(result);
			skill_level = atoi(row[0]);
			skill_formula = atoi(row[1]);
			skill_cap = atoi(row[2]);
			if (atoi(row[3]) > skill_cap)
				skill_cap2 = (atoi(row[3])-skill_cap)/10; //Split the post-50 skill cap into difference between pre-50 cap and post-50 cap / 10 to determine amount of points per level.
			skill_cap3 = atoi(row[4]);
		}
		mysql_free_result(result);
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
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query, "SELECT id, account_id, zonename, instanceid, x, y, z FROM character_ WHERE name='%s'", iName);
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			uint32 charid = atoi(row[0]);
			if (oAccID)
				*oAccID = atoi(row[1]);
			if (oZoneID)
				*oZoneID = GetZoneID(row[2]);
			if(oInstanceID)
				*oInstanceID = atoi(row[3]);
			if (oX)
				*oX = atof(row[4]);
			if (oY)
				*oY = atof(row[5]);
			if (oZ)
				*oZ = atof(row[6]);
			mysql_free_result(result);
			return charid;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetCharacterInfo query '" << query << "' " << errbuf << std::endl;
	}
	return 0;
}

bool Database::UpdateLiveChar(char* charname,uint32 lsaccount_id) {
	std::string errbuf;
	std::string query;

	StringFormat(query, "UPDATE account SET charname='%s' WHERE id=%i;",charname, lsaccount_id);

	if (!RunQuery(query, &errbuf)) {
		std::cerr << "Error in UpdateLiveChar query '" << query << "' " << errbuf << std::endl;
		return false;
	}
	return true;
}

bool Database::GetLiveChar(uint32 account_id, char* cname) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	StringFormat(query, "SELECT charname FROM account WHERE id=%i", account_id);
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			strcpy(cname,row[0]);
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetLiveChar query '" << query << "' " << errbuf << std::endl;
	}

	return false;
}

void Database::SetLFP(uint32 CharID, bool LFP) {

	std::string errbuf;
	std::string query;

	StringFormat(query,"update character_ set lfp=%i where id=%i",LFP, CharID);

	if (!RunQuery(query, &errbuf))
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, errbuf.c_str());


}

void Database::SetLoginFlags(uint32 CharID, bool LFP, bool LFG, uint8 firstlogon) {

	std::string errBuf;
	std::string query;

	StringFormat(query,"update character_ set lfp=%i, lfg=%i, firstlogon=%i where id=%i",LFP, LFG, firstlogon, CharID);

	if (!RunQuery(query, &errBuf))
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, errBuf.c_str());
}

void Database::SetLFG(uint32 CharID, bool LFG) {

	std::string errbuf;
	std::string query;

	StringFormat(query,"update character_ set lfg=%i where id=%i",LFG, CharID);

	if (!RunQuery(query, &errbuf))
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, errbuf.c_str());


}

void Database::SetFirstLogon(uint32 CharID, uint8 firstlogon) {

	std::string errbuf;
	std::string query;

	StringFormat(query,"update character_ set firstlogon=%i where id=%i",firstlogon, CharID);

	if (!RunQuery(query, &errbuf))
		LogFile->write(EQEMuLog::Error, "Error updating firstlogon for character %i : %s", CharID, errbuf.c_str());

}

void Database::AddReport(std::string who, std::string against, std::string lines)
{
	std::string errbuf;
	std::string query;
	std::string escape_str;
	DoEscapeString(escape_str, lines.c_str(), lines.size());

	StringFormat(query,"INSERT INTO reports (name, reported, reported_text) VALUES('%s', '%s', '%s')", who.c_str(), against.c_str(), escape_str.c_str());
	if (!RunQuery(query, &errbuf))
		LogFile->write(EQEMuLog::Error, "Error adding a report for %s: %s", who.c_str(), errbuf.c_str());

}

void Database::SetGroupID(const char* name,uint32 id, uint32 charid, uint32 ismerc){
	std::string errbuf;
	std::string query;
	if(id == 0) { //removing you from table

		StringFormat(query,"delete from group_id where charid=%i and name='%s' and ismerc=%i",charid, name, ismerc);

		if (!RunQuery(query, &errbuf))
			LogFile->write(EQEMuLog::Error, "Error deleting character from group id: %s", errbuf.c_str());
	}
	else {

		StringFormat(query, "replace into group_id set charid=%i, groupid=%i, name='%s', ismerc='%i'",charid, id, name, ismerc);

		if (!RunQuery(query, &errbuf))
			LogFile->write(EQEMuLog::Error, "Error adding character to group id: %s", errbuf.c_str());
	}
}

void Database::ClearGroup(uint32 gid) {
	ClearGroupLeader(gid);
	std::string errbuf;
	std::string query;

	if(gid == 0) { //clear all groups

		query = "delete from group_id";

		if (!RunQuery(query, &errbuf))
			printf("Unable to clear groups: %s\n",errbuf.c_str());
	} else {	//clear a specific group
		
		StringFormat(query,"delete from group_id where groupid = %lu", (unsigned long)gid);

		if (!RunQuery(query, &errbuf))
			printf("Unable to clear groups: %s\n",errbuf.c_str());
	}
}

uint32 Database::GetGroupID(const char* name){
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 groupid=0;

	StringFormat(query,"SELECT groupid from group_id where name='%s'", name);
	if (RunQuery(query, &errbuf, &result)) {
		if((row = mysql_fetch_row(result)))
		{
			if(row[0])
				groupid=atoi(row[0]);
		}
		else
			LogFile->write(EQEMuLog::Debug, "Character not in a group: %s", name);
		mysql_free_result(result);
	}
	else
	LogFile->write(EQEMuLog::Error, "Error getting group id: %s", errbuf.c_str());
	return groupid;
}

char* Database::GetGroupLeaderForLogin(const char* name,char* leaderbuf){
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	PlayerProfile_Struct pp;

	StringFormat(query, "SELECT profile from character_ where name='%s'", name);
	if (RunQuery(query, &errbuf, &result)) {
		row = mysql_fetch_row(result);
		unsigned long* lengths = mysql_fetch_lengths(result);
		if (lengths[0] == sizeof(PlayerProfile_Struct)) {
			memcpy(&pp, row[0], sizeof(PlayerProfile_Struct));
			strcpy(leaderbuf,pp.groupMembers[0]);
		}
		mysql_free_result(result);
	}
	else{
			printf("Unable to get leader name: %s\n",errbuf.c_str());
	}
	return leaderbuf;
}

void Database::SetGroupLeaderName(uint32 gid, const char* name) {
	std::string errbuf;
	std::string query;

	StringFormat(query,"Replace into group_leaders set gid=%lu, leadername='%s'",(unsigned long)gid,name);

	if (!RunQuery(query, &errbuf))
		printf("Unable to set group leader: %s\n",errbuf.c_str());

}

char *Database::GetGroupLeadershipInfo(uint32 gid, char* leaderbuf, char* maintank, char* assist, char* puller, char *marknpc, GroupLeadershipAA_Struct* GLAA){
	std::string errbuf;
	std::string query;
	MYSQL_RES* result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT leadername, maintank, assist, puller, marknpc, leadershipaa "
						"FROM group_leaders WHERE gid=%lu",(unsigned long)gid);

	if (RunQuery(query, &errbuf, &result)) {

		row = mysql_fetch_row(result);
		unsigned long* Lengths = mysql_fetch_lengths(result);

		if(row != nullptr){

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

			if(GLAA && (Lengths[5] == sizeof(GroupLeadershipAA_Struct)))
				memcpy(GLAA, row[5], sizeof(GroupLeadershipAA_Struct));

			mysql_free_result(result);
			return leaderbuf;
		}
	}

	if(leaderbuf)
		strcpy(leaderbuf, "UNKNOWN");

	if(maintank)
		maintank[0] = 0;

	if(assist)
		assist[0] = 0;

	if(puller)
		puller[0] = 0;

	if(marknpc)
		marknpc[0] = 0;

	return leaderbuf;
}

void Database::ClearGroupLeader(uint32 gid){
	std::string errbuf;
	std::string query;

	if(gid == 0) { //clear all group leaders
		query = "DELETE from group_leaders";

		if (!RunQuery(query, &errbuf))
			printf("Unable to clear group leaders: %s\n",errbuf.c_str());
	} else {	//clear a specific group leader
		StringFormat(query,"DELETE from group_leaders where gid = %lu", (unsigned long)gid);

		if (!RunQuery(query, &errbuf))
			printf("Unable to clear group leader: %s\n",errbuf.c_str());
	}
}

bool FetchRowMap(MYSQL_RES *result, std::map<std::string,std::string> &rowmap)
{
	MYSQL_FIELD *fields;
	MYSQL_ROW row;
	unsigned long num_fields,i;
	bool retval=false;
	rowmap.clear();
	if (result && (num_fields=mysql_num_fields(result)) && (row = mysql_fetch_row(result))!=nullptr && (fields = mysql_fetch_fields(result))!=nullptr) {
		retval=true;
		for(i=0;i<num_fields;i++) {
			rowmap[fields[i].name]=(row[i] ? row[i] : "");
		}
	}

	return retval;
}

uint8 Database::GetAgreementFlag(uint32 acctid)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES* result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT rulesflag FROM account WHERE id=%i",acctid);
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			uint8 flag = atoi(row[0]);
			mysql_free_result(result);
			return flag;
		}
	}
	return 0;
}

void Database::SetAgreementFlag(uint32 acctid)
{
	std::string errbuf;
	std::string query;

	StringFormat(query,"UPDATE account SET rulesflag=1 where id=%i",acctid);

	if (!RunQuery(query, &errbuf)) {
		std::cerr << "Unable to set agreement flag for account id: " << acctid << std::endl;
		std::cerr << "Error message was " << errbuf << std::endl;
	}
}

void Database::ClearRaid(uint32 rid) {
	std::string errbuf;
	std::string query;
	if(rid == 0) { //clear all raids

		query = "delete from raid_members";

		if (!RunQuery(query, &errbuf))
			printf("Unable to clear raids: %s\n",errbuf.c_str());
	} else {	//clear a specific group

		StringFormat(query,"delete from raid_members where raidid = %lu", (unsigned long)rid);

		if (!RunQuery(query, &errbuf))
			printf("Unable to clear raids: %s\n",errbuf.c_str());
	}
}

void Database::ClearRaidDetails(uint32 rid) {
	std::string errbuf;
	std::string query;

	if(rid == 0) { //clear all raids

		query = "delete from raid_details";

		if (!RunQuery(query, &errbuf))
			printf("Unable to clear raid details: %s\n",errbuf.c_str());
	} 
	else {	//clear a specific group

		StringFormat(query,"delete from raid_details where raidid = %lu", (unsigned long)rid);

		if (!RunQuery(query, &errbuf))
			printf("Unable to clear raid details: %s\n",errbuf.c_str());
	}
}

uint32 Database::GetRaidID(const char* name){
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 raidid=0;

	StringFormat(query,"SELECT raidid from raid_members where name='%s'", name);

	if (RunQuery(query, &errbuf, &result)) {
		if((row = mysql_fetch_row(result)))
		{
			if(row[0])
				raidid=atoi(row[0]);
		}
		else
			printf("Unable to get raid id, char not found!\n");
		mysql_free_result(result);
	}
	else
		printf("Unable to get raid id: %s\n",errbuf.c_str());

	return raidid;
}

const char *Database::GetRaidLeaderName(uint32 rid)
{
	static char name[128];

	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT name FROM raid_members WHERE raidid=%u AND israidleader=1", rid);

	if (RunQuery(query, &errbuf, &result)) {
		if((row = mysql_fetch_row(result)) != nullptr)
		{
			memset(name, 0, 128);
			strcpy(name, row[0]);
			mysql_free_result(result);
			return name;
		}
		else
			printf("Unable to get raid id, char not found!\n");
		mysql_free_result(result);
	}
	else
		printf("Unable to get raid id: %s\n",errbuf.c_str());

	return "UNKNOWN";
}

bool Database::VerifyInstanceAlive(uint16 instance_id, uint32 char_id)
{

	//we are not saved to this instance so set our instance to 0
	if(!GlobalInstance(instance_id) && !CharacterInInstanceGroup(instance_id, char_id))
	{
		return false;
	}

	if(CheckInstanceExpired(instance_id))
	{
		DeleteInstance(instance_id);
		return false;
	}
	return true;
}

bool Database::VerifyZoneInstance(uint32 zone_id, uint16 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;

	StringFormat(query,"SELECT id FROM instance_lockout where id=%u AND zone=%u",
						instance_id, zone_id);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) != 0)
		{
			mysql_free_result(result);
			return true;
		}
		else
		{
			mysql_free_result(result);
			return false;
		}
	}
	return false;
}

bool Database::CharacterInInstanceGroup(uint16 instance_id, uint32 char_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	bool lockout_instance_player = false;

	StringFormat(query,"SELECT charid FROM instance_lockout_player where id=%u AND charid=%u",
						instance_id, char_id);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) == 1)
		{
			lockout_instance_player = true;
		}
		mysql_free_result(result);
	}

	return lockout_instance_player;
}

void Database::DeleteInstance(uint16 instance_id)
{
	std::string errbuf;
	std::string query;

	StringFormat(query,"DELETE FROM instance_lockout WHERE id=%u", instance_id);

	RunQuery(query, &errbuf);

	StringFormat(query,"DELETE FROM instance_lockout_player WHERE id=%u", instance_id);

	RunQuery(query, &errbuf);

	StringFormat(query,"DELETE FROM respawn_times WHERE instance_id=%u", instance_id);

	RunQuery(query, &errbuf);

	StringFormat(query,"DELETE FROM spawn_condition_values WHERE instance_id=%u", instance_id);

	RunQuery(query, &errbuf);

	BuryCorpsesInInstance(instance_id);
}

bool Database::CheckInstanceExpired(uint16 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	int32 start_time = 0;
	int32 duration = 0;
	uint32 never_expires = 0;

	StringFormat(query,"SELECT start_time, duration, never_expires FROM instance_lockout WHERE id=%u", instance_id);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) != 0)
		{
			row = mysql_fetch_row(result);
			start_time = atoi(row[0]);
			duration = atoi(row[1]);
			never_expires = atoi(row[2]);
		}
		else
		{
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
	}
	else
	{
		return true;
	}

	if(never_expires == 1)
	{
		return false;
	}

	timeval tv;
	gettimeofday(&tv, nullptr);
	if((start_time + duration) <= tv.tv_sec)
	{
		return true;
	}
	return false;
}

uint32 Database::ZoneIDFromInstanceID(uint16 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 ret;

	StringFormat(query, "SELECT zone FROM instance_lockout where id=%u", instance_id);

	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) != 0)
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	return 0;
}

uint32 Database::VersionFromInstanceID(uint16 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 ret;

	StringFormat(query, "SELECT version FROM instance_lockout where id=%u", instance_id);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) != 0)
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	else
	{
		return 0;
	}
	return 0;
}

uint32 Database::GetTimeRemainingInstance(uint16 instance_id, bool &is_perma)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 start_time = 0;
	uint32 duration = 0;
	uint32 never_expires = 0;

	StringFormat(query,"SELECT start_time, duration, never_expires FROM instance_lockout WHERE id=%u",
						instance_id);
	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) != 0)
		{
			row = mysql_fetch_row(result);
			start_time = atoi(row[0]);
			duration = atoi(row[1]);
			never_expires = atoi(row[2]);
		}
		else
		{
			mysql_free_result(result);
			is_perma = false;
			return 0;
		}
		mysql_free_result(result);
	}
	else
	{
		is_perma = false;
		return 0;
	}

	if(never_expires == 1)
	{
		is_perma = true;
		return 0;
	}
	else
	{
		is_perma = false;
	}

	timeval tv;
	gettimeofday(&tv, nullptr);
	return ((start_time + duration) - tv.tv_sec);
}

bool Database::GetUnusedInstanceID(uint16 &instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query,"SELECT COUNT(*) FROM instance_lockout");
	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) != 0) {
			row = mysql_fetch_row(result);
			int count = atoi(row[0]);
			if(count == 0)
			{
				mysql_free_result(result);
				instance_id = RuleI(Zone, ReservedInstances) + 1;
				return true;
			}
		}
		else
		{
			mysql_free_result(result);
		}
		mysql_free_result(result);
	}
	else
	{
		instance_id = 0;
		return false;
	}

	int32 count = RuleI(Zone, ReservedInstances) + 1;
	int32 max = 65535;

	StringFormat(query,"SELECT id FROM instance_lockout where id >= %i ORDER BY id", count);

	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) != 0)
		{
			while((row = mysql_fetch_row(result)))
			{
				if(count < atoi(row[0]))
				{
					instance_id = count;
					mysql_free_result(result);
					return true;
				}
				else if(count > max)
				{
					instance_id = 0;
					mysql_free_result(result);
					return false;
				}
				else
				{
					count++;
				}
			}
		}
		else
		{
			mysql_free_result(result);
		}
	}
	instance_id = count;
	return true;
}

//perhaps purge any expireds too
bool Database::CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration)
{
	std::string errbuf;
	std::string query;

	StringFormat(query, "INSERT INTO instance_lockout (id, zone, version, start_time, duration)"
						" values(%lu, %lu, %lu, UNIX_TIMESTAMP(), %lu)", 
						(unsigned long)instance_id, (unsigned long)zone_id, 
						(unsigned long)version, (unsigned long)duration);
	
	return RunQuery(query, &errbuf);
}

void Database::PurgeExpiredInstances()
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	uint16 id = 0;
	query = "SELECT id FROM instance_lockout where "
			"(start_time+duration) <= UNIX_TIMESTAMP() and never_expires = 0";

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) > 0)
		{
			row = mysql_fetch_row(result);
			while(row != nullptr)
			{
				id = atoi(row[0]);
				DeleteInstance(id);
				row = mysql_fetch_row(result);
			}
		}
		mysql_free_result(result);
	}
}

bool Database::AddClientToInstance(uint16 instance_id, uint32 char_id)
{
	std::string errbuf;
	std::string query;

	StringFormat(query, "INSERT INTO instance_lockout_player(id, charid) "
						"values(%lu, %lu)", (unsigned long)instance_id, (unsigned long)char_id);

	return RunQuery(query, &errbuf);
}

bool Database::RemoveClientFromInstance(uint16 instance_id, uint32 char_id)
{
	std::string errbuf;
	std::string query;

	StringFormat(query, "DELETE FROM instance_lockout_player WHERE id=%lu AND charid=%lu",
						(unsigned long)instance_id, (unsigned long)char_id);

	return RunQuery(query, &errbuf);
}

bool Database::RemoveClientsFromInstance(uint16 instance_id)
{
	std::string errbuf;
	std::string query;

	StringFormat(query, "DELETE FROM instance_lockout_player WHERE id=%lu",
						(unsigned long)instance_id);

	return RunQuery(query, &errbuf);
}

bool Database::CheckInstanceExists(uint16 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;

	StringFormat(query,"SELECT * FROM instance_lockout where id=%u", instance_id);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) != 0)
		{
			mysql_free_result(result);
			return true;
		}
		mysql_free_result(result);
		return false;
	}

	return false;
}

void Database::BuryCorpsesInInstance(uint16 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;

	StringFormat(query,"UPDATE player_corpses SET IsBurried=1, instanceid=0 WHERE instanceid=%u",instance_id);

	if(RunQuery(query, &errbuf, &result))
	{
		mysql_free_result(result);
	}
}

uint16 Database::GetInstanceVersion(uint16 instance_id)
{
	if(instance_id < 1)
		return 0;

	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 ret;

	StringFormat(query,"SELECT version FROM instance_lockout where id=%u", instance_id);

	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) != 0)
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}

	return 0;
}

uint16 Database::GetInstanceID(const char* zone, uint32 charid, int16 version)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint16 ret;

	StringFormat(query, "SELECT instance_lockout.id FROM instance_lockout, instance_lockout_player "
						"WHERE instance_lockout.zone=%u AND instance_lockout.version=%u AND "
						"instance_lockout.id=instance_lockout_player.id AND "
						"instance_lockout_player.charid=%u LIMIT 1;", 
						GetZoneID(zone), version, charid, charid);

	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) != 0)
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	return 0;
}

uint16 Database::GetInstanceID(uint32 zone, uint32 charid, int16 version)
{
	if(!zone)
		return 0;

	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint16 ret;

	StringFormat(query, "SELECT instance_lockout.id FROM instance_lockout, instance_lockout_player "
						"WHERE instance_lockout.zone=%u AND instance_lockout.version=%u AND "
						"instance_lockout.id=instance_lockout_player.id AND "
						"instance_lockout_player.charid=%u LIMIT 1;", zone, version, charid);

	if (RunQuery(query, &errbuf, &result))
	{
		if (mysql_num_rows(result) != 0)
		{
			row = mysql_fetch_row(result);
			ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		else
		{
			mysql_free_result(result);
			return 0;
		}
	}
	return 0;
}

void Database::AssignGroupToInstance(uint32 gid, uint32 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 zone_id = ZoneIDFromInstanceID(instance_id);
	uint16 version = VersionFromInstanceID(instance_id);

	StringFormat(query,"SELECT charid FROM group_id WHERE groupid=%u", gid);

	if (RunQuery(query, &errbuf, &result)) {
		while((row = mysql_fetch_row(result)) != nullptr)
		{
			uint32 charid = atoi(row[0]);
			if(GetInstanceID(zone_id, charid, version) == 0)
			{
				AddClientToInstance(instance_id, charid);
			}
		}
		mysql_free_result(result);
	}
}

void Database::AssignRaidToInstance(uint32 rid, uint32 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 zone_id = ZoneIDFromInstanceID(instance_id);
	uint16 version = VersionFromInstanceID(instance_id);

	StringFormat(query,"SELECT charid FROM raid_members WHERE raidid=%u", rid);

	if (RunQuery(query, &errbuf, &result)) {
		while((row = mysql_fetch_row(result)) != nullptr)
		{
			uint32 charid = atoi(row[0]);
			if(GetInstanceID(zone_id, charid, version) == 0)
			{
				AddClientToInstance(instance_id, charid);
			}
		}
		mysql_free_result(result);
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
	std::string errbuf;
	std::string query;

	StringFormat(query, "UPDATE `instance_lockout` SET start_time=UNIX_TIMESTAMP(), "
						"duration=%u WHERE id=%u", new_duration, instance_id);

	RunQuery(query, &errbuf);
}

bool Database::GlobalInstance(uint16 instance_id)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query, "SELECT is_global from instance_lockout where id=%u LIMIT 1", instance_id);

	if (RunQuery(query, &errbuf, &result))
	{
		row = mysql_fetch_row(result);
		if(row)
		{
			return (atoi(row[0]) == 1) ? true : false;
		}
		else
		{
			mysql_free_result(result);
			return false;
		}
	}

	return false;
}

void Database::UpdateAdventureStatsEntry(uint32 char_id, uint8 theme, bool win)
{
	std::string errbuf;
	std::string query;
	uint32 affected = 0;

	std::string field;

	if(win)
	{
		switch(theme)
		{
			case 1:
			{
				field = "guk_wins";
				break;
			}
			case 2:
			{
				field = "mir_wins";
				break;
			}
			case 3:
			{
				field = "mmc_wins";
				break;
			}
			case 4:
			{
				field = "ruj_wins";
				break;
			}
			case 5:
			{
				field = "tak_wins";
				break;
			}
			default:
			{
				return;
			}
		}
	}
	else
	{
		switch(theme)
		{
			case 1:
			{
				field = "guk_losses";
				break;
			}
			case 2:
			{
				field = "mir_losses";
				break;
			}
			case 3:
			{
				field = "mmc_losses";
				break;
			}
			case 4:
			{
				field = "ruj_losses";
				break;
			}
			case 5:
			{
				field = "tak_losses";
				break;
			}
			default:
			{
				return;
			}
		}
	}

	StringFormat(query, "UPDATE `adventure_stats` SET %s=%s+1 WHERE player_id=%u",
						field.c_str(), field.c_str(), char_id);

	RunQuery(query, &errbuf, nullptr, &affected);

	if(affected == 0)
	{

		StringFormat(query, "INSERT INTO `adventure_stats` SET %s=1, player_id=%u",
							field.c_str(), char_id);

		RunQuery(query, &errbuf);
	}
}

bool Database::GetAdventureStats(uint32 char_id, uint32 &guk_w, uint32 &mir_w, uint32 &mmc_w, uint32 &ruj_w,
								uint32 &tak_w, uint32 &guk_l, uint32 &mir_l, uint32 &mmc_l, uint32 &ruj_l, uint32 &tak_l)
{
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	MYSQL_ROW row;

	StringFormat(query, "SELECT `guk_wins`, `mir_wins`, `mmc_wins`, `ruj_wins`, `tak_wins`, "
						"`guk_losses`, `mir_losses`, `mmc_losses`, `ruj_losses`, `tak_losses` "
						"FROM `adventure_stats` WHERE player_id=%u", char_id);

	if (RunQuery(query, &errbuf, &result))
	{
		while((row = mysql_fetch_row(result)) != nullptr)
		{
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
		}
		mysql_free_result(result);
		return true;
	}

	return false;
}

uint32 Database::GetGuildDBIDByCharID(uint32 char_id) {
	std::string errbuf;
	std::string query;
	MYSQL_RES *result;
	int retVal = 0;

	StringFormat(query, "SELECT guild_id FROM guild_members WHERE char_id='%i'", char_id);

	if (RunQuery(query, &errbuf, &result)) {
		if (mysql_num_rows(result) == 1) {
			MYSQL_ROW row = mysql_fetch_row(result);
			retVal = atoi(row[0]);
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetAccountIDByChar query '" << query << "' " << errbuf << std::endl;
	}

	return retVal;
}
