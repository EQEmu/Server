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

#ifndef EQEMU_BASE_MERCHANTLIST_TEMP_REPOSITORY_H
#define EQEMU_BASE_MERCHANTLIST_TEMP_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseMerchantlistTempRepository {
public:
	struct MerchantlistTemp {
		int npcid;
		int slot;
		int itemid;
		int charges;
	};

	static std::string PrimaryKey()
	{
		return std::string("npcid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"npcid",
			"slot",
			"itemid",
			"charges",
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
		return std::string("merchantlist_temp");
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

	static MerchantlistTemp NewEntity()
	{
		MerchantlistTemp entry{};

		entry.npcid   = 0;
		entry.slot    = 0;
		entry.itemid  = 0;
		entry.charges = 1;

		return entry;
	}

	static MerchantlistTemp GetMerchantlistTempEntry(
		const std::vector<MerchantlistTemp> &merchantlist_temps,
		int merchantlist_temp_id
	)
	{
		for (auto &merchantlist_temp : merchantlist_temps) {
			if (merchantlist_temp.npcid == merchantlist_temp_id) {
				return merchantlist_temp;
			}
		}

		return NewEntity();
	}

	static MerchantlistTemp FindOne(
		int merchantlist_temp_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				merchantlist_temp_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MerchantlistTemp entry{};

			entry.npcid   = atoi(row[0]);
			entry.slot    = atoi(row[1]);
			entry.itemid  = atoi(row[2]);
			entry.charges = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int merchantlist_temp_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merchantlist_temp_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		MerchantlistTemp merchantlist_temp_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(merchantlist_temp_entry.npcid));
		update_values.push_back(columns[1] + " = " + std::to_string(merchantlist_temp_entry.slot));
		update_values.push_back(columns[2] + " = " + std::to_string(merchantlist_temp_entry.itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(merchantlist_temp_entry.charges));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				merchantlist_temp_entry.npcid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MerchantlistTemp InsertOne(
		MerchantlistTemp merchantlist_temp_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(merchantlist_temp_entry.npcid));
		insert_values.push_back(std::to_string(merchantlist_temp_entry.slot));
		insert_values.push_back(std::to_string(merchantlist_temp_entry.itemid));
		insert_values.push_back(std::to_string(merchantlist_temp_entry.charges));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			merchantlist_temp_entry.npcid = results.LastInsertedID();
			return merchantlist_temp_entry;
		}

		merchantlist_temp_entry = NewEntity();

		return merchantlist_temp_entry;
	}

	static int InsertMany(
		std::vector<MerchantlistTemp> merchantlist_temp_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &merchantlist_temp_entry: merchantlist_temp_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(merchantlist_temp_entry.npcid));
			insert_values.push_back(std::to_string(merchantlist_temp_entry.slot));
			insert_values.push_back(std::to_string(merchantlist_temp_entry.itemid));
			insert_values.push_back(std::to_string(merchantlist_temp_entry.charges));

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

	static std::vector<MerchantlistTemp> All()
	{
		std::vector<MerchantlistTemp> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MerchantlistTemp entry{};

			entry.npcid   = atoi(row[0]);
			entry.slot    = atoi(row[1]);
			entry.itemid  = atoi(row[2]);
			entry.charges = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<MerchantlistTemp> GetWhere(std::string where_filter)
	{
		std::vector<MerchantlistTemp> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MerchantlistTemp entry{};

			entry.npcid   = atoi(row[0]);
			entry.slot    = atoi(row[1]);
			entry.itemid  = atoi(row[2]);
			entry.charges = atoi(row[3]);

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

#endif //EQEMU_BASE_MERCHANTLIST_TEMP_REPOSITORY_H
