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

#ifndef EQEMU_BASE_TASKS_REPOSITORY_H
#define EQEMU_BASE_TASKS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

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
		int         minlevel;
		int         maxlevel;
		int         repeatable;
		int         faction_reward;
		std::string completion_emote;
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
			"minlevel",
			"maxlevel",
			"repeatable",
			"faction_reward",
			"completion_emote",
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
		return std::string("tasks");
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

	static Tasks NewEntity()
	{
		Tasks entry{};

		entry.id               = 0;
		entry.type             = 0;
		entry.duration         = 0;
		entry.duration_code    = 0;
		entry.title            = "";
		entry.description      = "";
		entry.reward           = "";
		entry.rewardid         = 0;
		entry.cashreward       = 0;
		entry.xpreward         = 0;
		entry.rewardmethod     = 2;
		entry.minlevel         = 0;
		entry.maxlevel         = 0;
		entry.repeatable       = 1;
		entry.faction_reward   = 0;
		entry.completion_emote = "";

		return entry;
	}

	static Tasks GetTasksEntry(
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
		int tasks_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Tasks entry{};

			entry.id               = atoi(row[0]);
			entry.type             = atoi(row[1]);
			entry.duration         = atoi(row[2]);
			entry.duration_code    = atoi(row[3]);
			entry.title            = row[4] ? row[4] : "";
			entry.description      = row[5] ? row[5] : "";
			entry.reward           = row[6] ? row[6] : "";
			entry.rewardid         = atoi(row[7]);
			entry.cashreward       = atoi(row[8]);
			entry.xpreward         = atoi(row[9]);
			entry.rewardmethod     = atoi(row[10]);
			entry.minlevel         = atoi(row[11]);
			entry.maxlevel         = atoi(row[12]);
			entry.repeatable       = atoi(row[13]);
			entry.faction_reward   = atoi(row[14]);
			entry.completion_emote = row[15] ? row[15] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int tasks_id
	)
	{
		auto results = content_db.QueryDatabase(
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
		Tasks tasks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(tasks_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(tasks_entry.type));
		update_values.push_back(columns[2] + " = " + std::to_string(tasks_entry.duration));
		update_values.push_back(columns[3] + " = " + std::to_string(tasks_entry.duration_code));
		update_values.push_back(columns[4] + " = '" + EscapeString(tasks_entry.title) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(tasks_entry.description) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(tasks_entry.reward) + "'");
		update_values.push_back(columns[7] + " = " + std::to_string(tasks_entry.rewardid));
		update_values.push_back(columns[8] + " = " + std::to_string(tasks_entry.cashreward));
		update_values.push_back(columns[9] + " = " + std::to_string(tasks_entry.xpreward));
		update_values.push_back(columns[10] + " = " + std::to_string(tasks_entry.rewardmethod));
		update_values.push_back(columns[11] + " = " + std::to_string(tasks_entry.minlevel));
		update_values.push_back(columns[12] + " = " + std::to_string(tasks_entry.maxlevel));
		update_values.push_back(columns[13] + " = " + std::to_string(tasks_entry.repeatable));
		update_values.push_back(columns[14] + " = " + std::to_string(tasks_entry.faction_reward));
		update_values.push_back(columns[15] + " = '" + EscapeString(tasks_entry.completion_emote) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				tasks_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Tasks InsertOne(
		Tasks tasks_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(tasks_entry.id));
		insert_values.push_back(std::to_string(tasks_entry.type));
		insert_values.push_back(std::to_string(tasks_entry.duration));
		insert_values.push_back(std::to_string(tasks_entry.duration_code));
		insert_values.push_back("'" + EscapeString(tasks_entry.title) + "'");
		insert_values.push_back("'" + EscapeString(tasks_entry.description) + "'");
		insert_values.push_back("'" + EscapeString(tasks_entry.reward) + "'");
		insert_values.push_back(std::to_string(tasks_entry.rewardid));
		insert_values.push_back(std::to_string(tasks_entry.cashreward));
		insert_values.push_back(std::to_string(tasks_entry.xpreward));
		insert_values.push_back(std::to_string(tasks_entry.rewardmethod));
		insert_values.push_back(std::to_string(tasks_entry.minlevel));
		insert_values.push_back(std::to_string(tasks_entry.maxlevel));
		insert_values.push_back(std::to_string(tasks_entry.repeatable));
		insert_values.push_back(std::to_string(tasks_entry.faction_reward));
		insert_values.push_back("'" + EscapeString(tasks_entry.completion_emote) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			tasks_entry.id = results.LastInsertedID();
			return tasks_entry;
		}

		tasks_entry = NewEntity();

		return tasks_entry;
	}

	static int InsertMany(
		std::vector<Tasks> tasks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &tasks_entry: tasks_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(tasks_entry.id));
			insert_values.push_back(std::to_string(tasks_entry.type));
			insert_values.push_back(std::to_string(tasks_entry.duration));
			insert_values.push_back(std::to_string(tasks_entry.duration_code));
			insert_values.push_back("'" + EscapeString(tasks_entry.title) + "'");
			insert_values.push_back("'" + EscapeString(tasks_entry.description) + "'");
			insert_values.push_back("'" + EscapeString(tasks_entry.reward) + "'");
			insert_values.push_back(std::to_string(tasks_entry.rewardid));
			insert_values.push_back(std::to_string(tasks_entry.cashreward));
			insert_values.push_back(std::to_string(tasks_entry.xpreward));
			insert_values.push_back(std::to_string(tasks_entry.rewardmethod));
			insert_values.push_back(std::to_string(tasks_entry.minlevel));
			insert_values.push_back(std::to_string(tasks_entry.maxlevel));
			insert_values.push_back(std::to_string(tasks_entry.repeatable));
			insert_values.push_back(std::to_string(tasks_entry.faction_reward));
			insert_values.push_back("'" + EscapeString(tasks_entry.completion_emote) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Tasks> All()
	{
		std::vector<Tasks> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tasks entry{};

			entry.id               = atoi(row[0]);
			entry.type             = atoi(row[1]);
			entry.duration         = atoi(row[2]);
			entry.duration_code    = atoi(row[3]);
			entry.title            = row[4] ? row[4] : "";
			entry.description      = row[5] ? row[5] : "";
			entry.reward           = row[6] ? row[6] : "";
			entry.rewardid         = atoi(row[7]);
			entry.cashreward       = atoi(row[8]);
			entry.xpreward         = atoi(row[9]);
			entry.rewardmethod     = atoi(row[10]);
			entry.minlevel         = atoi(row[11]);
			entry.maxlevel         = atoi(row[12]);
			entry.repeatable       = atoi(row[13]);
			entry.faction_reward   = atoi(row[14]);
			entry.completion_emote = row[15] ? row[15] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Tasks> GetWhere(std::string where_filter)
	{
		std::vector<Tasks> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tasks entry{};

			entry.id               = atoi(row[0]);
			entry.type             = atoi(row[1]);
			entry.duration         = atoi(row[2]);
			entry.duration_code    = atoi(row[3]);
			entry.title            = row[4] ? row[4] : "";
			entry.description      = row[5] ? row[5] : "";
			entry.reward           = row[6] ? row[6] : "";
			entry.rewardid         = atoi(row[7]);
			entry.cashreward       = atoi(row[8]);
			entry.xpreward         = atoi(row[9]);
			entry.rewardmethod     = atoi(row[10]);
			entry.minlevel         = atoi(row[11]);
			entry.maxlevel         = atoi(row[12]);
			entry.repeatable       = atoi(row[13]);
			entry.faction_reward   = atoi(row[14]);
			entry.completion_emote = row[15] ? row[15] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_TASKS_REPOSITORY_H
