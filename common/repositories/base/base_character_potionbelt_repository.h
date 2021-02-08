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

#ifndef EQEMU_BASE_CHARACTER_POTIONBELT_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_POTIONBELT_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterPotionbeltRepository {
public:
	struct CharacterPotionbelt {
		int id;
		int potion_id;
		int item_id;
		int icon;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"potion_id",
			"item_id",
			"icon",
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
		return std::string("character_potionbelt");
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

	static CharacterPotionbelt NewEntity()
	{
		CharacterPotionbelt entry{};

		entry.id        = 0;
		entry.potion_id = 0;
		entry.item_id   = 0;
		entry.icon      = 0;

		return entry;
	}

	static CharacterPotionbelt GetCharacterPotionbeltEntry(
		const std::vector<CharacterPotionbelt> &character_potionbelts,
		int character_potionbelt_id
	)
	{
		for (auto &character_potionbelt : character_potionbelts) {
			if (character_potionbelt.id == character_potionbelt_id) {
				return character_potionbelt;
			}
		}

		return NewEntity();
	}

	static CharacterPotionbelt FindOne(
		Database& db,
		int character_potionbelt_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_potionbelt_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterPotionbelt entry{};

			entry.id        = atoi(row[0]);
			entry.potion_id = atoi(row[1]);
			entry.item_id   = atoi(row[2]);
			entry.icon      = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_potionbelt_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_potionbelt_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterPotionbelt character_potionbelt_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_potionbelt_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_potionbelt_entry.potion_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_potionbelt_entry.item_id));
		update_values.push_back(columns[3] + " = " + std::to_string(character_potionbelt_entry.icon));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_potionbelt_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterPotionbelt InsertOne(
		Database& db,
		CharacterPotionbelt character_potionbelt_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_potionbelt_entry.id));
		insert_values.push_back(std::to_string(character_potionbelt_entry.potion_id));
		insert_values.push_back(std::to_string(character_potionbelt_entry.item_id));
		insert_values.push_back(std::to_string(character_potionbelt_entry.icon));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_potionbelt_entry.id = results.LastInsertedID();
			return character_potionbelt_entry;
		}

		character_potionbelt_entry = NewEntity();

		return character_potionbelt_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterPotionbelt> character_potionbelt_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_potionbelt_entry: character_potionbelt_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_potionbelt_entry.id));
			insert_values.push_back(std::to_string(character_potionbelt_entry.potion_id));
			insert_values.push_back(std::to_string(character_potionbelt_entry.item_id));
			insert_values.push_back(std::to_string(character_potionbelt_entry.icon));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<CharacterPotionbelt> All(Database& db)
	{
		std::vector<CharacterPotionbelt> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPotionbelt entry{};

			entry.id        = atoi(row[0]);
			entry.potion_id = atoi(row[1]);
			entry.item_id   = atoi(row[2]);
			entry.icon      = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterPotionbelt> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterPotionbelt> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPotionbelt entry{};

			entry.id        = atoi(row[0]);
			entry.potion_id = atoi(row[1]);
			entry.item_id   = atoi(row[2]);
			entry.icon      = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, std::string where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_CHARACTER_POTIONBELT_REPOSITORY_H
