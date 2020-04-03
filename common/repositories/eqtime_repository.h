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

#ifndef EQEMU_EQTIME_REPOSITORY_H
#define EQEMU_EQTIME_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class EqtimeRepository {
public:
	struct Eqtime {
		int8 minute;
		int8 hour;
		int8 day;
		int8 month;
		int  year;
		int  realtime;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"minute",
			"hour",
			"day",
			"month",
			"year",
			"realtime",
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
		return std::string("eqtime");
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

	static Eqtime NewEntity()
	{
		Eqtime entry{};

		entry.minute   = 0;
		entry.hour     = 0;
		entry.day      = 0;
		entry.month    = 0;
		entry.year     = 0;
		entry.realtime = 0;

		return entry;
	}

	static Eqtime GetEqtimeEntry(
		const std::vector<Eqtime> &eqtimes,
		int eqtime_id
	)
	{
		for (auto &eqtime : eqtimes) {
			if (eqtime. == eqtime_id) {
				return eqtime;
			}
		}

		return NewEntity();
	}

	static Eqtime FindOne(
		int eqtime_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				eqtime_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Eqtime entry{};

			entry.minute   = atoi(row[0]);
			entry.hour     = atoi(row[1]);
			entry.day      = atoi(row[2]);
			entry.month    = atoi(row[3]);
			entry.year     = atoi(row[4]);
			entry.realtime = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int eqtime_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				eqtime_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Eqtime eqtime_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(eqtime_entry.minute));
		update_values.push_back(columns[1] + " = " + std::to_string(eqtime_entry.hour));
		update_values.push_back(columns[2] + " = " + std::to_string(eqtime_entry.day));
		update_values.push_back(columns[3] + " = " + std::to_string(eqtime_entry.month));
		update_values.push_back(columns[4] + " = " + std::to_string(eqtime_entry.year));
		update_values.push_back(columns[5] + " = " + std::to_string(eqtime_entry.realtime));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				eqtime_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Eqtime InsertOne(
		Eqtime eqtime_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(eqtime_entry.minute));
		insert_values.push_back(std::to_string(eqtime_entry.hour));
		insert_values.push_back(std::to_string(eqtime_entry.day));
		insert_values.push_back(std::to_string(eqtime_entry.month));
		insert_values.push_back(std::to_string(eqtime_entry.year));
		insert_values.push_back(std::to_string(eqtime_entry.realtime));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			eqtime_entry.id = results.LastInsertedID();
			return eqtime_entry;
		}

		eqtime_entry = InstanceListRepository::NewEntity();

		return eqtime_entry;
	}

	static int InsertMany(
		std::vector<Eqtime> eqtime_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &eqtime_entry: eqtime_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(eqtime_entry.minute));
			insert_values.push_back(std::to_string(eqtime_entry.hour));
			insert_values.push_back(std::to_string(eqtime_entry.day));
			insert_values.push_back(std::to_string(eqtime_entry.month));
			insert_values.push_back(std::to_string(eqtime_entry.year));
			insert_values.push_back(std::to_string(eqtime_entry.realtime));

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

	static std::vector<Eqtime> All()
	{
		std::vector<Eqtime> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Eqtime entry{};

			entry.minute   = atoi(row[0]);
			entry.hour     = atoi(row[1]);
			entry.day      = atoi(row[2]);
			entry.month    = atoi(row[3]);
			entry.year     = atoi(row[4]);
			entry.realtime = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_EQTIME_REPOSITORY_H
