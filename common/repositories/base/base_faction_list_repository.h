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

#ifndef EQEMU_BASE_FACTION_LIST_REPOSITORY_H
#define EQEMU_BASE_FACTION_LIST_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseFactionListRepository {
public:
	struct FactionList {
		int         id;
		std::string name;
		int         base;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"base",
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
		return std::string("faction_list");
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

	static FactionList NewEntity()
	{
		FactionList entry{};

		entry.id   = 0;
		entry.name = "";
		entry.base = 0;

		return entry;
	}

	static FactionList GetFactionListEntry(
		const std::vector<FactionList> &faction_lists,
		int faction_list_id
	)
	{
		for (auto &faction_list : faction_lists) {
			if (faction_list.id == faction_list_id) {
				return faction_list;
			}
		}

		return NewEntity();
	}

	static FactionList FindOne(
		Database& db,
		int faction_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				faction_list_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FactionList entry{};

			entry.id   = atoi(row[0]);
			entry.name = row[1] ? row[1] : "";
			entry.base = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int faction_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				faction_list_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		FactionList faction_list_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(faction_list_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(faction_list_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(faction_list_entry.base));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				faction_list_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static FactionList InsertOne(
		Database& db,
		FactionList faction_list_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(faction_list_entry.id));
		insert_values.push_back("'" + EscapeString(faction_list_entry.name) + "'");
		insert_values.push_back(std::to_string(faction_list_entry.base));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			faction_list_entry.id = results.LastInsertedID();
			return faction_list_entry;
		}

		faction_list_entry = NewEntity();

		return faction_list_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<FactionList> faction_list_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &faction_list_entry: faction_list_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(faction_list_entry.id));
			insert_values.push_back("'" + EscapeString(faction_list_entry.name) + "'");
			insert_values.push_back(std::to_string(faction_list_entry.base));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<FactionList> All(Database& db)
	{
		std::vector<FactionList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionList entry{};

			entry.id   = atoi(row[0]);
			entry.name = row[1] ? row[1] : "";
			entry.base = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<FactionList> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<FactionList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionList entry{};

			entry.id   = atoi(row[0]);
			entry.name = row[1] ? row[1] : "";
			entry.base = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, std::string where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_FACTION_LIST_REPOSITORY_H
