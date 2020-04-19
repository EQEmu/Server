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

#ifndef EQEMU_BASE_RAID_MEMBERS_REPOSITORY_H
#define EQEMU_BASE_RAID_MEMBERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseRaidMembersRepository {
public:
	struct RaidMembers {
		int         raidid;
		int         charid;
		int         groupid;
		int         _class;
		int         level;
		std::string name;
		int         isgroupleader;
		int         israidleader;
		int         islooter;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"raidid",
			"charid",
			"groupid",
			"_class",
			"level",
			"name",
			"isgroupleader",
			"israidleader",
			"islooter",
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
		return std::string("raid_members");
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

	static RaidMembers NewEntity()
	{
		RaidMembers entry{};

		entry.raidid        = 0;
		entry.charid        = 0;
		entry.groupid       = 0;
		entry._class        = 0;
		entry.level         = 0;
		entry.name          = "";
		entry.isgroupleader = 0;
		entry.israidleader  = 0;
		entry.islooter      = 0;

		return entry;
	}

	static RaidMembers GetRaidMembersEntry(
		const std::vector<RaidMembers> &raid_memberss,
		int raid_members_id
	)
	{
		for (auto &raid_members : raid_memberss) {
			if (raid_members.charid == raid_members_id) {
				return raid_members;
			}
		}

		return NewEntity();
	}

	static RaidMembers FindOne(
		int raid_members_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				raid_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RaidMembers entry{};

			entry.raidid        = atoi(row[0]);
			entry.charid        = atoi(row[1]);
			entry.groupid       = atoi(row[2]);
			entry._class        = atoi(row[3]);
			entry.level         = atoi(row[4]);
			entry.name          = row[5] ? row[5] : "";
			entry.isgroupleader = atoi(row[6]);
			entry.israidleader  = atoi(row[7]);
			entry.islooter      = atoi(row[8]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int raid_members_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				raid_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		RaidMembers raid_members_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(raid_members_entry.raidid));
		update_values.push_back(columns[1] + " = " + std::to_string(raid_members_entry.charid));
		update_values.push_back(columns[2] + " = " + std::to_string(raid_members_entry.groupid));
		update_values.push_back(columns[3] + " = " + std::to_string(raid_members_entry._class));
		update_values.push_back(columns[4] + " = " + std::to_string(raid_members_entry.level));
		update_values.push_back(columns[5] + " = '" + EscapeString(raid_members_entry.name) + "'");
		update_values.push_back(columns[6] + " = " + std::to_string(raid_members_entry.isgroupleader));
		update_values.push_back(columns[7] + " = " + std::to_string(raid_members_entry.israidleader));
		update_values.push_back(columns[8] + " = " + std::to_string(raid_members_entry.islooter));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				raid_members_entry.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RaidMembers InsertOne(
		RaidMembers raid_members_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(raid_members_entry.raidid));
		insert_values.push_back(std::to_string(raid_members_entry.charid));
		insert_values.push_back(std::to_string(raid_members_entry.groupid));
		insert_values.push_back(std::to_string(raid_members_entry._class));
		insert_values.push_back(std::to_string(raid_members_entry.level));
		insert_values.push_back("'" + EscapeString(raid_members_entry.name) + "'");
		insert_values.push_back(std::to_string(raid_members_entry.isgroupleader));
		insert_values.push_back(std::to_string(raid_members_entry.israidleader));
		insert_values.push_back(std::to_string(raid_members_entry.islooter));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			raid_members_entry.charid = results.LastInsertedID();
			return raid_members_entry;
		}

		raid_members_entry = NewEntity();

		return raid_members_entry;
	}

	static int InsertMany(
		std::vector<RaidMembers> raid_members_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &raid_members_entry: raid_members_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(raid_members_entry.raidid));
			insert_values.push_back(std::to_string(raid_members_entry.charid));
			insert_values.push_back(std::to_string(raid_members_entry.groupid));
			insert_values.push_back(std::to_string(raid_members_entry._class));
			insert_values.push_back(std::to_string(raid_members_entry.level));
			insert_values.push_back("'" + EscapeString(raid_members_entry.name) + "'");
			insert_values.push_back(std::to_string(raid_members_entry.isgroupleader));
			insert_values.push_back(std::to_string(raid_members_entry.israidleader));
			insert_values.push_back(std::to_string(raid_members_entry.islooter));

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

	static std::vector<RaidMembers> All()
	{
		std::vector<RaidMembers> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidMembers entry{};

			entry.raidid        = atoi(row[0]);
			entry.charid        = atoi(row[1]);
			entry.groupid       = atoi(row[2]);
			entry._class        = atoi(row[3]);
			entry.level         = atoi(row[4]);
			entry.name          = row[5] ? row[5] : "";
			entry.isgroupleader = atoi(row[6]);
			entry.israidleader  = atoi(row[7]);
			entry.islooter      = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<RaidMembers> GetWhere(std::string where_filter)
	{
		std::vector<RaidMembers> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidMembers entry{};

			entry.raidid        = atoi(row[0]);
			entry.charid        = atoi(row[1]);
			entry.groupid       = atoi(row[2]);
			entry._class        = atoi(row[3]);
			entry.level         = atoi(row[4]);
			entry.name          = row[5] ? row[5] : "";
			entry.isgroupleader = atoi(row[6]);
			entry.israidleader  = atoi(row[7]);
			entry.islooter      = atoi(row[8]);

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

#endif //EQEMU_BASE_RAID_MEMBERS_REPOSITORY_H
