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

#ifndef EQEMU_BASE_HORSES_REPOSITORY_H
#define EQEMU_BASE_HORSES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseHorsesRepository {
public:
	struct Horses {
		std::string filename;
		int16       race;
		int8        gender;
		int8        texture;
		float       mountspeed;
		std::string notes;
	};

	static std::string PrimaryKey()
	{
		return std::string("filename");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"filename",
			"race",
			"gender",
			"texture",
			"mountspeed",
			"notes",
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
		return std::string("horses");
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

	static Horses NewEntity()
	{
		Horses entry{};

		entry.filename   = "";
		entry.race       = 216;
		entry.gender     = 0;
		entry.texture    = 0;
		entry.mountspeed = 0.75;
		entry.notes      = "Notes";

		return entry;
	}

	static Horses GetHorsesEntry(
		const std::vector<Horses> &horsess,
		int horses_id
	)
	{
		for (auto &horses : horsess) {
			if (horses.filename == horses_id) {
				return horses;
			}
		}

		return NewEntity();
	}

	static Horses FindOne(
		int horses_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				horses_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Horses entry{};

			entry.filename   = row[0] ? row[0] : "";
			entry.race       = atoi(row[1]);
			entry.gender     = atoi(row[2]);
			entry.texture    = atoi(row[3]);
			entry.mountspeed = atof(row[4]);
			entry.notes      = row[5] ? row[5] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int horses_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				horses_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Horses horses_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = '" + EscapeString(horses_entry.filename) + "'");
		update_values.push_back(columns[1] + " = " + std::to_string(horses_entry.race));
		update_values.push_back(columns[2] + " = " + std::to_string(horses_entry.gender));
		update_values.push_back(columns[3] + " = " + std::to_string(horses_entry.texture));
		update_values.push_back(columns[4] + " = " + std::to_string(horses_entry.mountspeed));
		update_values.push_back(columns[5] + " = '" + EscapeString(horses_entry.notes) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				horses_entry.filename
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Horses InsertOne(
		Horses horses_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(horses_entry.filename) + "'");
		insert_values.push_back(std::to_string(horses_entry.race));
		insert_values.push_back(std::to_string(horses_entry.gender));
		insert_values.push_back(std::to_string(horses_entry.texture));
		insert_values.push_back(std::to_string(horses_entry.mountspeed));
		insert_values.push_back("'" + EscapeString(horses_entry.notes) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			horses_entry.filename = results.LastInsertedID();
			return horses_entry;
		}

		horses_entry = NewEntity();

		return horses_entry;
	}

	static int InsertMany(
		std::vector<Horses> horses_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &horses_entry: horses_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(horses_entry.filename) + "'");
			insert_values.push_back(std::to_string(horses_entry.race));
			insert_values.push_back(std::to_string(horses_entry.gender));
			insert_values.push_back(std::to_string(horses_entry.texture));
			insert_values.push_back(std::to_string(horses_entry.mountspeed));
			insert_values.push_back("'" + EscapeString(horses_entry.notes) + "'");

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

	static std::vector<Horses> All()
	{
		std::vector<Horses> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Horses entry{};

			entry.filename   = row[0] ? row[0] : "";
			entry.race       = atoi(row[1]);
			entry.gender     = atoi(row[2]);
			entry.texture    = atoi(row[3]);
			entry.mountspeed = atof(row[4]);
			entry.notes      = row[5] ? row[5] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Horses> GetWhere(std::string where_filter)
	{
		std::vector<Horses> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Horses entry{};

			entry.filename   = row[0] ? row[0] : "";
			entry.race       = atoi(row[1]);
			entry.gender     = atoi(row[2]);
			entry.texture    = atoi(row[3]);
			entry.mountspeed = atof(row[4]);
			entry.notes      = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_HORSES_REPOSITORY_H
