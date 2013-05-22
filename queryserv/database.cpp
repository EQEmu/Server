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


#include "../common/debug.h"
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
#include "../common/unix.h"
#include <netinet/in.h>
#endif

#include "database.h"
#include "../common/eq_packet_structs.h"
#include "../common/MiscFunctions.h"
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

}



void Database::HandleMysqlError(uint32 errnum) {
}

/*

Close the connection to the database
*/
Database::~Database()
{
}

bool Database::GetVariable(const char* varname, char* varvalue, uint16 varvalue_len) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (!RunQuery(query,MakeAnyLenString(&query, "select `value` from `variables` where `varname`='%s'", varname), errbuf, &result)) {

		_log(UCS__ERROR, "Unable to get message count from database. %s %s", query, errbuf);

		safe_delete_array(query);

		return false;
	}

	safe_delete_array(query);

	if (mysql_num_rows(result) != 1) {

		mysql_free_result(result);

		return false;
	}

	row = mysql_fetch_row(result);

	snprintf(varvalue, varvalue_len, "%s", row[0]);

	mysql_free_result(result);

	return true;
}


void Database::AddSpeech(const char* from, const char* to, const char* message, uint16 minstatus, uint32 guilddbid, uint8 type) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	char *S1 = new char[strlen(from) * 2 + 1];
	char *S2 = new char[strlen(to) * 2 + 1];
	char *S3 = new char[strlen(message) * 2 + 1];
	DoEscapeString(S1, from, strlen(from));
	DoEscapeString(S2, to, strlen(to));
	DoEscapeString(S3, message, strlen(message));

	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_speech` SET `from`='%s', `to`='%s', `message`='%s', `minstatus`='%i', `guilddbid`='%i', `type`='%i'", S1, S2, S3, minstatus, guilddbid, type), errbuf, 0, 0)) {
		_log(NET__WORLD, "Failed Speech Entry Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	safe_delete_array(query);
	safe_delete_array(S1);
	safe_delete_array(S2);
	safe_delete_array(S3);
}

void Database::LogPlayerTrade(QSPlayerLogTrade_Struct* QS, uint32 Items) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 lastid = 0;
	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_trade_record` SET `time`=NOW(), "
		"`char1_id`='%i', `char1_pp`='%i', `char1_gp`='%i', `char1_sp`='%i', `char1_cp`='%i', `char1_items`='%i', "
		"`char2_id`='%i', `char2_pp`='%i', `char2_gp`='%i', `char2_sp`='%i', `char2_cp`='%i', `char2_items`='%i'",
		QS->char1_id, QS->char1_money.platinum, QS->char1_money.gold, QS->char1_money.silver, QS->char1_money.copper, QS->char1_count,
		QS->char2_id, QS->char2_money.platinum, QS->char2_money.gold, QS->char2_money.silver, QS->char2_money.copper, QS->char2_count),
		errbuf, 0, 0, &lastid)) {
		_log(NET__WORLD, "Failed Trade Log Record Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	if(Items > 0) {
		for(int i = 0; i < Items; i++) {
			if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_trade_record_entries` SET `event_id`='%i', "
				"`from_id`='%i', `from_slot`='%i', `to_id`='%i', `to_slot`='%i', `item_id`='%i', "
				"`charges`='%i', `aug_1`='%i', `aug_2`='%i', `aug_3`='%i', `aug_4`='%i', `aug_5`='%i'",
				lastid, QS->items[i].from_id, QS->items[i].from_slot, QS->items[i].to_id, QS->items[i].to_slot, QS->items[i].item_id,
				QS->items[i].charges, QS->items[i].aug_1, QS->items[i].aug_2, QS->items[i].aug_3, QS->items[i].aug_4, QS->items[i].aug_5,
				errbuf, 0, 0))) {
				_log(NET__WORLD, "Failed Trade Log Record Entry Insert: %s", errbuf);
				_log(NET__WORLD, "%s", query);
			}
		}
	}
}

void Database::LogPlayerHandin(QSPlayerLogHandin_Struct* QS, uint32 Items) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 lastid = 0;
	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_handin_record` SET `time`=NOW(), `quest_id`='%i', "
		"`char_id`='%i', `char_pp`='%i', `char_gp`='%i', `char_sp`='%i', `char_cp`='%i', `char_items`='%i', "
		"`npc_id`='%i', `npc_pp`='%i', `npc_gp`='%i', `npc_sp`='%i', `npc_cp`='%i', `npc_items`='%i'",
		QS->quest_id, QS->char_id, QS->char_money.platinum, QS->char_money.gold, QS->char_money.silver, QS->char_money.copper, QS->char_count,
		QS->npc_id, QS->npc_money.platinum, QS->npc_money.gold, QS->npc_money.silver, QS->npc_money.copper, QS->npc_count),
		errbuf, 0, 0, &lastid)) {
		_log(NET__WORLD, "Failed Handin Log Record Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	if(Items > 0) {
		for(int i = 0; i < Items; i++) {
			if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_handin_record_entries` SET `event_id`='%i', "
				"`action_type`='%s', `char_slot`='%i', `item_id`='%i', `charges`='%i', "
				"`aug_1`='%i', `aug_2`='%i', `aug_3`='%i', `aug_4`='%i', `aug_5`='%i'",
				lastid, QS->items[i].action_type, QS->items[i].char_slot, QS->items[i].item_id, QS->items[i].charges,
				QS->items[i].aug_1, QS->items[i].aug_2, QS->items[i].aug_3, QS->items[i].aug_4, QS->items[i].aug_5,
				errbuf, 0, 0))) {
				_log(NET__WORLD, "Failed Handin Log Record Entry Insert: %s", errbuf);
				_log(NET__WORLD, "%s", query);
			}
		}
	}
}

void Database::LogPlayerNPCKill(QSPlayerLogNPCKill_Struct* QS, uint32 Members){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 lastid = 0;
	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_npc_kill_record` SET `npc_id`='%i', `type`='%i', `zone_id`='%i', `time`=NOW()", QS->s1.NPCID, QS->s1.Type, QS->s1.ZoneID), errbuf, 0, 0, &lastid)) {
		_log(NET__WORLD, "Failed NPC Kill Log Record Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	if(Members > 0){
		for (int i = 0; i < Members; i++) {
			if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_npc_kill_record_entries` SET `event_id`='%i', `char_id`='%i'", lastid, QS->Chars[i].char_id, errbuf, 0, 0))) {
				_log(NET__WORLD, "Failed NPC Kill Log Entry Insert: %s", errbuf);
				_log(NET__WORLD, "%s", query);
			}
		}
	}
}

void Database::LogPlayerDelete(QSPlayerLogDelete_Struct* QS, uint32 Items) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 lastid = 0;
	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_delete_record` SET `time`=NOW(), "
		"`char_id`='%i', `stack_size`='%i', `char_items`='%i'",
		QS->char_id, QS->stack_size, QS->char_count, QS->char_count),
		errbuf, 0, 0, &lastid)) {
		_log(NET__WORLD, "Failed Delete Log Record Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	if(Items > 0) {
		for(int i = 0; i < Items; i++) {
			if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_delete_record_entries` SET `event_id`='%i', "
				"`char_slot`='%i', `item_id`='%i', `charges`='%i', `aug_1`='%i', "
				"`aug_2`='%i', `aug_3`='%i', `aug_4`='%i', `aug_5`='%i'",
				lastid, QS->items[i].char_slot, QS->items[i].item_id, QS->items[i].charges, QS->items[i].aug_1,
				QS->items[i].aug_2, QS->items[i].aug_3, QS->items[i].aug_4, QS->items[i].aug_5,
				errbuf, 0, 0))) {
				_log(NET__WORLD, "Failed Delete Log Record Entry Insert: %s", errbuf);
				_log(NET__WORLD, "%s", query);
			}
		}
	}
}

void Database::LogPlayerMove(QSPlayerLogMove_Struct* QS, uint32 Items) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 lastid = 0;
	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_move_record` SET `time`=NOW(), "
		"`char_id`='%i', `from_slot`='%i', `to_slot`='%i', `stack_size`='%i', `char_items`='%i', `postaction`='%i'",
		QS->char_id, QS->from_slot, QS->to_slot, QS->stack_size, QS->char_count, QS->postaction),
		errbuf, 0, 0, &lastid)) {
		_log(NET__WORLD, "Failed Move Log Record Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	if(Items > 0) {
		for(int i = 0; i < Items; i++) {
			if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_player_move_record_entries` SET `event_id`='%i', "
				"`from_slot`='%i', `to_slot`='%i', `item_id`='%i', `charges`='%i', "
				"`aug_1`='%i', `aug_2`='%i', `aug_3`='%i', `aug_4`='%i', `aug_5`='%i'", lastid,
				QS->items[i].from_slot, QS->items[i].to_slot, QS->items[i].item_id, QS->items[i].charges,
				QS->items[i].aug_1, QS->items[i].aug_2, QS->items[i].aug_3, QS->items[i].aug_4, QS->items[i].aug_5,
				errbuf, 0, 0))) {
				_log(NET__WORLD, "Failed Move Log Record Entry Insert: %s", errbuf);
				_log(NET__WORLD, "%s", query);
			}
		}
	}
}

void Database::LogMerchantTransaction(QSMerchantLogTransaction_Struct* QS, uint32 Items) {
	// Merchant transactions are from the perspective of the merchant, not the player -U

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 lastid = 0;
	if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_merchant_transaction_record` SET `time`=NOW(), "
		"`zone_id`='%i', `merchant_id`='%i', `merchant_pp`='%i', `merchant_gp`='%i', `merchant_sp`='%i', `merchant_cp`='%i', `merchant_items`='%i', "
		"`char_id`='%i', `char_pp`='%i', `char_gp`='%i', `char_sp`='%i', `char_cp`='%i', `char_items`='%i'",
		QS->zone_id, QS->merchant_id, QS->merchant_money.platinum, QS->merchant_money.gold, QS->merchant_money.silver, QS->merchant_money.copper, QS->merchant_count,
		QS->char_id, QS->char_money.platinum, QS->char_money.gold, QS->char_money.silver, QS->char_money.copper, QS->char_count),
		errbuf, 0, 0, &lastid)) {
		_log(NET__WORLD, "Failed Transaction Log Record Insert: %s", errbuf);
		_log(NET__WORLD, "%s", query);
	}

	if(Items > 0) {
		for(int i = 0; i < Items; i++) {
			if(!RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `qs_merchant_transaction_record_entries` SET `event_id`='%i', "
				"`char_slot`='%i', `item_id`='%i', `charges`='%i', `aug_1`='%i', "
				"`aug_2`='%i', `aug_3`='%i', `aug_4`='%i', `aug_5`='%i'",
				lastid, QS->items[i].char_slot, QS->items[i].item_id, QS->items[i].charges, QS->items[i].aug_1,
				QS->items[i].aug_2, QS->items[i].aug_3, QS->items[i].aug_4, QS->items[i].aug_5,
				errbuf, 0, 0))) {
				_log(NET__WORLD, "Failed Transaction Log Record Entry Insert: %s", errbuf);
				_log(NET__WORLD, "%s", query);
			}
		}
	}
}

