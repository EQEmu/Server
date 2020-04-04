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
 */

#ifndef EQEMU_COMPLETED_TASKS_REPOSITORY_H
#define EQEMU_COMPLETED_TASKS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class CompletedTasksRepository {
public:
	struct CompletedTasks {
		int charid;
		int completedtime;
		int taskid;
		int activityid;
	};

	static std::string PrimaryKey()
	{
		return std::string("activityid");
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
			if (completed_tasks.activityid == completed_tasks_id) {
				return completed_tasks;
			}
		}

		return NewEntity();
	}

	static CompletedTasks FindOne(
		int completed_tasks_id
	)
	{
		auto results = database.QueryDatabase(
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
		int completed_tasks_id
	)
	{
		auto results = database.QueryDatabase(
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
		CompletedTasks completed_tasks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();



		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				completed_tasks_entry.activityid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CompletedTasks InsertOne(
		CompletedTasks completed_tasks_entry
	)
	{
		std::vector<std::string> insert_values;



		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			completed_tasks_entry.id = results.LastInsertedID();
			return completed_tasks_entry;
		}

		completed_tasks_entry = CompletedTasksRepository::NewEntity();

		return completed_tasks_entry;
	}

	static int InsertMany(
		std::vector<CompletedTasks> completed_tasks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &completed_tasks_entry: completed_tasks_entries) {
			std::vector<std::string> insert_values;



			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<CompletedTasks> All()
	{
		std::vector<CompletedTasks> all_entries;

		auto results = database.QueryDatabase(
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

	static std::vector<CompletedTasks> GetWhere(std::string where_filter)
	{
		std::vector<CompletedTasks> all_entries;

		auto results = database.QueryDatabase(
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

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_COMPLETED_TASKS_REPOSITORY_H
