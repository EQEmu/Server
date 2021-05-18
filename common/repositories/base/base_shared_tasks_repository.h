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

#ifndef EQEMU_BASE_SHARED_TASKS_REPOSITORY_H
#define EQEMU_BASE_SHARED_TASKS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSharedTasksRepository {
public:
	struct SharedTasks {
		int         id;
		int         task_id;
		int         dynamic_zone_id;
		std::string accepted_time;
		std::string completion_time;
		int         is_locked;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"task_id",
			"dynamic_zone_id",
			"accepted_time",
			"completion_time",
			"is_locked",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("shared_tasks");
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

	static SharedTasks NewEntity()
	{
		SharedTasks entry{};

		entry.id              = 0;
		entry.task_id         = 0;
		entry.dynamic_zone_id = 0;
		entry.accepted_time   = "";
		entry.completion_time = "";
		entry.is_locked       = 0;

		return entry;
	}

	static SharedTasks GetSharedTasksEntry(
		const std::vector<SharedTasks> &shared_taskss,
		int shared_tasks_id
	)
	{
		for (auto &shared_tasks : shared_taskss) {
			if (shared_tasks.id == shared_tasks_id) {
				return shared_tasks;
			}
		}

		return NewEntity();
	}

	static SharedTasks FindOne(
		Database& db,
		int shared_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				shared_tasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SharedTasks entry{};

			entry.id              = atoi(row[0]);
			entry.task_id         = atoi(row[1]);
			entry.dynamic_zone_id = atoi(row[2]);
			entry.accepted_time   = row[3] ? row[3] : "";
			entry.completion_time = row[4] ? row[4] : "";
			entry.is_locked       = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int shared_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				shared_tasks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		SharedTasks shared_tasks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(shared_tasks_entry.task_id));
		update_values.push_back(columns[2] + " = " + std::to_string(shared_tasks_entry.dynamic_zone_id));
		update_values.push_back(columns[3] + " = '" + EscapeString(shared_tasks_entry.accepted_time) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(shared_tasks_entry.completion_time) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(shared_tasks_entry.is_locked));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				shared_tasks_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SharedTasks InsertOne(
		Database& db,
		SharedTasks shared_tasks_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(shared_tasks_entry.id));
		insert_values.push_back(std::to_string(shared_tasks_entry.task_id));
		insert_values.push_back(std::to_string(shared_tasks_entry.dynamic_zone_id));
		insert_values.push_back("'" + EscapeString(shared_tasks_entry.accepted_time) + "'");
		insert_values.push_back("'" + EscapeString(shared_tasks_entry.completion_time) + "'");
		insert_values.push_back(std::to_string(shared_tasks_entry.is_locked));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			shared_tasks_entry.id = results.LastInsertedID();
			return shared_tasks_entry;
		}

		shared_tasks_entry = NewEntity();

		return shared_tasks_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<SharedTasks> shared_tasks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &shared_tasks_entry: shared_tasks_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(shared_tasks_entry.id));
			insert_values.push_back(std::to_string(shared_tasks_entry.task_id));
			insert_values.push_back(std::to_string(shared_tasks_entry.dynamic_zone_id));
			insert_values.push_back("'" + EscapeString(shared_tasks_entry.accepted_time) + "'");
			insert_values.push_back("'" + EscapeString(shared_tasks_entry.completion_time) + "'");
			insert_values.push_back(std::to_string(shared_tasks_entry.is_locked));

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

	static std::vector<SharedTasks> All(Database& db)
	{
		std::vector<SharedTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SharedTasks entry{};

			entry.id              = atoi(row[0]);
			entry.task_id         = atoi(row[1]);
			entry.dynamic_zone_id = atoi(row[2]);
			entry.accepted_time   = row[3] ? row[3] : "";
			entry.completion_time = row[4] ? row[4] : "";
			entry.is_locked       = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SharedTasks> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SharedTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SharedTasks entry{};

			entry.id              = atoi(row[0]);
			entry.task_id         = atoi(row[1]);
			entry.dynamic_zone_id = atoi(row[2]);
			entry.accepted_time   = row[3] ? row[3] : "";
			entry.completion_time = row[4] ? row[4] : "";
			entry.is_locked       = atoi(row[5]);

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

#endif //EQEMU_BASE_SHARED_TASKS_REPOSITORY_H
