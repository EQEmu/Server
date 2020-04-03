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
 */

#ifndef EQEMU_LOOTTABLE_REPOSITORY_H
#define EQEMU_LOOTTABLE_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class LoottableRepository {
public:
	struct Loottable {
		int         id;
		std::string name;
		int         mincash;
		int         maxcash;
		int         avgcoin;
		int8        done;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"mincash",
			"maxcash",
			"avgcoin",
			"done",
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
		return std::string("loottable");
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

	static Loottable NewEntity()
	{
		Loottable entry{};

		entry.id      = 0;
		entry.name    = "";
		entry.mincash = 0;
		entry.maxcash = 0;
		entry.avgcoin = 0;
		entry.done    = 0;

		return entry;
	}

	static Loottable GetLoottableEntry(
		const std::vector<Loottable> &loottables,
		int loottable_id
	)
	{
		for (auto &loottable : loottables) {
			if (loottable.id == loottable_id) {
				return loottable;
			}
		}

		return NewEntity();
	}

	static Loottable FindOne(
		int loottable_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				loottable_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Loottable entry{};

			entry.id      = atoi(row[0]);
			entry.name    = row[1];
			entry.mincash = atoi(row[2]);
			entry.maxcash = atoi(row[3]);
			entry.avgcoin = atoi(row[4]);
			entry.done    = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int loottable_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				loottable_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Loottable loottable_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(loottable_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(loottable_entry.mincash));
		update_values.push_back(columns[3] + " = " + std::to_string(loottable_entry.maxcash));
		update_values.push_back(columns[4] + " = " + std::to_string(loottable_entry.avgcoin));
		update_values.push_back(columns[5] + " = " + std::to_string(loottable_entry.done));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				loottable_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Loottable InsertOne(
		Loottable loottable_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(loottable_entry.name) + "'");
		insert_values.push_back(std::to_string(loottable_entry.mincash));
		insert_values.push_back(std::to_string(loottable_entry.maxcash));
		insert_values.push_back(std::to_string(loottable_entry.avgcoin));
		insert_values.push_back(std::to_string(loottable_entry.done));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			loottable_entry.id = results.LastInsertedID();
			return loottable_entry;
		}

		loottable_entry = InstanceListRepository::NewEntity();

		return loottable_entry;
	}

	static int InsertMany(
		std::vector<Loottable> loottable_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &loottable_entry: loottable_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(loottable_entry.name) + "'");
			insert_values.push_back(std::to_string(loottable_entry.mincash));
			insert_values.push_back(std::to_string(loottable_entry.maxcash));
			insert_values.push_back(std::to_string(loottable_entry.avgcoin));
			insert_values.push_back(std::to_string(loottable_entry.done));

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

	static std::vector<Loottable> All()
	{
		std::vector<Loottable> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Loottable entry{};

			entry.id      = atoi(row[0]);
			entry.name    = row[1];
			entry.mincash = atoi(row[2]);
			entry.maxcash = atoi(row[3]);
			entry.avgcoin = atoi(row[4]);
			entry.done    = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_LOOTTABLE_REPOSITORY_H
