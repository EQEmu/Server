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

#ifndef EQEMU_BASE_GOALLISTS_REPOSITORY_H
#define EQEMU_BASE_GOALLISTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGoallistsRepository {
public:
	struct Goallists {
		int listid;
		int entry;
	};

	static std::string PrimaryKey()
	{
		return std::string("listid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"listid",
			"entry",
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
		return std::string("goallists");
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

	static Goallists NewEntity()
	{
		Goallists entry{};

		entry.listid = 0;
		entry.entry  = 0;

		return entry;
	}

	static Goallists GetGoallistsEntry(
		const std::vector<Goallists> &goallistss,
		int goallists_id
	)
	{
		for (auto &goallists : goallistss) {
			if (goallists.listid == goallists_id) {
				return goallists;
			}
		}

		return NewEntity();
	}

	static Goallists FindOne(
		int goallists_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				goallists_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Goallists entry{};

			entry.listid = atoi(row[0]);
			entry.entry  = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int goallists_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				goallists_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Goallists goallists_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(goallists_entry.listid));
		update_values.push_back(columns[1] + " = " + std::to_string(goallists_entry.entry));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				goallists_entry.listid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Goallists InsertOne(
		Goallists goallists_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(goallists_entry.listid));
		insert_values.push_back(std::to_string(goallists_entry.entry));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			goallists_entry.listid = results.LastInsertedID();
			return goallists_entry;
		}

		goallists_entry = NewEntity();

		return goallists_entry;
	}

	static int InsertMany(
		std::vector<Goallists> goallists_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &goallists_entry: goallists_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(goallists_entry.listid));
			insert_values.push_back(std::to_string(goallists_entry.entry));

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

	static std::vector<Goallists> All()
	{
		std::vector<Goallists> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Goallists entry{};

			entry.listid = atoi(row[0]);
			entry.entry  = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Goallists> GetWhere(std::string where_filter)
	{
		std::vector<Goallists> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Goallists entry{};

			entry.listid = atoi(row[0]);
			entry.entry  = atoi(row[1]);

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

#endif //EQEMU_BASE_GOALLISTS_REPOSITORY_H
