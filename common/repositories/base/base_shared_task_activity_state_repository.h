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

#ifndef EQEMU_BASE_SHARED_TASK_ACTIVITY_STATE_REPOSITORY_H
#define EQEMU_BASE_SHARED_TASK_ACTIVITY_STATE_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseSharedTaskActivityStateRepository {
public:
	struct SharedTaskActivityState {
		int64  shared_task_id;
		int    activity_id;
		int    done_count;
		time_t updated_time;
		time_t completed_time;
	};

	static std::string PrimaryKey()
	{
		return std::string("shared_task_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"shared_task_id",
			"activity_id",
			"done_count",
			"updated_time",
			"completed_time",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"shared_task_id",
			"activity_id",
			"done_count",
			"UNIX_TIMESTAMP(updated_time)",
			"UNIX_TIMESTAMP(completed_time)",
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
		return std::string("shared_task_activity_state");
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

	static SharedTaskActivityState NewEntity()
	{
		SharedTaskActivityState entry{};

		entry.shared_task_id = 0;
		entry.activity_id    = 0;
		entry.done_count     = 0;
		entry.updated_time   = 0;
		entry.completed_time = 0;

		return entry;
	}

	static SharedTaskActivityState GetSharedTaskActivityStateEntry(
		const std::vector<SharedTaskActivityState> &shared_task_activity_states,
		int shared_task_activity_state_id
	)
	{
		for (auto &shared_task_activity_state : shared_task_activity_states) {
			if (shared_task_activity_state.shared_task_id == shared_task_activity_state_id) {
				return shared_task_activity_state;
			}
		}

		return NewEntity();
	}

	static SharedTaskActivityState FindOne(
		Database& db,
		int shared_task_activity_state_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				shared_task_activity_state_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SharedTaskActivityState entry{};

			entry.shared_task_id = strtoll(row[0], nullptr, 10);
			entry.activity_id    = atoi(row[1]);
			entry.done_count     = atoi(row[2]);
			entry.updated_time   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.completed_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int shared_task_activity_state_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				shared_task_activity_state_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		SharedTaskActivityState shared_task_activity_state_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(shared_task_activity_state_entry.shared_task_id));
		update_values.push_back(columns[1] + " = " + std::to_string(shared_task_activity_state_entry.activity_id));
		update_values.push_back(columns[2] + " = " + std::to_string(shared_task_activity_state_entry.done_count));
		update_values.push_back(columns[3] + " = FROM_UNIXTIME(" + (shared_task_activity_state_entry.updated_time > 0 ? std::to_string(shared_task_activity_state_entry.updated_time) : "null") + ")");
		update_values.push_back(columns[4] + " = FROM_UNIXTIME(" + (shared_task_activity_state_entry.completed_time > 0 ? std::to_string(shared_task_activity_state_entry.completed_time) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				shared_task_activity_state_entry.shared_task_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SharedTaskActivityState InsertOne(
		Database& db,
		SharedTaskActivityState shared_task_activity_state_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(shared_task_activity_state_entry.shared_task_id));
		insert_values.push_back(std::to_string(shared_task_activity_state_entry.activity_id));
		insert_values.push_back(std::to_string(shared_task_activity_state_entry.done_count));
		insert_values.push_back("FROM_UNIXTIME(" + (shared_task_activity_state_entry.updated_time > 0 ? std::to_string(shared_task_activity_state_entry.updated_time) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (shared_task_activity_state_entry.completed_time > 0 ? std::to_string(shared_task_activity_state_entry.completed_time) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			shared_task_activity_state_entry.shared_task_id = results.LastInsertedID();
			return shared_task_activity_state_entry;
		}

		shared_task_activity_state_entry = NewEntity();

		return shared_task_activity_state_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<SharedTaskActivityState> shared_task_activity_state_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &shared_task_activity_state_entry: shared_task_activity_state_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(shared_task_activity_state_entry.shared_task_id));
			insert_values.push_back(std::to_string(shared_task_activity_state_entry.activity_id));
			insert_values.push_back(std::to_string(shared_task_activity_state_entry.done_count));
			insert_values.push_back("FROM_UNIXTIME(" + (shared_task_activity_state_entry.updated_time > 0 ? std::to_string(shared_task_activity_state_entry.updated_time) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (shared_task_activity_state_entry.completed_time > 0 ? std::to_string(shared_task_activity_state_entry.completed_time) : "null") + ")");

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

	static std::vector<SharedTaskActivityState> All(Database& db)
	{
		std::vector<SharedTaskActivityState> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SharedTaskActivityState entry{};

			entry.shared_task_id = strtoll(row[0], nullptr, 10);
			entry.activity_id    = atoi(row[1]);
			entry.done_count     = atoi(row[2]);
			entry.updated_time   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.completed_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SharedTaskActivityState> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SharedTaskActivityState> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SharedTaskActivityState entry{};

			entry.shared_task_id = strtoll(row[0], nullptr, 10);
			entry.activity_id    = atoi(row[1]);
			entry.done_count     = atoi(row[2]);
			entry.updated_time   = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			entry.completed_time = strtoll(row[4] ? row[4] : "-1", nullptr, 10);

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

#endif //EQEMU_BASE_SHARED_TASK_ACTIVITY_STATE_REPOSITORY_H
