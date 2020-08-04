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

#ifndef EQEMU_BASE_GRID_REPOSITORY_H
#define EQEMU_BASE_GRID_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGridRepository {
public:
	struct Grid {
		int id;
		int zoneid;
		int type;
		int type2;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zoneid",
			"type",
			"type2",
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
		return std::string("grid");
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

	static Grid NewEntity()
	{
		Grid entry{};

		entry.id     = 0;
		entry.zoneid = 0;
		entry.type   = 0;
		entry.type2  = 0;

		return entry;
	}

	static Grid GetGridEntry(
		const std::vector<Grid> &grids,
		int grid_id
	)
	{
		for (auto &grid : grids) {
			if (grid.id == grid_id) {
				return grid;
			}
		}

		return NewEntity();
	}

	static Grid FindOne(
		int grid_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				grid_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Grid entry{};

			entry.id     = atoi(row[0]);
			entry.zoneid = atoi(row[1]);
			entry.type   = atoi(row[2]);
			entry.type2  = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int grid_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				grid_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Grid grid_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(grid_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(grid_entry.zoneid));
		update_values.push_back(columns[2] + " = " + std::to_string(grid_entry.type));
		update_values.push_back(columns[3] + " = " + std::to_string(grid_entry.type2));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				grid_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Grid InsertOne(
		Grid grid_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(grid_entry.id));
		insert_values.push_back(std::to_string(grid_entry.zoneid));
		insert_values.push_back(std::to_string(grid_entry.type));
		insert_values.push_back(std::to_string(grid_entry.type2));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			grid_entry.id = results.LastInsertedID();
			return grid_entry;
		}

		grid_entry = NewEntity();

		return grid_entry;
	}

	static int InsertMany(
		std::vector<Grid> grid_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &grid_entry: grid_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(grid_entry.id));
			insert_values.push_back(std::to_string(grid_entry.zoneid));
			insert_values.push_back(std::to_string(grid_entry.type));
			insert_values.push_back(std::to_string(grid_entry.type2));

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

	static std::vector<Grid> All()
	{
		std::vector<Grid> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Grid entry{};

			entry.id     = atoi(row[0]);
			entry.zoneid = atoi(row[1]);
			entry.type   = atoi(row[2]);
			entry.type2  = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Grid> GetWhere(std::string where_filter)
	{
		std::vector<Grid> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Grid entry{};

			entry.id     = atoi(row[0]);
			entry.zoneid = atoi(row[1]);
			entry.type   = atoi(row[2]);
			entry.type2  = atoi(row[3]);

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

};

#endif //EQEMU_BASE_GRID_REPOSITORY_H
