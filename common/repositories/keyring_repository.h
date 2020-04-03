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

#ifndef EQEMU_KEYRING_REPOSITORY_H
#define EQEMU_KEYRING_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class KeyringRepository {
public:
	struct Keyring {
		int char_id;
		int item_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"item_id",
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
		return std::string("keyring");
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

	static Keyring NewEntity()
	{
		Keyring entry{};

		entry.char_id = 0;
		entry.item_id = 0;

		return entry;
	}

	static Keyring GetKeyringEntry(
		const std::vector<Keyring> &keyrings,
		int keyring_id
	)
	{
		for (auto &keyring : keyrings) {
			if (keyring.== keyring_id) {
				return keyring;
			}
		}

		return NewEntity();
	}

	static Keyring FindOne(
		int keyring_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				keyring_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Keyring entry{};

			entry.char_id = atoi(row[0]);
			entry.item_id = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int keyring_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				keyring_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Keyring keyring_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(keyring_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(keyring_entry.item_id));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				keyring_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Keyring InsertOne(
		Keyring keyring_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(keyring_entry.char_id));
		insert_values.push_back(std::to_string(keyring_entry.item_id));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			keyring_entry.id = results.LastInsertedID();
			return keyring_entry;
		}

		keyring_entry = InstanceListRepository::NewEntity();

		return keyring_entry;
	}

	static int InsertMany(
		std::vector<Keyring> keyring_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &keyring_entry: keyring_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(keyring_entry.char_id));
			insert_values.push_back(std::to_string(keyring_entry.item_id));

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

	static std::vector<Keyring> All()
	{
		std::vector<Keyring> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Keyring entry{};

			entry.char_id = atoi(row[0]);
			entry.item_id = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_KEYRING_REPOSITORY_H
