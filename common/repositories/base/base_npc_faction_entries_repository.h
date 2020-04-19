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

#ifndef EQEMU_BASE_NPC_FACTION_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_NPC_FACTION_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcFactionEntriesRepository {
public:
	struct NpcFactionEntries {
		int npc_faction_id;
		int faction_id;
		int value;
		int npc_value;
		int temp;
	};

	static std::string PrimaryKey()
	{
		return std::string("npc_faction_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"npc_faction_id",
			"faction_id",
			"value",
			"npc_value",
			"temp",
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
		return std::string("npc_faction_entries");
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

	static NpcFactionEntries NewEntity()
	{
		NpcFactionEntries entry{};

		entry.npc_faction_id = 0;
		entry.faction_id     = 0;
		entry.value          = 0;
		entry.npc_value      = 0;
		entry.temp           = 0;

		return entry;
	}

	static NpcFactionEntries GetNpcFactionEntriesEntry(
		const std::vector<NpcFactionEntries> &npc_faction_entriess,
		int npc_faction_entries_id
	)
	{
		for (auto &npc_faction_entries : npc_faction_entriess) {
			if (npc_faction_entries.npc_faction_id == npc_faction_entries_id) {
				return npc_faction_entries;
			}
		}

		return NewEntity();
	}

	static NpcFactionEntries FindOne(
		int npc_faction_entries_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_faction_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcFactionEntries entry{};

			entry.npc_faction_id = atoi(row[0]);
			entry.faction_id     = atoi(row[1]);
			entry.value          = atoi(row[2]);
			entry.npc_value      = atoi(row[3]);
			entry.temp           = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int npc_faction_entries_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_faction_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		NpcFactionEntries npc_faction_entries_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(npc_faction_entries_entry.npc_faction_id));
		update_values.push_back(columns[1] + " = " + std::to_string(npc_faction_entries_entry.faction_id));
		update_values.push_back(columns[2] + " = " + std::to_string(npc_faction_entries_entry.value));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_faction_entries_entry.npc_value));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_faction_entries_entry.temp));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_faction_entries_entry.npc_faction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcFactionEntries InsertOne(
		NpcFactionEntries npc_faction_entries_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_faction_entries_entry.npc_faction_id));
		insert_values.push_back(std::to_string(npc_faction_entries_entry.faction_id));
		insert_values.push_back(std::to_string(npc_faction_entries_entry.value));
		insert_values.push_back(std::to_string(npc_faction_entries_entry.npc_value));
		insert_values.push_back(std::to_string(npc_faction_entries_entry.temp));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_faction_entries_entry.npc_faction_id = results.LastInsertedID();
			return npc_faction_entries_entry;
		}

		npc_faction_entries_entry = NewEntity();

		return npc_faction_entries_entry;
	}

	static int InsertMany(
		std::vector<NpcFactionEntries> npc_faction_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_faction_entries_entry: npc_faction_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_faction_entries_entry.npc_faction_id));
			insert_values.push_back(std::to_string(npc_faction_entries_entry.faction_id));
			insert_values.push_back(std::to_string(npc_faction_entries_entry.value));
			insert_values.push_back(std::to_string(npc_faction_entries_entry.npc_value));
			insert_values.push_back(std::to_string(npc_faction_entries_entry.temp));

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

	static std::vector<NpcFactionEntries> All()
	{
		std::vector<NpcFactionEntries> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcFactionEntries entry{};

			entry.npc_faction_id = atoi(row[0]);
			entry.faction_id     = atoi(row[1]);
			entry.value          = atoi(row[2]);
			entry.npc_value      = atoi(row[3]);
			entry.temp           = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcFactionEntries> GetWhere(std::string where_filter)
	{
		std::vector<NpcFactionEntries> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcFactionEntries entry{};

			entry.npc_faction_id = atoi(row[0]);
			entry.faction_id     = atoi(row[1]);
			entry.value          = atoi(row[2]);
			entry.npc_value      = atoi(row[3]);
			entry.temp           = atoi(row[4]);

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

#endif //EQEMU_BASE_NPC_FACTION_ENTRIES_REPOSITORY_H
