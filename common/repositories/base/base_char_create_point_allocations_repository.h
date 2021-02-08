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

#ifndef EQEMU_BASE_CHAR_CREATE_POINT_ALLOCATIONS_REPOSITORY_H
#define EQEMU_BASE_CHAR_CREATE_POINT_ALLOCATIONS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharCreatePointAllocationsRepository {
public:
	struct CharCreatePointAllocations {
		int id;
		int base_str;
		int base_sta;
		int base_dex;
		int base_agi;
		int base_int;
		int base_wis;
		int base_cha;
		int alloc_str;
		int alloc_sta;
		int alloc_dex;
		int alloc_agi;
		int alloc_int;
		int alloc_wis;
		int alloc_cha;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"base_str",
			"base_sta",
			"base_dex",
			"base_agi",
			"base_int",
			"base_wis",
			"base_cha",
			"alloc_str",
			"alloc_sta",
			"alloc_dex",
			"alloc_agi",
			"alloc_int",
			"alloc_wis",
			"alloc_cha",
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
		return std::string("char_create_point_allocations");
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

	static CharCreatePointAllocations NewEntity()
	{
		CharCreatePointAllocations entry{};

		entry.id        = 0;
		entry.base_str  = 0;
		entry.base_sta  = 0;
		entry.base_dex  = 0;
		entry.base_agi  = 0;
		entry.base_int  = 0;
		entry.base_wis  = 0;
		entry.base_cha  = 0;
		entry.alloc_str = 0;
		entry.alloc_sta = 0;
		entry.alloc_dex = 0;
		entry.alloc_agi = 0;
		entry.alloc_int = 0;
		entry.alloc_wis = 0;
		entry.alloc_cha = 0;

		return entry;
	}

	static CharCreatePointAllocations GetCharCreatePointAllocationsEntry(
		const std::vector<CharCreatePointAllocations> &char_create_point_allocationss,
		int char_create_point_allocations_id
	)
	{
		for (auto &char_create_point_allocations : char_create_point_allocationss) {
			if (char_create_point_allocations.id == char_create_point_allocations_id) {
				return char_create_point_allocations;
			}
		}

		return NewEntity();
	}

	static CharCreatePointAllocations FindOne(
		Database& db,
		int char_create_point_allocations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				char_create_point_allocations_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharCreatePointAllocations entry{};

			entry.id        = atoi(row[0]);
			entry.base_str  = atoi(row[1]);
			entry.base_sta  = atoi(row[2]);
			entry.base_dex  = atoi(row[3]);
			entry.base_agi  = atoi(row[4]);
			entry.base_int  = atoi(row[5]);
			entry.base_wis  = atoi(row[6]);
			entry.base_cha  = atoi(row[7]);
			entry.alloc_str = atoi(row[8]);
			entry.alloc_sta = atoi(row[9]);
			entry.alloc_dex = atoi(row[10]);
			entry.alloc_agi = atoi(row[11]);
			entry.alloc_int = atoi(row[12]);
			entry.alloc_wis = atoi(row[13]);
			entry.alloc_cha = atoi(row[14]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int char_create_point_allocations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				char_create_point_allocations_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharCreatePointAllocations char_create_point_allocations_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(char_create_point_allocations_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(char_create_point_allocations_entry.base_str));
		update_values.push_back(columns[2] + " = " + std::to_string(char_create_point_allocations_entry.base_sta));
		update_values.push_back(columns[3] + " = " + std::to_string(char_create_point_allocations_entry.base_dex));
		update_values.push_back(columns[4] + " = " + std::to_string(char_create_point_allocations_entry.base_agi));
		update_values.push_back(columns[5] + " = " + std::to_string(char_create_point_allocations_entry.base_int));
		update_values.push_back(columns[6] + " = " + std::to_string(char_create_point_allocations_entry.base_wis));
		update_values.push_back(columns[7] + " = " + std::to_string(char_create_point_allocations_entry.base_cha));
		update_values.push_back(columns[8] + " = " + std::to_string(char_create_point_allocations_entry.alloc_str));
		update_values.push_back(columns[9] + " = " + std::to_string(char_create_point_allocations_entry.alloc_sta));
		update_values.push_back(columns[10] + " = " + std::to_string(char_create_point_allocations_entry.alloc_dex));
		update_values.push_back(columns[11] + " = " + std::to_string(char_create_point_allocations_entry.alloc_agi));
		update_values.push_back(columns[12] + " = " + std::to_string(char_create_point_allocations_entry.alloc_int));
		update_values.push_back(columns[13] + " = " + std::to_string(char_create_point_allocations_entry.alloc_wis));
		update_values.push_back(columns[14] + " = " + std::to_string(char_create_point_allocations_entry.alloc_cha));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				char_create_point_allocations_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharCreatePointAllocations InsertOne(
		Database& db,
		CharCreatePointAllocations char_create_point_allocations_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(char_create_point_allocations_entry.id));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_str));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_sta));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_dex));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_agi));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_int));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_wis));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_cha));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_str));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_sta));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_dex));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_agi));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_int));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_wis));
		insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_cha));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			char_create_point_allocations_entry.id = results.LastInsertedID();
			return char_create_point_allocations_entry;
		}

		char_create_point_allocations_entry = NewEntity();

		return char_create_point_allocations_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharCreatePointAllocations> char_create_point_allocations_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &char_create_point_allocations_entry: char_create_point_allocations_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(char_create_point_allocations_entry.id));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_str));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_sta));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_dex));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_agi));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_int));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_wis));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.base_cha));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_str));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_sta));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_dex));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_agi));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_int));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_wis));
			insert_values.push_back(std::to_string(char_create_point_allocations_entry.alloc_cha));

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

	static std::vector<CharCreatePointAllocations> All(Database& db)
	{
		std::vector<CharCreatePointAllocations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharCreatePointAllocations entry{};

			entry.id        = atoi(row[0]);
			entry.base_str  = atoi(row[1]);
			entry.base_sta  = atoi(row[2]);
			entry.base_dex  = atoi(row[3]);
			entry.base_agi  = atoi(row[4]);
			entry.base_int  = atoi(row[5]);
			entry.base_wis  = atoi(row[6]);
			entry.base_cha  = atoi(row[7]);
			entry.alloc_str = atoi(row[8]);
			entry.alloc_sta = atoi(row[9]);
			entry.alloc_dex = atoi(row[10]);
			entry.alloc_agi = atoi(row[11]);
			entry.alloc_int = atoi(row[12]);
			entry.alloc_wis = atoi(row[13]);
			entry.alloc_cha = atoi(row[14]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharCreatePointAllocations> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharCreatePointAllocations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharCreatePointAllocations entry{};

			entry.id        = atoi(row[0]);
			entry.base_str  = atoi(row[1]);
			entry.base_sta  = atoi(row[2]);
			entry.base_dex  = atoi(row[3]);
			entry.base_agi  = atoi(row[4]);
			entry.base_int  = atoi(row[5]);
			entry.base_wis  = atoi(row[6]);
			entry.base_cha  = atoi(row[7]);
			entry.alloc_str = atoi(row[8]);
			entry.alloc_sta = atoi(row[9]);
			entry.alloc_dex = atoi(row[10]);
			entry.alloc_agi = atoi(row[11]);
			entry.alloc_int = atoi(row[12]);
			entry.alloc_wis = atoi(row[13]);
			entry.alloc_cha = atoi(row[14]);

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

#endif //EQEMU_BASE_CHAR_CREATE_POINT_ALLOCATIONS_REPOSITORY_H
