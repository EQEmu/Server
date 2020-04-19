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

#ifndef EQEMU_BASE_ADVENTURE_MEMBERS_REPOSITORY_H
#define EQEMU_BASE_ADVENTURE_MEMBERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAdventureMembersRepository {
public:
	struct AdventureMembers {
		int id;
		int charid;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"charid",
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
		return std::string("adventure_members");
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

	static AdventureMembers NewEntity()
	{
		AdventureMembers entry{};

		entry.id     = 0;
		entry.charid = 0;

		return entry;
	}

	static AdventureMembers GetAdventureMembersEntry(
		const std::vector<AdventureMembers> &adventure_memberss,
		int adventure_members_id
	)
	{
		for (auto &adventure_members : adventure_memberss) {
			if (adventure_members.charid == adventure_members_id) {
				return adventure_members;
			}
		}

		return NewEntity();
	}

	static AdventureMembers FindOne(
		int adventure_members_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				adventure_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureMembers entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int adventure_members_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		AdventureMembers adventure_members_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(adventure_members_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(adventure_members_entry.charid));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				adventure_members_entry.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureMembers InsertOne(
		AdventureMembers adventure_members_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(adventure_members_entry.id));
		insert_values.push_back(std::to_string(adventure_members_entry.charid));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			adventure_members_entry.charid = results.LastInsertedID();
			return adventure_members_entry;
		}

		adventure_members_entry = NewEntity();

		return adventure_members_entry;
	}

	static int InsertMany(
		std::vector<AdventureMembers> adventure_members_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &adventure_members_entry: adventure_members_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(adventure_members_entry.id));
			insert_values.push_back(std::to_string(adventure_members_entry.charid));

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

	static std::vector<AdventureMembers> All()
	{
		std::vector<AdventureMembers> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureMembers entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AdventureMembers> GetWhere(std::string where_filter)
	{
		std::vector<AdventureMembers> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureMembers entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

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
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_ADVENTURE_MEMBERS_REPOSITORY_H
