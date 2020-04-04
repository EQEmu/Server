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
 */

#ifndef EQEMU_INSTANCE_LIST_PLAYER_REPOSITORY_H
#define EQEMU_INSTANCE_LIST_PLAYER_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class InstanceListPlayerRepository {
public:
	struct InstanceListPlayer {
		int id;
		int charid;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"charid",
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
		return std::string("instance_list_player");
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

	static InstanceListPlayer NewEntity()
	{
		InstanceListPlayer entry{};

		entry.id     = 0;
		entry.charid = 0;

		return entry;
	}

	static InstanceListPlayer GetInstanceListPlayerEntry(
		const std::vector<InstanceListPlayer> &instance_list_players,
		int instance_list_player_id
	)
	{
		for (auto &instance_list_player : instance_list_players) {
			if (instance_list_player.charid == instance_list_player_id) {
				return instance_list_player;
			}
		}

		return NewEntity();
	}

	static InstanceListPlayer FindOne(
		int instance_list_player_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				instance_list_player_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			InstanceListPlayer entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int instance_list_player_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				instance_list_player_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		InstanceListPlayer instance_list_player_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();



		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				instance_list_player_entry.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static InstanceListPlayer InsertOne(
		InstanceListPlayer instance_list_player_entry
	)
	{
		std::vector<std::string> insert_values;



		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			instance_list_player_entry.id = results.LastInsertedID();
			return instance_list_player_entry;
		}

		instance_list_player_entry = InstanceListPlayerRepository::NewEntity();

		return instance_list_player_entry;
	}

	static int InsertMany(
		std::vector<InstanceListPlayer> instance_list_player_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &instance_list_player_entry: instance_list_player_entries) {
			std::vector<std::string> insert_values;



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

	static std::vector<InstanceListPlayer> All()
	{
		std::vector<InstanceListPlayer> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstanceListPlayer entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<InstanceListPlayer> GetWhere(std::string where_filter)
	{
		std::vector<InstanceListPlayer> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstanceListPlayer entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

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
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_INSTANCE_LIST_PLAYER_REPOSITORY_H
