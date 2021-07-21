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
#include "../../string_util.h"

class BaseSharedTaskDynamicZonesRepository {
public:
	struct SharedTaskDynamicZones {
		int64 shared_task_id;
		int   dynamic_zone_id;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("shared_task_dynamic_zones");
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

	static SharedTaskDynamicZones NewEntity()
	{
		SharedTaskDynamicZones entry{};

		entry.shared_task_id  = 0;
		entry.dynamic_zone_id = 0;

		return entry;
	}

	static SharedTaskDynamicZones GetSharedTaskDynamicZonesEntry(
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
			SharedTaskDynamicZones entry{};

			entry.shared_task_id  = strtoll(row[0], NULL, 10);
			entry.dynamic_zone_id = atoi(row[1]);

			return entry;
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
		SharedTaskDynamicZones shared_task_dynamic_zones_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(shared_task_dynamic_zones_entry.shared_task_id));
		update_values.push_back(columns[1] + " = " + std::to_string(shared_task_dynamic_zones_entry.dynamic_zone_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				shared_task_dynamic_zones_entry.shared_task_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SharedTaskDynamicZones InsertOne(
		Database& db,
		SharedTaskDynamicZones shared_task_dynamic_zones_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(shared_task_dynamic_zones_entry.shared_task_id));
		insert_values.push_back(std::to_string(shared_task_dynamic_zones_entry.dynamic_zone_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			shared_task_dynamic_zones_entry.shared_task_id = results.LastInsertedID();
			return shared_task_dynamic_zones_entry;
		}

		shared_task_dynamic_zones_entry = NewEntity();

		return shared_task_dynamic_zones_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<SharedTaskDynamicZones> shared_task_dynamic_zones_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &shared_task_dynamic_zones_entry: shared_task_dynamic_zones_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(shared_task_dynamic_zones_entry.shared_task_id));
			insert_values.push_back(std::to_string(shared_task_dynamic_zones_entry.dynamic_zone_id));

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
			SharedTaskDynamicZones entry{};

			entry.shared_task_id  = strtoll(row[0], NULL, 10);
			entry.dynamic_zone_id = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SharedTaskDynamicZones> GetWhere(Database& db, std::string where_filter)
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
			SharedTaskDynamicZones entry{};

			entry.shared_task_id  = strtoll(row[0], NULL, 10);
			entry.dynamic_zone_id = atoi(row[1]);

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

#endif //EQEMU_BASE_SHARED_TASK_DYNAMIC_ZONES_REPOSITORY_H
