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

#ifndef EQEMU_BASE_COMPLETED_TASKS_REPOSITORY_H
#define EQEMU_BASE_COMPLETED_TASKS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCompletedTasksRepository {
public:
	struct CompletedTasks {
		uint32_t charid;
		uint32_t completedtime;
		uint32_t taskid;
		int32_t  activityid;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"completedtime",
			"taskid",
			"activityid",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"charid",
			"completedtime",
			"taskid",
			"activityid",
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
		return std::string("completed_tasks");
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

	static CompletedTasks NewEntity()
	{
		CompletedTasks e{};

		e.charid        = 0;
		e.completedtime = 0;
		e.taskid        = 0;
		e.activityid    = 0;

		return e;
	}

	static CompletedTasks GetCompletedTasks(
		const std::vector<CompletedTasks> &completed_taskss,
		int completed_tasks_id
	)
	{
		for (auto &completed_tasks : completed_taskss) {
			if (completed_tasks.charid == completed_tasks_id) {
				return completed_tasks;
			}
		}

		return NewEntity();
	}

	static CompletedTasks FindOne(
		Database& db,
		int completed_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				completed_tasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CompletedTasks e{};

			e.charid        = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.completedtime = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.taskid        = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.activityid    = static_cast<int32_t>(atoi(row[3]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int completed_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				completed_tasks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CompletedTasks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.charid));
		v.push_back(columns[1] + " = " + std::to_string(e.completedtime));
		v.push_back(columns[2] + " = " + std::to_string(e.taskid));
		v.push_back(columns[3] + " = " + std::to_string(e.activityid));

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

	static CompletedTasks InsertOne(
		Database& db,
		CompletedTasks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.completedtime));
		v.push_back(std::to_string(e.taskid));
		v.push_back(std::to_string(e.activityid));

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
		const std::vector<CompletedTasks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.completedtime));
			v.push_back(std::to_string(e.taskid));
			v.push_back(std::to_string(e.activityid));

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

	static std::vector<CompletedTasks> All(Database& db)
	{
		std::vector<CompletedTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedTasks e{};

			e.charid        = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.completedtime = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.taskid        = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.activityid    = static_cast<int32_t>(atoi(row[3]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CompletedTasks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CompletedTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedTasks e{};

			e.charid        = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.completedtime = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.taskid        = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.activityid    = static_cast<int32_t>(atoi(row[3]));

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

#endif //EQEMU_BASE_COMPLETED_TASKS_REPOSITORY_H
