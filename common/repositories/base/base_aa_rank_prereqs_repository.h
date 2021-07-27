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

#ifndef EQEMU_BASE_AA_RANK_PREREQS_REPOSITORY_H
#define EQEMU_BASE_AA_RANK_PREREQS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAaRankPrereqsRepository {
public:
	struct AaRankPrereqs {
		int rank_id;
		int aa_id;
		int points;
	};

	static std::string PrimaryKey()
	{
		return std::string("rank_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"rank_id",
			"aa_id",
			"points",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("aa_rank_prereqs");
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

	static AaRankPrereqs NewEntity()
	{
		AaRankPrereqs entry{};

		entry.rank_id = 0;
		entry.aa_id   = 0;
		entry.points  = 0;

		return entry;
	}

	static AaRankPrereqs GetAaRankPrereqsEntry(
		const std::vector<AaRankPrereqs> &aa_rank_prereqss,
		int aa_rank_prereqs_id
	)
	{
		for (auto &aa_rank_prereqs : aa_rank_prereqss) {
			if (aa_rank_prereqs.rank_id == aa_rank_prereqs_id) {
				return aa_rank_prereqs;
			}
		}

		return NewEntity();
	}

	static AaRankPrereqs FindOne(
		Database& db,
		int aa_rank_prereqs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				aa_rank_prereqs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AaRankPrereqs entry{};

			entry.rank_id = atoi(row[0]);
			entry.aa_id   = atoi(row[1]);
			entry.points  = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int aa_rank_prereqs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				aa_rank_prereqs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		AaRankPrereqs aa_rank_prereqs_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(aa_rank_prereqs_entry.rank_id));
		update_values.push_back(columns[1] + " = " + std::to_string(aa_rank_prereqs_entry.aa_id));
		update_values.push_back(columns[2] + " = " + std::to_string(aa_rank_prereqs_entry.points));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				aa_rank_prereqs_entry.rank_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AaRankPrereqs InsertOne(
		Database& db,
		AaRankPrereqs aa_rank_prereqs_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(aa_rank_prereqs_entry.rank_id));
		insert_values.push_back(std::to_string(aa_rank_prereqs_entry.aa_id));
		insert_values.push_back(std::to_string(aa_rank_prereqs_entry.points));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			aa_rank_prereqs_entry.rank_id = results.LastInsertedID();
			return aa_rank_prereqs_entry;
		}

		aa_rank_prereqs_entry = NewEntity();

		return aa_rank_prereqs_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AaRankPrereqs> aa_rank_prereqs_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &aa_rank_prereqs_entry: aa_rank_prereqs_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(aa_rank_prereqs_entry.rank_id));
			insert_values.push_back(std::to_string(aa_rank_prereqs_entry.aa_id));
			insert_values.push_back(std::to_string(aa_rank_prereqs_entry.points));

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

	static std::vector<AaRankPrereqs> All(Database& db)
	{
		std::vector<AaRankPrereqs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaRankPrereqs entry{};

			entry.rank_id = atoi(row[0]);
			entry.aa_id   = atoi(row[1]);
			entry.points  = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AaRankPrereqs> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<AaRankPrereqs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaRankPrereqs entry{};

			entry.rank_id = atoi(row[0]);
			entry.aa_id   = atoi(row[1]);
			entry.points  = atoi(row[2]);

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

#endif //EQEMU_BASE_AA_RANK_PREREQS_REPOSITORY_H
