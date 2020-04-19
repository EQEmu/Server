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

#ifndef EQEMU_BASE_SPAWN_CONDITIONS_REPOSITORY_H
#define EQEMU_BASE_SPAWN_CONDITIONS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpawnConditionsRepository {
public:
	struct SpawnConditions {
		std::string zone;
		int         id;
		int         value;
		int         onchange;
		std::string name;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"zone",
			"id",
			"value",
			"onchange",
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
		return std::string("spawn_conditions");
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

	static SpawnConditions NewEntity()
	{
		SpawnConditions entry{};

		entry.zone     = "";
		entry.id       = 1;
		entry.value    = 0;
		entry.onchange = 0;
		entry.name     = "";

		return entry;
	}

	static SpawnConditions GetSpawnConditionsEntry(
		const std::vector<SpawnConditions> &spawn_conditionss,
		int spawn_conditions_id
	)
	{
		for (auto &spawn_conditions : spawn_conditionss) {
			if (spawn_conditions.id == spawn_conditions_id) {
				return spawn_conditions;
			}
		}

		return NewEntity();
	}

	static SpawnConditions FindOne(
		int spawn_conditions_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spawn_conditions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpawnConditions entry{};

			entry.zone     = row[0] ? row[0] : "";
			entry.id       = atoi(row[1]);
			entry.value    = atoi(row[2]);
			entry.onchange = atoi(row[3]);
			entry.name     = row[4] ? row[4] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int spawn_conditions_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawn_conditions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		SpawnConditions spawn_conditions_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = '" + EscapeString(spawn_conditions_entry.zone) + "'");
		update_values.push_back(columns[1] + " = " + std::to_string(spawn_conditions_entry.id));
		update_values.push_back(columns[2] + " = " + std::to_string(spawn_conditions_entry.value));
		update_values.push_back(columns[3] + " = " + std::to_string(spawn_conditions_entry.onchange));
		update_values.push_back(columns[4] + " = '" + EscapeString(spawn_conditions_entry.name) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spawn_conditions_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpawnConditions InsertOne(
		SpawnConditions spawn_conditions_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(spawn_conditions_entry.zone) + "'");
		insert_values.push_back(std::to_string(spawn_conditions_entry.id));
		insert_values.push_back(std::to_string(spawn_conditions_entry.value));
		insert_values.push_back(std::to_string(spawn_conditions_entry.onchange));
		insert_values.push_back("'" + EscapeString(spawn_conditions_entry.name) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spawn_conditions_entry.id = results.LastInsertedID();
			return spawn_conditions_entry;
		}

		spawn_conditions_entry = NewEntity();

		return spawn_conditions_entry;
	}

	static int InsertMany(
		std::vector<SpawnConditions> spawn_conditions_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spawn_conditions_entry: spawn_conditions_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(spawn_conditions_entry.zone) + "'");
			insert_values.push_back(std::to_string(spawn_conditions_entry.id));
			insert_values.push_back(std::to_string(spawn_conditions_entry.value));
			insert_values.push_back(std::to_string(spawn_conditions_entry.onchange));
			insert_values.push_back("'" + EscapeString(spawn_conditions_entry.name) + "'");

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

	static std::vector<SpawnConditions> All()
	{
		std::vector<SpawnConditions> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnConditions entry{};

			entry.zone     = row[0] ? row[0] : "";
			entry.id       = atoi(row[1]);
			entry.value    = atoi(row[2]);
			entry.onchange = atoi(row[3]);
			entry.name     = row[4] ? row[4] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SpawnConditions> GetWhere(std::string where_filter)
	{
		std::vector<SpawnConditions> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnConditions entry{};

			entry.zone     = row[0] ? row[0] : "";
			entry.id       = atoi(row[1]);
			entry.value    = atoi(row[2]);
			entry.onchange = atoi(row[3]);
			entry.name     = row[4] ? row[4] : "";

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

	static int Truncate()
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_SPAWN_CONDITIONS_REPOSITORY_H
