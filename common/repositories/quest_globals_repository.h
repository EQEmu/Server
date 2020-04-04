/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EQEMU_QUEST_GLOBALS_REPOSITORY_H
#define EQEMU_QUEST_GLOBALS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class QuestGlobalsRepository {
public:
	struct QuestGlobals {
		int         charid;
		int         npcid;
		int         zoneid;
		std::string name;
		std::string value;
		int         expdate;
	};

	static std::string PrimaryKey()
	{
		return std::string("name");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"npcid",
			"zoneid",
			"name",
			"value",
			"expdate",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("quest_globals");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			ColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			InsertColumnsRaw()
		);
	}

	static QuestGlobals NewEntity()
	{
		QuestGlobals entry{};

		entry.charid  = 0;
		entry.npcid   = 0;
		entry.zoneid  = 0;
		entry.name    = "";
		entry.value   = "?";
		entry.expdate = 0;

		return entry;
	}

	static QuestGlobals GetQuestGlobalsEntry(
		const std::vector<QuestGlobals> &quest_globalss,
		int quest_globals_id
	)
	{
		for (auto &quest_globals : quest_globalss) {
			if (quest_globals.name == quest_globals_id) {
				return quest_globals;
			}
		}

		return NewEntity();
	}

	static QuestGlobals FindOne(
		int quest_globals_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				quest_globals_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			QuestGlobals entry{};

			entry.charid  = atoi(row[0]);
			entry.npcid   = atoi(row[1]);
			entry.zoneid  = atoi(row[2]);
			entry.name    = row[3];
			entry.value   = row[4];
			entry.expdate = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int quest_globals_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				quest_globals_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		QuestGlobals quest_globals_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[4] + " = '" + EscapeString(quest_globals_entry.value) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(quest_globals_entry.expdate));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				quest_globals_entry.name
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static QuestGlobals InsertOne(
		QuestGlobals quest_globals_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(quest_globals_entry.value) + "'");
		insert_values.push_back(std::to_string(quest_globals_entry.expdate));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			quest_globals_entry.id = results.LastInsertedID();
			return quest_globals_entry;
		}

		quest_globals_entry = QuestGlobalsRepository::NewEntity();

		return quest_globals_entry;
	}

	static int InsertMany(
		std::vector<QuestGlobals> quest_globals_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &quest_globals_entry: quest_globals_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(quest_globals_entry.value) + "'");
			insert_values.push_back(std::to_string(quest_globals_entry.expdate));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<QuestGlobals> All()
	{
		std::vector<QuestGlobals> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			QuestGlobals entry{};

			entry.charid  = atoi(row[0]);
			entry.npcid   = atoi(row[1]);
			entry.zoneid  = atoi(row[2]);
			entry.name    = row[3];
			entry.value   = row[4];
			entry.expdate = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<QuestGlobals> GetWhere(std::string where_filter)
	{
		std::vector<QuestGlobals> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			QuestGlobals entry{};

			entry.charid  = atoi(row[0]);
			entry.npcid   = atoi(row[1]);
			entry.zoneid  = atoi(row[2]);
			entry.name    = row[3];
			entry.value   = row[4];
			entry.expdate = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_QUEST_GLOBALS_REPOSITORY_H
