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

#ifndef EQEMU_BASE_SPAWNENTRY_REPOSITORY_H
#define EQEMU_BASE_SPAWNENTRY_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpawnentryRepository {
public:
	struct Spawnentry {
		int spawngroupID;
		int npcID;
		int chance;
		int condition_value_filter;
	};

	static std::string PrimaryKey()
	{
		return std::string("spawngroupID");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"spawngroupID",
			"npcID",
			"chance",
			"condition_value_filter",
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
		return std::string("spawnentry");
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

	static Spawnentry NewEntity()
	{
		Spawnentry entry{};

		entry.spawngroupID           = 0;
		entry.npcID                  = 0;
		entry.chance                 = 0;
		entry.condition_value_filter = 1;

		return entry;
	}

	static Spawnentry GetSpawnentryEntry(
		const std::vector<Spawnentry> &spawnentrys,
		int spawnentry_id
	)
	{
		for (auto &spawnentry : spawnentrys) {
			if (spawnentry.spawngroupID == spawnentry_id) {
				return spawnentry;
			}
		}

		return NewEntity();
	}

	static Spawnentry FindOne(
		int spawnentry_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spawnentry_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Spawnentry entry{};

			entry.spawngroupID           = atoi(row[0]);
			entry.npcID                  = atoi(row[1]);
			entry.chance                 = atoi(row[2]);
			entry.condition_value_filter = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int spawnentry_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawnentry_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Spawnentry spawnentry_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(spawnentry_entry.spawngroupID));
		update_values.push_back(columns[1] + " = " + std::to_string(spawnentry_entry.npcID));
		update_values.push_back(columns[2] + " = " + std::to_string(spawnentry_entry.chance));
		update_values.push_back(columns[3] + " = " + std::to_string(spawnentry_entry.condition_value_filter));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spawnentry_entry.spawngroupID
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Spawnentry InsertOne(
		Spawnentry spawnentry_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spawnentry_entry.spawngroupID));
		insert_values.push_back(std::to_string(spawnentry_entry.npcID));
		insert_values.push_back(std::to_string(spawnentry_entry.chance));
		insert_values.push_back(std::to_string(spawnentry_entry.condition_value_filter));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spawnentry_entry.spawngroupID = results.LastInsertedID();
			return spawnentry_entry;
		}

		spawnentry_entry = NewEntity();

		return spawnentry_entry;
	}

	static int InsertMany(
		std::vector<Spawnentry> spawnentry_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spawnentry_entry: spawnentry_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spawnentry_entry.spawngroupID));
			insert_values.push_back(std::to_string(spawnentry_entry.npcID));
			insert_values.push_back(std::to_string(spawnentry_entry.chance));
			insert_values.push_back(std::to_string(spawnentry_entry.condition_value_filter));

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

	static std::vector<Spawnentry> All()
	{
		std::vector<Spawnentry> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawnentry entry{};

			entry.spawngroupID           = atoi(row[0]);
			entry.npcID                  = atoi(row[1]);
			entry.chance                 = atoi(row[2]);
			entry.condition_value_filter = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Spawnentry> GetWhere(std::string where_filter)
	{
		std::vector<Spawnentry> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawnentry entry{};

			entry.spawngroupID           = atoi(row[0]);
			entry.npcID                  = atoi(row[1]);
			entry.chance                 = atoi(row[2]);
			entry.condition_value_filter = atoi(row[3]);

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

#endif //EQEMU_BASE_SPAWNENTRY_REPOSITORY_H
