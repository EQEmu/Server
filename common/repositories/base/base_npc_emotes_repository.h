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
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_NPC_EMOTES_REPOSITORY_H
#define EQEMU_BASE_NPC_EMOTES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcEmotesRepository {
public:
	struct NpcEmotes {
		int         id;
		int         emoteid;
		int         event_;
		int         type;
		std::string text;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"emoteid",
			"event_",
			"type",
			"text",
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
		return std::string("npc_emotes");
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

	static NpcEmotes NewEntity()
	{
		NpcEmotes entry{};

		entry.id      = 0;
		entry.emoteid = 0;
		entry.event_  = 0;
		entry.type    = 0;
		entry.text    = "";

		return entry;
	}

	static NpcEmotes GetNpcEmotesEntry(
		const std::vector<NpcEmotes> &npc_emotess,
		int npc_emotes_id
	)
	{
		for (auto &npc_emotes : npc_emotess) {
			if (npc_emotes.id == npc_emotes_id) {
				return npc_emotes;
			}
		}

		return NewEntity();
	}

	static NpcEmotes FindOne(
		int npc_emotes_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_emotes_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcEmotes entry{};

			entry.id      = atoi(row[0]);
			entry.emoteid = atoi(row[1]);
			entry.event_  = atoi(row[2]);
			entry.type    = atoi(row[3]);
			entry.text    = row[4] ? row[4] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int npc_emotes_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_emotes_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		NpcEmotes npc_emotes_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(npc_emotes_entry.emoteid));
		update_values.push_back(columns[2] + " = " + std::to_string(npc_emotes_entry.event_));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_emotes_entry.type));
		update_values.push_back(columns[4] + " = '" + EscapeString(npc_emotes_entry.text) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_emotes_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcEmotes InsertOne(
		NpcEmotes npc_emotes_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_emotes_entry.emoteid));
		insert_values.push_back(std::to_string(npc_emotes_entry.event_));
		insert_values.push_back(std::to_string(npc_emotes_entry.type));
		insert_values.push_back("'" + EscapeString(npc_emotes_entry.text) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_emotes_entry.id = results.LastInsertedID();
			return npc_emotes_entry;
		}

		npc_emotes_entry = NewEntity();

		return npc_emotes_entry;
	}

	static int InsertMany(
		std::vector<NpcEmotes> npc_emotes_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_emotes_entry: npc_emotes_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_emotes_entry.emoteid));
			insert_values.push_back(std::to_string(npc_emotes_entry.event_));
			insert_values.push_back(std::to_string(npc_emotes_entry.type));
			insert_values.push_back("'" + EscapeString(npc_emotes_entry.text) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<NpcEmotes> All()
	{
		std::vector<NpcEmotes> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcEmotes entry{};

			entry.id      = atoi(row[0]);
			entry.emoteid = atoi(row[1]);
			entry.event_  = atoi(row[2]);
			entry.type    = atoi(row[3]);
			entry.text    = row[4] ? row[4] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcEmotes> GetWhere(std::string where_filter)
	{
		std::vector<NpcEmotes> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcEmotes entry{};

			entry.id      = atoi(row[0]);
			entry.emoteid = atoi(row[1]);
			entry.event_  = atoi(row[2]);
			entry.type    = atoi(row[3]);
			entry.text    = row[4] ? row[4] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_NPC_EMOTES_REPOSITORY_H
