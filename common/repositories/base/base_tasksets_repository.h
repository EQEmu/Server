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

#ifndef EQEMU_BASE_TASKSETS_REPOSITORY_H
#define EQEMU_BASE_TASKSETS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTasksetsRepository {
public:
	struct Tasksets {
		int id;
		int taskid;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"taskid",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("tasksets");
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

	static Tasksets NewEntity()
	{
		Tasksets entry{};

		entry.id     = 0;
		entry.taskid = 0;

		return entry;
	}

	static Tasksets GetTasksetsEntry(
		const std::vector<Tasksets> &tasksetss,
		int tasksets_id
	)
	{
		for (auto &tasksets : tasksetss) {
			if (tasksets.id == tasksets_id) {
				return tasksets;
			}
		}

		return NewEntity();
	}

	static Tasksets FindOne(
		Database& db,
		int tasksets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tasksets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Tasksets entry{};

			entry.id     = atoi(row[0]);
			entry.taskid = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tasksets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tasksets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Tasksets tasksets_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(tasksets_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(tasksets_entry.taskid));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				tasksets_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Tasksets InsertOne(
		Database& db,
		Tasksets tasksets_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(tasksets_entry.id));
		insert_values.push_back(std::to_string(tasksets_entry.taskid));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			tasksets_entry.id = results.LastInsertedID();
			return tasksets_entry;
		}

		tasksets_entry = NewEntity();

		return tasksets_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Tasksets> tasksets_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &tasksets_entry: tasksets_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(tasksets_entry.id));
			insert_values.push_back(std::to_string(tasksets_entry.taskid));

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

	static std::vector<Tasksets> All(Database& db)
	{
		std::vector<Tasksets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tasksets entry{};

			entry.id     = atoi(row[0]);
			entry.taskid = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Tasksets> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Tasksets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tasksets entry{};

			entry.id     = atoi(row[0]);
			entry.taskid = atoi(row[1]);

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

#endif //EQEMU_BASE_TASKSETS_REPOSITORY_H
