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
#include "../../string_util.h"

class BaseAaRankEffectsRepository {
public:
	struct AaRankEffects {
		int rank_id;
		int slot;
		int effect_id;
		int base1;
		int base2;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("aa_rank_effects");
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
			ColumnsRaw()
		);
	}

	static AaRankEffects NewEntity()
	{
		AaRankEffects entry{};

		entry.rank_id   = 0;
		entry.slot      = 1;
		entry.effect_id = 0;
		entry.base1     = 0;
		entry.base2     = 0;

		return entry;
	}

	static AaRankEffects GetAaRankEffectsEntry(
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
			AaRankEffects entry{};

			entry.rank_id   = atoi(row[0]);
			entry.slot      = atoi(row[1]);
			entry.effect_id = atoi(row[2]);
			entry.base1     = atoi(row[3]);
			entry.base2     = atoi(row[4]);

			return entry;
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
		AaRankEffects aa_rank_effects_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(aa_rank_effects_entry.rank_id));
		update_values.push_back(columns[1] + " = " + std::to_string(aa_rank_effects_entry.slot));
		update_values.push_back(columns[2] + " = " + std::to_string(aa_rank_effects_entry.effect_id));
		update_values.push_back(columns[3] + " = " + std::to_string(aa_rank_effects_entry.base1));
		update_values.push_back(columns[4] + " = " + std::to_string(aa_rank_effects_entry.base2));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				aa_rank_effects_entry.rank_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AaRankEffects InsertOne(
		Database& db,
		AaRankEffects aa_rank_effects_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(aa_rank_effects_entry.rank_id));
		insert_values.push_back(std::to_string(aa_rank_effects_entry.slot));
		insert_values.push_back(std::to_string(aa_rank_effects_entry.effect_id));
		insert_values.push_back(std::to_string(aa_rank_effects_entry.base1));
		insert_values.push_back(std::to_string(aa_rank_effects_entry.base2));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			aa_rank_effects_entry.rank_id = results.LastInsertedID();
			return aa_rank_effects_entry;
		}

		aa_rank_effects_entry = NewEntity();

		return aa_rank_effects_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AaRankEffects> aa_rank_effects_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &aa_rank_effects_entry: aa_rank_effects_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(aa_rank_effects_entry.rank_id));
			insert_values.push_back(std::to_string(aa_rank_effects_entry.slot));
			insert_values.push_back(std::to_string(aa_rank_effects_entry.effect_id));
			insert_values.push_back(std::to_string(aa_rank_effects_entry.base1));
			insert_values.push_back(std::to_string(aa_rank_effects_entry.base2));

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
			AaRankEffects entry{};

			entry.rank_id   = atoi(row[0]);
			entry.slot      = atoi(row[1]);
			entry.effect_id = atoi(row[2]);
			entry.base1     = atoi(row[3]);
			entry.base2     = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AaRankEffects> GetWhere(Database& db, std::string where_filter)
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
			AaRankEffects entry{};

			entry.rank_id   = atoi(row[0]);
			entry.slot      = atoi(row[1]);
			entry.effect_id = atoi(row[2]);
			entry.base1     = atoi(row[3]);
			entry.base2     = atoi(row[4]);

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

#endif //EQEMU_BASE_AA_RANK_EFFECTS_REPOSITORY_H
