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

#ifndef EQEMU_BASE_TIMERS_REPOSITORY_H
#define EQEMU_BASE_TIMERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTimersRepository {
public:
	struct Timers {
		int char_id;
		int type;
		int start;
		int duration;
		int enable;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"type",
			"start",
			"duration",
			"enable",
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
		return std::string("timers");
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

	static Timers NewEntity()
	{
		Timers entry{};

		entry.char_id  = 0;
		entry.type     = 0;
		entry.start    = 0;
		entry.duration = 0;
		entry.enable   = 0;

		return entry;
	}

	static Timers GetTimersEntry(
		const std::vector<Timers> &timerss,
		int timers_id
	)
	{
		for (auto &timers : timerss) {
			if (timers.char_id == timers_id) {
				return timers;
			}
		}

		return NewEntity();
	}

	static Timers FindOne(
		int timers_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				timers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Timers entry{};

			entry.char_id  = atoi(row[0]);
			entry.type     = atoi(row[1]);
			entry.start    = atoi(row[2]);
			entry.duration = atoi(row[3]);
			entry.enable   = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int timers_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				timers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Timers timers_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(timers_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(timers_entry.type));
		update_values.push_back(columns[2] + " = " + std::to_string(timers_entry.start));
		update_values.push_back(columns[3] + " = " + std::to_string(timers_entry.duration));
		update_values.push_back(columns[4] + " = " + std::to_string(timers_entry.enable));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				timers_entry.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Timers InsertOne(
		Timers timers_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(timers_entry.char_id));
		insert_values.push_back(std::to_string(timers_entry.type));
		insert_values.push_back(std::to_string(timers_entry.start));
		insert_values.push_back(std::to_string(timers_entry.duration));
		insert_values.push_back(std::to_string(timers_entry.enable));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			timers_entry.char_id = results.LastInsertedID();
			return timers_entry;
		}

		timers_entry = NewEntity();

		return timers_entry;
	}

	static int InsertMany(
		std::vector<Timers> timers_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &timers_entry: timers_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(timers_entry.char_id));
			insert_values.push_back(std::to_string(timers_entry.type));
			insert_values.push_back(std::to_string(timers_entry.start));
			insert_values.push_back(std::to_string(timers_entry.duration));
			insert_values.push_back(std::to_string(timers_entry.enable));

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

	static std::vector<Timers> All()
	{
		std::vector<Timers> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Timers entry{};

			entry.char_id  = atoi(row[0]);
			entry.type     = atoi(row[1]);
			entry.start    = atoi(row[2]);
			entry.duration = atoi(row[3]);
			entry.enable   = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Timers> GetWhere(std::string where_filter)
	{
		std::vector<Timers> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Timers entry{};

			entry.char_id  = atoi(row[0]);
			entry.type     = atoi(row[1]);
			entry.start    = atoi(row[2]);
			entry.duration = atoi(row[3]);
			entry.enable   = atoi(row[4]);

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

#endif //EQEMU_BASE_TIMERS_REPOSITORY_H
