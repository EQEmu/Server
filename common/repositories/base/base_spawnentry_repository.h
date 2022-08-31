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

#ifndef EQEMU_BASE_SPAWNENTRY_REPOSITORY_H
#define EQEMU_BASE_SPAWNENTRY_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseSpawnentryRepository {
public:
	struct Spawnentry {
		int32_t     spawngroupID;
		int32_t     npcID;
		int16_t     chance;
		int32_t     condition_value_filter;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("spawngroupID");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"spawngroupID",
			"npcID",
			"chance",
			"condition_value_filter",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"spawngroupID",
			"npcID",
			"chance",
			"condition_value_filter",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("spawnentry");
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

	static Spawnentry NewEntity()
	{
		Spawnentry e{};

		e.spawngroupID           = 0;
		e.npcID                  = 0;
		e.chance                 = 0;
		e.condition_value_filter = 1;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static Spawnentry GetSpawnentry(
		const std::vector<Spawnentry> &spawnentrys,
		int spawnentry_id
	)
	{
		for (auto &spawnentry : spawnentrys) {
			if (spawnentry.spawngroupID == spawnentry_id) {
				return spawnentry;
			}
		}

		return NewEntity();
	}

	static Spawnentry FindOne(
		Database& db,
		int spawnentry_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spawnentry_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Spawnentry e{};

			e.spawngroupID           = static_cast<int32_t>(atoi(row[0]));
			e.npcID                  = static_cast<int32_t>(atoi(row[1]));
			e.chance                 = static_cast<int16_t>(atoi(row[2]));
			e.condition_value_filter = static_cast<int32_t>(atoi(row[3]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[4]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[5]));
			e.content_flags          = row[6] ? row[6] : "";
			e.content_flags_disabled = row[7] ? row[7] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawnentry_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawnentry_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Spawnentry &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.spawngroupID));
		v.push_back(columns[1] + " = " + std::to_string(e.npcID));
		v.push_back(columns[2] + " = " + std::to_string(e.chance));
		v.push_back(columns[3] + " = " + std::to_string(e.condition_value_filter));
		v.push_back(columns[4] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[5] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[6] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.spawngroupID
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Spawnentry InsertOne(
		Database& db,
		Spawnentry e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.spawngroupID));
		v.push_back(std::to_string(e.npcID));
		v.push_back(std::to_string(e.chance));
		v.push_back(std::to_string(e.condition_value_filter));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.spawngroupID = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Spawnentry> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.spawngroupID));
			v.push_back(std::to_string(e.npcID));
			v.push_back(std::to_string(e.chance));
			v.push_back(std::to_string(e.condition_value_filter));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static std::vector<Spawnentry> All(Database& db)
	{
		std::vector<Spawnentry> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawnentry e{};

			e.spawngroupID           = static_cast<int32_t>(atoi(row[0]));
			e.npcID                  = static_cast<int32_t>(atoi(row[1]));
			e.chance                 = static_cast<int16_t>(atoi(row[2]));
			e.condition_value_filter = static_cast<int32_t>(atoi(row[3]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[4]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[5]));
			e.content_flags          = row[6] ? row[6] : "";
			e.content_flags_disabled = row[7] ? row[7] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Spawnentry> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Spawnentry> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawnentry e{};

			e.spawngroupID           = static_cast<int32_t>(atoi(row[0]));
			e.npcID                  = static_cast<int32_t>(atoi(row[1]));
			e.chance                 = static_cast<int16_t>(atoi(row[2]));
			e.condition_value_filter = static_cast<int32_t>(atoi(row[3]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[4]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[5]));
			e.content_flags          = row[6] ? row[6] : "";
			e.content_flags_disabled = row[7] ? row[7] : "";

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

#endif //EQEMU_BASE_SPAWNENTRY_REPOSITORY_H
