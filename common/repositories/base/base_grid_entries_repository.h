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

#ifndef EQEMU_BASE_GRID_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_GRID_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseGridEntriesRepository {
public:
	struct GridEntries {
		int32_t gridid;
		int32_t zoneid;
		int32_t number;
		float   x;
		float   y;
		float   z;
		float   heading;
		int32_t pause;
		int8_t  centerpoint;
	};

	static std::string PrimaryKey()
	{
		return std::string("gridid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"gridid",
			"zoneid",
			"number",
			"x",
			"y",
			"z",
			"heading",
			"pause",
			"centerpoint",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"gridid",
			"zoneid",
			"number",
			"x",
			"y",
			"z",
			"heading",
			"pause",
			"centerpoint",
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
		return std::string("grid_entries");
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

	static GridEntries NewEntity()
	{
		GridEntries e{};

		e.gridid      = 0;
		e.zoneid      = 0;
		e.number      = 0;
		e.x           = 0;
		e.y           = 0;
		e.z           = 0;
		e.heading     = 0;
		e.pause       = 0;
		e.centerpoint = 0;

		return e;
	}

	static GridEntries GetGridEntries(
		const std::vector<GridEntries> &grid_entriess,
		int grid_entries_id
	)
	{
		for (auto &grid_entries : grid_entriess) {
			if (grid_entries.gridid == grid_entries_id) {
				return grid_entries;
			}
		}

		return NewEntity();
	}

	static GridEntries FindOne(
		Database& db,
		int grid_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				grid_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GridEntries e{};

			e.gridid      = static_cast<int32_t>(atoi(row[0]));
			e.zoneid      = static_cast<int32_t>(atoi(row[1]));
			e.number      = static_cast<int32_t>(atoi(row[2]));
			e.x           = strtof(row[3], nullptr);
			e.y           = strtof(row[4], nullptr);
			e.z           = strtof(row[5], nullptr);
			e.heading     = strtof(row[6], nullptr);
			e.pause       = static_cast<int32_t>(atoi(row[7]));
			e.centerpoint = static_cast<int8_t>(atoi(row[8]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int grid_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				grid_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GridEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.gridid));
		v.push_back(columns[1] + " = " + std::to_string(e.zoneid));
		v.push_back(columns[2] + " = " + std::to_string(e.number));
		v.push_back(columns[3] + " = " + std::to_string(e.x));
		v.push_back(columns[4] + " = " + std::to_string(e.y));
		v.push_back(columns[5] + " = " + std::to_string(e.z));
		v.push_back(columns[6] + " = " + std::to_string(e.heading));
		v.push_back(columns[7] + " = " + std::to_string(e.pause));
		v.push_back(columns[8] + " = " + std::to_string(e.centerpoint));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.gridid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GridEntries InsertOne(
		Database& db,
		GridEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.gridid));
		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.number));
		v.push_back(std::to_string(e.x));
		v.push_back(std::to_string(e.y));
		v.push_back(std::to_string(e.z));
		v.push_back(std::to_string(e.heading));
		v.push_back(std::to_string(e.pause));
		v.push_back(std::to_string(e.centerpoint));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.gridid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<GridEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.gridid));
			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.number));
			v.push_back(std::to_string(e.x));
			v.push_back(std::to_string(e.y));
			v.push_back(std::to_string(e.z));
			v.push_back(std::to_string(e.heading));
			v.push_back(std::to_string(e.pause));
			v.push_back(std::to_string(e.centerpoint));

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

	static std::vector<GridEntries> All(Database& db)
	{
		std::vector<GridEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GridEntries e{};

			e.gridid      = static_cast<int32_t>(atoi(row[0]));
			e.zoneid      = static_cast<int32_t>(atoi(row[1]));
			e.number      = static_cast<int32_t>(atoi(row[2]));
			e.x           = strtof(row[3], nullptr);
			e.y           = strtof(row[4], nullptr);
			e.z           = strtof(row[5], nullptr);
			e.heading     = strtof(row[6], nullptr);
			e.pause       = static_cast<int32_t>(atoi(row[7]));
			e.centerpoint = static_cast<int8_t>(atoi(row[8]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GridEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GridEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GridEntries e{};

			e.gridid      = static_cast<int32_t>(atoi(row[0]));
			e.zoneid      = static_cast<int32_t>(atoi(row[1]));
			e.number      = static_cast<int32_t>(atoi(row[2]));
			e.x           = strtof(row[3], nullptr);
			e.y           = strtof(row[4], nullptr);
			e.z           = strtof(row[5], nullptr);
			e.heading     = strtof(row[6], nullptr);
			e.pause       = static_cast<int32_t>(atoi(row[7]));
			e.centerpoint = static_cast<int8_t>(atoi(row[8]));

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

#endif //EQEMU_BASE_GRID_ENTRIES_REPOSITORY_H
