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

#ifndef EQEMU_BASE_CHAR_CREATE_COMBINATIONS_REPOSITORY_H
#define EQEMU_BASE_CHAR_CREATE_COMBINATIONS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharCreateCombinationsRepository {
public:
	struct CharCreateCombinations {
		int allocation_id;
		int race;
		int class_;
		int deity;
		int start_zone;
		int expansions_req;
	};

	static std::string PrimaryKey()
	{
		return std::string("race");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"allocation_id",
			"race",
			"`class`",
			"deity",
			"start_zone",
			"expansions_req",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"allocation_id",
			"race",
			"`class`",
			"deity",
			"start_zone",
			"expansions_req",
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
		return std::string("char_create_combinations");
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

	static CharCreateCombinations NewEntity()
	{
		CharCreateCombinations e{};

		e.allocation_id  = 0;
		e.race           = 0;
		e.class_         = 0;
		e.deity          = 0;
		e.start_zone     = 0;
		e.expansions_req = 0;

		return e;
	}

	static CharCreateCombinations GetCharCreateCombinations(
		const std::vector<CharCreateCombinations> &char_create_combinationss,
		int char_create_combinations_id
	)
	{
		for (auto &char_create_combinations : char_create_combinationss) {
			if (char_create_combinations.race == char_create_combinations_id) {
				return char_create_combinations;
			}
		}

		return NewEntity();
	}

	static CharCreateCombinations FindOne(
		Database& db,
		int char_create_combinations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				char_create_combinations_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharCreateCombinations e{};

			e.allocation_id  = atoi(row[0]);
			e.race           = atoi(row[1]);
			e.class_         = atoi(row[2]);
			e.deity          = atoi(row[3]);
			e.start_zone     = atoi(row[4]);
			e.expansions_req = atoi(row[5]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int char_create_combinations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				char_create_combinations_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharCreateCombinations &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.allocation_id));
		v.push_back(columns[1] + " = " + std::to_string(e.race));
		v.push_back(columns[2] + " = " + std::to_string(e.class_));
		v.push_back(columns[3] + " = " + std::to_string(e.deity));
		v.push_back(columns[4] + " = " + std::to_string(e.start_zone));
		v.push_back(columns[5] + " = " + std::to_string(e.expansions_req));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.race
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharCreateCombinations InsertOne(
		Database& db,
		CharCreateCombinations e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.allocation_id));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.deity));
		v.push_back(std::to_string(e.start_zone));
		v.push_back(std::to_string(e.expansions_req));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.race = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharCreateCombinations> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.allocation_id));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.deity));
			v.push_back(std::to_string(e.start_zone));
			v.push_back(std::to_string(e.expansions_req));

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

	static std::vector<CharCreateCombinations> All(Database& db)
	{
		std::vector<CharCreateCombinations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharCreateCombinations e{};

			e.allocation_id  = atoi(row[0]);
			e.race           = atoi(row[1]);
			e.class_         = atoi(row[2]);
			e.deity          = atoi(row[3]);
			e.start_zone     = atoi(row[4]);
			e.expansions_req = atoi(row[5]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharCreateCombinations> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharCreateCombinations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharCreateCombinations e{};

			e.allocation_id  = atoi(row[0]);
			e.race           = atoi(row[1]);
			e.class_         = atoi(row[2]);
			e.deity          = atoi(row[3]);
			e.start_zone     = atoi(row[4]);
			e.expansions_req = atoi(row[5]);

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

#endif //EQEMU_BASE_CHAR_CREATE_COMBINATIONS_REPOSITORY_H
