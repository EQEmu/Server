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
#include "../common/strings.h"
#include "../common/servertalk.h"

void QSDatabase::AddSpeech(
	const char *from,
	const char *to,
	const char *message,
	uint16 minstatus,
	uint32 guilddbid,
	uint8 type
)
{

	auto escapedFrom    = new char[strlen(from) * 2 + 1];
	auto escapedTo      = new char[strlen(to) * 2 + 1];
	auto escapedMessage = new char[strlen(message) * 2 + 1];
	DoEscapeString(escapedFrom, from, strlen(from));
	DoEscapeString(escapedTo, to, strlen(to));
	DoEscapeString(escapedMessage, message, strlen(message));

	std::string query = StringFormat(
		"INSERT INTO `qs_player_speech` "
		"SET `from` = '%s', `to` = '%s', `message`='%s', "
		"`minstatus`='%i', `guilddbid`='%i', `type`='%i'",
		escapedFrom, escapedTo, escapedMessage, minstatus, guilddbid, type
	);
	safe_delete_array(escapedFrom);
	safe_delete_array(escapedTo);
	safe_delete_array(escapedMessage);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Speech Entry Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}


}

void QSDatabase::LogPlayerDropItem(QSPlayerDropItem_Struct *QS)
{

	std::string query = StringFormat(
		"INSERT INTO `qs_player_drop_record` SET `time` = NOW(), "
		"`char_id` = '%i', `pickup` = '%i', "
		"`zone_id` = '%i', `x` = '%i', `y` = '%i', `z` = '%i' ",
		QS->char_id, QS->pickup, QS->zone_id, QS->x, QS->y, QS->z
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Drop Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	if (QS->_detail_count == 0) {
		return;
	}

	int lastIndex = results.LastInsertedID();

	for (int i = 0; i < QS->_detail_count; i++) {
		query   = StringFormat(
			"INSERT INTO `qs_player_drop_record_entries` SET `event_id` = '%i', "
			"`item_id` = '%i', `charges` = '%i', `aug_1` = '%i', `aug_2` = '%i', "
			"`aug_3` = '%i', `aug_4` = '%i', `aug_5` = '%i'",
			lastIndex, QS->items[i].item_id,
			QS->items[i].charges, QS->items[i].aug_1, QS->items[i].aug_2,
			QS->items[i].aug_3, QS->items[i].aug_4, QS->items[i].aug_5
		);
		results = QueryDatabase(query);
		if (!results.Success()) {
			LogInfo("Failed Drop Record Entry Insert: [{}]", results.ErrorMessage().c_str());
			LogInfo("[{}]", query.c_str());
		}
	}
}

void QSDatabase::LogPlayerTrade(PlayerLogTrade_Struct *QS, uint32 detailCount)
{

	std::string query   = StringFormat(
		"INSERT INTO `qs_player_trade_record` SET `time` = NOW(), "
		"`char1_id` = '%i', `char1_pp` = '%i', `char1_gp` = '%i', "
		"`char1_sp` = '%i', `char1_cp` = '%i', `char1_items` = '%i', "
		"`char2_id` = '%i', `char2_pp` = '%i', `char2_gp` = '%i', "
		"`char2_sp` = '%i', `char2_cp` = '%i', `char2_items` = '%i'",
		QS->character_1_id, QS->character_1_money.platinum, QS->character_1_money.gold,
		QS->character_1_money.silver, QS->character_1_money.copper, QS->character_1_item_count,
		QS->character_2_id, QS->character_2_money.platinum, QS->character_2_money.gold,
		QS->character_2_money.silver, QS->character_2_money.copper, QS->character_2_item_count
	);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Trade Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	if (detailCount == 0) {
		return;
	}

	int lastIndex = results.LastInsertedID();

	for (int i = 0; i < detailCount; i++) {
		query   = StringFormat(
			"INSERT INTO `qs_player_trade_record_entries` SET `event_id` = '%i', "
			"`from_id` = '%i', `from_slot` = '%i', `to_id` = '%i', `to_slot` = '%i', "
			"`item_id` = '%i', `charges` = '%i', `aug_1` = '%i', `aug_2` = '%i', "
			"`aug_3` = '%i', `aug_4` = '%i', `aug_5` = '%i'",
			lastIndex, QS->item_entries[i].from_character_id, QS->item_entries[i].from_slot,
			QS->item_entries[i].to_character_id, QS->item_entries[i].to_slot, QS->item_entries[i].item_id,
			QS->item_entries[i].charges, QS->item_entries[i].aug_1, QS->item_entries[i].aug_2,
			QS->item_entries[i].aug_3, QS->item_entries[i].aug_4, QS->item_entries[i].aug_5
		);
		results = QueryDatabase(query);
		if (!results.Success()) {
			LogInfo("Failed Trade Log Record Entry Insert: [{}]", results.ErrorMessage().c_str());
			LogInfo("[{}]", query.c_str());
		}

	}

}

void QSDatabase::LogPlayerHandin(QSPlayerLogHandin_Struct *QS, uint32 detailCount)
{

	std::string query   = StringFormat(
		"INSERT INTO `qs_player_handin_record` SET `time` = NOW(), "
		"`quest_id` = '%i', `char_id` = '%i', `char_pp` = '%i', "
		"`char_gp` = '%i', `char_sp` = '%i', `char_cp` = '%i', "
		"`char_items` = '%i', `npc_id` = '%i', `npc_pp` = '%i', "
		"`npc_gp` = '%i', `npc_sp` = '%i', `npc_cp` = '%i', "
		"`npc_items`='%i'",
		QS->quest_id, QS->char_id, QS->char_money.platinum,
		QS->char_money.gold, QS->char_money.silver, QS->char_money.copper,
		QS->char_count, QS->npc_id, QS->npc_money.platinum,
		QS->npc_money.gold, QS->npc_money.silver, QS->npc_money.copper,
		QS->npc_count
	);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Handin Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	if (detailCount == 0) {
		return;
	}

	int lastIndex = results.LastInsertedID();

	for (int i = 0; i < detailCount; i++) {
		query = StringFormat(
			"INSERT INTO `qs_player_handin_record_entries` SET `event_id` = '%i', "
			"`action_type` = '%s', `char_slot` = '%i', `item_id` = '%i', "
			"`charges` = '%i', `aug_1` = '%i', `aug_2` = '%i', `aug_3` = '%i', "
			"`aug_4` = '%i', `aug_5` = '%i'",
			lastIndex, QS->items[i].action_type, QS->items[i].char_slot,
			QS->items[i].item_id, QS->items[i].charges, QS->items[i].aug_1,
			QS->items[i].aug_2, QS->items[i].aug_3, QS->items[i].aug_4,
			QS->items[i].aug_5
		);
		auto results = QueryDatabase(query);

		if (!results.Success()) {
			LogInfo("Failed Handin Log Record Entry Insert: [{}]", results.ErrorMessage().c_str());
			LogInfo("[{}]", query.c_str());
		}
	}

}

void QSDatabase::LogPlayerNPCKill(QSPlayerLogNPCKill_Struct *QS, uint32 members)
{

	std::string query   = StringFormat(
		"INSERT INTO `qs_player_npc_kill_record` "
		"SET `npc_id` = '%i', `type` = '%i', "
		"`zone_id` = '%i', `time` = NOW()",
		QS->s1.NPCID, QS->s1.Type, QS->s1.ZoneID
	);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed NPC Kill Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	if (members == 0) {
		return;
	}

	int lastIndex = results.LastInsertedID();

	for (int i = 0; i < members; i++) {
		query = StringFormat(
			"INSERT INTO `qs_player_npc_kill_record_entries` "
			"SET `event_id` = '%i', `char_id` = '%i'",
			lastIndex, QS->Chars[i].char_id
		);
		auto results = QueryDatabase(query);
		if (!results.Success()) {
			LogInfo("Failed NPC Kill Log Entry Insert: [{}]", results.ErrorMessage().c_str());
			LogInfo("[{}]", query.c_str());
		}

	}

}

void QSDatabase::LogPlayerDelete(QSPlayerLogDelete_Struct *QS, uint32 items)
{

	std::string query   = StringFormat(
		"INSERT INTO `qs_player_delete_record` SET `time` = NOW(), "
		"`char_id` = '%i', `stack_size` = '%i', `char_items` = '%i'",
		QS->char_id, QS->stack_size, QS->char_count, QS->char_count
	);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Delete Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	if (items == 0) {
		return;
	}

	int lastIndex = results.LastInsertedID();

	for (int i = 0; i < items; i++) {
		query   = StringFormat(
			"INSERT INTO `qs_player_delete_record_entries` SET `event_id` = '%i', "
			"`char_slot` = '%i', `item_id` = '%i', `charges` = '%i', `aug_1` = '%i', "
			"`aug_2` = '%i', `aug_3` = '%i', `aug_4` = '%i', `aug_5` = '%i'",
			lastIndex, QS->items[i].char_slot, QS->items[i].item_id, QS->items[i].charges,
			QS->items[i].aug_1, QS->items[i].aug_2, QS->items[i].aug_3, QS->items[i].aug_4,
			QS->items[i].aug_5
		);
		results = QueryDatabase(query);
		if (!results.Success()) {
			LogInfo("Failed Delete Log Record Entry Insert: [{}]", results.ErrorMessage().c_str());
			LogInfo("[{}]", query.c_str());
		}

	}

}

void QSDatabase::LogPlayerMove(QSPlayerLogMove_Struct *QS, uint32 items)
{
	/* These are item moves */

	std::string query   = StringFormat(
		"INSERT INTO `qs_player_move_record` SET `time` = NOW(), "
		"`char_id` = '%i', `from_slot` = '%i', `to_slot` = '%i', "
		"`stack_size` = '%i', `char_items` = '%i', `postaction` = '%i'",
		QS->char_id, QS->from_slot, QS->to_slot, QS->stack_size,
		QS->char_count, QS->postaction
	);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Move Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	if (items == 0) {
		return;
	}

	int lastIndex = results.LastInsertedID();

	for (int i = 0; i < items; i++) {
		query   = StringFormat(
			"INSERT INTO `qs_player_move_record_entries` SET `event_id` = '%i', "
			"`from_slot` = '%i', `to_slot` = '%i', `item_id` = '%i', `charges` = '%i', "
			"`aug_1` = '%i', `aug_2` = '%i', `aug_3` = '%i', `aug_4` = '%i', `aug_5` = '%i'",
			lastIndex, QS->items[i].from_slot, QS->items[i].to_slot, QS->items[i].item_id,
			QS->items[i].charges, QS->items[i].aug_1, QS->items[i].aug_2,
			QS->items[i].aug_3, QS->items[i].aug_4, QS->items[i].aug_5
		);
		results = QueryDatabase(query);
		if (!results.Success()) {
			LogInfo("Failed Move Log Record Entry Insert: [{}]", results.ErrorMessage().c_str());
			LogInfo("[{}]", query.c_str());
		}
	}
}

void QSDatabase::LogMerchantTransaction(QSMerchantLogTransaction_Struct *QS, uint32 items)
{
	/* Merchant transactions are from the perspective of the merchant, not the player */
	std::string query   = StringFormat(
		"INSERT INTO `qs_merchant_transaction_record` SET `time` = NOW(), "
		"`zone_id` = '%i', `merchant_id` = '%i', `merchant_pp` = '%i', "
		"`merchant_gp` = '%i', `merchant_sp` = '%i', `merchant_cp` = '%i', "
		"`merchant_items` = '%i', `char_id` = '%i', `char_pp` = '%i', "
		"`char_gp` = '%i', `char_sp` = '%i', `char_cp` = '%i', "
		"`char_items` = '%i'",
		QS->zone_id, QS->merchant_id, QS->merchant_money.platinum,
		QS->merchant_money.gold, QS->merchant_money.silver,
		QS->merchant_money.copper, QS->merchant_count, QS->char_id,
		QS->char_money.platinum, QS->char_money.gold, QS->char_money.silver,
		QS->char_money.copper, QS->char_count
	);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Transaction Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	if (items == 0) {
		return;
	}

	int lastIndex = results.LastInsertedID();

	for (int i = 0; i < items; i++) {
		query   = StringFormat(
			"INSERT INTO `qs_merchant_transaction_record_entries` SET `event_id` = '%i', "
			"`char_slot` = '%i', `item_id` = '%i', `charges` = '%i', `aug_1` = '%i', "
			"`aug_2` = '%i', `aug_3` = '%i', `aug_4` = '%i', `aug_5` = '%i'",
			lastIndex, QS->items[i].char_slot, QS->items[i].item_id, QS->items[i].charges,
			QS->items[i].aug_1, QS->items[i].aug_2, QS->items[i].aug_3, QS->items[i].aug_4,
			QS->items[i].aug_5
		);
		results = QueryDatabase(query);
		if (!results.Success()) {
			LogInfo("Failed Transaction Log Record Entry Insert: [{}]", results.ErrorMessage().c_str());
			LogInfo("[{}]", query.c_str());
		}

	}

}

// this function does not delete the ServerPacket, so it must be handled at call site
void QSDatabase::GeneralQueryReceive(ServerPacket *pack)
{
	/*
		These are general queries passed from anywhere in zone instead of packing structures and breaking them down again and again
	*/
	auto queryBuffer    = new char[pack->ReadUInt32() + 1];
	pack->ReadString(queryBuffer);

	std::string query(queryBuffer);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Delete Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	safe_delete_array(queryBuffer);
}
