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

#ifndef EQEMU_BASE_SPAWN2_DISABLED_REPOSITORY_H
#define EQEMU_BASE_SPAWN2_DISABLED_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>


class BaseSpawn2DisabledRepository {
public:
	struct Spawn2Disabled {
		int64_t id;
		int32_t spawn2_id;
		int32_t instance_id;
		int16_t disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"spawn2_id",
			"instance_id",
			"disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"spawn2_id",
			"instance_id",
			"disabled",
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
		return std::string("spawn2_disabled");
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

	static Spawn2Disabled NewEntity()
	{
		Spawn2Disabled e{};

		e.id          = 0;
		e.spawn2_id   = 0;
		e.instance_id = 0;
		e.disabled    = 0;

		return e;
	}

	static Spawn2Disabled GetSpawn2Disabled(
		const std::vector<Spawn2Disabled> &spawn2_disableds,
		int spawn2_disabled_id
	)
	{
		for (auto &spawn2_disabled : spawn2_disableds) {
			if (spawn2_disabled.id == spawn2_disabled_id) {
				return spawn2_disabled;
			}
		}

		return NewEntity();
	}

	static Spawn2Disabled FindOne(
		Database& db,
		int spawn2_disabled_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				spawn2_disabled_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Spawn2Disabled e{};

			e.id          = strtoll(row[0], nullptr, 10);
			e.spawn2_id   = static_cast<int32_t>(atoi(row[1]));
			e.instance_id = static_cast<int32_t>(atoi(row[2]));
			e.disabled    = static_cast<int16_t>(atoi(row[3]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawn2_disabled_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawn2_disabled_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Spawn2Disabled &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.spawn2_id));
		v.push_back(columns[2] + " = " + std::to_string(e.instance_id));
		v.push_back(columns[3] + " = " + std::to_string(e.disabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Spawn2Disabled InsertOne(
		Database& db,
		Spawn2Disabled e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.spawn2_id));
		v.push_back(std::to_string(e.instance_id));
		v.push_back(std::to_string(e.disabled));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Spawn2Disabled> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.spawn2_id));
			v.push_back(std::to_string(e.instance_id));
			v.push_back(std::to_string(e.disabled));

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

	static std::vector<Spawn2Disabled> All(Database& db)
	{
		std::vector<Spawn2Disabled> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawn2Disabled e{};

			e.id          = strtoll(row[0], nullptr, 10);
			e.spawn2_id   = static_cast<int32_t>(atoi(row[1]));
			e.instance_id = static_cast<int32_t>(atoi(row[2]));
			e.disabled    = static_cast<int16_t>(atoi(row[3]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Spawn2Disabled> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Spawn2Disabled> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawn2Disabled e{};

			e.id          = strtoll(row[0], nullptr, 10);
			e.spawn2_id   = static_cast<int32_t>(atoi(row[1]));
			e.instance_id = static_cast<int32_t>(atoi(row[2]));
			e.disabled    = static_cast<int16_t>(atoi(row[3]));

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

#endif //EQEMU_BASE_SPAWN2_DISABLED_REPOSITORY_H
