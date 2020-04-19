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

#ifndef EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H
#define EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseFactionBaseDataRepository {
public:
	struct FactionBaseData {
		int client_faction_id;
		int min;
		int max;
		int unk_hero1;
		int unk_hero2;
		int unk_hero3;
	};

	static std::string PrimaryKey()
	{
		return std::string("client_faction_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"client_faction_id",
			"min",
			"max",
			"unk_hero1",
			"unk_hero2",
			"unk_hero3",
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
		return std::string("faction_base_data");
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

	static FactionBaseData NewEntity()
	{
		FactionBaseData entry{};

		entry.client_faction_id = 0;
		entry.min               = -2000;
		entry.max               = 2000;
		entry.unk_hero1         = 0;
		entry.unk_hero2         = 0;
		entry.unk_hero3         = 0;

		return entry;
	}

	static FactionBaseData GetFactionBaseDataEntry(
		const std::vector<FactionBaseData> &faction_base_datas,
		int faction_base_data_id
	)
	{
		for (auto &faction_base_data : faction_base_datas) {
			if (faction_base_data.client_faction_id == faction_base_data_id) {
				return faction_base_data;
			}
		}

		return NewEntity();
	}

	static FactionBaseData FindOne(
		int faction_base_data_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				faction_base_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FactionBaseData entry{};

			entry.client_faction_id = atoi(row[0]);
			entry.min               = atoi(row[1]);
			entry.max               = atoi(row[2]);
			entry.unk_hero1         = atoi(row[3]);
			entry.unk_hero2         = atoi(row[4]);
			entry.unk_hero3         = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int faction_base_data_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				faction_base_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		FactionBaseData faction_base_data_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(faction_base_data_entry.client_faction_id));
		update_values.push_back(columns[1] + " = " + std::to_string(faction_base_data_entry.min));
		update_values.push_back(columns[2] + " = " + std::to_string(faction_base_data_entry.max));
		update_values.push_back(columns[3] + " = " + std::to_string(faction_base_data_entry.unk_hero1));
		update_values.push_back(columns[4] + " = " + std::to_string(faction_base_data_entry.unk_hero2));
		update_values.push_back(columns[5] + " = " + std::to_string(faction_base_data_entry.unk_hero3));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				faction_base_data_entry.client_faction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static FactionBaseData InsertOne(
		FactionBaseData faction_base_data_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(faction_base_data_entry.client_faction_id));
		insert_values.push_back(std::to_string(faction_base_data_entry.min));
		insert_values.push_back(std::to_string(faction_base_data_entry.max));
		insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero1));
		insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero2));
		insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero3));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			faction_base_data_entry.client_faction_id = results.LastInsertedID();
			return faction_base_data_entry;
		}

		faction_base_data_entry = NewEntity();

		return faction_base_data_entry;
	}

	static int InsertMany(
		std::vector<FactionBaseData> faction_base_data_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &faction_base_data_entry: faction_base_data_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(faction_base_data_entry.client_faction_id));
			insert_values.push_back(std::to_string(faction_base_data_entry.min));
			insert_values.push_back(std::to_string(faction_base_data_entry.max));
			insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero1));
			insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero2));
			insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero3));

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

	static std::vector<FactionBaseData> All()
	{
		std::vector<FactionBaseData> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionBaseData entry{};

			entry.client_faction_id = atoi(row[0]);
			entry.min               = atoi(row[1]);
			entry.max               = atoi(row[2]);
			entry.unk_hero1         = atoi(row[3]);
			entry.unk_hero2         = atoi(row[4]);
			entry.unk_hero3         = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<FactionBaseData> GetWhere(std::string where_filter)
	{
		std::vector<FactionBaseData> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionBaseData entry{};

			entry.client_faction_id = atoi(row[0]);
			entry.min               = atoi(row[1]);
			entry.max               = atoi(row[2]);
			entry.unk_hero1         = atoi(row[3]);
			entry.unk_hero2         = atoi(row[4]);
			entry.unk_hero3         = atoi(row[5]);

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

#endif //EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H
