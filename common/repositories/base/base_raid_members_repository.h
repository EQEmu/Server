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
#include "../../strings.h"
#include <ctime>

class BaseRaidMembersRepository {
public:
	struct RaidMembers {
		int32_t     raidid;
		int32_t     charid;
		uint32_t    groupid;
		int8_t      _class;
		int8_t      level;
		std::string name;
		int8_t      isgroupleader;
		int8_t      israidleader;
		int8_t      islooter;
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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("raid_members");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
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
		RaidMembers e{};

		e.raidid        = 0;
		e.charid        = 0;
		e.groupid       = 0;
		e._class        = 0;
		e.level         = 0;
		e.name          = "";
		e.isgroupleader = 0;
		e.israidleader  = 0;
		e.islooter      = 0;

		return e;
	}

	static RaidMembers GetRaidMembers(
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
			RaidMembers e{};

			e.raidid        = static_cast<int32_t>(atoi(row[0]));
			e.charid        = static_cast<int32_t>(atoi(row[1]));
			e.groupid       = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e._class        = static_cast<int8_t>(atoi(row[3]));
			e.level         = static_cast<int8_t>(atoi(row[4]));
			e.name          = row[5] ? row[5] : "";
			e.isgroupleader = static_cast<int8_t>(atoi(row[6]));
			e.israidleader  = static_cast<int8_t>(atoi(row[7]));
			e.islooter      = static_cast<int8_t>(atoi(row[8]));

			return e;
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
		const RaidMembers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.raidid));
		v.push_back(columns[1] + " = " + std::to_string(e.charid));
		v.push_back(columns[2] + " = " + std::to_string(e.groupid));
		v.push_back(columns[3] + " = " + std::to_string(e._class));
		v.push_back(columns[4] + " = " + std::to_string(e.level));
		v.push_back(columns[5] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[6] + " = " + std::to_string(e.isgroupleader));
		v.push_back(columns[7] + " = " + std::to_string(e.israidleader));
		v.push_back(columns[8] + " = " + std::to_string(e.islooter));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RaidMembers InsertOne(
		Database& db,
		RaidMembers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.raidid));
		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.groupid));
		v.push_back(std::to_string(e._class));
		v.push_back(std::to_string(e.level));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.isgroupleader));
		v.push_back(std::to_string(e.israidleader));
		v.push_back(std::to_string(e.islooter));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.charid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<RaidMembers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.raidid));
			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.groupid));
			v.push_back(std::to_string(e._class));
			v.push_back(std::to_string(e.level));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.isgroupleader));
			v.push_back(std::to_string(e.israidleader));
			v.push_back(std::to_string(e.islooter));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
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
			RaidMembers e{};

			e.raidid        = static_cast<int32_t>(atoi(row[0]));
			e.charid        = static_cast<int32_t>(atoi(row[1]));
			e.groupid       = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e._class        = static_cast<int8_t>(atoi(row[3]));
			e.level         = static_cast<int8_t>(atoi(row[4]));
			e.name          = row[5] ? row[5] : "";
			e.isgroupleader = static_cast<int8_t>(atoi(row[6]));
			e.israidleader  = static_cast<int8_t>(atoi(row[7]));
			e.islooter      = static_cast<int8_t>(atoi(row[8]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RaidMembers> GetWhere(Database& db, const std::string &where_filter)
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
			RaidMembers e{};

			e.raidid        = static_cast<int32_t>(atoi(row[0]));
			e.charid        = static_cast<int32_t>(atoi(row[1]));
			e.groupid       = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e._class        = static_cast<int8_t>(atoi(row[3]));
			e.level         = static_cast<int8_t>(atoi(row[4]));
			e.name          = row[5] ? row[5] : "";
			e.isgroupleader = static_cast<int8_t>(atoi(row[6]));
			e.israidleader  = static_cast<int8_t>(atoi(row[7]));
			e.islooter      = static_cast<int8_t>(atoi(row[8]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_RAID_MEMBERS_REPOSITORY_H
