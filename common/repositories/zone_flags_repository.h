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

#ifndef EQEMU_ZONE_FLAGS_REPOSITORY_H
#define EQEMU_ZONE_FLAGS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class ZoneFlagsRepository {
public:
	struct ZoneFlags {
		int charID;
		int zoneID;
	};

	static std::string PrimaryKey()
	{
		return std::string("zoneID");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charID",
			"zoneID",
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
		return std::string("zone_flags");
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

	static ZoneFlags NewEntity()
	{
		ZoneFlags entry{};

		entry.charID = 0;
		entry.zoneID = 0;

		return entry;
	}

	static ZoneFlags GetZoneFlagsEntry(
		const std::vector<ZoneFlags> &zone_flagss,
		int zone_flags_id
	)
	{
		for (auto &zone_flags : zone_flagss) {
			if (zone_flags.zoneID == zone_flags_id) {
				return zone_flags;
			}
		}

		return NewEntity();
	}

	static ZoneFlags FindOne(
		int zone_flags_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				zone_flags_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ZoneFlags entry{};

			entry.charID = atoi(row[0]);
			entry.zoneID = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int zone_flags_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				zone_flags_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		ZoneFlags zone_flags_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();


		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				zone_flags_entry.zoneID
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ZoneFlags InsertOne(
		ZoneFlags zone_flags_entry
	)
	{
		std::vector<std::string> insert_values;


		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			zone_flags_entry.id = results.LastInsertedID();
			return zone_flags_entry;
		}

		zone_flags_entry = InstanceListRepository::NewEntity();

		return zone_flags_entry;
	}

	static int InsertMany(
		std::vector<ZoneFlags> zone_flags_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &zone_flags_entry: zone_flags_entries) {
			std::vector<std::string> insert_values;


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

	static std::vector<ZoneFlags> All()
	{
		std::vector<ZoneFlags> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZoneFlags entry{};

			entry.charID = atoi(row[0]);
			entry.zoneID = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_ZONE_FLAGS_REPOSITORY_H
