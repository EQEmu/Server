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

#ifndef EQEMU_BASE_LAUNCHER_ZONES_REPOSITORY_H
#define EQEMU_BASE_LAUNCHER_ZONES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLauncherZonesRepository {
public:
	struct LauncherZones {
		std::string launcher;
		std::string zone;
		int         port;
	};

	static std::string PrimaryKey()
	{
		return std::string("zone");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"launcher",
			"zone",
			"port",
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
		return std::string("launcher_zones");
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

	static LauncherZones NewEntity()
	{
		LauncherZones entry{};

		entry.launcher = "";
		entry.zone     = "";
		entry.port     = 0;

		return entry;
	}

	static LauncherZones GetLauncherZonesEntry(
		const std::vector<LauncherZones> &launcher_zoness,
		int launcher_zones_id
	)
	{
		for (auto &launcher_zones : launcher_zoness) {
			if (launcher_zones.zone == launcher_zones_id) {
				return launcher_zones;
			}
		}

		return NewEntity();
	}

	static LauncherZones FindOne(
		int launcher_zones_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				launcher_zones_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LauncherZones entry{};

			entry.launcher = row[0] ? row[0] : "";
			entry.zone     = row[1] ? row[1] : "";
			entry.port     = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int launcher_zones_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				launcher_zones_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		LauncherZones launcher_zones_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[2] + " = " + std::to_string(launcher_zones_entry.port));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				launcher_zones_entry.zone
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LauncherZones InsertOne(
		LauncherZones launcher_zones_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(launcher_zones_entry.port));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			launcher_zones_entry.id = results.LastInsertedID();
			return launcher_zones_entry;
		}

		launcher_zones_entry = NewEntity();

		return launcher_zones_entry;
	}

	static int InsertMany(
		std::vector<LauncherZones> launcher_zones_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &launcher_zones_entry: launcher_zones_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(launcher_zones_entry.port));

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

	static std::vector<LauncherZones> All()
	{
		std::vector<LauncherZones> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LauncherZones entry{};

			entry.launcher = row[0] ? row[0] : "";
			entry.zone     = row[1] ? row[1] : "";
			entry.port     = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LauncherZones> GetWhere(std::string where_filter)
	{
		std::vector<LauncherZones> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LauncherZones entry{};

			entry.launcher = row[0] ? row[0] : "";
			entry.zone     = row[1] ? row[1] : "";
			entry.port     = atoi(row[2]);

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

#endif //EQEMU_BASE_LAUNCHER_ZONES_REPOSITORY_H
