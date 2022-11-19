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

#ifndef EQEMU_BASE_RAID_DETAILS_REPOSITORY_H
#define EQEMU_BASE_RAID_DETAILS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseRaidDetailsRepository {
public:
	struct RaidDetails {
		int32_t     raidid;
		int32_t     loottype;
		int8_t      locked;
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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("raid_details");
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

	static RaidDetails NewEntity()
	{
		RaidDetails e{};

		e.raidid   = 0;
		e.loottype = 0;
		e.locked   = 0;
		e.motd     = "";

		return e;
	}

	static RaidDetails GetRaidDetails(
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
		Database& db,
		int raid_details_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				raid_details_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RaidDetails e{};

			e.raidid   = static_cast<int32_t>(atoi(row[0]));
			e.loottype = static_cast<int32_t>(atoi(row[1]));
			e.locked   = static_cast<int8_t>(atoi(row[2]));
			e.motd     = row[3] ? row[3] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int raid_details_id
	)
	{
		auto results = db.QueryDatabase(
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
		Database& db,
		const RaidDetails &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.raidid));
		v.push_back(columns[1] + " = " + std::to_string(e.loottype));
		v.push_back(columns[2] + " = " + std::to_string(e.locked));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.motd) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.raidid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RaidDetails InsertOne(
		Database& db,
		RaidDetails e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.raidid));
		v.push_back(std::to_string(e.loottype));
		v.push_back(std::to_string(e.locked));
		v.push_back("'" + Strings::Escape(e.motd) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.raidid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<RaidDetails> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.raidid));
			v.push_back(std::to_string(e.loottype));
			v.push_back(std::to_string(e.locked));
			v.push_back("'" + Strings::Escape(e.motd) + "'");

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

	static std::vector<RaidDetails> All(Database& db)
	{
		std::vector<RaidDetails> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidDetails e{};

			e.raidid   = static_cast<int32_t>(atoi(row[0]));
			e.loottype = static_cast<int32_t>(atoi(row[1]));
			e.locked   = static_cast<int8_t>(atoi(row[2]));
			e.motd     = row[3] ? row[3] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RaidDetails> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<RaidDetails> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RaidDetails e{};

			e.raidid   = static_cast<int32_t>(atoi(row[0]));
			e.loottype = static_cast<int32_t>(atoi(row[1]));
			e.locked   = static_cast<int8_t>(atoi(row[2]));
			e.motd     = row[3] ? row[3] : "";

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

#endif //EQEMU_BASE_RAID_DETAILS_REPOSITORY_H
