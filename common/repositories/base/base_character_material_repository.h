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

#ifndef EQEMU_BASE_CHARACTER_MATERIAL_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_MATERIAL_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterMaterialRepository {
public:
	struct CharacterMaterial {
		int id;
		int slot;
		int blue;
		int green;
		int red;
		int use_tint;
		int color;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"slot",
			"blue",
			"green",
			"red",
			"use_tint",
			"color",
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
		return std::string("character_material");
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

	static CharacterMaterial NewEntity()
	{
		CharacterMaterial entry{};

		entry.id       = 0;
		entry.slot     = 0;
		entry.blue     = 0;
		entry.green    = 0;
		entry.red      = 0;
		entry.use_tint = 0;
		entry.color    = 0;

		return entry;
	}

	static CharacterMaterial GetCharacterMaterialEntry(
		const std::vector<CharacterMaterial> &character_materials,
		int character_material_id
	)
	{
		for (auto &character_material : character_materials) {
			if (character_material.id == character_material_id) {
				return character_material;
			}
		}

		return NewEntity();
	}

	static CharacterMaterial FindOne(
		Database& db,
		int character_material_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_material_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterMaterial entry{};

			entry.id       = atoi(row[0]);
			entry.slot     = atoi(row[1]);
			entry.blue     = atoi(row[2]);
			entry.green    = atoi(row[3]);
			entry.red      = atoi(row[4]);
			entry.use_tint = atoi(row[5]);
			entry.color    = atoi(row[6]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_material_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_material_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterMaterial character_material_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(character_material_entry.slot));
		update_values.push_back(columns[2] + " = " + std::to_string(character_material_entry.blue));
		update_values.push_back(columns[3] + " = " + std::to_string(character_material_entry.green));
		update_values.push_back(columns[4] + " = " + std::to_string(character_material_entry.red));
		update_values.push_back(columns[5] + " = " + std::to_string(character_material_entry.use_tint));
		update_values.push_back(columns[6] + " = " + std::to_string(character_material_entry.color));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_material_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterMaterial InsertOne(
		Database& db,
		CharacterMaterial character_material_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_material_entry.slot));
		insert_values.push_back(std::to_string(character_material_entry.blue));
		insert_values.push_back(std::to_string(character_material_entry.green));
		insert_values.push_back(std::to_string(character_material_entry.red));
		insert_values.push_back(std::to_string(character_material_entry.use_tint));
		insert_values.push_back(std::to_string(character_material_entry.color));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_material_entry.id = results.LastInsertedID();
			return character_material_entry;
		}

		character_material_entry = NewEntity();

		return character_material_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterMaterial> character_material_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_material_entry: character_material_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_material_entry.slot));
			insert_values.push_back(std::to_string(character_material_entry.blue));
			insert_values.push_back(std::to_string(character_material_entry.green));
			insert_values.push_back(std::to_string(character_material_entry.red));
			insert_values.push_back(std::to_string(character_material_entry.use_tint));
			insert_values.push_back(std::to_string(character_material_entry.color));

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

	static std::vector<CharacterMaterial> All(Database& db)
	{
		std::vector<CharacterMaterial> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterMaterial entry{};

			entry.id       = atoi(row[0]);
			entry.slot     = atoi(row[1]);
			entry.blue     = atoi(row[2]);
			entry.green    = atoi(row[3]);
			entry.red      = atoi(row[4]);
			entry.use_tint = atoi(row[5]);
			entry.color    = atoi(row[6]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterMaterial> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterMaterial> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterMaterial entry{};

			entry.id       = atoi(row[0]);
			entry.slot     = atoi(row[1]);
			entry.blue     = atoi(row[2]);
			entry.green    = atoi(row[3]);
			entry.red      = atoi(row[4]);
			entry.use_tint = atoi(row[5]);
			entry.color    = atoi(row[6]);

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

#endif //EQEMU_BASE_CHARACTER_MATERIAL_REPOSITORY_H
