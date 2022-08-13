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

#ifndef EQEMU_BASE_ADVENTURE_STATS_REPOSITORY_H
#define EQEMU_BASE_ADVENTURE_STATS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAdventureStatsRepository {
public:
	struct AdventureStats {
		int player_id;
		int guk_wins;
		int mir_wins;
		int mmc_wins;
		int ruj_wins;
		int tak_wins;
		int guk_losses;
		int mir_losses;
		int mmc_losses;
		int ruj_losses;
		int tak_losses;
	};

	static std::string PrimaryKey()
	{
		return std::string("player_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"player_id",
			"guk_wins",
			"mir_wins",
			"mmc_wins",
			"ruj_wins",
			"tak_wins",
			"guk_losses",
			"mir_losses",
			"mmc_losses",
			"ruj_losses",
			"tak_losses",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"player_id",
			"guk_wins",
			"mir_wins",
			"mmc_wins",
			"ruj_wins",
			"tak_wins",
			"guk_losses",
			"mir_losses",
			"mmc_losses",
			"ruj_losses",
			"tak_losses",
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
		return std::string("adventure_stats");
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

	static AdventureStats NewEntity()
	{
		AdventureStats e{};

		e.player_id  = 0;
		e.guk_wins   = 0;
		e.mir_wins   = 0;
		e.mmc_wins   = 0;
		e.ruj_wins   = 0;
		e.tak_wins   = 0;
		e.guk_losses = 0;
		e.mir_losses = 0;
		e.mmc_losses = 0;
		e.ruj_losses = 0;
		e.tak_losses = 0;

		return e;
	}

	static AdventureStats GetAdventureStats(
		const std::vector<AdventureStats> &adventure_statss,
		int adventure_stats_id
	)
	{
		for (auto &adventure_stats : adventure_statss) {
			if (adventure_stats.player_id == adventure_stats_id) {
				return adventure_stats;
			}
		}

		return NewEntity();
	}

	static AdventureStats FindOne(
		Database& db,
		int adventure_stats_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				adventure_stats_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureStats e{};

			e.player_id  = atoi(row[0]);
			e.guk_wins   = atoi(row[1]);
			e.mir_wins   = atoi(row[2]);
			e.mmc_wins   = atoi(row[3]);
			e.ruj_wins   = atoi(row[4]);
			e.tak_wins   = atoi(row[5]);
			e.guk_losses = atoi(row[6]);
			e.mir_losses = atoi(row[7]);
			e.mmc_losses = atoi(row[8]);
			e.ruj_losses = atoi(row[9]);
			e.tak_losses = atoi(row[10]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int adventure_stats_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_stats_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AdventureStats &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.player_id));
		v.push_back(columns[1] + " = " + std::to_string(e.guk_wins));
		v.push_back(columns[2] + " = " + std::to_string(e.mir_wins));
		v.push_back(columns[3] + " = " + std::to_string(e.mmc_wins));
		v.push_back(columns[4] + " = " + std::to_string(e.ruj_wins));
		v.push_back(columns[5] + " = " + std::to_string(e.tak_wins));
		v.push_back(columns[6] + " = " + std::to_string(e.guk_losses));
		v.push_back(columns[7] + " = " + std::to_string(e.mir_losses));
		v.push_back(columns[8] + " = " + std::to_string(e.mmc_losses));
		v.push_back(columns[9] + " = " + std::to_string(e.ruj_losses));
		v.push_back(columns[10] + " = " + std::to_string(e.tak_losses));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.player_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureStats InsertOne(
		Database& db,
		AdventureStats e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.player_id));
		v.push_back(std::to_string(e.guk_wins));
		v.push_back(std::to_string(e.mir_wins));
		v.push_back(std::to_string(e.mmc_wins));
		v.push_back(std::to_string(e.ruj_wins));
		v.push_back(std::to_string(e.tak_wins));
		v.push_back(std::to_string(e.guk_losses));
		v.push_back(std::to_string(e.mir_losses));
		v.push_back(std::to_string(e.mmc_losses));
		v.push_back(std::to_string(e.ruj_losses));
		v.push_back(std::to_string(e.tak_losses));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.player_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<AdventureStats> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.player_id));
			v.push_back(std::to_string(e.guk_wins));
			v.push_back(std::to_string(e.mir_wins));
			v.push_back(std::to_string(e.mmc_wins));
			v.push_back(std::to_string(e.ruj_wins));
			v.push_back(std::to_string(e.tak_wins));
			v.push_back(std::to_string(e.guk_losses));
			v.push_back(std::to_string(e.mir_losses));
			v.push_back(std::to_string(e.mmc_losses));
			v.push_back(std::to_string(e.ruj_losses));
			v.push_back(std::to_string(e.tak_losses));

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

	static std::vector<AdventureStats> All(Database& db)
	{
		std::vector<AdventureStats> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureStats e{};

			e.player_id  = atoi(row[0]);
			e.guk_wins   = atoi(row[1]);
			e.mir_wins   = atoi(row[2]);
			e.mmc_wins   = atoi(row[3]);
			e.ruj_wins   = atoi(row[4]);
			e.tak_wins   = atoi(row[5]);
			e.guk_losses = atoi(row[6]);
			e.mir_losses = atoi(row[7]);
			e.mmc_losses = atoi(row[8]);
			e.ruj_losses = atoi(row[9]);
			e.tak_losses = atoi(row[10]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AdventureStats> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AdventureStats> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureStats e{};

			e.player_id  = atoi(row[0]);
			e.guk_wins   = atoi(row[1]);
			e.mir_wins   = atoi(row[2]);
			e.mmc_wins   = atoi(row[3]);
			e.ruj_wins   = atoi(row[4]);
			e.tak_wins   = atoi(row[5]);
			e.guk_losses = atoi(row[6]);
			e.mir_losses = atoi(row[7]);
			e.mmc_losses = atoi(row[8]);
			e.ruj_losses = atoi(row[9]);
			e.tak_losses = atoi(row[10]);

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

#endif //EQEMU_BASE_ADVENTURE_STATS_REPOSITORY_H
