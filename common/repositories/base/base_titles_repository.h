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

#ifndef EQEMU_BASE_TITLES_REPOSITORY_H
#define EQEMU_BASE_TITLES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTitlesRepository {
public:
	struct Titles {
		int         id;
		int         skill_id;
		int         min_skill_value;
		int         max_skill_value;
		int         min_aa_points;
		int         max_aa_points;
		int         class;
		int         gender;
		int         char_id;
		int         status;
		int         item_id;
		std::string prefix;
		std::string suffix;
		int         title_set;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"skill_id",
			"min_skill_value",
			"max_skill_value",
			"min_aa_points",
			"max_aa_points",
			"class",
			"gender",
			"char_id",
			"status",
			"item_id",
			"prefix",
			"suffix",
			"title_set",
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
		return std::string("titles");
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

	static Titles NewEntity()
	{
		Titles entry{};

		entry.id              = 0;
		entry.skill_id        = -1;
		entry.min_skill_value = -1;
		entry.max_skill_value = -1;
		entry.min_aa_points   = -1;
		entry.max_aa_points   = -1;
		entry.class           = -1;
		entry.gender          = -1;
		entry.char_id         = -1;
		entry.status          = -1;
		entry.item_id         = -1;
		entry.prefix          = "";
		entry.suffix          = "";
		entry.title_set       = 0;

		return entry;
	}

	static Titles GetTitlesEntry(
		const std::vector<Titles> &titless,
		int titles_id
	)
	{
		for (auto &titles : titless) {
			if (titles.id == titles_id) {
				return titles;
			}
		}

		return NewEntity();
	}

	static Titles FindOne(
		Database& db,
		int titles_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				titles_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Titles entry{};

			entry.id              = atoi(row[0]);
			entry.skill_id        = atoi(row[1]);
			entry.min_skill_value = atoi(row[2]);
			entry.max_skill_value = atoi(row[3]);
			entry.min_aa_points   = atoi(row[4]);
			entry.max_aa_points   = atoi(row[5]);
			entry.class           = atoi(row[6]);
			entry.gender          = atoi(row[7]);
			entry.char_id         = atoi(row[8]);
			entry.status          = atoi(row[9]);
			entry.item_id         = atoi(row[10]);
			entry.prefix          = row[11] ? row[11] : "";
			entry.suffix          = row[12] ? row[12] : "";
			entry.title_set       = atoi(row[13]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int titles_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				titles_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Titles titles_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(titles_entry.skill_id));
		update_values.push_back(columns[2] + " = " + std::to_string(titles_entry.min_skill_value));
		update_values.push_back(columns[3] + " = " + std::to_string(titles_entry.max_skill_value));
		update_values.push_back(columns[4] + " = " + std::to_string(titles_entry.min_aa_points));
		update_values.push_back(columns[5] + " = " + std::to_string(titles_entry.max_aa_points));
		update_values.push_back(columns[6] + " = " + std::to_string(titles_entry.class));
		update_values.push_back(columns[7] + " = " + std::to_string(titles_entry.gender));
		update_values.push_back(columns[8] + " = " + std::to_string(titles_entry.char_id));
		update_values.push_back(columns[9] + " = " + std::to_string(titles_entry.status));
		update_values.push_back(columns[10] + " = " + std::to_string(titles_entry.item_id));
		update_values.push_back(columns[11] + " = '" + EscapeString(titles_entry.prefix) + "'");
		update_values.push_back(columns[12] + " = '" + EscapeString(titles_entry.suffix) + "'");
		update_values.push_back(columns[13] + " = " + std::to_string(titles_entry.title_set));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				titles_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Titles InsertOne(
		Database& db,
		Titles titles_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(titles_entry.skill_id));
		insert_values.push_back(std::to_string(titles_entry.min_skill_value));
		insert_values.push_back(std::to_string(titles_entry.max_skill_value));
		insert_values.push_back(std::to_string(titles_entry.min_aa_points));
		insert_values.push_back(std::to_string(titles_entry.max_aa_points));
		insert_values.push_back(std::to_string(titles_entry.class));
		insert_values.push_back(std::to_string(titles_entry.gender));
		insert_values.push_back(std::to_string(titles_entry.char_id));
		insert_values.push_back(std::to_string(titles_entry.status));
		insert_values.push_back(std::to_string(titles_entry.item_id));
		insert_values.push_back("'" + EscapeString(titles_entry.prefix) + "'");
		insert_values.push_back("'" + EscapeString(titles_entry.suffix) + "'");
		insert_values.push_back(std::to_string(titles_entry.title_set));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			titles_entry.id = results.LastInsertedID();
			return titles_entry;
		}

		titles_entry = NewEntity();

		return titles_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Titles> titles_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &titles_entry: titles_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(titles_entry.skill_id));
			insert_values.push_back(std::to_string(titles_entry.min_skill_value));
			insert_values.push_back(std::to_string(titles_entry.max_skill_value));
			insert_values.push_back(std::to_string(titles_entry.min_aa_points));
			insert_values.push_back(std::to_string(titles_entry.max_aa_points));
			insert_values.push_back(std::to_string(titles_entry.class));
			insert_values.push_back(std::to_string(titles_entry.gender));
			insert_values.push_back(std::to_string(titles_entry.char_id));
			insert_values.push_back(std::to_string(titles_entry.status));
			insert_values.push_back(std::to_string(titles_entry.item_id));
			insert_values.push_back("'" + EscapeString(titles_entry.prefix) + "'");
			insert_values.push_back("'" + EscapeString(titles_entry.suffix) + "'");
			insert_values.push_back(std::to_string(titles_entry.title_set));

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

	static std::vector<Titles> All(Database& db)
	{
		std::vector<Titles> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Titles entry{};

			entry.id              = atoi(row[0]);
			entry.skill_id        = atoi(row[1]);
			entry.min_skill_value = atoi(row[2]);
			entry.max_skill_value = atoi(row[3]);
			entry.min_aa_points   = atoi(row[4]);
			entry.max_aa_points   = atoi(row[5]);
			entry.class           = atoi(row[6]);
			entry.gender          = atoi(row[7]);
			entry.char_id         = atoi(row[8]);
			entry.status          = atoi(row[9]);
			entry.item_id         = atoi(row[10]);
			entry.prefix          = row[11] ? row[11] : "";
			entry.suffix          = row[12] ? row[12] : "";
			entry.title_set       = atoi(row[13]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Titles> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Titles> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Titles entry{};

			entry.id              = atoi(row[0]);
			entry.skill_id        = atoi(row[1]);
			entry.min_skill_value = atoi(row[2]);
			entry.max_skill_value = atoi(row[3]);
			entry.min_aa_points   = atoi(row[4]);
			entry.max_aa_points   = atoi(row[5]);
			entry.class           = atoi(row[6]);
			entry.gender          = atoi(row[7]);
			entry.char_id         = atoi(row[8]);
			entry.status          = atoi(row[9]);
			entry.item_id         = atoi(row[10]);
			entry.prefix          = row[11] ? row[11] : "";
			entry.suffix          = row[12] ? row[12] : "";
			entry.title_set       = atoi(row[13]);

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

#endif //EQEMU_BASE_TITLES_REPOSITORY_H
