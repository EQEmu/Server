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
 * This repository was automatically generated on Apr 5, 2020 and is NOT
 * to be modified directly. Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_GRID_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_GRID_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGridEntriesRepository {
public:
	struct GridEntries {
		int   gridid;
		int   zoneid;
		int   number;
		float x;
		float y;
		float z;
		float heading;
		int   pause;
		int8  centerpoint;
	};

	static std::string PrimaryKey()
	{
		return std::string("number");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"gridid",
			"zoneid",
			"number",
			"x",
			"y",
			"z",
			"heading",
			"pause",
			"centerpoint",
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
		return std::string("grid_entries");
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

	static GridEntries NewEntity()
	{
		GridEntries entry{};

		entry.gridid      = 0;
		entry.zoneid      = 0;
		entry.number      = 0;
		entry.x           = 0;
		entry.y           = 0;
		entry.z           = 0;
		entry.heading     = 0;
		entry.pause       = 0;
		entry.centerpoint = 0;

		return entry;
	}

	static GridEntries GetGridEntriesEntry(
		const std::vector<GridEntries> &grid_entriess,
		int grid_entries_id
	)
	{
		for (auto &grid_entries : grid_entriess) {
			if (grid_entries.number == grid_entries_id) {
				return grid_entries;
			}
		}

		return NewEntity();
	}

	static GridEntries FindOne(
		int grid_entries_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				grid_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GridEntries entry{};

			entry.gridid      = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.number      = atoi(row[2]);
			entry.x           = atof(row[3]);
			entry.y           = atof(row[4]);
			entry.z           = atof(row[5]);
			entry.heading     = atof(row[6]);
			entry.pause       = atoi(row[7]);
			entry.centerpoint = atoi(row[8]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int grid_entries_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				grid_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		GridEntries grid_entries_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[3] + " = " + std::to_string(grid_entries_entry.x));
		update_values.push_back(columns[4] + " = " + std::to_string(grid_entries_entry.y));
		update_values.push_back(columns[5] + " = " + std::to_string(grid_entries_entry.z));
		update_values.push_back(columns[6] + " = " + std::to_string(grid_entries_entry.heading));
		update_values.push_back(columns[7] + " = " + std::to_string(grid_entries_entry.pause));
		update_values.push_back(columns[8] + " = " + std::to_string(grid_entries_entry.centerpoint));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				grid_entries_entry.number
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GridEntries InsertOne(
		GridEntries grid_entries_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(grid_entries_entry.x));
		insert_values.push_back(std::to_string(grid_entries_entry.y));
		insert_values.push_back(std::to_string(grid_entries_entry.z));
		insert_values.push_back(std::to_string(grid_entries_entry.heading));
		insert_values.push_back(std::to_string(grid_entries_entry.pause));
		insert_values.push_back(std::to_string(grid_entries_entry.centerpoint));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			grid_entries_entry.id = results.LastInsertedID();
			return grid_entries_entry;
		}

		grid_entries_entry = NewEntity();

		return grid_entries_entry;
	}

	static int InsertMany(
		std::vector<GridEntries> grid_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &grid_entries_entry: grid_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(grid_entries_entry.x));
			insert_values.push_back(std::to_string(grid_entries_entry.y));
			insert_values.push_back(std::to_string(grid_entries_entry.z));
			insert_values.push_back(std::to_string(grid_entries_entry.heading));
			insert_values.push_back(std::to_string(grid_entries_entry.pause));
			insert_values.push_back(std::to_string(grid_entries_entry.centerpoint));

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

	static std::vector<GridEntries> All()
	{
		std::vector<GridEntries> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GridEntries entry{};

			entry.gridid      = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.number      = atoi(row[2]);
			entry.x           = atof(row[3]);
			entry.y           = atof(row[4]);
			entry.z           = atof(row[5]);
			entry.heading     = atof(row[6]);
			entry.pause       = atoi(row[7]);
			entry.centerpoint = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GridEntries> GetWhere(std::string where_filter)
	{
		std::vector<GridEntries> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GridEntries entry{};

			entry.gridid      = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.number      = atoi(row[2]);
			entry.x           = atof(row[3]);
			entry.y           = atof(row[4]);
			entry.z           = atof(row[5]);
			entry.heading     = atof(row[6]);
			entry.pause       = atoi(row[7]);
			entry.centerpoint = atoi(row[8]);

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
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_GRID_ENTRIES_REPOSITORY_H
