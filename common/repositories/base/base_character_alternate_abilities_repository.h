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

#ifndef EQEMU_BASE_CHARACTER_ALTERNATE_ABILITIES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_ALTERNATE_ABILITIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterAlternateAbilitiesRepository {
public:
	struct CharacterAlternateAbilities {
		int id;
		int aa_id;
		int aa_value;
		int charges;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"aa_id",
			"aa_value",
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
		return std::string("character_alternate_abilities");
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

	static CharacterAlternateAbilities NewEntity()
	{
		CharacterAlternateAbilities entry{};

		entry.id       = 0;
		entry.aa_id    = 0;
		entry.aa_value = 0;
		entry.charges  = 0;

		return entry;
	}

	static CharacterAlternateAbilities GetCharacterAlternateAbilitiesEntry(
		const std::vector<CharacterAlternateAbilities> &character_alternate_abilitiess,
		int character_alternate_abilities_id
	)
	{
		for (auto &character_alternate_abilities : character_alternate_abilitiess) {
			if (character_alternate_abilities.id == character_alternate_abilities_id) {
				return character_alternate_abilities;
			}
		}

		return NewEntity();
	}

	static CharacterAlternateAbilities FindOne(
		int character_alternate_abilities_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_alternate_abilities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterAlternateAbilities entry{};

			entry.id       = atoi(row[0]);
			entry.aa_id    = atoi(row[1]);
			entry.aa_value = atoi(row[2]);
			entry.charges  = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int character_alternate_abilities_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_alternate_abilities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		CharacterAlternateAbilities character_alternate_abilities_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_alternate_abilities_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_alternate_abilities_entry.aa_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_alternate_abilities_entry.aa_value));
		update_values.push_back(columns[3] + " = " + std::to_string(character_alternate_abilities_entry.charges));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_alternate_abilities_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterAlternateAbilities InsertOne(
		CharacterAlternateAbilities character_alternate_abilities_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_alternate_abilities_entry.id));
		insert_values.push_back(std::to_string(character_alternate_abilities_entry.aa_id));
		insert_values.push_back(std::to_string(character_alternate_abilities_entry.aa_value));
		insert_values.push_back(std::to_string(character_alternate_abilities_entry.charges));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_alternate_abilities_entry.id = results.LastInsertedID();
			return character_alternate_abilities_entry;
		}

		character_alternate_abilities_entry = NewEntity();

		return character_alternate_abilities_entry;
	}

	static int InsertMany(
		std::vector<CharacterAlternateAbilities> character_alternate_abilities_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_alternate_abilities_entry: character_alternate_abilities_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_alternate_abilities_entry.id));
			insert_values.push_back(std::to_string(character_alternate_abilities_entry.aa_id));
			insert_values.push_back(std::to_string(character_alternate_abilities_entry.aa_value));
			insert_values.push_back(std::to_string(character_alternate_abilities_entry.charges));

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

	static std::vector<CharacterAlternateAbilities> All()
	{
		std::vector<CharacterAlternateAbilities> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAlternateAbilities entry{};

			entry.id       = atoi(row[0]);
			entry.aa_id    = atoi(row[1]);
			entry.aa_value = atoi(row[2]);
			entry.charges  = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterAlternateAbilities> GetWhere(std::string where_filter)
	{
		std::vector<CharacterAlternateAbilities> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAlternateAbilities entry{};

			entry.id       = atoi(row[0]);
			entry.aa_id    = atoi(row[1]);
			entry.aa_value = atoi(row[2]);
			entry.charges  = atoi(row[3]);

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

#endif //EQEMU_BASE_CHARACTER_ALTERNATE_ABILITIES_REPOSITORY_H
