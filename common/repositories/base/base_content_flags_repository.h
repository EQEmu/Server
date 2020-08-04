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

#ifndef EQEMU_BASE_CONTENT_FLAGS_REPOSITORY_H
#define EQEMU_BASE_CONTENT_FLAGS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseContentFlagsRepository {
public:
	struct ContentFlags {
		int         id;
		std::string flag_name;
		int         enabled;
		std::string notes;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"flag_name",
			"enabled",
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
		return std::string("content_flags");
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

	static ContentFlags NewEntity()
	{
		ContentFlags entry{};

		entry.id        = 0;
		entry.flag_name = "";
		entry.enabled   = 0;
		entry.notes     = "";

		return entry;
	}

	static ContentFlags GetContentFlagsEntry(
		const std::vector<ContentFlags> &content_flagss,
		int content_flags_id
	)
	{
		for (auto &content_flags : content_flagss) {
			if (content_flags.id == content_flags_id) {
				return content_flags;
			}
		}

		return NewEntity();
	}

	static ContentFlags FindOne(
		int content_flags_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				content_flags_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ContentFlags entry{};

			entry.id        = atoi(row[0]);
			entry.flag_name = row[1] ? row[1] : "";
			entry.enabled   = atoi(row[2]);
			entry.notes     = row[3] ? row[3] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int content_flags_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				content_flags_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		ContentFlags content_flags_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(content_flags_entry.flag_name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(content_flags_entry.enabled));
		update_values.push_back(columns[3] + " = '" + EscapeString(content_flags_entry.notes) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				content_flags_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ContentFlags InsertOne(
		ContentFlags content_flags_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(content_flags_entry.flag_name) + "'");
		insert_values.push_back(std::to_string(content_flags_entry.enabled));
		insert_values.push_back("'" + EscapeString(content_flags_entry.notes) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			content_flags_entry.id = results.LastInsertedID();
			return content_flags_entry;
		}

		content_flags_entry = NewEntity();

		return content_flags_entry;
	}

	static int InsertMany(
		std::vector<ContentFlags> content_flags_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &content_flags_entry: content_flags_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(content_flags_entry.flag_name) + "'");
			insert_values.push_back(std::to_string(content_flags_entry.enabled));
			insert_values.push_back("'" + EscapeString(content_flags_entry.notes) + "'");

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

	static std::vector<ContentFlags> All()
	{
		std::vector<ContentFlags> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ContentFlags entry{};

			entry.id        = atoi(row[0]);
			entry.flag_name = row[1] ? row[1] : "";
			entry.enabled   = atoi(row[2]);
			entry.notes     = row[3] ? row[3] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<ContentFlags> GetWhere(std::string where_filter)
	{
		std::vector<ContentFlags> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ContentFlags entry{};

			entry.id        = atoi(row[0]);
			entry.flag_name = row[1] ? row[1] : "";
			entry.enabled   = atoi(row[2]);
			entry.notes     = row[3] ? row[3] : "";

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

#endif //EQEMU_BASE_CONTENT_FLAGS_REPOSITORY_H
