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

#ifndef EQEMU_BASE_AA_RANK_EFFECTS_REPOSITORY_H
#define EQEMU_BASE_AA_RANK_EFFECTS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAaRankEffectsRepository {
public:
	struct AaRankEffects {
		uint32_t rank_id;
		uint32_t slot;
		int32_t  effect_id;
		int32_t  base1;
		int32_t  base2;
	};

	static std::string PrimaryKey()
	{
		return std::string("rank_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"rank_id",
			"slot",
			"effect_id",
			"base1",
			"base2",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"rank_id",
			"slot",
			"effect_id",
			"base1",
			"base2",
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
		return std::string("aa_rank_effects");
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

	static AaRankEffects NewEntity()
	{
		AaRankEffects e{};

		e.rank_id   = 0;
		e.slot      = 1;
		e.effect_id = 0;
		e.base1     = 0;
		e.base2     = 0;

		return e;
	}

	static AaRankEffects GetAaRankEffects(
		const std::vector<AaRankEffects> &aa_rank_effectss,
		int aa_rank_effects_id
	)
	{
		for (auto &aa_rank_effects : aa_rank_effectss) {
			if (aa_rank_effects.rank_id == aa_rank_effects_id) {
				return aa_rank_effects;
			}
		}

		return NewEntity();
	}

	static AaRankEffects FindOne(
		Database& db,
		int aa_rank_effects_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				aa_rank_effects_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AaRankEffects e{};

			e.rank_id   = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.slot      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.effect_id = static_cast<int32_t>(atoi(row[2]));
			e.base1     = static_cast<int32_t>(atoi(row[3]));
			e.base2     = static_cast<int32_t>(atoi(row[4]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int aa_rank_effects_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				aa_rank_effects_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AaRankEffects &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.rank_id));
		v.push_back(columns[1] + " = " + std::to_string(e.slot));
		v.push_back(columns[2] + " = " + std::to_string(e.effect_id));
		v.push_back(columns[3] + " = " + std::to_string(e.base1));
		v.push_back(columns[4] + " = " + std::to_string(e.base2));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.rank_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AaRankEffects InsertOne(
		Database& db,
		AaRankEffects e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.rank_id));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.effect_id));
		v.push_back(std::to_string(e.base1));
		v.push_back(std::to_string(e.base2));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.rank_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<AaRankEffects> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.rank_id));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.effect_id));
			v.push_back(std::to_string(e.base1));
			v.push_back(std::to_string(e.base2));

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

	static std::vector<AaRankEffects> All(Database& db)
	{
		std::vector<AaRankEffects> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaRankEffects e{};

			e.rank_id   = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.slot      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.effect_id = static_cast<int32_t>(atoi(row[2]));
			e.base1     = static_cast<int32_t>(atoi(row[3]));
			e.base2     = static_cast<int32_t>(atoi(row[4]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AaRankEffects> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AaRankEffects> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaRankEffects e{};

			e.rank_id   = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.slot      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.effect_id = static_cast<int32_t>(atoi(row[2]));
			e.base1     = static_cast<int32_t>(atoi(row[3]));
			e.base2     = static_cast<int32_t>(atoi(row[4]));

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

#endif //EQEMU_BASE_AA_RANK_EFFECTS_REPOSITORY_H
