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

#ifndef EQEMU_BASE_SHARED_TASK_DYNAMIC_ZONES_REPOSITORY_H
#define EQEMU_BASE_SHARED_TASK_DYNAMIC_ZONES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseSharedTaskDynamicZonesRepository {
public:
	struct SharedTaskDynamicZones {
		int64_t  shared_task_id;
		uint32_t dynamic_zone_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("shared_task_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"shared_task_id",
			"dynamic_zone_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"shared_task_id",
			"dynamic_zone_id",
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
		return std::string("shared_task_dynamic_zones");
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

	static SharedTaskDynamicZones NewEntity()
	{
		SharedTaskDynamicZones e{};

		e.shared_task_id  = 0;
		e.dynamic_zone_id = 0;

		return e;
	}

	static SharedTaskDynamicZones GetSharedTaskDynamicZones(
		const std::vector<SharedTaskDynamicZones> &shared_task_dynamic_zoness,
		int shared_task_dynamic_zones_id
	)
	{
		for (auto &shared_task_dynamic_zones : shared_task_dynamic_zoness) {
			if (shared_task_dynamic_zones.shared_task_id == shared_task_dynamic_zones_id) {
				return shared_task_dynamic_zones;
			}
		}

		return NewEntity();
	}

	static SharedTaskDynamicZones FindOne(
		Database& db,
		int shared_task_dynamic_zones_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				shared_task_dynamic_zones_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SharedTaskDynamicZones e{};

			e.shared_task_id  = strtoll(row[0], nullptr, 10);
			e.dynamic_zone_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int shared_task_dynamic_zones_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				shared_task_dynamic_zones_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const SharedTaskDynamicZones &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.shared_task_id));
		v.push_back(columns[1] + " = " + std::to_string(e.dynamic_zone_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.shared_task_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SharedTaskDynamicZones InsertOne(
		Database& db,
		SharedTaskDynamicZones e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.shared_task_id));
		v.push_back(std::to_string(e.dynamic_zone_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.shared_task_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<SharedTaskDynamicZones> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.shared_task_id));
			v.push_back(std::to_string(e.dynamic_zone_id));

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

	static std::vector<SharedTaskDynamicZones> All(Database& db)
	{
		std::vector<SharedTaskDynamicZones> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SharedTaskDynamicZones e{};

			e.shared_task_id  = strtoll(row[0], nullptr, 10);
			e.dynamic_zone_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<SharedTaskDynamicZones> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<SharedTaskDynamicZones> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SharedTaskDynamicZones e{};

			e.shared_task_id  = strtoll(row[0], nullptr, 10);
			e.dynamic_zone_id = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));

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

#endif //EQEMU_BASE_SHARED_TASK_DYNAMIC_ZONES_REPOSITORY_H
