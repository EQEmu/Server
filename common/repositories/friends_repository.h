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

#ifndef EQEMU_FRIENDS_REPOSITORY_H
#define EQEMU_FRIENDS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class FriendsRepository {
public:
	struct Friends {
		int         charid;
		int8        type;
		std::string name;
	};

	static std::string PrimaryKey()
	{
		return std::string("name");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"type",
			"name",
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
		return std::string("friends");
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

	static Friends NewEntity()
	{
		Friends entry{};

		entry.charid = 0;
		entry.type   = 1;
		entry.name   = "";

		return entry;
	}

	static Friends GetFriendsEntry(
		const std::vector<Friends> &friendss,
		int friends_id
	)
	{
		for (auto &friends : friendss) {
			if (friends.name == friends_id) {
				return friends;
			}
		}

		return NewEntity();
	}

	static Friends FindOne(
		int friends_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				friends_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Friends entry{};

			entry.charid = atoi(row[0]);
			entry.type   = atoi(row[1]);
			entry.name   = row[2];

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int friends_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				friends_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Friends friends_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();



		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				friends_entry.name
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Friends InsertOne(
		Friends friends_entry
	)
	{
		std::vector<std::string> insert_values;



		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			friends_entry.id = results.LastInsertedID();
			return friends_entry;
		}

		friends_entry = FriendsRepository::NewEntity();

		return friends_entry;
	}

	static int InsertMany(
		std::vector<Friends> friends_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &friends_entry: friends_entries) {
			std::vector<std::string> insert_values;



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

	static std::vector<Friends> All()
	{
		std::vector<Friends> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Friends entry{};

			entry.charid = atoi(row[0]);
			entry.type   = atoi(row[1]);
			entry.name   = row[2];

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Friends> GetWhere(std::string where_filter)
	{
		std::vector<Friends> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Friends entry{};

			entry.charid = atoi(row[0]);
			entry.type   = atoi(row[1]);
			entry.name   = row[2];

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

#endif //EQEMU_FRIENDS_REPOSITORY_H
