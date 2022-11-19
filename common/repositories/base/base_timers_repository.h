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

#ifndef EQEMU_BASE_TIMERS_REPOSITORY_H
#define EQEMU_BASE_TIMERS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseTimersRepository {
public:
	struct Timers {
		int32_t  char_id;
		uint32_t type;
		uint32_t start;
		uint32_t duration;
		int8_t   enable;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"type",
			"start",
			"duration",
			"enable",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"char_id",
			"type",
			"start",
			"duration",
			"enable",
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
		return std::string("timers");
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

	static Timers NewEntity()
	{
		Timers e{};

		e.char_id  = 0;
		e.type     = 0;
		e.start    = 0;
		e.duration = 0;
		e.enable   = 0;

		return e;
	}

	static Timers GetTimers(
		const std::vector<Timers> &timerss,
		int timers_id
	)
	{
		for (auto &timers : timerss) {
			if (timers.char_id == timers_id) {
				return timers;
			}
		}

		return NewEntity();
	}

	static Timers FindOne(
		Database& db,
		int timers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				timers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Timers e{};

			e.char_id  = static_cast<int32_t>(atoi(row[0]));
			e.type     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.start    = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.duration = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.enable   = static_cast<int8_t>(atoi(row[4]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int timers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				timers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Timers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.char_id));
		v.push_back(columns[1] + " = " + std::to_string(e.type));
		v.push_back(columns[2] + " = " + std::to_string(e.start));
		v.push_back(columns[3] + " = " + std::to_string(e.duration));
		v.push_back(columns[4] + " = " + std::to_string(e.enable));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Timers InsertOne(
		Database& db,
		Timers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.start));
		v.push_back(std::to_string(e.duration));
		v.push_back(std::to_string(e.enable));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.char_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Timers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.start));
			v.push_back(std::to_string(e.duration));
			v.push_back(std::to_string(e.enable));

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

	static std::vector<Timers> All(Database& db)
	{
		std::vector<Timers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Timers e{};

			e.char_id  = static_cast<int32_t>(atoi(row[0]));
			e.type     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.start    = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.duration = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.enable   = static_cast<int8_t>(atoi(row[4]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Timers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Timers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Timers e{};

			e.char_id  = static_cast<int32_t>(atoi(row[0]));
			e.type     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.start    = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.duration = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.enable   = static_cast<int8_t>(atoi(row[4]));

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

#endif //EQEMU_BASE_TIMERS_REPOSITORY_H
