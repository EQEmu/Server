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

#ifndef EQEMU_BASE_AA_RANKS_REPOSITORY_H
#define EQEMU_BASE_AA_RANKS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAaRanksRepository {
public:
	struct AaRanks {
		int id;
		int upper_hotkey_sid;
		int lower_hotkey_sid;
		int title_sid;
		int desc_sid;
		int cost;
		int level_req;
		int spell;
		int spell_type;
		int recast_time;
		int expansion;
		int prev_id;
		int next_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"upper_hotkey_sid",
			"lower_hotkey_sid",
			"title_sid",
			"desc_sid",
			"cost",
			"level_req",
			"spell",
			"spell_type",
			"recast_time",
			"expansion",
			"prev_id",
			"next_id",
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
		return std::string("aa_ranks");
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

	static AaRanks NewEntity()
	{
		AaRanks entry{};

		entry.id               = 0;
		entry.upper_hotkey_sid = -1;
		entry.lower_hotkey_sid = -1;
		entry.title_sid        = -1;
		entry.desc_sid         = -1;
		entry.cost             = 1;
		entry.level_req        = 51;
		entry.spell            = -1;
		entry.spell_type       = 0;
		entry.recast_time      = 0;
		entry.expansion        = 0;
		entry.prev_id          = -1;
		entry.next_id          = -1;

		return entry;
	}

	static AaRanks GetAaRanksEntry(
		const std::vector<AaRanks> &aa_rankss,
		int aa_ranks_id
	)
	{
		for (auto &aa_ranks : aa_rankss) {
			if (aa_ranks.id == aa_ranks_id) {
				return aa_ranks;
			}
		}

		return NewEntity();
	}

	static AaRanks FindOne(
		int aa_ranks_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				aa_ranks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AaRanks entry{};

			entry.id               = atoi(row[0]);
			entry.upper_hotkey_sid = atoi(row[1]);
			entry.lower_hotkey_sid = atoi(row[2]);
			entry.title_sid        = atoi(row[3]);
			entry.desc_sid         = atoi(row[4]);
			entry.cost             = atoi(row[5]);
			entry.level_req        = atoi(row[6]);
			entry.spell            = atoi(row[7]);
			entry.spell_type       = atoi(row[8]);
			entry.recast_time      = atoi(row[9]);
			entry.expansion        = atoi(row[10]);
			entry.prev_id          = atoi(row[11]);
			entry.next_id          = atoi(row[12]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int aa_ranks_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				aa_ranks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		AaRanks aa_ranks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(aa_ranks_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(aa_ranks_entry.upper_hotkey_sid));
		update_values.push_back(columns[2] + " = " + std::to_string(aa_ranks_entry.lower_hotkey_sid));
		update_values.push_back(columns[3] + " = " + std::to_string(aa_ranks_entry.title_sid));
		update_values.push_back(columns[4] + " = " + std::to_string(aa_ranks_entry.desc_sid));
		update_values.push_back(columns[5] + " = " + std::to_string(aa_ranks_entry.cost));
		update_values.push_back(columns[6] + " = " + std::to_string(aa_ranks_entry.level_req));
		update_values.push_back(columns[7] + " = " + std::to_string(aa_ranks_entry.spell));
		update_values.push_back(columns[8] + " = " + std::to_string(aa_ranks_entry.spell_type));
		update_values.push_back(columns[9] + " = " + std::to_string(aa_ranks_entry.recast_time));
		update_values.push_back(columns[10] + " = " + std::to_string(aa_ranks_entry.expansion));
		update_values.push_back(columns[11] + " = " + std::to_string(aa_ranks_entry.prev_id));
		update_values.push_back(columns[12] + " = " + std::to_string(aa_ranks_entry.next_id));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				aa_ranks_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AaRanks InsertOne(
		AaRanks aa_ranks_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(aa_ranks_entry.id));
		insert_values.push_back(std::to_string(aa_ranks_entry.upper_hotkey_sid));
		insert_values.push_back(std::to_string(aa_ranks_entry.lower_hotkey_sid));
		insert_values.push_back(std::to_string(aa_ranks_entry.title_sid));
		insert_values.push_back(std::to_string(aa_ranks_entry.desc_sid));
		insert_values.push_back(std::to_string(aa_ranks_entry.cost));
		insert_values.push_back(std::to_string(aa_ranks_entry.level_req));
		insert_values.push_back(std::to_string(aa_ranks_entry.spell));
		insert_values.push_back(std::to_string(aa_ranks_entry.spell_type));
		insert_values.push_back(std::to_string(aa_ranks_entry.recast_time));
		insert_values.push_back(std::to_string(aa_ranks_entry.expansion));
		insert_values.push_back(std::to_string(aa_ranks_entry.prev_id));
		insert_values.push_back(std::to_string(aa_ranks_entry.next_id));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			aa_ranks_entry.id = results.LastInsertedID();
			return aa_ranks_entry;
		}

		aa_ranks_entry = NewEntity();

		return aa_ranks_entry;
	}

	static int InsertMany(
		std::vector<AaRanks> aa_ranks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &aa_ranks_entry: aa_ranks_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(aa_ranks_entry.id));
			insert_values.push_back(std::to_string(aa_ranks_entry.upper_hotkey_sid));
			insert_values.push_back(std::to_string(aa_ranks_entry.lower_hotkey_sid));
			insert_values.push_back(std::to_string(aa_ranks_entry.title_sid));
			insert_values.push_back(std::to_string(aa_ranks_entry.desc_sid));
			insert_values.push_back(std::to_string(aa_ranks_entry.cost));
			insert_values.push_back(std::to_string(aa_ranks_entry.level_req));
			insert_values.push_back(std::to_string(aa_ranks_entry.spell));
			insert_values.push_back(std::to_string(aa_ranks_entry.spell_type));
			insert_values.push_back(std::to_string(aa_ranks_entry.recast_time));
			insert_values.push_back(std::to_string(aa_ranks_entry.expansion));
			insert_values.push_back(std::to_string(aa_ranks_entry.prev_id));
			insert_values.push_back(std::to_string(aa_ranks_entry.next_id));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<AaRanks> All()
	{
		std::vector<AaRanks> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaRanks entry{};

			entry.id               = atoi(row[0]);
			entry.upper_hotkey_sid = atoi(row[1]);
			entry.lower_hotkey_sid = atoi(row[2]);
			entry.title_sid        = atoi(row[3]);
			entry.desc_sid         = atoi(row[4]);
			entry.cost             = atoi(row[5]);
			entry.level_req        = atoi(row[6]);
			entry.spell            = atoi(row[7]);
			entry.spell_type       = atoi(row[8]);
			entry.recast_time      = atoi(row[9]);
			entry.expansion        = atoi(row[10]);
			entry.prev_id          = atoi(row[11]);
			entry.next_id          = atoi(row[12]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AaRanks> GetWhere(std::string where_filter)
	{
		std::vector<AaRanks> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AaRanks entry{};

			entry.id               = atoi(row[0]);
			entry.upper_hotkey_sid = atoi(row[1]);
			entry.lower_hotkey_sid = atoi(row[2]);
			entry.title_sid        = atoi(row[3]);
			entry.desc_sid         = atoi(row[4]);
			entry.cost             = atoi(row[5]);
			entry.level_req        = atoi(row[6]);
			entry.spell            = atoi(row[7]);
			entry.spell_type       = atoi(row[8]);
			entry.recast_time      = atoi(row[9]);
			entry.expansion        = atoi(row[10]);
			entry.prev_id          = atoi(row[11]);
			entry.next_id          = atoi(row[12]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_AA_RANKS_REPOSITORY_H
