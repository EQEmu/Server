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

#ifndef EQEMU_BASE_FORAGE_REPOSITORY_H
#define EQEMU_BASE_FORAGE_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseForageRepository {
public:
	struct Forage {
		int         id;
		int         zoneid;
		int         Itemid;
		int         level;
		int         chance;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
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
			"Itemid",
			"level",
			"chance",
			"min_expansion",
			"max_expansion",
			"content_flags",
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
		return std::string("forage");
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

	static Forage NewEntity()
	{
		Forage entry{};

		entry.id            = 0;
		entry.zoneid        = 0;
		entry.Itemid        = 0;
		entry.level         = 0;
		entry.chance        = 0;
		entry.min_expansion = 0;
		entry.max_expansion = 0;
		entry.content_flags = "";

		return entry;
	}

	static Forage GetForageEntry(
		const std::vector<Forage> &forages,
		int forage_id
	)
	{
		for (auto &forage : forages) {
			if (forage.id == forage_id) {
				return forage;
			}
		}

		return NewEntity();
	}

	static Forage FindOne(
		int forage_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				forage_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Forage entry{};

			entry.id            = atoi(row[0]);
			entry.zoneid        = atoi(row[1]);
			entry.Itemid        = atoi(row[2]);
			entry.level         = atoi(row[3]);
			entry.chance        = atoi(row[4]);
			entry.min_expansion = atoi(row[5]);
			entry.max_expansion = atoi(row[6]);
			entry.content_flags = row[7] ? row[7] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int forage_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				forage_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Forage forage_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(forage_entry.zoneid));
		update_values.push_back(columns[2] + " = " + std::to_string(forage_entry.Itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(forage_entry.level));
		update_values.push_back(columns[4] + " = " + std::to_string(forage_entry.chance));
		update_values.push_back(columns[5] + " = " + std::to_string(forage_entry.min_expansion));
		update_values.push_back(columns[6] + " = " + std::to_string(forage_entry.max_expansion));
		update_values.push_back(columns[7] + " = '" + EscapeString(forage_entry.content_flags) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				forage_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Forage InsertOne(
		Forage forage_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(forage_entry.zoneid));
		insert_values.push_back(std::to_string(forage_entry.Itemid));
		insert_values.push_back(std::to_string(forage_entry.level));
		insert_values.push_back(std::to_string(forage_entry.chance));
		insert_values.push_back(std::to_string(forage_entry.min_expansion));
		insert_values.push_back(std::to_string(forage_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(forage_entry.content_flags) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			forage_entry.id = results.LastInsertedID();
			return forage_entry;
		}

		forage_entry = NewEntity();

		return forage_entry;
	}

	static int InsertMany(
		std::vector<Forage> forage_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &forage_entry: forage_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(forage_entry.zoneid));
			insert_values.push_back(std::to_string(forage_entry.Itemid));
			insert_values.push_back(std::to_string(forage_entry.level));
			insert_values.push_back(std::to_string(forage_entry.chance));
			insert_values.push_back(std::to_string(forage_entry.min_expansion));
			insert_values.push_back(std::to_string(forage_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(forage_entry.content_flags) + "'");

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

	static std::vector<Forage> All()
	{
		std::vector<Forage> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Forage entry{};

			entry.id            = atoi(row[0]);
			entry.zoneid        = atoi(row[1]);
			entry.Itemid        = atoi(row[2]);
			entry.level         = atoi(row[3]);
			entry.chance        = atoi(row[4]);
			entry.min_expansion = atoi(row[5]);
			entry.max_expansion = atoi(row[6]);
			entry.content_flags = row[7] ? row[7] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Forage> GetWhere(std::string where_filter)
	{
		std::vector<Forage> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Forage entry{};

			entry.id            = atoi(row[0]);
			entry.zoneid        = atoi(row[1]);
			entry.Itemid        = atoi(row[2]);
			entry.level         = atoi(row[3]);
			entry.chance        = atoi(row[4]);
			entry.min_expansion = atoi(row[5]);
			entry.max_expansion = atoi(row[6]);
			entry.content_flags = row[7] ? row[7] : "";

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

#endif //EQEMU_BASE_FORAGE_REPOSITORY_H
