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

#ifndef EQEMU_SHAREDBANK_REPOSITORY_H
#define EQEMU_SHAREDBANK_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class SharedbankRepository {
public:
	struct Sharedbank {
		int         acctid;
		int         slotid;
		int         itemid;
		int16       charges;
		int         augslot1;
		int         augslot2;
		int         augslot3;
		int         augslot4;
		int         augslot5;
		int         augslot6;
		std::string custom_data;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"acctid",
			"slotid",
			"itemid",
			"charges",
			"augslot1",
			"augslot2",
			"augslot3",
			"augslot4",
			"augslot5",
			"augslot6",
			"custom_data",
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
		return std::string("sharedbank");
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

	static Sharedbank NewEntity()
	{
		Sharedbank entry{};

		entry.acctid      = 0;
		entry.slotid      = 0;
		entry.itemid      = 0;
		entry.charges     = 0;
		entry.augslot1    = 0;
		entry.augslot2    = 0;
		entry.augslot3    = 0;
		entry.augslot4    = 0;
		entry.augslot5    = 0;
		entry.augslot6    = 0;
		entry.custom_data = 0;

		return entry;
	}

	static Sharedbank GetSharedbankEntry(
		const std::vector<Sharedbank> &sharedbanks,
		int sharedbank_id
	)
	{
		for (auto &sharedbank : sharedbanks) {
			if (sharedbank. == sharedbank_id) {
				return sharedbank;
			}
		}

		return NewEntity();
	}

	static Sharedbank FindOne(
		int sharedbank_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				sharedbank_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Sharedbank entry{};

			entry.acctid      = atoi(row[0]);
			entry.slotid      = atoi(row[1]);
			entry.itemid      = atoi(row[2]);
			entry.charges     = atoi(row[3]);
			entry.augslot1    = atoi(row[4]);
			entry.augslot2    = atoi(row[5]);
			entry.augslot3    = atoi(row[6]);
			entry.augslot4    = atoi(row[7]);
			entry.augslot5    = atoi(row[8]);
			entry.augslot6    = atoi(row[9]);
			entry.custom_data = row[10];

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int sharedbank_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				sharedbank_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Sharedbank sharedbank_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(sharedbank_entry.acctid));
		update_values.push_back(columns[1] + " = " + std::to_string(sharedbank_entry.slotid));
		update_values.push_back(columns[2] + " = " + std::to_string(sharedbank_entry.itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(sharedbank_entry.charges));
		update_values.push_back(columns[4] + " = " + std::to_string(sharedbank_entry.augslot1));
		update_values.push_back(columns[5] + " = " + std::to_string(sharedbank_entry.augslot2));
		update_values.push_back(columns[6] + " = " + std::to_string(sharedbank_entry.augslot3));
		update_values.push_back(columns[7] + " = " + std::to_string(sharedbank_entry.augslot4));
		update_values.push_back(columns[8] + " = " + std::to_string(sharedbank_entry.augslot5));
		update_values.push_back(columns[9] + " = " + std::to_string(sharedbank_entry.augslot6));
		update_values.push_back(columns[10] + " = '" + EscapeString(sharedbank_entry.custom_data) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				sharedbank_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Sharedbank InsertOne(
		Sharedbank sharedbank_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(sharedbank_entry.acctid));
		insert_values.push_back(std::to_string(sharedbank_entry.slotid));
		insert_values.push_back(std::to_string(sharedbank_entry.itemid));
		insert_values.push_back(std::to_string(sharedbank_entry.charges));
		insert_values.push_back(std::to_string(sharedbank_entry.augslot1));
		insert_values.push_back(std::to_string(sharedbank_entry.augslot2));
		insert_values.push_back(std::to_string(sharedbank_entry.augslot3));
		insert_values.push_back(std::to_string(sharedbank_entry.augslot4));
		insert_values.push_back(std::to_string(sharedbank_entry.augslot5));
		insert_values.push_back(std::to_string(sharedbank_entry.augslot6));
		insert_values.push_back("'" + EscapeString(sharedbank_entry.custom_data) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			sharedbank_entry.id = results.LastInsertedID();
			return sharedbank_entry;
		}

		sharedbank_entry = InstanceListRepository::NewEntity();

		return sharedbank_entry;
	}

	static int InsertMany(
		std::vector<Sharedbank> sharedbank_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &sharedbank_entry: sharedbank_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(sharedbank_entry.acctid));
			insert_values.push_back(std::to_string(sharedbank_entry.slotid));
			insert_values.push_back(std::to_string(sharedbank_entry.itemid));
			insert_values.push_back(std::to_string(sharedbank_entry.charges));
			insert_values.push_back(std::to_string(sharedbank_entry.augslot1));
			insert_values.push_back(std::to_string(sharedbank_entry.augslot2));
			insert_values.push_back(std::to_string(sharedbank_entry.augslot3));
			insert_values.push_back(std::to_string(sharedbank_entry.augslot4));
			insert_values.push_back(std::to_string(sharedbank_entry.augslot5));
			insert_values.push_back(std::to_string(sharedbank_entry.augslot6));
			insert_values.push_back("'" + EscapeString(sharedbank_entry.custom_data) + "'");

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

	static std::vector<Sharedbank> All()
	{
		std::vector<Sharedbank> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Sharedbank entry{};

			entry.acctid      = atoi(row[0]);
			entry.slotid      = atoi(row[1]);
			entry.itemid      = atoi(row[2]);
			entry.charges     = atoi(row[3]);
			entry.augslot1    = atoi(row[4]);
			entry.augslot2    = atoi(row[5]);
			entry.augslot3    = atoi(row[6]);
			entry.augslot4    = atoi(row[7]);
			entry.augslot5    = atoi(row[8]);
			entry.augslot6    = atoi(row[9]);
			entry.custom_data = row[10];

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_SHAREDBANK_REPOSITORY_H
