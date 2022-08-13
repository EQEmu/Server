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

#ifndef EQEMU_BASE_TASKS_REPOSITORY_H
#define EQEMU_BASE_TASKS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseTasksRepository {
public:
	struct Tasks {
		int         id;
		int         type;
		int         duration;
		int         duration_code;
		std::string title;
		std::string description;
		std::string reward;
		int         rewardid;
		int         cashreward;
		int         xpreward;
		int         rewardmethod;
		int         reward_points;
		int         reward_point_type;
		int         minlevel;
		int         maxlevel;
		int         level_spread;
		int         min_players;
		int         max_players;
		int         repeatable;
		int         faction_reward;
		std::string completion_emote;
		int         replay_timer_group;
		int         replay_timer_seconds;
		int         request_timer_group;
		int         request_timer_seconds;
		int         dz_template_id;
		int         lock_activity_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"type",
			"duration",
			"duration_code",
			"title",
			"description",
			"reward",
			"rewardid",
			"cashreward",
			"xpreward",
			"rewardmethod",
			"reward_points",
			"reward_point_type",
			"minlevel",
			"maxlevel",
			"level_spread",
			"min_players",
			"max_players",
			"repeatable",
			"faction_reward",
			"completion_emote",
			"replay_timer_group",
			"replay_timer_seconds",
			"request_timer_group",
			"request_timer_seconds",
			"dz_template_id",
			"lock_activity_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"type",
			"duration",
			"duration_code",
			"title",
			"description",
			"reward",
			"rewardid",
			"cashreward",
			"xpreward",
			"rewardmethod",
			"reward_points",
			"reward_point_type",
			"minlevel",
			"maxlevel",
			"level_spread",
			"min_players",
			"max_players",
			"repeatable",
			"faction_reward",
			"completion_emote",
			"replay_timer_group",
			"replay_timer_seconds",
			"request_timer_group",
			"request_timer_seconds",
			"dz_template_id",
			"lock_activity_id",
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
		return std::string("tasks");
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

	static Tasks NewEntity()
	{
		Tasks e{};

		e.id                    = 0;
		e.type                  = 0;
		e.duration              = 0;
		e.duration_code         = 0;
		e.title                 = "";
		e.description           = "";
		e.reward                = "";
		e.rewardid              = 0;
		e.cashreward            = 0;
		e.xpreward              = 0;
		e.rewardmethod          = 2;
		e.reward_points         = 0;
		e.reward_point_type     = 0;
		e.minlevel              = 0;
		e.maxlevel              = 0;
		e.level_spread          = 0;
		e.min_players           = 0;
		e.max_players           = 0;
		e.repeatable            = 1;
		e.faction_reward        = 0;
		e.completion_emote      = "";
		e.replay_timer_group    = 0;
		e.replay_timer_seconds  = 0;
		e.request_timer_group   = 0;
		e.request_timer_seconds = 0;
		e.dz_template_id        = 0;
		e.lock_activity_id      = -1;

		return e;
	}

	static Tasks GetTasks(
		const std::vector<Tasks> &taskss,
		int tasks_id
	)
	{
		for (auto &tasks : taskss) {
			if (tasks.id == tasks_id) {
				return tasks;
			}
		}

		return NewEntity();
	}

	static Tasks FindOne(
		Database& db,
		int tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Tasks e{};

			e.id                    = atoi(row[0]);
			e.type                  = atoi(row[1]);
			e.duration              = atoi(row[2]);
			e.duration_code         = atoi(row[3]);
			e.title                 = row[4] ? row[4] : "";
			e.description           = row[5] ? row[5] : "";
			e.reward                = row[6] ? row[6] : "";
			e.rewardid              = atoi(row[7]);
			e.cashreward            = atoi(row[8]);
			e.xpreward              = atoi(row[9]);
			e.rewardmethod          = atoi(row[10]);
			e.reward_points         = atoi(row[11]);
			e.reward_point_type     = atoi(row[12]);
			e.minlevel              = atoi(row[13]);
			e.maxlevel              = atoi(row[14]);
			e.level_spread          = atoi(row[15]);
			e.min_players           = atoi(row[16]);
			e.max_players           = atoi(row[17]);
			e.repeatable            = atoi(row[18]);
			e.faction_reward        = atoi(row[19]);
			e.completion_emote      = row[20] ? row[20] : "";
			e.replay_timer_group    = atoi(row[21]);
			e.replay_timer_seconds  = atoi(row[22]);
			e.request_timer_group   = atoi(row[23]);
			e.request_timer_seconds = atoi(row[24]);
			e.dz_template_id        = atoi(row[25]);
			e.lock_activity_id      = atoi(row[26]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tasks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Tasks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.type));
		v.push_back(columns[2] + " = " + std::to_string(e.duration));
		v.push_back(columns[3] + " = " + std::to_string(e.duration_code));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.title) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.description) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.reward) + "'");
		v.push_back(columns[7] + " = " + std::to_string(e.rewardid));
		v.push_back(columns[8] + " = " + std::to_string(e.cashreward));
		v.push_back(columns[9] + " = " + std::to_string(e.xpreward));
		v.push_back(columns[10] + " = " + std::to_string(e.rewardmethod));
		v.push_back(columns[11] + " = " + std::to_string(e.reward_points));
		v.push_back(columns[12] + " = " + std::to_string(e.reward_point_type));
		v.push_back(columns[13] + " = " + std::to_string(e.minlevel));
		v.push_back(columns[14] + " = " + std::to_string(e.maxlevel));
		v.push_back(columns[15] + " = " + std::to_string(e.level_spread));
		v.push_back(columns[16] + " = " + std::to_string(e.min_players));
		v.push_back(columns[17] + " = " + std::to_string(e.max_players));
		v.push_back(columns[18] + " = " + std::to_string(e.repeatable));
		v.push_back(columns[19] + " = " + std::to_string(e.faction_reward));
		v.push_back(columns[20] + " = '" + Strings::Escape(e.completion_emote) + "'");
		v.push_back(columns[21] + " = " + std::to_string(e.replay_timer_group));
		v.push_back(columns[22] + " = " + std::to_string(e.replay_timer_seconds));
		v.push_back(columns[23] + " = " + std::to_string(e.request_timer_group));
		v.push_back(columns[24] + " = " + std::to_string(e.request_timer_seconds));
		v.push_back(columns[25] + " = " + std::to_string(e.dz_template_id));
		v.push_back(columns[26] + " = " + std::to_string(e.lock_activity_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Tasks InsertOne(
		Database& db,
		Tasks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.duration_code));
		v.push_back("'" + Strings::Escape(e.title) + "'");
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back("'" + Strings::Escape(e.reward) + "'");
		v.push_back(std::to_string(e.rewardid));
		v.push_back(std::to_string(e.cashreward));
		v.push_back(std::to_string(e.xpreward));
		v.push_back(std::to_string(e.rewardmethod));
		v.push_back(std::to_string(e.reward_points));
		v.push_back(std::to_string(e.reward_point_type));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.level_spread));
		v.push_back(std::to_string(e.min_players));
		v.push_back(std::to_string(e.max_players));
		v.push_back(std::to_string(e.repeatable));
		v.push_back(std::to_string(e.faction_reward));
		v.push_back("'" + Strings::Escape(e.completion_emote) + "'");
		v.push_back(std::to_string(e.replay_timer_group));
		v.push_back(std::to_string(e.replay_timer_seconds));
		v.push_back(std::to_string(e.request_timer_group));
		v.push_back(std::to_string(e.request_timer_seconds));
		v.push_back(std::to_string(e.dz_template_id));
		v.push_back(std::to_string(e.lock_activity_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Tasks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.duration_code));
			v.push_back("'" + Strings::Escape(e.title) + "'");
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back("'" + Strings::Escape(e.reward) + "'");
			v.push_back(std::to_string(e.rewardid));
			v.push_back(std::to_string(e.cashreward));
			v.push_back(std::to_string(e.xpreward));
			v.push_back(std::to_string(e.rewardmethod));
			v.push_back(std::to_string(e.reward_points));
			v.push_back(std::to_string(e.reward_point_type));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.level_spread));
			v.push_back(std::to_string(e.min_players));
			v.push_back(std::to_string(e.max_players));
			v.push_back(std::to_string(e.repeatable));
			v.push_back(std::to_string(e.faction_reward));
			v.push_back("'" + Strings::Escape(e.completion_emote) + "'");
			v.push_back(std::to_string(e.replay_timer_group));
			v.push_back(std::to_string(e.replay_timer_seconds));
			v.push_back(std::to_string(e.request_timer_group));
			v.push_back(std::to_string(e.request_timer_seconds));
			v.push_back(std::to_string(e.dz_template_id));
			v.push_back(std::to_string(e.lock_activity_id));

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

	static std::vector<Tasks> All(Database& db)
	{
		std::vector<Tasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tasks e{};

			e.id                    = atoi(row[0]);
			e.type                  = atoi(row[1]);
			e.duration              = atoi(row[2]);
			e.duration_code         = atoi(row[3]);
			e.title                 = row[4] ? row[4] : "";
			e.description           = row[5] ? row[5] : "";
			e.reward                = row[6] ? row[6] : "";
			e.rewardid              = atoi(row[7]);
			e.cashreward            = atoi(row[8]);
			e.xpreward              = atoi(row[9]);
			e.rewardmethod          = atoi(row[10]);
			e.reward_points         = atoi(row[11]);
			e.reward_point_type     = atoi(row[12]);
			e.minlevel              = atoi(row[13]);
			e.maxlevel              = atoi(row[14]);
			e.level_spread          = atoi(row[15]);
			e.min_players           = atoi(row[16]);
			e.max_players           = atoi(row[17]);
			e.repeatable            = atoi(row[18]);
			e.faction_reward        = atoi(row[19]);
			e.completion_emote      = row[20] ? row[20] : "";
			e.replay_timer_group    = atoi(row[21]);
			e.replay_timer_seconds  = atoi(row[22]);
			e.request_timer_group   = atoi(row[23]);
			e.request_timer_seconds = atoi(row[24]);
			e.dz_template_id        = atoi(row[25]);
			e.lock_activity_id      = atoi(row[26]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Tasks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Tasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tasks e{};

			e.id                    = atoi(row[0]);
			e.type                  = atoi(row[1]);
			e.duration              = atoi(row[2]);
			e.duration_code         = atoi(row[3]);
			e.title                 = row[4] ? row[4] : "";
			e.description           = row[5] ? row[5] : "";
			e.reward                = row[6] ? row[6] : "";
			e.rewardid              = atoi(row[7]);
			e.cashreward            = atoi(row[8]);
			e.xpreward              = atoi(row[9]);
			e.rewardmethod          = atoi(row[10]);
			e.reward_points         = atoi(row[11]);
			e.reward_point_type     = atoi(row[12]);
			e.minlevel              = atoi(row[13]);
			e.maxlevel              = atoi(row[14]);
			e.level_spread          = atoi(row[15]);
			e.min_players           = atoi(row[16]);
			e.max_players           = atoi(row[17]);
			e.repeatable            = atoi(row[18]);
			e.faction_reward        = atoi(row[19]);
			e.completion_emote      = row[20] ? row[20] : "";
			e.replay_timer_group    = atoi(row[21]);
			e.replay_timer_seconds  = atoi(row[22]);
			e.request_timer_group   = atoi(row[23]);
			e.request_timer_seconds = atoi(row[24]);
			e.dz_template_id        = atoi(row[25]);
			e.lock_activity_id      = atoi(row[26]);

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

#endif //EQEMU_BASE_TASKS_REPOSITORY_H
