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

#ifndef EQEMU_BASE_TASK_ACTIVITIES_REPOSITORY_H
#define EQEMU_BASE_TASK_ACTIVITIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseTaskActivitiesRepository {
public:
	struct TaskActivities {
		int         taskid;
		int         activityid;
		int         step;
		int         activitytype;
		std::string target_name;
		std::string item_list;
		std::string skill_list;
		std::string spell_list;
		std::string description_override;
		int         goalid;
		std::string goal_match_list;
		int         goalmethod;
		int         goalcount;
		int         delivertonpc;
		std::string zones;
		int         optional;
	};

	static std::string PrimaryKey()
	{
		return std::string("taskid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"taskid",
			"activityid",
			"step",
			"activitytype",
			"target_name",
			"item_list",
			"skill_list",
			"spell_list",
			"description_override",
			"goalid",
			"goal_match_list",
			"goalmethod",
			"goalcount",
			"delivertonpc",
			"zones",
			"optional",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"taskid",
			"activityid",
			"step",
			"activitytype",
			"target_name",
			"item_list",
			"skill_list",
			"spell_list",
			"description_override",
			"goalid",
			"goal_match_list",
			"goalmethod",
			"goalcount",
			"delivertonpc",
			"zones",
			"optional",
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
		return std::string("task_activities");
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

	static TaskActivities NewEntity()
	{
		TaskActivities entry{};

		entry.taskid               = 0;
		entry.activityid           = 0;
		entry.step                 = 0;
		entry.activitytype         = 0;
		entry.target_name          = "";
		entry.item_list            = "";
		entry.skill_list           = "-1";
		entry.spell_list           = "0";
		entry.description_override = "";
		entry.goalid               = 0;
		entry.goal_match_list      = "";
		entry.goalmethod           = 0;
		entry.goalcount            = 1;
		entry.delivertonpc         = 0;
		entry.zones                = "";
		entry.optional             = 0;

		return entry;
	}

	static TaskActivities GetTaskActivitiesEntry(
		const std::vector<TaskActivities> &task_activitiess,
		int task_activities_id
	)
	{
		for (auto &task_activities : task_activitiess) {
			if (task_activities.taskid == task_activities_id) {
				return task_activities;
			}
		}

		return NewEntity();
	}

	static TaskActivities FindOne(
		Database& db,
		int task_activities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				task_activities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			TaskActivities entry{};

			entry.taskid               = atoi(row[0]);
			entry.activityid           = atoi(row[1]);
			entry.step                 = atoi(row[2]);
			entry.activitytype         = atoi(row[3]);
			entry.target_name          = row[4] ? row[4] : "";
			entry.item_list            = row[5] ? row[5] : "";
			entry.skill_list           = row[6] ? row[6] : "";
			entry.spell_list           = row[7] ? row[7] : "";
			entry.description_override = row[8] ? row[8] : "";
			entry.goalid               = atoi(row[9]);
			entry.goal_match_list      = row[10] ? row[10] : "";
			entry.goalmethod           = atoi(row[11]);
			entry.goalcount            = atoi(row[12]);
			entry.delivertonpc         = atoi(row[13]);
			entry.zones                = row[14] ? row[14] : "";
			entry.optional             = atoi(row[15]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int task_activities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				task_activities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		TaskActivities task_activities_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(task_activities_entry.taskid));
		update_values.push_back(columns[1] + " = " + std::to_string(task_activities_entry.activityid));
		update_values.push_back(columns[2] + " = " + std::to_string(task_activities_entry.step));
		update_values.push_back(columns[3] + " = " + std::to_string(task_activities_entry.activitytype));
		update_values.push_back(columns[4] + " = '" + EscapeString(task_activities_entry.target_name) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(task_activities_entry.item_list) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(task_activities_entry.skill_list) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(task_activities_entry.spell_list) + "'");
		update_values.push_back(columns[8] + " = '" + EscapeString(task_activities_entry.description_override) + "'");
		update_values.push_back(columns[9] + " = " + std::to_string(task_activities_entry.goalid));
		update_values.push_back(columns[10] + " = '" + EscapeString(task_activities_entry.goal_match_list) + "'");
		update_values.push_back(columns[11] + " = " + std::to_string(task_activities_entry.goalmethod));
		update_values.push_back(columns[12] + " = " + std::to_string(task_activities_entry.goalcount));
		update_values.push_back(columns[13] + " = " + std::to_string(task_activities_entry.delivertonpc));
		update_values.push_back(columns[14] + " = '" + EscapeString(task_activities_entry.zones) + "'");
		update_values.push_back(columns[15] + " = " + std::to_string(task_activities_entry.optional));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				task_activities_entry.taskid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static TaskActivities InsertOne(
		Database& db,
		TaskActivities task_activities_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(task_activities_entry.taskid));
		insert_values.push_back(std::to_string(task_activities_entry.activityid));
		insert_values.push_back(std::to_string(task_activities_entry.step));
		insert_values.push_back(std::to_string(task_activities_entry.activitytype));
		insert_values.push_back("'" + EscapeString(task_activities_entry.target_name) + "'");
		insert_values.push_back("'" + EscapeString(task_activities_entry.item_list) + "'");
		insert_values.push_back("'" + EscapeString(task_activities_entry.skill_list) + "'");
		insert_values.push_back("'" + EscapeString(task_activities_entry.spell_list) + "'");
		insert_values.push_back("'" + EscapeString(task_activities_entry.description_override) + "'");
		insert_values.push_back(std::to_string(task_activities_entry.goalid));
		insert_values.push_back("'" + EscapeString(task_activities_entry.goal_match_list) + "'");
		insert_values.push_back(std::to_string(task_activities_entry.goalmethod));
		insert_values.push_back(std::to_string(task_activities_entry.goalcount));
		insert_values.push_back(std::to_string(task_activities_entry.delivertonpc));
		insert_values.push_back("'" + EscapeString(task_activities_entry.zones) + "'");
		insert_values.push_back(std::to_string(task_activities_entry.optional));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			task_activities_entry.taskid = results.LastInsertedID();
			return task_activities_entry;
		}

		task_activities_entry = NewEntity();

		return task_activities_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<TaskActivities> task_activities_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &task_activities_entry: task_activities_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(task_activities_entry.taskid));
			insert_values.push_back(std::to_string(task_activities_entry.activityid));
			insert_values.push_back(std::to_string(task_activities_entry.step));
			insert_values.push_back(std::to_string(task_activities_entry.activitytype));
			insert_values.push_back("'" + EscapeString(task_activities_entry.target_name) + "'");
			insert_values.push_back("'" + EscapeString(task_activities_entry.item_list) + "'");
			insert_values.push_back("'" + EscapeString(task_activities_entry.skill_list) + "'");
			insert_values.push_back("'" + EscapeString(task_activities_entry.spell_list) + "'");
			insert_values.push_back("'" + EscapeString(task_activities_entry.description_override) + "'");
			insert_values.push_back(std::to_string(task_activities_entry.goalid));
			insert_values.push_back("'" + EscapeString(task_activities_entry.goal_match_list) + "'");
			insert_values.push_back(std::to_string(task_activities_entry.goalmethod));
			insert_values.push_back(std::to_string(task_activities_entry.goalcount));
			insert_values.push_back(std::to_string(task_activities_entry.delivertonpc));
			insert_values.push_back("'" + EscapeString(task_activities_entry.zones) + "'");
			insert_values.push_back(std::to_string(task_activities_entry.optional));

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

	static std::vector<TaskActivities> All(Database& db)
	{
		std::vector<TaskActivities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TaskActivities entry{};

			entry.taskid               = atoi(row[0]);
			entry.activityid           = atoi(row[1]);
			entry.step                 = atoi(row[2]);
			entry.activitytype         = atoi(row[3]);
			entry.target_name          = row[4] ? row[4] : "";
			entry.item_list            = row[5] ? row[5] : "";
			entry.skill_list           = row[6] ? row[6] : "";
			entry.spell_list           = row[7] ? row[7] : "";
			entry.description_override = row[8] ? row[8] : "";
			entry.goalid               = atoi(row[9]);
			entry.goal_match_list      = row[10] ? row[10] : "";
			entry.goalmethod           = atoi(row[11]);
			entry.goalcount            = atoi(row[12]);
			entry.delivertonpc         = atoi(row[13]);
			entry.zones                = row[14] ? row[14] : "";
			entry.optional             = atoi(row[15]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<TaskActivities> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<TaskActivities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TaskActivities entry{};

			entry.taskid               = atoi(row[0]);
			entry.activityid           = atoi(row[1]);
			entry.step                 = atoi(row[2]);
			entry.activitytype         = atoi(row[3]);
			entry.target_name          = row[4] ? row[4] : "";
			entry.item_list            = row[5] ? row[5] : "";
			entry.skill_list           = row[6] ? row[6] : "";
			entry.spell_list           = row[7] ? row[7] : "";
			entry.description_override = row[8] ? row[8] : "";
			entry.goalid               = atoi(row[9]);
			entry.goal_match_list      = row[10] ? row[10] : "";
			entry.goalmethod           = atoi(row[11]);
			entry.goalcount            = atoi(row[12]);
			entry.delivertonpc         = atoi(row[13]);
			entry.zones                = row[14] ? row[14] : "";
			entry.optional             = atoi(row[15]);

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

#endif //EQEMU_BASE_TASK_ACTIVITIES_REPOSITORY_H
