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

#ifndef EQEMU_BASE_RAID_DETAILS_REPOSITORY_H
#define EQEMU_BASE_RAID_DETAILS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseRaidDetailsRepository {
public:
	struct RaidDetails {
		int         raidid;
		int         loottype;
		int         locked;
		std::string motd;
	};

	static std::string PrimaryKey()
	{
		return std::string("raidid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"raidid",
			"loottype",
			"locked",
			"motd",
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
		return std::string("raid_details");
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

	static RaidDetails NewEntity()
	{
		RaidDetails entry{};

		entry.raidid   = 0;
		entry.loottype = 0;
		entry.locked   = 0;
		entry.motd     = "";

		return entry;
	}

	static RaidDetails GetRaidDetailsEntry(
		const std::vector<RaidDetails> &raid_detailss,
		int raid_details_id
	)
	{
		for (auto &raid_details : raid_detailss) {
			if (raid_details.raidid == raid_details_id) {
				return raid_details;
			}
		}

		return NewEntity();
	}

	static RaidDetails FindOne(
		int raid_details_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				raid_details_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RaidDetails entry{};

			entry.raidid   = atoi(row[0]);
			entry.loottype = atoi(row[1]);
			entry.locked   = atoi(row[2]);
			entry.motd     = row[3] ? row[3] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int raid_details_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				raid_details_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		RaidDetails raid_details_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(raid_details_entry.raidid));
		update_values.push_back(columns[1] + " = " + std::to_string(raid_details_entry.loottype));
		update_values.push_back(columns[2] + " = " + std::to_string(raid_details_entry.locked));
		update_values.push_back(columns[3] + " = '" + EscapeString(raid_details_entry.motd) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				raid_details_entry.raidid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RaidDetails InsertOne(
		RaidDetails raid_details_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(raid_details_entry.raidid));
		insert_values.push_back(std::to_string(raid_details_entry.loottype));
		insert_values.push_back(std::to_string(raid_details_entry.locked));
		insert_values.push_back("'" + EscapeString(raid_details_entry.motd) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			raid_details_entry.raidid = results.LastInsertedID();
			return raid_details_entry;
		}

		raid_details_entry = NewEntity();

		return raid_details_entry;
	}

	static int InsertMany(
		std::vector<RaidDetails> raid_details_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &raid_details_entry: raid_details_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(raid_details_entry.raidid));
			insert_values.push_back(std::to_string(raid_details_entry.loottype));
			insert_values.push_back(std::to_string(raid_details_entry.locked));
			insert_values.push_back("'" + EscapeString(raid_details_entry.motd) + "'");

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

	static std::vector<RaidDetails> All()
	{
		std::vector<RaidDetails> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidDetails entry{};

			entry.raidid   = atoi(row[0]);
			entry.loottype = atoi(row[1]);
			entry.locked   = atoi(row[2]);
			entry.motd     = row[3] ? row[3] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<RaidDetails> GetWhere(std::string where_filter)
	{
		std::vector<RaidDetails> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidDetails entry{};

			entry.raidid   = atoi(row[0]);
			entry.loottype = atoi(row[1]);
			entry.locked   = atoi(row[2]);
			entry.motd     = row[3] ? row[3] : "";

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

#endif //EQEMU_BASE_RAID_DETAILS_REPOSITORY_H
