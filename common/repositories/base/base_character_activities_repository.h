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

#ifndef EQEMU_BASE_CHARACTER_ACTIVITIES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_ACTIVITIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterActivitiesRepository {
public:
	struct CharacterActivities {
		int charid;
		int taskid;
		int activityid;
		int donecount;
		int completed;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"taskid",
			"activityid",
			"donecount",
			"completed",
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
		return std::string("character_activities");
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

	static CharacterActivities NewEntity()
	{
		CharacterActivities entry{};

		entry.charid     = 0;
		entry.taskid     = 0;
		entry.activityid = 0;
		entry.donecount  = 0;
		entry.completed  = 0;

		return entry;
	}

	static CharacterActivities GetCharacterActivitiesEntry(
		const std::vector<CharacterActivities> &character_activitiess,
		int character_activities_id
	)
	{
		for (auto &character_activities : character_activitiess) {
			if (character_activities.charid == character_activities_id) {
				return character_activities;
			}
		}

		return NewEntity();
	}

	static CharacterActivities FindOne(
		Database& db,
		int character_activities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_activities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterActivities entry{};

			entry.charid     = atoi(row[0]);
			entry.taskid     = atoi(row[1]);
			entry.activityid = atoi(row[2]);
			entry.donecount  = atoi(row[3]);
			entry.completed  = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_activities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_activities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterActivities character_activities_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_activities_entry.charid));
		update_values.push_back(columns[1] + " = " + std::to_string(character_activities_entry.taskid));
		update_values.push_back(columns[2] + " = " + std::to_string(character_activities_entry.activityid));
		update_values.push_back(columns[3] + " = " + std::to_string(character_activities_entry.donecount));
		update_values.push_back(columns[4] + " = " + std::to_string(character_activities_entry.completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_activities_entry.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterActivities InsertOne(
		Database& db,
		CharacterActivities character_activities_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_activities_entry.charid));
		insert_values.push_back(std::to_string(character_activities_entry.taskid));
		insert_values.push_back(std::to_string(character_activities_entry.activityid));
		insert_values.push_back(std::to_string(character_activities_entry.donecount));
		insert_values.push_back(std::to_string(character_activities_entry.completed));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_activities_entry.charid = results.LastInsertedID();
			return character_activities_entry;
		}

		character_activities_entry = NewEntity();

		return character_activities_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterActivities> character_activities_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_activities_entry: character_activities_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_activities_entry.charid));
			insert_values.push_back(std::to_string(character_activities_entry.taskid));
			insert_values.push_back(std::to_string(character_activities_entry.activityid));
			insert_values.push_back(std::to_string(character_activities_entry.donecount));
			insert_values.push_back(std::to_string(character_activities_entry.completed));

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

	static std::vector<CharacterActivities> All(Database& db)
	{
		std::vector<CharacterActivities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterActivities entry{};

			entry.charid     = atoi(row[0]);
			entry.taskid     = atoi(row[1]);
			entry.activityid = atoi(row[2]);
			entry.donecount  = atoi(row[3]);
			entry.completed  = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterActivities> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterActivities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterActivities entry{};

			entry.charid     = atoi(row[0]);
			entry.taskid     = atoi(row[1]);
			entry.activityid = atoi(row[2]);
			entry.donecount  = atoi(row[3]);
			entry.completed  = atoi(row[4]);

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

#endif //EQEMU_BASE_CHARACTER_ACTIVITIES_REPOSITORY_H
