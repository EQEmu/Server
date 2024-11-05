/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_GLOBAL_BUFFS_REPOSITORY_H
#define EQEMU_BASE_GLOBAL_BUFFS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseGlobalBuffsRepository {
public:
	struct GlobalBuffs {
		int32_t spell_id;
		int32_t duration;
	};

	static std::string PrimaryKey()
	{
		return std::string("spell_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"spell_id",
			"duration",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"spell_id",
			"duration",
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
		return std::string("global_buffs");
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

	static GlobalBuffs NewEntity()
	{
		GlobalBuffs e{};

		e.spell_id = 0;
		e.duration = 0;

		return e;
	}

	static GlobalBuffs GetGlobalBuffs(
		const std::vector<GlobalBuffs> &global_buffss,
		int global_buffs_id
	)
	{
		for (auto &global_buffs : global_buffss) {
			if (global_buffs.spell_id == global_buffs_id) {
				return global_buffs;
			}
		}

		return NewEntity();
	}

	static GlobalBuffs FindOne(
		Database& db,
		int global_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				global_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GlobalBuffs e{};

			e.spell_id = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.duration = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int global_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				global_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GlobalBuffs &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.spell_id));
		v.push_back(columns[1] + " = " + std::to_string(e.duration));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.spell_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GlobalBuffs InsertOne(
		Database& db,
		GlobalBuffs e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.duration));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.spell_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<GlobalBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.duration));

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

	static std::vector<GlobalBuffs> All(Database& db)
	{
		std::vector<GlobalBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GlobalBuffs e{};

			e.spell_id = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.duration = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GlobalBuffs> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GlobalBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GlobalBuffs e{};

			e.spell_id = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.duration = row[1] ? static_cast<int32_t>(atoi(row[1])) : 0;

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

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const GlobalBuffs &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.spell_id));
		v.push_back(std::to_string(e.duration));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<GlobalBuffs> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.spell_id));
			v.push_back(std::to_string(e.duration));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_BASE_GLOBAL_BUFFS_REPOSITORY_H
