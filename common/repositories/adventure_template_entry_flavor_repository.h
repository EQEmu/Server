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

#ifndef EQEMU_ADVENTURE_TEMPLATE_ENTRY_FLAVOR_REPOSITORY_H
#define EQEMU_ADVENTURE_TEMPLATE_ENTRY_FLAVOR_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class AdventureTemplateEntryFlavorRepository {
public:
	struct AdventureTemplateEntryFlavor {
		int         id;
		std::string text;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"text",
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
		return std::string("adventure_template_entry_flavor");
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

	static AdventureTemplateEntryFlavor NewEntity()
	{
		AdventureTemplateEntryFlavor entry{};

		entry.id   = 0;
		entry.text = 0;

		return entry;
	}

	static AdventureTemplateEntryFlavor GetAdventureTemplateEntryFlavorEntry(
		const std::vector<AdventureTemplateEntryFlavor> &adventure_template_entry_flavors,
		int adventure_template_entry_flavor_id
	)
	{
		for (auto &adventure_template_entry_flavor : adventure_template_entry_flavors) {
			if (adventure_template_entry_flavor.id == adventure_template_entry_flavor_id) {
				return adventure_template_entry_flavor;
			}
		}

		return NewEntity();
	}

	static AdventureTemplateEntryFlavor FindOne(
		int adventure_template_entry_flavor_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				adventure_template_entry_flavor_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureTemplateEntryFlavor entry{};

			entry.id   = atoi(row[0]);
			entry.text = row[1];

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int adventure_template_entry_flavor_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_template_entry_flavor_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		AdventureTemplateEntryFlavor adventure_template_entry_flavor_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(adventure_template_entry_flavor_entry.text) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				adventure_template_entry_flavor_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureTemplateEntryFlavor InsertOne(
		AdventureTemplateEntryFlavor adventure_template_entry_flavor_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(adventure_template_entry_flavor_entry.text) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			adventure_template_entry_flavor_entry.id = results.LastInsertedID();
			return adventure_template_entry_flavor_entry;
		}

		adventure_template_entry_flavor_entry = InstanceListRepository::NewEntity();

		return adventure_template_entry_flavor_entry;
	}

	static int InsertMany(
		std::vector<AdventureTemplateEntryFlavor> adventure_template_entry_flavor_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &adventure_template_entry_flavor_entry: adventure_template_entry_flavor_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(adventure_template_entry_flavor_entry.text) + "'");

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

	static std::vector<AdventureTemplateEntryFlavor> All()
	{
		std::vector<AdventureTemplateEntryFlavor> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureTemplateEntryFlavor entry{};

			entry.id   = atoi(row[0]);
			entry.text = row[1];

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_ADVENTURE_TEMPLATE_ENTRY_FLAVOR_REPOSITORY_H
