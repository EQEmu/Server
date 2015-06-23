/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/eqemu_logsys.h"
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
#include <vector>

// Disgrace: for windows compile
#ifdef _WINDOWS
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "../common/unix.h"
#include <netinet/in.h>
#endif

#include "database.h"
#include "../common/eq_packet_structs.h"
#include "../common/string_util.h"
#include "../common/servertalk.h"

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
		Log.Out(Logs::General, Logs::Error, "Failed to connect to database: Error: %s", errbuf);
		HandleMysqlError(errnum);

		return false;
	}
	else
	{
		Log.Out(Logs::General, Logs::Status, "Using database '%s' at %s:%d",database,host,port);
		return true;
	}
}

void Database::DBInitVars() {

}



void Database::HandleMysqlError(uint32 errnum) {
}

/*

Close the connection to the database
*/
Database::~Database()
{
}

void Database::AddSpeech(const char* from, const char* to, const char* message, uint16 minstatus, uint32 guilddbid, uint8 type) {

	char *escapedFrom = new char[strlen(from) * 2 + 1];
	char *escapedTo = new char[strlen(to) * 2 + 1];
	char *escapedMessage = new char[strlen(message) * 2 + 1];
	DoEscapeString(escapedFrom, from, strlen(from));
	DoEscapeString(escapedTo, to, strlen(to));
	DoEscapeString(escapedMessage, message, strlen(message));

    std::string query = StringFormat("INSERT INTO `qs_player_speech` "
                                    "SET `from` = '%s', `to` = '%s', `message`='%s', "
                                    "`minstatus`='%i', `guilddbid`='%i', `type`='%i'",
                                    escapedFrom, escapedTo, escapedMessage, minstatus, guilddbid, type);
    safe_delete_array(escapedFrom);
	safe_delete_array(escapedTo);
	safe_delete_array(escapedMessage);
	auto results = QueryDatabase(query);
	if(!results.Success()) {
		Log.Out(Logs::Detail, Logs::QS_Server, "Failed Speech Entry Insert: %s", results.ErrorMessage().c_str());
		Log.Out(Logs::Detail, Logs::QS_Server, "%s", query.c_str());
	}


}

void Database::GeneralQueryReceive(ServerPacket *pack) {
	/*
		These are general queries passed from anywhere in zone instead of packing structures and breaking them down again and again
	*/
	char *queryBuffer = new char[pack->ReadUInt32() + 1];
	pack->ReadString(queryBuffer);

	std::string query(queryBuffer);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		Log.Out(Logs::Detail, Logs::QS_Server, "Failed Delete Log Record Insert: %s", results.ErrorMessage().c_str());
		Log.Out(Logs::Detail, Logs::QS_Server, "%s", query.c_str());
	}

	safe_delete(pack);
	safe_delete(queryBuffer);
}

void Database::LoadLogSettings(EQEmuLogSys::LogSettings* log_settings){
	std::string query =
		"SELECT "
		"log_category_id, "
		"log_category_description, "
		"log_to_console, "
		"log_to_file, "
		"log_to_gmsay "
		"FROM "
		"logsys_categories "
		"ORDER BY log_category_id";
	auto results = QueryDatabase(query);

	int log_category = 0;
	Log.file_logs_enabled = false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		log_category = atoi(row[0]);
		log_settings[log_category].log_to_console = atoi(row[2]);
		log_settings[log_category].log_to_file = atoi(row[3]);
		log_settings[log_category].log_to_gmsay = atoi(row[4]);

		/* Determine if any output method is enabled for the category
			and set it to 1 so it can used to check if category is enabled */
		const bool log_to_console = log_settings[log_category].log_to_console > 0;
		const bool log_to_file = log_settings[log_category].log_to_file > 0;
		const bool log_to_gmsay = log_settings[log_category].log_to_gmsay > 0;
		const bool is_category_enabled = log_to_console || log_to_file || log_to_gmsay;

		if (is_category_enabled)
			log_settings[log_category].is_category_enabled = 1;

		/*
		This determines whether or not the process needs to actually file log anything.
		If we go through this whole loop and nothing is set to any debug level, there is no point to create a file or keep anything open
		*/
		if (log_settings[log_category].log_to_file > 0){
			Log.file_logs_enabled = true;
		}
	}
}