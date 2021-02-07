/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_ADVENTURE_STATS_REPOSITORY_H
#define EQEMU_BASE_ADVENTURE_STATS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("adventure_stats");
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
			InsertColumnsRaw()
		);
	}

	static AdventureStats NewEntity()
	{
		AdventureStats entry{};

		entry.player_id  = 0;
		entry.guk_wins   = 0;
		entry.mir_wins   = 0;
		entry.mmc_wins   = 0;
		entry.ruj_wins   = 0;
		entry.tak_wins   = 0;
		entry.guk_losses = 0;
		entry.mir_losses = 0;
		entry.mmc_losses = 0;
		entry.ruj_losses = 0;
		entry.tak_losses = 0;

		return entry;
	}

	static AdventureStats GetAdventureStatsEntry(
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
			AdventureStats entry{};

			entry.player_id  = atoi(row[0]);
			entry.guk_wins   = atoi(row[1]);
			entry.mir_wins   = atoi(row[2]);
			entry.mmc_wins   = atoi(row[3]);
			entry.ruj_wins   = atoi(row[4]);
			entry.tak_wins   = atoi(row[5]);
			entry.guk_losses = atoi(row[6]);
			entry.mir_losses = atoi(row[7]);
			entry.mmc_losses = atoi(row[8]);
			entry.ruj_losses = atoi(row[9]);
			entry.tak_losses = atoi(row[10]);

			return entry;
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
		AdventureStats adventure_stats_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(adventure_stats_entry.player_id));
		update_values.push_back(columns[1] + " = " + std::to_string(adventure_stats_entry.guk_wins));
		update_values.push_back(columns[2] + " = " + std::to_string(adventure_stats_entry.mir_wins));
		update_values.push_back(columns[3] + " = " + std::to_string(adventure_stats_entry.mmc_wins));
		update_values.push_back(columns[4] + " = " + std::to_string(adventure_stats_entry.ruj_wins));
		update_values.push_back(columns[5] + " = " + std::to_string(adventure_stats_entry.tak_wins));
		update_values.push_back(columns[6] + " = " + std::to_string(adventure_stats_entry.guk_losses));
		update_values.push_back(columns[7] + " = " + std::to_string(adventure_stats_entry.mir_losses));
		update_values.push_back(columns[8] + " = " + std::to_string(adventure_stats_entry.mmc_losses));
		update_values.push_back(columns[9] + " = " + std::to_string(adventure_stats_entry.ruj_losses));
		update_values.push_back(columns[10] + " = " + std::to_string(adventure_stats_entry.tak_losses));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				adventure_stats_entry.player_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureStats InsertOne(
		Database& db,
		AdventureStats adventure_stats_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(adventure_stats_entry.player_id));
		insert_values.push_back(std::to_string(adventure_stats_entry.guk_wins));
		insert_values.push_back(std::to_string(adventure_stats_entry.mir_wins));
		insert_values.push_back(std::to_string(adventure_stats_entry.mmc_wins));
		insert_values.push_back(std::to_string(adventure_stats_entry.ruj_wins));
		insert_values.push_back(std::to_string(adventure_stats_entry.tak_wins));
		insert_values.push_back(std::to_string(adventure_stats_entry.guk_losses));
		insert_values.push_back(std::to_string(adventure_stats_entry.mir_losses));
		insert_values.push_back(std::to_string(adventure_stats_entry.mmc_losses));
		insert_values.push_back(std::to_string(adventure_stats_entry.ruj_losses));
		insert_values.push_back(std::to_string(adventure_stats_entry.tak_losses));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			adventure_stats_entry.player_id = results.LastInsertedID();
			return adventure_stats_entry;
		}

		adventure_stats_entry = NewEntity();

		return adventure_stats_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AdventureStats> adventure_stats_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &adventure_stats_entry: adventure_stats_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(adventure_stats_entry.player_id));
			insert_values.push_back(std::to_string(adventure_stats_entry.guk_wins));
			insert_values.push_back(std::to_string(adventure_stats_entry.mir_wins));
			insert_values.push_back(std::to_string(adventure_stats_entry.mmc_wins));
			insert_values.push_back(std::to_string(adventure_stats_entry.ruj_wins));
			insert_values.push_back(std::to_string(adventure_stats_entry.tak_wins));
			insert_values.push_back(std::to_string(adventure_stats_entry.guk_losses));
			insert_values.push_back(std::to_string(adventure_stats_entry.mir_losses));
			insert_values.push_back(std::to_string(adventure_stats_entry.mmc_losses));
			insert_values.push_back(std::to_string(adventure_stats_entry.ruj_losses));
			insert_values.push_back(std::to_string(adventure_stats_entry.tak_losses));

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
			AdventureStats entry{};

			entry.player_id  = atoi(row[0]);
			entry.guk_wins   = atoi(row[1]);
			entry.mir_wins   = atoi(row[2]);
			entry.mmc_wins   = atoi(row[3]);
			entry.ruj_wins   = atoi(row[4]);
			entry.tak_wins   = atoi(row[5]);
			entry.guk_losses = atoi(row[6]);
			entry.mir_losses = atoi(row[7]);
			entry.mmc_losses = atoi(row[8]);
			entry.ruj_losses = atoi(row[9]);
			entry.tak_losses = atoi(row[10]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AdventureStats> GetWhere(Database& db, std::string where_filter)
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
			AdventureStats entry{};

			entry.player_id  = atoi(row[0]);
			entry.guk_wins   = atoi(row[1]);
			entry.mir_wins   = atoi(row[2]);
			entry.mmc_wins   = atoi(row[3]);
			entry.ruj_wins   = atoi(row[4]);
			entry.tak_wins   = atoi(row[5]);
			entry.guk_losses = atoi(row[6]);
			entry.mir_losses = atoi(row[7]);
			entry.mmc_losses = atoi(row[8]);
			entry.ruj_losses = atoi(row[9]);
			entry.tak_losses = atoi(row[10]);

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

#endif //EQEMU_BASE_ADVENTURE_STATS_REPOSITORY_H
