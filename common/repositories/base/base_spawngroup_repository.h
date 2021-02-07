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

#ifndef EQEMU_BASE_SPAWNGROUP_REPOSITORY_H
#define EQEMU_BASE_SPAWNGROUP_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpawngroupRepository {
public:
	struct Spawngroup {
		int         id;
		std::string name;
		int         spawn_limit;
		float       dist;
		float       max_x;
		float       min_x;
		float       max_y;
		float       min_y;
		int         delay;
		int         mindelay;
		int         despawn;
		int         despawn_timer;
		int         wp_spawns;
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
			"spawn_limit",
			"dist",
			"max_x",
			"min_x",
			"max_y",
			"min_y",
			"delay",
			"mindelay",
			"despawn",
			"despawn_timer",
			"wp_spawns",
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
		return std::string("spawngroup");
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

	static Spawngroup NewEntity()
	{
		Spawngroup entry{};

		entry.id            = 0;
		entry.name          = "";
		entry.spawn_limit   = 0;
		entry.dist          = 0;
		entry.max_x         = 0;
		entry.min_x         = 0;
		entry.max_y         = 0;
		entry.min_y         = 0;
		entry.delay         = 45000;
		entry.mindelay      = 15000;
		entry.despawn       = 0;
		entry.despawn_timer = 100;
		entry.wp_spawns     = 0;

		return entry;
	}

	static Spawngroup GetSpawngroupEntry(
		const std::vector<Spawngroup> &spawngroups,
		int spawngroup_id
	)
	{
		for (auto &spawngroup : spawngroups) {
			if (spawngroup.id == spawngroup_id) {
				return spawngroup;
			}
		}

		return NewEntity();
	}

	static Spawngroup FindOne(
		Database& db,
		int spawngroup_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spawngroup_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Spawngroup entry{};

			entry.id            = atoi(row[0]);
			entry.name          = row[1] ? row[1] : "";
			entry.spawn_limit   = atoi(row[2]);
			entry.dist          = static_cast<float>(atof(row[3]));
			entry.max_x         = static_cast<float>(atof(row[4]));
			entry.min_x         = static_cast<float>(atof(row[5]));
			entry.max_y         = static_cast<float>(atof(row[6]));
			entry.min_y         = static_cast<float>(atof(row[7]));
			entry.delay         = atoi(row[8]);
			entry.mindelay      = atoi(row[9]);
			entry.despawn       = atoi(row[10]);
			entry.despawn_timer = atoi(row[11]);
			entry.wp_spawns     = atoi(row[12]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawngroup_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawngroup_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Spawngroup spawngroup_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(spawngroup_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(spawngroup_entry.spawn_limit));
		update_values.push_back(columns[3] + " = " + std::to_string(spawngroup_entry.dist));
		update_values.push_back(columns[4] + " = " + std::to_string(spawngroup_entry.max_x));
		update_values.push_back(columns[5] + " = " + std::to_string(spawngroup_entry.min_x));
		update_values.push_back(columns[6] + " = " + std::to_string(spawngroup_entry.max_y));
		update_values.push_back(columns[7] + " = " + std::to_string(spawngroup_entry.min_y));
		update_values.push_back(columns[8] + " = " + std::to_string(spawngroup_entry.delay));
		update_values.push_back(columns[9] + " = " + std::to_string(spawngroup_entry.mindelay));
		update_values.push_back(columns[10] + " = " + std::to_string(spawngroup_entry.despawn));
		update_values.push_back(columns[11] + " = " + std::to_string(spawngroup_entry.despawn_timer));
		update_values.push_back(columns[12] + " = " + std::to_string(spawngroup_entry.wp_spawns));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spawngroup_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Spawngroup InsertOne(
		Database& db,
		Spawngroup spawngroup_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(spawngroup_entry.name) + "'");
		insert_values.push_back(std::to_string(spawngroup_entry.spawn_limit));
		insert_values.push_back(std::to_string(spawngroup_entry.dist));
		insert_values.push_back(std::to_string(spawngroup_entry.max_x));
		insert_values.push_back(std::to_string(spawngroup_entry.min_x));
		insert_values.push_back(std::to_string(spawngroup_entry.max_y));
		insert_values.push_back(std::to_string(spawngroup_entry.min_y));
		insert_values.push_back(std::to_string(spawngroup_entry.delay));
		insert_values.push_back(std::to_string(spawngroup_entry.mindelay));
		insert_values.push_back(std::to_string(spawngroup_entry.despawn));
		insert_values.push_back(std::to_string(spawngroup_entry.despawn_timer));
		insert_values.push_back(std::to_string(spawngroup_entry.wp_spawns));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spawngroup_entry.id = results.LastInsertedID();
			return spawngroup_entry;
		}

		spawngroup_entry = NewEntity();

		return spawngroup_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Spawngroup> spawngroup_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spawngroup_entry: spawngroup_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(spawngroup_entry.name) + "'");
			insert_values.push_back(std::to_string(spawngroup_entry.spawn_limit));
			insert_values.push_back(std::to_string(spawngroup_entry.dist));
			insert_values.push_back(std::to_string(spawngroup_entry.max_x));
			insert_values.push_back(std::to_string(spawngroup_entry.min_x));
			insert_values.push_back(std::to_string(spawngroup_entry.max_y));
			insert_values.push_back(std::to_string(spawngroup_entry.min_y));
			insert_values.push_back(std::to_string(spawngroup_entry.delay));
			insert_values.push_back(std::to_string(spawngroup_entry.mindelay));
			insert_values.push_back(std::to_string(spawngroup_entry.despawn));
			insert_values.push_back(std::to_string(spawngroup_entry.despawn_timer));
			insert_values.push_back(std::to_string(spawngroup_entry.wp_spawns));

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

	static std::vector<Spawngroup> All(Database& db)
	{
		std::vector<Spawngroup> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawngroup entry{};

			entry.id            = atoi(row[0]);
			entry.name          = row[1] ? row[1] : "";
			entry.spawn_limit   = atoi(row[2]);
			entry.dist          = static_cast<float>(atof(row[3]));
			entry.max_x         = static_cast<float>(atof(row[4]));
			entry.min_x         = static_cast<float>(atof(row[5]));
			entry.max_y         = static_cast<float>(atof(row[6]));
			entry.min_y         = static_cast<float>(atof(row[7]));
			entry.delay         = atoi(row[8]);
			entry.mindelay      = atoi(row[9]);
			entry.despawn       = atoi(row[10]);
			entry.despawn_timer = atoi(row[11]);
			entry.wp_spawns     = atoi(row[12]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Spawngroup> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Spawngroup> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawngroup entry{};

			entry.id            = atoi(row[0]);
			entry.name          = row[1] ? row[1] : "";
			entry.spawn_limit   = atoi(row[2]);
			entry.dist          = static_cast<float>(atof(row[3]));
			entry.max_x         = static_cast<float>(atof(row[4]));
			entry.min_x         = static_cast<float>(atof(row[5]));
			entry.max_y         = static_cast<float>(atof(row[6]));
			entry.min_y         = static_cast<float>(atof(row[7]));
			entry.delay         = atoi(row[8]);
			entry.mindelay      = atoi(row[9]);
			entry.despawn       = atoi(row[10]);
			entry.despawn_timer = atoi(row[11]);
			entry.wp_spawns     = atoi(row[12]);

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

#endif //EQEMU_BASE_SPAWNGROUP_REPOSITORY_H
