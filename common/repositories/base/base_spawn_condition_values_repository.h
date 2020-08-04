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

#ifndef EQEMU_BASE_SPAWN_CONDITION_VALUES_REPOSITORY_H
#define EQEMU_BASE_SPAWN_CONDITION_VALUES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpawnConditionValuesRepository {
public:
	struct SpawnConditionValues {
		int         id;
		int         value;
		std::string zone;
		int         instance_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"value",
			"zone",
			"instance_id",
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
		return std::string("spawn_condition_values");
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

	static SpawnConditionValues NewEntity()
	{
		SpawnConditionValues entry{};

		entry.id          = 0;
		entry.value       = 0;
		entry.zone        = "";
		entry.instance_id = 0;

		return entry;
	}

	static SpawnConditionValues GetSpawnConditionValuesEntry(
		const std::vector<SpawnConditionValues> &spawn_condition_valuess,
		int spawn_condition_values_id
	)
	{
		for (auto &spawn_condition_values : spawn_condition_valuess) {
			if (spawn_condition_values.id == spawn_condition_values_id) {
				return spawn_condition_values;
			}
		}

		return NewEntity();
	}

	static SpawnConditionValues FindOne(
		int spawn_condition_values_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spawn_condition_values_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpawnConditionValues entry{};

			entry.id          = atoi(row[0]);
			entry.value       = atoi(row[1]);
			entry.zone        = row[2] ? row[2] : "";
			entry.instance_id = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int spawn_condition_values_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawn_condition_values_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		SpawnConditionValues spawn_condition_values_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(spawn_condition_values_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(spawn_condition_values_entry.value));
		update_values.push_back(columns[2] + " = '" + EscapeString(spawn_condition_values_entry.zone) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(spawn_condition_values_entry.instance_id));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spawn_condition_values_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpawnConditionValues InsertOne(
		SpawnConditionValues spawn_condition_values_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spawn_condition_values_entry.id));
		insert_values.push_back(std::to_string(spawn_condition_values_entry.value));
		insert_values.push_back("'" + EscapeString(spawn_condition_values_entry.zone) + "'");
		insert_values.push_back(std::to_string(spawn_condition_values_entry.instance_id));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spawn_condition_values_entry.id = results.LastInsertedID();
			return spawn_condition_values_entry;
		}

		spawn_condition_values_entry = NewEntity();

		return spawn_condition_values_entry;
	}

	static int InsertMany(
		std::vector<SpawnConditionValues> spawn_condition_values_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spawn_condition_values_entry: spawn_condition_values_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spawn_condition_values_entry.id));
			insert_values.push_back(std::to_string(spawn_condition_values_entry.value));
			insert_values.push_back("'" + EscapeString(spawn_condition_values_entry.zone) + "'");
			insert_values.push_back(std::to_string(spawn_condition_values_entry.instance_id));

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

	static std::vector<SpawnConditionValues> All()
	{
		std::vector<SpawnConditionValues> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnConditionValues entry{};

			entry.id          = atoi(row[0]);
			entry.value       = atoi(row[1]);
			entry.zone        = row[2] ? row[2] : "";
			entry.instance_id = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SpawnConditionValues> GetWhere(std::string where_filter)
	{
		std::vector<SpawnConditionValues> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnConditionValues entry{};

			entry.id          = atoi(row[0]);
			entry.value       = atoi(row[1]);
			entry.zone        = row[2] ? row[2] : "";
			entry.instance_id = atoi(row[3]);

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

#endif //EQEMU_BASE_SPAWN_CONDITION_VALUES_REPOSITORY_H
