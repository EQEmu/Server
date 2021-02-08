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

#ifndef EQEMU_BASE_GRAVEYARD_REPOSITORY_H
#define EQEMU_BASE_GRAVEYARD_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGraveyardRepository {
public:
	struct Graveyard {
		int   id;
		int   zone_id;
		float x;
		float y;
		float z;
		float heading;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone_id",
			"x",
			"y",
			"z",
			"heading",
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
		return std::string("graveyard");
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

	static Graveyard NewEntity()
	{
		Graveyard entry{};

		entry.id      = 0;
		entry.zone_id = 0;
		entry.x       = 0;
		entry.y       = 0;
		entry.z       = 0;
		entry.heading = 0;

		return entry;
	}

	static Graveyard GetGraveyardEntry(
		const std::vector<Graveyard> &graveyards,
		int graveyard_id
	)
	{
		for (auto &graveyard : graveyards) {
			if (graveyard.id == graveyard_id) {
				return graveyard;
			}
		}

		return NewEntity();
	}

	static Graveyard FindOne(
		Database& db,
		int graveyard_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				graveyard_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Graveyard entry{};

			entry.id      = atoi(row[0]);
			entry.zone_id = atoi(row[1]);
			entry.x       = static_cast<float>(atof(row[2]));
			entry.y       = static_cast<float>(atof(row[3]));
			entry.z       = static_cast<float>(atof(row[4]));
			entry.heading = static_cast<float>(atof(row[5]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int graveyard_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				graveyard_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Graveyard graveyard_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(graveyard_entry.zone_id));
		update_values.push_back(columns[2] + " = " + std::to_string(graveyard_entry.x));
		update_values.push_back(columns[3] + " = " + std::to_string(graveyard_entry.y));
		update_values.push_back(columns[4] + " = " + std::to_string(graveyard_entry.z));
		update_values.push_back(columns[5] + " = " + std::to_string(graveyard_entry.heading));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				graveyard_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Graveyard InsertOne(
		Database& db,
		Graveyard graveyard_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(graveyard_entry.zone_id));
		insert_values.push_back(std::to_string(graveyard_entry.x));
		insert_values.push_back(std::to_string(graveyard_entry.y));
		insert_values.push_back(std::to_string(graveyard_entry.z));
		insert_values.push_back(std::to_string(graveyard_entry.heading));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			graveyard_entry.id = results.LastInsertedID();
			return graveyard_entry;
		}

		graveyard_entry = NewEntity();

		return graveyard_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Graveyard> graveyard_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &graveyard_entry: graveyard_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(graveyard_entry.zone_id));
			insert_values.push_back(std::to_string(graveyard_entry.x));
			insert_values.push_back(std::to_string(graveyard_entry.y));
			insert_values.push_back(std::to_string(graveyard_entry.z));
			insert_values.push_back(std::to_string(graveyard_entry.heading));

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

	static std::vector<Graveyard> All(Database& db)
	{
		std::vector<Graveyard> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Graveyard entry{};

			entry.id      = atoi(row[0]);
			entry.zone_id = atoi(row[1]);
			entry.x       = static_cast<float>(atof(row[2]));
			entry.y       = static_cast<float>(atof(row[3]));
			entry.z       = static_cast<float>(atof(row[4]));
			entry.heading = static_cast<float>(atof(row[5]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Graveyard> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Graveyard> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Graveyard entry{};

			entry.id      = atoi(row[0]);
			entry.zone_id = atoi(row[1]);
			entry.x       = static_cast<float>(atof(row[2]));
			entry.y       = static_cast<float>(atof(row[3]));
			entry.z       = static_cast<float>(atof(row[4]));
			entry.heading = static_cast<float>(atof(row[5]));

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

#endif //EQEMU_BASE_GRAVEYARD_REPOSITORY_H
