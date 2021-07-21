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

#ifndef EQEMU_BASE_CHARACTER_TASK_TIMERS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_TASK_TIMERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseCharacterTaskTimersRepository {
public:
	struct CharacterTaskTimers {
		int    id;
		int    character_id;
		int    task_id;
		int    timer_type;
		time_t expire_time;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"character_id",
			"task_id",
			"timer_type",
			"expire_time",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"character_id",
			"task_id",
			"timer_type",
			"UNIX_TIMESTAMP(expire_time)",
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
		return std::string("character_task_timers");
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

	static CharacterTaskTimers NewEntity()
	{
		CharacterTaskTimers entry{};

		entry.id           = 0;
		entry.character_id = 0;
		entry.task_id      = 0;
		entry.timer_type   = 0;
		entry.expire_time  = std::time(nullptr);

		return entry;
	}

	static CharacterTaskTimers GetCharacterTaskTimersEntry(
		const std::vector<CharacterTaskTimers> &character_task_timerss,
		int character_task_timers_id
	)
	{
		for (auto &character_task_timers : character_task_timerss) {
			if (character_task_timers.id == character_task_timers_id) {
				return character_task_timers;
			}
		}

		return NewEntity();
	}

	static CharacterTaskTimers FindOne(
		Database& db,
		int character_task_timers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_task_timers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterTaskTimers entry{};

			entry.id           = atoi(row[0]);
			entry.character_id = atoi(row[1]);
			entry.task_id      = atoi(row[2]);
			entry.timer_type   = atoi(row[3]);
			entry.expire_time  = strtoll(row[4], nullptr, 10);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_task_timers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_task_timers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterTaskTimers character_task_timers_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(character_task_timers_entry.character_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_task_timers_entry.task_id));
		update_values.push_back(columns[3] + " = " + std::to_string(character_task_timers_entry.timer_type));
		update_values.push_back(columns[4] + " = FROM_UNIXTIME(" + std::to_string(character_task_timers_entry.expire_time) + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_task_timers_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterTaskTimers InsertOne(
		Database& db,
		CharacterTaskTimers character_task_timers_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_task_timers_entry.id));
		insert_values.push_back(std::to_string(character_task_timers_entry.character_id));
		insert_values.push_back(std::to_string(character_task_timers_entry.task_id));
		insert_values.push_back(std::to_string(character_task_timers_entry.timer_type));
		insert_values.push_back("FROM_UNIXTIME(" + std::to_string(character_task_timers_entry.expire_time) + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_task_timers_entry.id = results.LastInsertedID();
			return character_task_timers_entry;
		}

		character_task_timers_entry = NewEntity();

		return character_task_timers_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterTaskTimers> character_task_timers_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_task_timers_entry: character_task_timers_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_task_timers_entry.id));
			insert_values.push_back(std::to_string(character_task_timers_entry.character_id));
			insert_values.push_back(std::to_string(character_task_timers_entry.task_id));
			insert_values.push_back(std::to_string(character_task_timers_entry.timer_type));
			insert_values.push_back("FROM_UNIXTIME(" + std::to_string(character_task_timers_entry.expire_time) + ")");

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

	static std::vector<CharacterTaskTimers> All(Database& db)
	{
		std::vector<CharacterTaskTimers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTaskTimers entry{};

			entry.id           = atoi(row[0]);
			entry.character_id = atoi(row[1]);
			entry.task_id      = atoi(row[2]);
			entry.timer_type   = atoi(row[3]);
			entry.expire_time  = strtoll(row[4], nullptr, 10);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterTaskTimers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterTaskTimers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTaskTimers entry{};

			entry.id           = atoi(row[0]);
			entry.character_id = atoi(row[1]);
			entry.task_id      = atoi(row[2]);
			entry.timer_type   = atoi(row[3]);
			entry.expire_time  = strtoll(row[4], nullptr, 10);

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

#endif //EQEMU_BASE_CHARACTER_TASK_TIMERS_REPOSITORY_H
