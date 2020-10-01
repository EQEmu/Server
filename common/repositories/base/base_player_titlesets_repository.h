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

#ifndef EQEMU_BASE_PLAYER_TITLESETS_REPOSITORY_H
#define EQEMU_BASE_PLAYER_TITLESETS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BasePlayerTitlesetsRepository {
public:
	struct PlayerTitlesets {
		int id;
		int char_id;
		int title_set;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"char_id",
			"title_set",
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
		return std::string("player_titlesets");
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

	static PlayerTitlesets NewEntity()
	{
		PlayerTitlesets entry{};

		entry.id        = 0;
		entry.char_id   = 0;
		entry.title_set = 0;

		return entry;
	}

	static PlayerTitlesets GetPlayerTitlesetsEntry(
		const std::vector<PlayerTitlesets> &player_titlesetss,
		int player_titlesets_id
	)
	{
		for (auto &player_titlesets : player_titlesetss) {
			if (player_titlesets.id == player_titlesets_id) {
				return player_titlesets;
			}
		}

		return NewEntity();
	}

	static PlayerTitlesets FindOne(
		int player_titlesets_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				player_titlesets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerTitlesets entry{};

			entry.id        = atoi(row[0]);
			entry.char_id   = atoi(row[1]);
			entry.title_set = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int player_titlesets_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_titlesets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		PlayerTitlesets player_titlesets_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(player_titlesets_entry.char_id));
		update_values.push_back(columns[2] + " = " + std::to_string(player_titlesets_entry.title_set));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				player_titlesets_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PlayerTitlesets InsertOne(
		PlayerTitlesets player_titlesets_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(player_titlesets_entry.char_id));
		insert_values.push_back(std::to_string(player_titlesets_entry.title_set));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			player_titlesets_entry.id = results.LastInsertedID();
			return player_titlesets_entry;
		}

		player_titlesets_entry = NewEntity();

		return player_titlesets_entry;
	}

	static int InsertMany(
		std::vector<PlayerTitlesets> player_titlesets_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &player_titlesets_entry: player_titlesets_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(player_titlesets_entry.char_id));
			insert_values.push_back(std::to_string(player_titlesets_entry.title_set));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<PlayerTitlesets> All()
	{
		std::vector<PlayerTitlesets> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerTitlesets entry{};

			entry.id        = atoi(row[0]);
			entry.char_id   = atoi(row[1]);
			entry.title_set = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<PlayerTitlesets> GetWhere(std::string where_filter)
	{
		std::vector<PlayerTitlesets> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerTitlesets entry{};

			entry.id        = atoi(row[0]);
			entry.char_id   = atoi(row[1]);
			entry.title_set = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
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
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_PLAYER_TITLESETS_REPOSITORY_H
