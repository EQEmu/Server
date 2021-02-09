/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 * 
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://eqemu.gitbook.io/server/in-development/developer-area/repositories
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
			ColumnsRaw()
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
		Database& db,
		int raid_members_id
	)
	{
		auto results = db.QueryDatabase(
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
		Database& db,
		int raid_members_id
	)
	{
		auto results = db.QueryDatabase(
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
		Database& db,
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

		auto results = db.QueryDatabase(
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
		Database& db,
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

		auto results = db.QueryDatabase(
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
		Database& db,
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

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<RaidMembers> All(Database& db)
	{
		std::vector<RaidMembers> all_entries;

		auto results = db.QueryDatabase(
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

	static std::vector<RaidMembers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<RaidMembers> all_entries;

		auto results = db.QueryDatabase(
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

	static int DeleteWhere(Database& db, std::string where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_RAID_MEMBERS_REPOSITORY_H
