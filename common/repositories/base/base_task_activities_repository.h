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
#include "../../strings.h"
#include <ctime>

class BaseTaskActivitiesRepository {
public:
	struct TaskActivities {
		int         taskid;
		int         activityid;
		int         req_activity_id;
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
		int         zone_version;
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
			"req_activity_id",
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
			"zone_version",
			"optional",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"taskid",
			"activityid",
			"req_activity_id",
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
			"zone_version",
			"optional",
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
		TaskActivities e{};

		e.taskid               = 0;
		e.activityid           = 0;
		e.req_activity_id      = -1;
		e.step                 = 0;
		e.activitytype         = 0;
		e.target_name          = "";
		e.item_list            = "";
		e.skill_list           = "-1";
		e.spell_list           = "0";
		e.description_override = "";
		e.goalid               = 0;
		e.goal_match_list      = "";
		e.goalmethod           = 0;
		e.goalcount            = 1;
		e.delivertonpc         = 0;
		e.zones                = "";
		e.zone_version         = -1;
		e.optional             = 0;

		return e;
	}

	static TaskActivities GetTaskActivities(
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
			TaskActivities e{};

			e.taskid               = atoi(row[0]);
			e.activityid           = atoi(row[1]);
			e.req_activity_id      = atoi(row[2]);
			e.step                 = atoi(row[3]);
			e.activitytype         = atoi(row[4]);
			e.target_name          = row[5] ? row[5] : "";
			e.item_list            = row[6] ? row[6] : "";
			e.skill_list           = row[7] ? row[7] : "";
			e.spell_list           = row[8] ? row[8] : "";
			e.description_override = row[9] ? row[9] : "";
			e.goalid               = atoi(row[10]);
			e.goal_match_list      = row[11] ? row[11] : "";
			e.goalmethod           = atoi(row[12]);
			e.goalcount            = atoi(row[13]);
			e.delivertonpc         = atoi(row[14]);
			e.zones                = row[15] ? row[15] : "";
			e.zone_version         = atoi(row[16]);
			e.optional             = atoi(row[17]);

			return e;
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
		const TaskActivities &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.taskid));
		v.push_back(columns[1] + " = " + std::to_string(e.activityid));
		v.push_back(columns[2] + " = " + std::to_string(e.req_activity_id));
		v.push_back(columns[3] + " = " + std::to_string(e.step));
		v.push_back(columns[4] + " = " + std::to_string(e.activitytype));
		v.push_back(columns[5] + " = '" + Strings::Escape(e.target_name) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.item_list) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.skill_list) + "'");
		v.push_back(columns[8] + " = '" + Strings::Escape(e.spell_list) + "'");
		v.push_back(columns[9] + " = '" + Strings::Escape(e.description_override) + "'");
		v.push_back(columns[10] + " = " + std::to_string(e.goalid));
		v.push_back(columns[11] + " = '" + Strings::Escape(e.goal_match_list) + "'");
		v.push_back(columns[12] + " = " + std::to_string(e.goalmethod));
		v.push_back(columns[13] + " = " + std::to_string(e.goalcount));
		v.push_back(columns[14] + " = " + std::to_string(e.delivertonpc));
		v.push_back(columns[15] + " = '" + Strings::Escape(e.zones) + "'");
		v.push_back(columns[16] + " = " + std::to_string(e.zone_version));
		v.push_back(columns[17] + " = " + std::to_string(e.optional));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.taskid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static TaskActivities InsertOne(
		Database& db,
		TaskActivities e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.taskid));
		v.push_back(std::to_string(e.activityid));
		v.push_back(std::to_string(e.req_activity_id));
		v.push_back(std::to_string(e.step));
		v.push_back(std::to_string(e.activitytype));
		v.push_back("'" + Strings::Escape(e.target_name) + "'");
		v.push_back("'" + Strings::Escape(e.item_list) + "'");
		v.push_back("'" + Strings::Escape(e.skill_list) + "'");
		v.push_back("'" + Strings::Escape(e.spell_list) + "'");
		v.push_back("'" + Strings::Escape(e.description_override) + "'");
		v.push_back(std::to_string(e.goalid));
		v.push_back("'" + Strings::Escape(e.goal_match_list) + "'");
		v.push_back(std::to_string(e.goalmethod));
		v.push_back(std::to_string(e.goalcount));
		v.push_back(std::to_string(e.delivertonpc));
		v.push_back("'" + Strings::Escape(e.zones) + "'");
		v.push_back(std::to_string(e.zone_version));
		v.push_back(std::to_string(e.optional));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.taskid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<TaskActivities> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.taskid));
			v.push_back(std::to_string(e.activityid));
			v.push_back(std::to_string(e.req_activity_id));
			v.push_back(std::to_string(e.step));
			v.push_back(std::to_string(e.activitytype));
			v.push_back("'" + Strings::Escape(e.target_name) + "'");
			v.push_back("'" + Strings::Escape(e.item_list) + "'");
			v.push_back("'" + Strings::Escape(e.skill_list) + "'");
			v.push_back("'" + Strings::Escape(e.spell_list) + "'");
			v.push_back("'" + Strings::Escape(e.description_override) + "'");
			v.push_back(std::to_string(e.goalid));
			v.push_back("'" + Strings::Escape(e.goal_match_list) + "'");
			v.push_back(std::to_string(e.goalmethod));
			v.push_back(std::to_string(e.goalcount));
			v.push_back(std::to_string(e.delivertonpc));
			v.push_back("'" + Strings::Escape(e.zones) + "'");
			v.push_back(std::to_string(e.zone_version));
			v.push_back(std::to_string(e.optional));

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
			TaskActivities e{};

			e.taskid               = atoi(row[0]);
			e.activityid           = atoi(row[1]);
			e.req_activity_id      = atoi(row[2]);
			e.step                 = atoi(row[3]);
			e.activitytype         = atoi(row[4]);
			e.target_name          = row[5] ? row[5] : "";
			e.item_list            = row[6] ? row[6] : "";
			e.skill_list           = row[7] ? row[7] : "";
			e.spell_list           = row[8] ? row[8] : "";
			e.description_override = row[9] ? row[9] : "";
			e.goalid               = atoi(row[10]);
			e.goal_match_list      = row[11] ? row[11] : "";
			e.goalmethod           = atoi(row[12]);
			e.goalcount            = atoi(row[13]);
			e.delivertonpc         = atoi(row[14]);
			e.zones                = row[15] ? row[15] : "";
			e.zone_version         = atoi(row[16]);
			e.optional             = atoi(row[17]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<TaskActivities> GetWhere(Database& db, const std::string &where_filter)
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
			TaskActivities e{};

			e.taskid               = atoi(row[0]);
			e.activityid           = atoi(row[1]);
			e.req_activity_id      = atoi(row[2]);
			e.step                 = atoi(row[3]);
			e.activitytype         = atoi(row[4]);
			e.target_name          = row[5] ? row[5] : "";
			e.item_list            = row[6] ? row[6] : "";
			e.skill_list           = row[7] ? row[7] : "";
			e.spell_list           = row[8] ? row[8] : "";
			e.description_override = row[9] ? row[9] : "";
			e.goalid               = atoi(row[10]);
			e.goal_match_list      = row[11] ? row[11] : "";
			e.goalmethod           = atoi(row[12]);
			e.goalcount            = atoi(row[13]);
			e.delivertonpc         = atoi(row[14]);
			e.zones                = row[15] ? row[15] : "";
			e.zone_version         = atoi(row[16]);
			e.optional             = atoi(row[17]);

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

#endif //EQEMU_BASE_TASK_ACTIVITIES_REPOSITORY_H
