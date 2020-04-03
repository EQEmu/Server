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

#ifndef EQEMU_CHARACTER_ENABLEDTASKS_REPOSITORY_H
#define EQEMU_CHARACTER_ENABLEDTASKS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class CharacterEnabledtasksRepository {
public:
	struct CharacterEnabledtasks {
		int charid;
		int taskid;
	};

	static std::string PrimaryKey()
	{
		return std::string("taskid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"taskid",
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
		return std::string("character_enabledtasks");
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

	static CharacterEnabledtasks NewEntity()
	{
		CharacterEnabledtasks entry{};

		entry.charid = 0;
		entry.taskid = 0;

		return entry;
	}

	static CharacterEnabledtasks GetCharacterEnabledtasksEntry(
		const std::vector<CharacterEnabledtasks> &character_enabledtaskss,
		int character_enabledtasks_id
	)
	{
		for (auto &character_enabledtasks : character_enabledtaskss) {
			if (character_enabledtasks.taskid == character_enabledtasks_id) {
				return character_enabledtasks;
			}
		}

		return NewEntity();
	}

	static CharacterEnabledtasks FindOne(
		int character_enabledtasks_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_enabledtasks_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterEnabledtasks entry{};

			entry.charid = atoi(row[0]);
			entry.taskid = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int character_enabledtasks_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_enabledtasks_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		CharacterEnabledtasks character_enabledtasks_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();


		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_enabledtasks_entry.taskid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterEnabledtasks InsertOne(
		CharacterEnabledtasks character_enabledtasks_entry
	)
	{
		std::vector<std::string> insert_values;


		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_enabledtasks_entry.id = results.LastInsertedID();
			return character_enabledtasks_entry;
		}

		character_enabledtasks_entry = InstanceListRepository::NewEntity();

		return character_enabledtasks_entry;
	}

	static int InsertMany(
		std::vector<CharacterEnabledtasks> character_enabledtasks_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_enabledtasks_entry: character_enabledtasks_entries) {
			std::vector<std::string> insert_values;


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

	static std::vector<CharacterEnabledtasks> All()
	{
		std::vector<CharacterEnabledtasks> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterEnabledtasks entry{};

			entry.charid = atoi(row[0]);
			entry.taskid = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_CHARACTER_ENABLEDTASKS_REPOSITORY_H
