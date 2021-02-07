/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_COMPLETED_TASKS_REPOSITORY_H
#define EQEMU_BASE_COMPLETED_TASKS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCompletedTasksRepository {
public:
	struct CompletedTasks {
		int charid;
		int completedtime;
		int taskid;
		int activityid;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("completed_tasks");
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
			InsertColumnsRaw()
		);
	}

	static CompletedTasks NewEntity()
	{
		CompletedTasks entry{};

		entry.charid        = 0;
		entry.completedtime = 0;
		entry.taskid        = 0;
		entry.activityid    = 0;

		return entry;
	}

	static CompletedTasks GetCompletedTasksEntry(
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
			CompletedTasks entry{};

			entry.charid        = atoi(row[0]);
			entry.completedtime = atoi(row[1]);
			entry.taskid        = atoi(row[2]);
			entry.activityid    = atoi(row[3]);

			return entry;
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
		CompletedTasks completed_tasks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(completed_tasks_entry.charid));
		update_values.push_back(columns[1] + " = " + std::to_string(completed_tasks_entry.completedtime));
		update_values.push_back(columns[2] + " = " + std::to_string(completed_tasks_entry.taskid));
		update_values.push_back(columns[3] + " = " + std::to_string(completed_tasks_entry.activityid));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				completed_tasks_entry.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CompletedTasks InsertOne(
		Database& db,
		CompletedTasks completed_tasks_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(completed_tasks_entry.charid));
		insert_values.push_back(std::to_string(completed_tasks_entry.completedtime));
		insert_values.push_back(std::to_string(completed_tasks_entry.taskid));
		insert_values.push_back(std::to_string(completed_tasks_entry.activityid));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			completed_tasks_entry.charid = results.LastInsertedID();
			return completed_tasks_entry;
		}

		completed_tasks_entry = NewEntity();

		return completed_tasks_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CompletedTasks> completed_tasks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &completed_tasks_entry: completed_tasks_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(completed_tasks_entry.charid));
			insert_values.push_back(std::to_string(completed_tasks_entry.completedtime));
			insert_values.push_back(std::to_string(completed_tasks_entry.taskid));
			insert_values.push_back(std::to_string(completed_tasks_entry.activityid));

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
			CompletedTasks entry{};

			entry.charid        = atoi(row[0]);
			entry.completedtime = atoi(row[1]);
			entry.taskid        = atoi(row[2]);
			entry.activityid    = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CompletedTasks> GetWhere(Database& db, std::string where_filter)
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
			CompletedTasks entry{};

			entry.charid        = atoi(row[0]);
			entry.completedtime = atoi(row[1]);
			entry.taskid        = atoi(row[2]);
			entry.activityid    = atoi(row[3]);

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

#endif //EQEMU_BASE_COMPLETED_TASKS_REPOSITORY_H
