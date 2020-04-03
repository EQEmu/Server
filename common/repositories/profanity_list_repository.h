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

#ifndef EQEMU_PROFANITY_LIST_REPOSITORY_H
#define EQEMU_PROFANITY_LIST_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class ProfanityListRepository {
public:
	struct ProfanityList {
		std::string word;
	};

	static std::string PrimaryKey()
	{
		return std::string("");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"word",
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
		return std::string("profanity_list");
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

	static ProfanityList NewEntity()
	{
		ProfanityList entry{};

		entry.word = 0;

		return entry;
	}

	static ProfanityList GetProfanityListEntry(
		const std::vector<ProfanityList> &profanity_lists,
		int profanity_list_id
	)
	{
		for (auto &profanity_list : profanity_lists) {
			if (profanity_list.== profanity_list_id) {
				return profanity_list;
			}
		}

		return NewEntity();
	}

	static ProfanityList FindOne(
		int profanity_list_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				profanity_list_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ProfanityList entry{};

			entry.word = row[0];

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int profanity_list_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				profanity_list_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		ProfanityList profanity_list_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = '" + EscapeString(profanity_list_entry.word) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				profanity_list_entry.
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ProfanityList InsertOne(
		ProfanityList profanity_list_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(profanity_list_entry.word) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			profanity_list_entry.id = results.LastInsertedID();
			return profanity_list_entry;
		}

		profanity_list_entry = InstanceListRepository::NewEntity();

		return profanity_list_entry;
	}

	static int InsertMany(
		std::vector<ProfanityList> profanity_list_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &profanity_list_entry: profanity_list_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(profanity_list_entry.word) + "'");

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

	static std::vector<ProfanityList> All()
	{
		std::vector<ProfanityList> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ProfanityList entry{};

			entry.word = row[0];

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_PROFANITY_LIST_REPOSITORY_H
