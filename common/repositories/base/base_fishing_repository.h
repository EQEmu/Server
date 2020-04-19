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

#ifndef EQEMU_BASE_FISHING_REPOSITORY_H
#define EQEMU_BASE_FISHING_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseFishingRepository {
public:
	struct Fishing {
		int         id;
		int         zoneid;
		int         Itemid;
		int         skill_level;
		int         chance;
		int         npc_id;
		int         npc_chance;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zoneid",
			"Itemid",
			"skill_level",
			"chance",
			"npc_id",
			"npc_chance",
			"min_expansion",
			"max_expansion",
			"content_flags",
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
		return std::string("fishing");
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

	static Fishing NewEntity()
	{
		Fishing entry{};

		entry.id            = 0;
		entry.zoneid        = 0;
		entry.Itemid        = 0;
		entry.skill_level   = 0;
		entry.chance        = 0;
		entry.npc_id        = 0;
		entry.npc_chance    = 0;
		entry.min_expansion = 0;
		entry.max_expansion = 0;
		entry.content_flags = "";

		return entry;
	}

	static Fishing GetFishingEntry(
		const std::vector<Fishing> &fishings,
		int fishing_id
	)
	{
		for (auto &fishing : fishings) {
			if (fishing.id == fishing_id) {
				return fishing;
			}
		}

		return NewEntity();
	}

	static Fishing FindOne(
		int fishing_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				fishing_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Fishing entry{};

			entry.id            = atoi(row[0]);
			entry.zoneid        = atoi(row[1]);
			entry.Itemid        = atoi(row[2]);
			entry.skill_level   = atoi(row[3]);
			entry.chance        = atoi(row[4]);
			entry.npc_id        = atoi(row[5]);
			entry.npc_chance    = atoi(row[6]);
			entry.min_expansion = atoi(row[7]);
			entry.max_expansion = atoi(row[8]);
			entry.content_flags = row[9] ? row[9] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int fishing_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				fishing_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Fishing fishing_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(fishing_entry.zoneid));
		update_values.push_back(columns[2] + " = " + std::to_string(fishing_entry.Itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(fishing_entry.skill_level));
		update_values.push_back(columns[4] + " = " + std::to_string(fishing_entry.chance));
		update_values.push_back(columns[5] + " = " + std::to_string(fishing_entry.npc_id));
		update_values.push_back(columns[6] + " = " + std::to_string(fishing_entry.npc_chance));
		update_values.push_back(columns[7] + " = " + std::to_string(fishing_entry.min_expansion));
		update_values.push_back(columns[8] + " = " + std::to_string(fishing_entry.max_expansion));
		update_values.push_back(columns[9] + " = '" + EscapeString(fishing_entry.content_flags) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				fishing_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Fishing InsertOne(
		Fishing fishing_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(fishing_entry.zoneid));
		insert_values.push_back(std::to_string(fishing_entry.Itemid));
		insert_values.push_back(std::to_string(fishing_entry.skill_level));
		insert_values.push_back(std::to_string(fishing_entry.chance));
		insert_values.push_back(std::to_string(fishing_entry.npc_id));
		insert_values.push_back(std::to_string(fishing_entry.npc_chance));
		insert_values.push_back(std::to_string(fishing_entry.min_expansion));
		insert_values.push_back(std::to_string(fishing_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(fishing_entry.content_flags) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			fishing_entry.id = results.LastInsertedID();
			return fishing_entry;
		}

		fishing_entry = NewEntity();

		return fishing_entry;
	}

	static int InsertMany(
		std::vector<Fishing> fishing_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &fishing_entry: fishing_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(fishing_entry.zoneid));
			insert_values.push_back(std::to_string(fishing_entry.Itemid));
			insert_values.push_back(std::to_string(fishing_entry.skill_level));
			insert_values.push_back(std::to_string(fishing_entry.chance));
			insert_values.push_back(std::to_string(fishing_entry.npc_id));
			insert_values.push_back(std::to_string(fishing_entry.npc_chance));
			insert_values.push_back(std::to_string(fishing_entry.min_expansion));
			insert_values.push_back(std::to_string(fishing_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(fishing_entry.content_flags) + "'");

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

	static std::vector<Fishing> All()
	{
		std::vector<Fishing> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Fishing entry{};

			entry.id            = atoi(row[0]);
			entry.zoneid        = atoi(row[1]);
			entry.Itemid        = atoi(row[2]);
			entry.skill_level   = atoi(row[3]);
			entry.chance        = atoi(row[4]);
			entry.npc_id        = atoi(row[5]);
			entry.npc_chance    = atoi(row[6]);
			entry.min_expansion = atoi(row[7]);
			entry.max_expansion = atoi(row[8]);
			entry.content_flags = row[9] ? row[9] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Fishing> GetWhere(std::string where_filter)
	{
		std::vector<Fishing> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Fishing entry{};

			entry.id            = atoi(row[0]);
			entry.zoneid        = atoi(row[1]);
			entry.Itemid        = atoi(row[2]);
			entry.skill_level   = atoi(row[3]);
			entry.chance        = atoi(row[4]);
			entry.npc_id        = atoi(row[5]);
			entry.npc_chance    = atoi(row[6]);
			entry.min_expansion = atoi(row[7]);
			entry.max_expansion = atoi(row[8]);
			entry.content_flags = row[9] ? row[9] : "";

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

#endif //EQEMU_BASE_FISHING_REPOSITORY_H
