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

#ifndef EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H
#define EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseStartingItemsRepository {
public:
	struct StartingItems {
		int         id;
		int         race;
		int         class;
		int         deityid;
		int         zoneid;
		int         itemid;
		int         item_charges;
		int         gm;
		int         slot;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"race",
			"class",
			"deityid",
			"zoneid",
			"itemid",
			"item_charges",
			"gm",
			"slot",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("starting_items");
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

	static StartingItems NewEntity()
	{
		StartingItems entry{};

		entry.id                     = 0;
		entry.race                   = 0;
		entry.class                  = 0;
		entry.deityid                = 0;
		entry.zoneid                 = 0;
		entry.itemid                 = 0;
		entry.item_charges           = 1;
		entry.gm                     = 0;
		entry.slot                   = -1;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static StartingItems GetStartingItemsEntry(
		const std::vector<StartingItems> &starting_itemss,
		int starting_items_id
	)
	{
		for (auto &starting_items : starting_itemss) {
			if (starting_items.id == starting_items_id) {
				return starting_items;
			}
		}

		return NewEntity();
	}

	static StartingItems FindOne(
		int starting_items_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				starting_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			StartingItems entry{};

			entry.id                     = atoi(row[0]);
			entry.race                   = atoi(row[1]);
			entry.class                  = atoi(row[2]);
			entry.deityid                = atoi(row[3]);
			entry.zoneid                 = atoi(row[4]);
			entry.itemid                 = atoi(row[5]);
			entry.item_charges           = atoi(row[6]);
			entry.gm                     = atoi(row[7]);
			entry.slot                   = atoi(row[8]);
			entry.min_expansion          = atoi(row[9]);
			entry.max_expansion          = atoi(row[10]);
			entry.content_flags          = row[11] ? row[11] : "";
			entry.content_flags_disabled = row[12] ? row[12] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int starting_items_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				starting_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		StartingItems starting_items_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(starting_items_entry.race));
		update_values.push_back(columns[2] + " = " + std::to_string(starting_items_entry.class));
		update_values.push_back(columns[3] + " = " + std::to_string(starting_items_entry.deityid));
		update_values.push_back(columns[4] + " = " + std::to_string(starting_items_entry.zoneid));
		update_values.push_back(columns[5] + " = " + std::to_string(starting_items_entry.itemid));
		update_values.push_back(columns[6] + " = " + std::to_string(starting_items_entry.item_charges));
		update_values.push_back(columns[7] + " = " + std::to_string(starting_items_entry.gm));
		update_values.push_back(columns[8] + " = " + std::to_string(starting_items_entry.slot));
		update_values.push_back(columns[9] + " = " + std::to_string(starting_items_entry.min_expansion));
		update_values.push_back(columns[10] + " = " + std::to_string(starting_items_entry.max_expansion));
		update_values.push_back(columns[11] + " = '" + EscapeString(starting_items_entry.content_flags) + "'");
		update_values.push_back(columns[12] + " = '" + EscapeString(starting_items_entry.content_flags_disabled) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				starting_items_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static StartingItems InsertOne(
		StartingItems starting_items_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(starting_items_entry.race));
		insert_values.push_back(std::to_string(starting_items_entry.class));
		insert_values.push_back(std::to_string(starting_items_entry.deityid));
		insert_values.push_back(std::to_string(starting_items_entry.zoneid));
		insert_values.push_back(std::to_string(starting_items_entry.itemid));
		insert_values.push_back(std::to_string(starting_items_entry.item_charges));
		insert_values.push_back(std::to_string(starting_items_entry.gm));
		insert_values.push_back(std::to_string(starting_items_entry.slot));
		insert_values.push_back(std::to_string(starting_items_entry.min_expansion));
		insert_values.push_back(std::to_string(starting_items_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(starting_items_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(starting_items_entry.content_flags_disabled) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			starting_items_entry.id = results.LastInsertedID();
			return starting_items_entry;
		}

		starting_items_entry = NewEntity();

		return starting_items_entry;
	}

	static int InsertMany(
		std::vector<StartingItems> starting_items_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &starting_items_entry: starting_items_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(starting_items_entry.race));
			insert_values.push_back(std::to_string(starting_items_entry.class));
			insert_values.push_back(std::to_string(starting_items_entry.deityid));
			insert_values.push_back(std::to_string(starting_items_entry.zoneid));
			insert_values.push_back(std::to_string(starting_items_entry.itemid));
			insert_values.push_back(std::to_string(starting_items_entry.item_charges));
			insert_values.push_back(std::to_string(starting_items_entry.gm));
			insert_values.push_back(std::to_string(starting_items_entry.slot));
			insert_values.push_back(std::to_string(starting_items_entry.min_expansion));
			insert_values.push_back(std::to_string(starting_items_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(starting_items_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(starting_items_entry.content_flags_disabled) + "'");

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

	static std::vector<StartingItems> All()
	{
		std::vector<StartingItems> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			StartingItems entry{};

			entry.id                     = atoi(row[0]);
			entry.race                   = atoi(row[1]);
			entry.class                  = atoi(row[2]);
			entry.deityid                = atoi(row[3]);
			entry.zoneid                 = atoi(row[4]);
			entry.itemid                 = atoi(row[5]);
			entry.item_charges           = atoi(row[6]);
			entry.gm                     = atoi(row[7]);
			entry.slot                   = atoi(row[8]);
			entry.min_expansion          = atoi(row[9]);
			entry.max_expansion          = atoi(row[10]);
			entry.content_flags          = row[11] ? row[11] : "";
			entry.content_flags_disabled = row[12] ? row[12] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<StartingItems> GetWhere(std::string where_filter)
	{
		std::vector<StartingItems> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			StartingItems entry{};

			entry.id                     = atoi(row[0]);
			entry.race                   = atoi(row[1]);
			entry.class                  = atoi(row[2]);
			entry.deityid                = atoi(row[3]);
			entry.zoneid                 = atoi(row[4]);
			entry.itemid                 = atoi(row[5]);
			entry.item_charges           = atoi(row[6]);
			entry.gm                     = atoi(row[7]);
			entry.slot                   = atoi(row[8]);
			entry.min_expansion          = atoi(row[9]);
			entry.max_expansion          = atoi(row[10]);
			entry.content_flags          = row[11] ? row[11] : "";
			entry.content_flags_disabled = row[12] ? row[12] : "";

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

#endif //EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H
