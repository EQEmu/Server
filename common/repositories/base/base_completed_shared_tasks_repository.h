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

#ifndef EQEMU_BASE_COMPLETED_SHARED_TASKS_REPOSITORY_H
#define EQEMU_BASE_COMPLETED_SHARED_TASKS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseCompletedSharedTasksRepository {
public:
	struct CompletedSharedTasks {
		int64  id;
		int    task_id;
		time_t accepted_time;
		time_t expire_time;
		time_t completion_time;
		int    is_locked;
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
			"accepted_time",
			"expire_time",
			"completion_time",
			"is_locked",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"task_id",
			"UNIX_TIMESTAMP(accepted_time)",
			"UNIX_TIMESTAMP(expire_time)",
			"UNIX_TIMESTAMP(completion_time)",
			"is_locked",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("completed_shared_tasks");
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

	static CompletedSharedTasks NewEntity()
	{
		CompletedSharedTasks entry{};

		entry.id              = 0;
		entry.task_id         = 0;
		entry.accepted_time   = 0;
		entry.expire_time     = 0;
		entry.completion_time = 0;
		entry.is_locked       = 0;

		return entry;
	}

	static CompletedSharedTasks GetCompletedSharedTasksEntry(
		const std::vector<CompletedSharedTasks> &completed_shared_taskss,
		int completed_shared_tasks_id
	)
	{
		for (auto &completed_shared_tasks : completed_shared_taskss) {
			if (completed_shared_tasks.id == completed_shared_tasks_id) {
				return completed_shared_tasks;
			}
		}

		return NewEntity();
	}

	static CompletedSharedTasks FindOne(
		Database& db,
		int completed_shared_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				completed_shared_tasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CompletedSharedTasks entry{};

			entry.id              = strtoll(row[0], nullptr, 10);
			entry.task_id         = atoi(row[1]);
			entry.accepted_time   = strtoll(row[2] ? row[2] : "-1", nullptr, 10);
			entry.expire_time     = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.completion_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
			entry.is_locked       = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int completed_shared_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				completed_shared_tasks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CompletedSharedTasks completed_shared_tasks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(completed_shared_tasks_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(completed_shared_tasks_entry.task_id));
		update_values.push_back(columns[2] + " = FROM_UNIXTIME(" + (completed_shared_tasks_entry.accepted_time > 0 ? std::to_string(completed_shared_tasks_entry.accepted_time) : "null") + ")");
		update_values.push_back(columns[3] + " = FROM_UNIXTIME(" + (completed_shared_tasks_entry.expire_time > 0 ? std::to_string(completed_shared_tasks_entry.expire_time) : "null") + ")");
		update_values.push_back(columns[4] + " = FROM_UNIXTIME(" + (completed_shared_tasks_entry.completion_time > 0 ? std::to_string(completed_shared_tasks_entry.completion_time) : "null") + ")");
		update_values.push_back(columns[5] + " = " + std::to_string(completed_shared_tasks_entry.is_locked));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				completed_shared_tasks_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CompletedSharedTasks InsertOne(
		Database& db,
		CompletedSharedTasks completed_shared_tasks_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(completed_shared_tasks_entry.id));
		insert_values.push_back(std::to_string(completed_shared_tasks_entry.task_id));
		insert_values.push_back("FROM_UNIXTIME(" + (completed_shared_tasks_entry.accepted_time > 0 ? std::to_string(completed_shared_tasks_entry.accepted_time) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (completed_shared_tasks_entry.expire_time > 0 ? std::to_string(completed_shared_tasks_entry.expire_time) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (completed_shared_tasks_entry.completion_time > 0 ? std::to_string(completed_shared_tasks_entry.completion_time) : "null") + ")");
		insert_values.push_back(std::to_string(completed_shared_tasks_entry.is_locked));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			completed_shared_tasks_entry.id = results.LastInsertedID();
			return completed_shared_tasks_entry;
		}

		completed_shared_tasks_entry = NewEntity();

		return completed_shared_tasks_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CompletedSharedTasks> completed_shared_tasks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &completed_shared_tasks_entry: completed_shared_tasks_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(completed_shared_tasks_entry.id));
			insert_values.push_back(std::to_string(completed_shared_tasks_entry.task_id));
			insert_values.push_back("FROM_UNIXTIME(" + (completed_shared_tasks_entry.accepted_time > 0 ? std::to_string(completed_shared_tasks_entry.accepted_time) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (completed_shared_tasks_entry.expire_time > 0 ? std::to_string(completed_shared_tasks_entry.expire_time) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (completed_shared_tasks_entry.completion_time > 0 ? std::to_string(completed_shared_tasks_entry.completion_time) : "null") + ")");
			insert_values.push_back(std::to_string(completed_shared_tasks_entry.is_locked));

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

	static std::vector<CompletedSharedTasks> All(Database& db)
	{
		std::vector<CompletedSharedTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedSharedTasks entry{};

			entry.id              = strtoll(row[0], nullptr, 10);
			entry.task_id         = atoi(row[1]);
			entry.accepted_time   = strtoll(row[2] ? row[2] : "-1", nullptr, 10);
			entry.expire_time     = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.completion_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
			entry.is_locked       = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CompletedSharedTasks> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CompletedSharedTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedSharedTasks entry{};

			entry.id              = strtoll(row[0], nullptr, 10);
			entry.task_id         = atoi(row[1]);
			entry.accepted_time   = strtoll(row[2] ? row[2] : "-1", nullptr, 10);
			entry.expire_time     = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.completion_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);
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

#endif //EQEMU_BASE_COMPLETED_SHARED_TASKS_REPOSITORY_H
