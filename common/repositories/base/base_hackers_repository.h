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

#ifndef EQEMU_BASE_HACKERS_REPOSITORY_H
#define EQEMU_BASE_HACKERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseHackersRepository {
public:
	struct Hackers {
		int         id;
		std::string account;
		std::string name;
		std::string hacked;
		std::string zone;
		std::string date;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account",
			"name",
			"hacked",
			"zone",
			"date",
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
		return std::string("hackers");
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

	static Hackers NewEntity()
	{
		Hackers entry{};

		entry.id      = 0;
		entry.account = "";
		entry.name    = "";
		entry.hacked  = "";
		entry.zone    = "";
		entry.date    = current_timestamp();

		return entry;
	}

	static Hackers GetHackersEntry(
		const std::vector<Hackers> &hackerss,
		int hackers_id
	)
	{
		for (auto &hackers : hackerss) {
			if (hackers.id == hackers_id) {
				return hackers;
			}
		}

		return NewEntity();
	}

	static Hackers FindOne(
		int hackers_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				hackers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Hackers entry{};

			entry.id      = atoi(row[0]);
			entry.account = row[1] ? row[1] : "";
			entry.name    = row[2] ? row[2] : "";
			entry.hacked  = row[3] ? row[3] : "";
			entry.zone    = row[4] ? row[4] : "";
			entry.date    = row[5] ? row[5] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int hackers_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				hackers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Hackers hackers_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(hackers_entry.account) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(hackers_entry.name) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(hackers_entry.hacked) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(hackers_entry.zone) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(hackers_entry.date) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				hackers_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Hackers InsertOne(
		Hackers hackers_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(hackers_entry.account) + "'");
		insert_values.push_back("'" + EscapeString(hackers_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(hackers_entry.hacked) + "'");
		insert_values.push_back("'" + EscapeString(hackers_entry.zone) + "'");
		insert_values.push_back("'" + EscapeString(hackers_entry.date) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			hackers_entry.id = results.LastInsertedID();
			return hackers_entry;
		}

		hackers_entry = NewEntity();

		return hackers_entry;
	}

	static int InsertMany(
		std::vector<Hackers> hackers_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &hackers_entry: hackers_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(hackers_entry.account) + "'");
			insert_values.push_back("'" + EscapeString(hackers_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(hackers_entry.hacked) + "'");
			insert_values.push_back("'" + EscapeString(hackers_entry.zone) + "'");
			insert_values.push_back("'" + EscapeString(hackers_entry.date) + "'");

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

	static std::vector<Hackers> All()
	{
		std::vector<Hackers> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Hackers entry{};

			entry.id      = atoi(row[0]);
			entry.account = row[1] ? row[1] : "";
			entry.name    = row[2] ? row[2] : "";
			entry.hacked  = row[3] ? row[3] : "";
			entry.zone    = row[4] ? row[4] : "";
			entry.date    = row[5] ? row[5] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Hackers> GetWhere(std::string where_filter)
	{
		std::vector<Hackers> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Hackers entry{};

			entry.id      = atoi(row[0]);
			entry.account = row[1] ? row[1] : "";
			entry.name    = row[2] ? row[2] : "";
			entry.hacked  = row[3] ? row[3] : "";
			entry.zone    = row[4] ? row[4] : "";
			entry.date    = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_HACKERS_REPOSITORY_H
