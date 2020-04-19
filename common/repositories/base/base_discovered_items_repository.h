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

#ifndef EQEMU_BASE_DISCOVERED_ITEMS_REPOSITORY_H
#define EQEMU_BASE_DISCOVERED_ITEMS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseDiscoveredItemsRepository {
public:
	struct DiscoveredItems {
		int         item_id;
		std::string char_name;
		int         discovered_date;
		int         account_status;
	};

	static std::string PrimaryKey()
	{
		return std::string("item_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"item_id",
			"char_name",
			"discovered_date",
			"account_status",
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
		return std::string("discovered_items");
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

	static DiscoveredItems NewEntity()
	{
		DiscoveredItems entry{};

		entry.item_id         = 0;
		entry.char_name       = "";
		entry.discovered_date = 0;
		entry.account_status  = 0;

		return entry;
	}

	static DiscoveredItems GetDiscoveredItemsEntry(
		const std::vector<DiscoveredItems> &discovered_itemss,
		int discovered_items_id
	)
	{
		for (auto &discovered_items : discovered_itemss) {
			if (discovered_items.item_id == discovered_items_id) {
				return discovered_items;
			}
		}

		return NewEntity();
	}

	static DiscoveredItems FindOne(
		int discovered_items_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				discovered_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DiscoveredItems entry{};

			entry.item_id         = atoi(row[0]);
			entry.char_name       = row[1] ? row[1] : "";
			entry.discovered_date = atoi(row[2]);
			entry.account_status  = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int discovered_items_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				discovered_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		DiscoveredItems discovered_items_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(discovered_items_entry.item_id));
		update_values.push_back(columns[1] + " = '" + EscapeString(discovered_items_entry.char_name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(discovered_items_entry.discovered_date));
		update_values.push_back(columns[3] + " = " + std::to_string(discovered_items_entry.account_status));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				discovered_items_entry.item_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DiscoveredItems InsertOne(
		DiscoveredItems discovered_items_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(discovered_items_entry.item_id));
		insert_values.push_back("'" + EscapeString(discovered_items_entry.char_name) + "'");
		insert_values.push_back(std::to_string(discovered_items_entry.discovered_date));
		insert_values.push_back(std::to_string(discovered_items_entry.account_status));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			discovered_items_entry.item_id = results.LastInsertedID();
			return discovered_items_entry;
		}

		discovered_items_entry = NewEntity();

		return discovered_items_entry;
	}

	static int InsertMany(
		std::vector<DiscoveredItems> discovered_items_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &discovered_items_entry: discovered_items_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(discovered_items_entry.item_id));
			insert_values.push_back("'" + EscapeString(discovered_items_entry.char_name) + "'");
			insert_values.push_back(std::to_string(discovered_items_entry.discovered_date));
			insert_values.push_back(std::to_string(discovered_items_entry.account_status));

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

	static std::vector<DiscoveredItems> All()
	{
		std::vector<DiscoveredItems> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DiscoveredItems entry{};

			entry.item_id         = atoi(row[0]);
			entry.char_name       = row[1] ? row[1] : "";
			entry.discovered_date = atoi(row[2]);
			entry.account_status  = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<DiscoveredItems> GetWhere(std::string where_filter)
	{
		std::vector<DiscoveredItems> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DiscoveredItems entry{};

			entry.item_id         = atoi(row[0]);
			entry.char_name       = row[1] ? row[1] : "";
			entry.discovered_date = atoi(row[2]);
			entry.account_status  = atoi(row[3]);

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

#endif //EQEMU_BASE_DISCOVERED_ITEMS_REPOSITORY_H
