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

#ifndef EQEMU_INVENTORY_VERSIONS_REPOSITORY_H
#define EQEMU_INVENTORY_VERSIONS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class InventoryVersionsRepository {
public:
	struct InventoryVersions {
		int version;
		int step;
		int bot_step;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"version",
			"step",
			"bot_step",
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
		return std::string("inventory_versions");
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

	static InventoryVersions NewEntity()
	{
		InventoryVersions entry{};

		entry.version  = 0;
		entry.step     = 0;
		entry.bot_step = 0;

		return entry;
	}

	static InventoryVersions GetInventoryVersionsEntry(
		const std::vector<InventoryVersions> &inventory_versionss,
		int inventory_versions_id
	)
	{
		for (auto &inventory_versions : inventory_versionss) {
			if (inventory_versions. == inventory_versions_id) {
				return inventory_versions;
			}
		}

		return NewEntity();
	}

	static InventoryVersions FindOne(
		int inventory_versions_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				inventory_versions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			InventoryVersions entry{};

			entry.version  = atoi(row[0]);
			entry.step     = atoi(row[1]);
			entry.bot_step = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int inventory_versions_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				inventory_versions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		InventoryVersions inventory_versions_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(inventory_versions_entry.version));
		update_values.push_back(columns[1] + " = " + std::to_string(inventory_versions_entry.step));
		update_values.push_back(columns[2] + " = " + std::to_string(inventory_versions_entry.bot_step));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				inventory_versions_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static InventoryVersions InsertOne(
		InventoryVersions inventory_versions_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(inventory_versions_entry.version));
		insert_values.push_back(std::to_string(inventory_versions_entry.step));
		insert_values.push_back(std::to_string(inventory_versions_entry.bot_step));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			inventory_versions_entry.id = results.LastInsertedID();
			return inventory_versions_entry;
		}

		inventory_versions_entry = InstanceListRepository::NewEntity();

		return inventory_versions_entry;
	}

	static int InsertMany(
		std::vector<InventoryVersions> inventory_versions_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &inventory_versions_entry: inventory_versions_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(inventory_versions_entry.version));
			insert_values.push_back(std::to_string(inventory_versions_entry.step));
			insert_values.push_back(std::to_string(inventory_versions_entry.bot_step));

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

	static std::vector<InventoryVersions> All()
	{
		std::vector<InventoryVersions> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InventoryVersions entry{};

			entry.version  = atoi(row[0]);
			entry.step     = atoi(row[1]);
			entry.bot_step = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_INVENTORY_VERSIONS_REPOSITORY_H
