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
#include "string_util.h"
#include "extprofile.h"
extern Client client;


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
	char errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(host, user, passwd, database, port, &errnum, errbuf))
	{
		LogFile->write(EQEMuLog::Error, "Failed to connect to database: Error: %s", errbuf);
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

bool Database::AddBannedIP(char* bannedIP, const char* notes)
{
	std::string query = StringFormat("INSERT into Banned_IPs SET ip_address='%s', notes='%s'", bannedIP, notes);

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in ReserveName query '" << query << "' " << results.ErrorMessage() << std::endl;
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

void Database::LoginIP(uint32 AccountID, const char* LoginIP)
{
	std::string query = StringFormat("INSERT INTO account_ip SET accid=%i, ip='%s' ON DUPLICATE KEY UPDATE count=count+1, lastused=now()", AccountID, LoginIP);

	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cerr << "Error in Log IP query '" << query << "' " << results.ErrorMessage() << std::endl;
}

int16 Database::CheckStatus(uint32 account_id)
{
	std::string query = StringFormat("SELECT `status`, UNIX_TIMESTAMP(`suspendeduntil`) as `suspendeduntil`, UNIX_TIMESTAMP() as `current`"
							" FROM `account` WHERE `id` = %i", account_id);

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
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

	if (!results.Success())
	{
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
	std::string query = StringFormat("UPDATE account SET password=MD5('%s') where id=%i;", password, accid);

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

bool Database::ReserveName(uint32 account_id, char* name)
{
	std::string query = StringFormat("INSERT into character_ SET account_id=%i, name='%s', profile=NULL", account_id, name);

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in ReserveName query '" << query << "' " << results.ErrorMessage() << std::endl;
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
	std::string query=StringFormat("SELECT id from character_ WHERE name='%s'", name);
	int charid;

	if(!name ||	!strlen(name))
	{
		std::cerr << "DeleteCharacter: request to delete without a name (empty char slot)" << std::endl;
		return false;
	}

// get id from character_ before deleting record so we can clean up inventory and qglobal

#if DEBUG >= 5
	std::cout << "DeleteCharacter: Attempting to delete '" << name << "'" << std::endl;
#endif

	auto results = QueryDatabase(query);

	if(results.RowCount() != 1)
	{
		std::cerr << "DeleteCharacter error: got " << results.RowCount() << " rows matching '" << name << "'" << std::endl;
		return false;
	}

	auto row = results.begin();
	charid = atoi(row[0]);

#if DEBUG >= 5
	std::cout << "DeleteCharacter: found '" <<  name << "' with char id: " << charid << std::endl;
	std::cout << "DeleteCharacter: deleting << '" << name << "' (id " << charid << "): " << std::endl;
	std::cout << " quest_globals";
#endif

	query = StringFormat("DELETE from quest_globals WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " character_tasks";
#endif

	query = StringFormat("DELETE from character_tasks WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " character_activities";
#endif

	query = StringFormat("DELETE from character_activities WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " character_enabledtasks";
#endif

	query = StringFormat("DELETE from character_enabledtasks WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " completed_tasks";
#endif

	query = StringFormat("DELETE from completed_tasks WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " friends";
#endif

	query = StringFormat("DELETE from friends WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " mail";
#endif

	query = StringFormat( "DELETE from mail WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " ptimers";
#endif

	query = StringFormat("DELETE from timers WHERE char_id='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " inventory";
#endif

	query = StringFormat("DELETE from inventory WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " guild_members";
#endif

#ifdef BOTS
	query = StringFormat("DELETE FROM guild_members WHERE char_id='%d' AND GetMobTypeById(%i) = 'C'", charid);
#else
	query = StringFormat("DELETE FROM guild_members WHERE char_id='%d'", charid);
#endif
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " recipes";
#endif

	query = StringFormat("DELETE FROM char_recipe_list WHERE char_id='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " adventure_stats";
#endif

	query = StringFormat("DELETE FROM adventure_stats WHERE player_id='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " zone_flags";
#endif

	query = StringFormat("DELETE FROM zone_flags WHERE charID='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " titles";
#endif

	query = StringFormat("DELETE FROM titles WHERE char_id='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " titlesets";
#endif

	query = StringFormat("DELETE FROM player_titlesets WHERE char_id='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " keyring";
#endif

	query = StringFormat("DELETE FROM keyring WHERE char_id='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " factions";
#endif

	query = StringFormat("DELETE FROM faction_values WHERE char_id='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " instances";
#endif

	query = StringFormat("DELETE FROM instance_list_player WHERE charid='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << " _character";
#endif

	query = StringFormat("DELETE from character_ WHERE id='%d'", charid);
	results = QueryDatabase(query);

	if(results.RowsAffected() != 1)	// here we have to have a match or it's an error
	{
		LogFile->write(EQEMuLog::Error, "DeleteCharacter: error: delete operation affected %d rows\n", results.RowsAffected());
		return false;
	}

#if DEBUG >= 5
	std::cout << " alternate currency";
#endif

	query = StringFormat("DELETE FROM character_alt_currency WHERE char_id='%d'", charid);
	QueryDatabase(query);

#if DEBUG >= 5
	std::cout << std::endl;
#endif
	std::cout << "DeleteCharacter: successfully deleted '" << name << "' (id " << charid << ")" << std::endl;

	return true;
}

// Store new character information into the character_ and inventory tables
bool Database::StoreCharacter(uint32 account_id, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext)
{
	char query[256+sizeof(PlayerProfile_Struct)*2+sizeof(ExtendedProfile_Struct)*2+5];
	char* end = query;
	uint32 charid = 0;
	char zone[50];
	float x, y, z;

	charid = GetCharacterID(pp->name);

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

	// construct the character_ query
	end += sprintf(end,
		"UPDATE character_ SET timelaston=0, "
		"zonename=\'%s\', x=%f, y=%f, z=%f, profile=\'",
		zone, x, y, z
	);
	end += DoEscapeString(end, (char*)pp, sizeof(PlayerProfile_Struct));
	end += sprintf(end, "\', extprofile=\'");
	end += DoEscapeString(end, (char*)ext, sizeof(ExtendedProfile_Struct));
	end += sprintf(end, "\' WHERE account_id=%d AND name='%s'",account_id, pp->name);

	auto results = QueryDatabase(query, (uint32) (end - query));
	// stack assigned query, no need to delete it.

	if(!results.RowsAffected())
	{
		LogFile->write(EQEMuLog::Error, "StoreCharacter query '%s' %s", query, results.ErrorMessage().c_str());
		return false;
	}

	// now the inventory
	std::string invquery;
	for (int16 i=EmuConstants::EQUIPMENT_BEGIN; i<=EmuConstants::BANK_BAGS_END;)
	{
		const ItemInst* newinv = inv->GetItem(i);
		if (newinv)
		{
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
	std::string query = StringFormat("SELECT account_id, id FROM character_ WHERE name='%s'", charname);

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
	std::string query = StringFormat("SELECT account_id FROM character_ WHERE id=%i", char_id);

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
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

	std::string query = StringFormat("SELECT id, status, lsaccount_id FROM account WHERE name='%s'", accname);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetAccountIDByAcc query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();

	uint32 id = atoi(row[0]);

	if (status)
		*status = atoi(row[1]);

	if (lsid)
	{
		if (row[2])
			*lsid = atoi(row[2]);
		else
			*lsid = 0;
	}

	return id;
}

void Database::GetAccountName(uint32 accountid, char* name, uint32* oLSAccountID) {
	std::string query = StringFormat("SELECT name, lsaccount_id FROM account WHERE id='%i'", accountid);

	auto results = QueryDatabase(query);

	if (!results.Success())
	{
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
	
	std::string query = StringFormat("SELECT name FROM character_ WHERE id='%i'", char_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetCharName query '" << query << "' " << results.ErrorMessage() << std::endl;
		return;
	}

	auto row = results.begin();
	for (auto row = results.begin(); row != results.end(); ++row) {
		strcpy(name, row[0]);
	}
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

	std::string query = StringFormat("SELECT id, name, status FROM account WHERE lsaccount_id=%i", iLSID);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetAccountIDFromLSID query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();

	uint32 account_id = atoi(row[0]);

	if (oAccountName)
		strcpy(oAccountName, row[1]);
	if (oStatus)
		*oStatus = atoi(row[2]);

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

	std::string query = StringFormat("UPDATE character_ SET name='%s' WHERE name='%s';", newname, oldname);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowsAffected() == 0)
		return false;

	return true;
}

// If the name is used or an error occurs, it returns false, otherwise it returns true
bool Database::CheckUsedName(const char* name)
{
	std::string query = StringFormat("SELECT id FROM character_ where name='%s'", name);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in CheckUsedName query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	if (results.RowCount() > 0)
		return false;

	return true;
}

uint8 Database::GetServerType()
{
	std::string query("SELECT value FROM variables WHERE varname='ServerType'");
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetServerType query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

bool Database::MoveCharacterToZone(const char* charname, const char* zonename,uint32 zoneid) {
	if(zonename == nullptr || strlen(zonename) == 0)
		return false;

	std::string query = StringFormat("UPDATE character_ SET zonename = '%s',zoneid=%i,x=-1, y=-1, z=-1 WHERE name='%s'", zonename,zoneid, charname);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
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

	std::string query = StringFormat("UPDATE character_ SET zonename = '%s', zoneid=%i, x=-1, y=-1, z=-1 WHERE id=%i", iZonename, GetZoneID(iZonename), iCharID);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in MoveCharacterToZone(id) query '" << query << "' " << results.ErrorMessage() << std::endl;
		return false;
	}

	return results.RowsAffected() != 0;
}

uint8 Database::CopyCharacter(const char* oldname, const char* newname, uint32 acctid) {
	
	PlayerProfile_Struct* pp;
	ExtendedProfile_Struct* ext;

	std::string query = StringFormat("SELECT profile, extprofile FROM character_ WHERE name='%s'", oldname);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in CopyCharacter read query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	auto row = results.begin();

	pp = (PlayerProfile_Struct*)row[0];
	strcpy(pp->name, newname);

	ext = (ExtendedProfile_Struct*)row[1];

	char query2[276 + sizeof(PlayerProfile_Struct)*2 + sizeof(ExtendedProfile_Struct)*2 + 1];
	char* end=query2;

	end += sprintf(end, "INSERT INTO character_ SET zonename=\'%s\', x = %f, y = %f, z = %f, profile=\'", GetZoneName(pp->zone_id), pp->x, pp->y, pp->z);
	end += DoEscapeString(end, (char*) pp, sizeof(PlayerProfile_Struct));
	end += sprintf(end,"\', extprofile=\'");
	end += DoEscapeString(end, (char*) ext, sizeof(ExtendedProfile_Struct));
	end += sprintf(end, "\', account_id=%d, name='%s'", acctid, newname);

	results = QueryDatabase(query2, (uint32) (end - query2));

	if (!results.Success())
	{
		std::cerr << "Error in CopyCharacter query '" << query2 << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowsAffected() == 0)
		return 0;

	return 1;
}

bool Database::SetHackerFlag(const char* accountname, const char* charactername, const char* hacked) {
	
	std::string query = StringFormat("INSERT INTO hackers(account,name,hacked) values('%s','%s','%s')", accountname, charactername, hacked);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
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
	
	std::string query = StringFormat("SELECT id, account_id, zonename, instanceid, x, y, z FROM character_ WHERE name='%s'", iName);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cerr << "Error in GetCharacterInfo query '" << query << "' " << results.ErrorMessage() << std::endl;
		return 0;
	}

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();

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

	std::string query = StringFormat("update character_ set lfp=%i where id=%i",LFP, CharID);
	auto results = QueryDatabase(query);

	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, results.ErrorMessage().c_str());
}

void Database::SetLoginFlags(uint32 CharID, bool LFP, bool LFG, uint8 firstlogon) {
	
	std::string query = StringFormat("update character_ set lfp=%i, lfg=%i, firstlogon=%i where id=%i",LFP, LFG, firstlogon, CharID);
	auto results = QueryDatabase(query);

	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, results.ErrorMessage().c_str());
}

void Database::SetLFG(uint32 CharID, bool LFG) {

	std::string query = StringFormat("update character_ set lfg=%i where id=%i",LFG, CharID);
	auto results = QueryDatabase(query);

	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error updating LFP for character %i : %s", CharID, results.ErrorMessage().c_str());
}

void Database::SetFirstLogon(uint32 CharID, uint8 firstlogon) {
	
	std::string query = StringFormat( "update character_ set firstlogon=%i where id=%i",firstlogon, CharID);
	auto results = QueryDatabase(query);

	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error updating firstlogon for character %i : %s", CharID, results.ErrorMessage().c_str());
}

void Database::AddReport(std::string who, std::string against, std::string lines)
{
	
	char *escape_str = new char[lines.size()*2+1];
	DoEscapeString(escape_str, lines.c_str(), lines.size());

	std::string query = StringFormat("INSERT INTO reports (name, reported, reported_text) VALUES('%s', '%s', '%s')", who.c_str(), against.c_str(), escape_str);
	auto results = QueryDatabase(query);
	safe_delete_array(escape_str);

	if (!results.Success())
		LogFile->write(EQEMuLog::Error, "Error adding a report for %s: %s", who.c_str(), results.ErrorMessage().c_str());
}

void Database::SetGroupID(const char* name, uint32 id, uint32 charid, uint32 ismerc){
	
	std::string query;
	if (id == 0)
	{
		// removing from group
		query = StringFormat("delete from group_id where charid=%i and name='%s' and ismerc=%i",charid, name, ismerc);
		auto results = QueryDatabase(query);

		if (!results.Success())
			LogFile->write(EQEMuLog::Error, "Error deleting character from group id: %s", results.ErrorMessage().c_str());

		return;
	}

	// adding to group
	query = StringFormat("replace into group_id set charid=%i, groupid=%i, name='%s', ismerc='%i'",charid, id, name, ismerc);
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

char* Database::GetGroupLeaderForLogin(const char* name,char* leaderbuf){
	
	PlayerProfile_Struct pp;

	std::string query = StringFormat("SELECT profile from character_ where name='%s'", name);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		std::cout << "Unable to get leader name: " << results.ErrorMessage() << std::endl;
		return leaderbuf;
	}

	if (results.LengthOfColumn(0) != sizeof(PlayerProfile_Struct))
		return leaderbuf;

	auto row = results.begin();

	memcpy(&pp, row[0], sizeof(PlayerProfile_Struct));
	strcpy(leaderbuf,pp.groupMembers[0]);

	return leaderbuf;
}

void Database::SetGroupLeaderName(uint32 gid, const char* name) {

	std::string query = StringFormat("Replace into group_leaders set gid=%lu, leadername='%s'",(unsigned long)gid,name);
	auto results = QueryDatabase(query);

	if (!results.Success())
		std::cout << "Unable to set group leader: " << results.ErrorMessage() << std::endl;
}

char *Database::GetGroupLeadershipInfo(uint32 gid, char* leaderbuf, char* maintank, char* assist, char* puller, char *marknpc, GroupLeadershipAA_Struct* GLAA){
	
	std::string query = StringFormat("SELECT leadername, maintank, assist, puller, marknpc, leadershipaa FROM group_leaders WHERE gid=%lu",(unsigned long)gid);
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() == 0)
	{
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
void Database::ClearAllGroupLeaders(void)
{
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

uint8 Database::GetAgreementFlag(uint32 acctid)
{

	std::string query = StringFormat("SELECT rulesflag FROM account WHERE id=%i",acctid);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() != 1)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

void Database::SetAgreementFlag(uint32 acctid)
{
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

void Database::ClearAllRaids(void)
{

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
