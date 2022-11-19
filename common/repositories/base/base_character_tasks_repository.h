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

#ifndef EQEMU_BASE_CHARACTER_TASKS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_TASKS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterTasksRepository {
public:
	struct CharacterTasks {
		uint32_t charid;
		uint32_t taskid;
		uint32_t slot;
		int8_t   type;
		uint32_t acceptedtime;
		int8_t   was_rewarded;
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
			"slot",
			"type",
			"acceptedtime",
			"was_rewarded",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"charid",
			"taskid",
			"slot",
			"type",
			"acceptedtime",
			"was_rewarded",
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
		return std::string("character_tasks");
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

	static CharacterTasks NewEntity()
	{
		CharacterTasks e{};

		e.charid       = 0;
		e.taskid       = 0;
		e.slot         = 0;
		e.type         = 0;
		e.acceptedtime = 0;
		e.was_rewarded = 0;

		return e;
	}

	static CharacterTasks GetCharacterTasks(
		const std::vector<CharacterTasks> &character_taskss,
		int character_tasks_id
	)
	{
		for (auto &character_tasks : character_taskss) {
			if (character_tasks.charid == character_tasks_id) {
				return character_tasks;
			}
		}

		return NewEntity();
	}

	static CharacterTasks FindOne(
		Database& db,
		int character_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_tasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterTasks e{};

			e.charid       = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.taskid       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.slot         = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.type         = static_cast<int8_t>(atoi(row[3]));
			e.acceptedtime = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.was_rewarded = static_cast<int8_t>(atoi(row[5]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_tasks_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_tasks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterTasks &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.charid));
		v.push_back(columns[1] + " = " + std::to_string(e.taskid));
		v.push_back(columns[2] + " = " + std::to_string(e.slot));
		v.push_back(columns[3] + " = " + std::to_string(e.type));
		v.push_back(columns[4] + " = " + std::to_string(e.acceptedtime));
		v.push_back(columns[5] + " = " + std::to_string(e.was_rewarded));

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

	static CharacterTasks InsertOne(
		Database& db,
		CharacterTasks e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.taskid));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.acceptedtime));
		v.push_back(std::to_string(e.was_rewarded));

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
		const std::vector<CharacterTasks> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.taskid));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.acceptedtime));
			v.push_back(std::to_string(e.was_rewarded));

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

	static std::vector<CharacterTasks> All(Database& db)
	{
		std::vector<CharacterTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTasks e{};

			e.charid       = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.taskid       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.slot         = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.type         = static_cast<int8_t>(atoi(row[3]));
			e.acceptedtime = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.was_rewarded = static_cast<int8_t>(atoi(row[5]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterTasks> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterTasks> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterTasks e{};

			e.charid       = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.taskid       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.slot         = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.type         = static_cast<int8_t>(atoi(row[3]));
			e.acceptedtime = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.was_rewarded = static_cast<int8_t>(atoi(row[5]));

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

#endif //EQEMU_BASE_CHARACTER_TASKS_REPOSITORY_H
