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

#ifndef EQEMU_BASE_LOOTTABLE_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_LOOTTABLE_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLoottableEntriesRepository {
public:
	struct LoottableEntries {
		int   loottable_id;
		int   lootdrop_id;
		int   multiplier;
		int   droplimit;
		int   mindrop;
		float probability;
	};

	static std::string PrimaryKey()
	{
		return std::string("loottable_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"loottable_id",
			"lootdrop_id",
			"multiplier",
			"droplimit",
			"mindrop",
			"probability",
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
		return std::string("loottable_entries");
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

	static LoottableEntries NewEntity()
	{
		LoottableEntries entry{};

		entry.loottable_id = 0;
		entry.lootdrop_id  = 0;
		entry.multiplier   = 1;
		entry.droplimit    = 0;
		entry.mindrop      = 0;
		entry.probability  = 100;

		return entry;
	}

	static LoottableEntries GetLoottableEntriesEntry(
		const std::vector<LoottableEntries> &loottable_entriess,
		int loottable_entries_id
	)
	{
		for (auto &loottable_entries : loottable_entriess) {
			if (loottable_entries.loottable_id == loottable_entries_id) {
				return loottable_entries;
			}
		}

		return NewEntity();
	}

	static LoottableEntries FindOne(
		Database& db,
		int loottable_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				loottable_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoottableEntries entry{};

			entry.loottable_id = atoi(row[0]);
			entry.lootdrop_id  = atoi(row[1]);
			entry.multiplier   = atoi(row[2]);
			entry.droplimit    = atoi(row[3]);
			entry.mindrop      = atoi(row[4]);
			entry.probability  = static_cast<float>(atof(row[5]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int loottable_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				loottable_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LoottableEntries loottable_entries_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(loottable_entries_entry.loottable_id));
		update_values.push_back(columns[1] + " = " + std::to_string(loottable_entries_entry.lootdrop_id));
		update_values.push_back(columns[2] + " = " + std::to_string(loottable_entries_entry.multiplier));
		update_values.push_back(columns[3] + " = " + std::to_string(loottable_entries_entry.droplimit));
		update_values.push_back(columns[4] + " = " + std::to_string(loottable_entries_entry.mindrop));
		update_values.push_back(columns[5] + " = " + std::to_string(loottable_entries_entry.probability));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				loottable_entries_entry.loottable_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoottableEntries InsertOne(
		Database& db,
		LoottableEntries loottable_entries_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(loottable_entries_entry.loottable_id));
		insert_values.push_back(std::to_string(loottable_entries_entry.lootdrop_id));
		insert_values.push_back(std::to_string(loottable_entries_entry.multiplier));
		insert_values.push_back(std::to_string(loottable_entries_entry.droplimit));
		insert_values.push_back(std::to_string(loottable_entries_entry.mindrop));
		insert_values.push_back(std::to_string(loottable_entries_entry.probability));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			loottable_entries_entry.loottable_id = results.LastInsertedID();
			return loottable_entries_entry;
		}

		loottable_entries_entry = NewEntity();

		return loottable_entries_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LoottableEntries> loottable_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &loottable_entries_entry: loottable_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(loottable_entries_entry.loottable_id));
			insert_values.push_back(std::to_string(loottable_entries_entry.lootdrop_id));
			insert_values.push_back(std::to_string(loottable_entries_entry.multiplier));
			insert_values.push_back(std::to_string(loottable_entries_entry.droplimit));
			insert_values.push_back(std::to_string(loottable_entries_entry.mindrop));
			insert_values.push_back(std::to_string(loottable_entries_entry.probability));

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

	static std::vector<LoottableEntries> All(Database& db)
	{
		std::vector<LoottableEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoottableEntries entry{};

			entry.loottable_id = atoi(row[0]);
			entry.lootdrop_id  = atoi(row[1]);
			entry.multiplier   = atoi(row[2]);
			entry.droplimit    = atoi(row[3]);
			entry.mindrop      = atoi(row[4]);
			entry.probability  = static_cast<float>(atof(row[5]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LoottableEntries> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LoottableEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoottableEntries entry{};

			entry.loottable_id = atoi(row[0]);
			entry.lootdrop_id  = atoi(row[1]);
			entry.multiplier   = atoi(row[2]);
			entry.droplimit    = atoi(row[3]);
			entry.mindrop      = atoi(row[4]);
			entry.probability  = static_cast<float>(atof(row[5]));

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

#endif //EQEMU_BASE_LOOTTABLE_ENTRIES_REPOSITORY_H
