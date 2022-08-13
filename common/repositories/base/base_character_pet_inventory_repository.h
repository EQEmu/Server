/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://eqemu.gitbook.io/server/in-development/developer-area/repositories
 */

#ifndef EQEMU_BASE_CHARACTER_PET_INVENTORY_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_PET_INVENTORY_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterPetInventoryRepository {
public:
	struct CharacterPetInventory {
		int char_id;
		int pet;
		int slot;
		int item_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"pet",
			"slot",
			"item_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"char_id",
			"pet",
			"slot",
			"item_id",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("character_pet_inventory");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static CharacterPetInventory NewEntity()
	{
		CharacterPetInventory e{};

		e.char_id = 0;
		e.pet     = 0;
		e.slot    = 0;
		e.item_id = 0;

		return e;
	}

	static CharacterPetInventory GetCharacterPetInventorye(
		const std::vector<CharacterPetInventory> &character_pet_inventorys,
		int character_pet_inventory_id
	)
	{
		for (auto &character_pet_inventory : character_pet_inventorys) {
			if (character_pet_inventory.char_id == character_pet_inventory_id) {
				return character_pet_inventory;
			}
		}

		return NewEntity();
	}

	static CharacterPetInventory FindOne(
		Database& db,
		int character_pet_inventory_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_pet_inventory_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterPetInventory e{};

			e.char_id = atoi(row[0]);
			e.pet     = atoi(row[1]);
			e.slot    = atoi(row[2]);
			e.item_id = atoi(row[3]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_pet_inventory_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_pet_inventory_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterPetInventory character_pet_inventory_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_pet_inventory_e.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_pet_inventory_e.pet));
		update_values.push_back(columns[2] + " = " + std::to_string(character_pet_inventory_e.slot));
		update_values.push_back(columns[3] + " = " + std::to_string(character_pet_inventory_e.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				character_pet_inventory_e.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterPetInventory InsertOne(
		Database& db,
		CharacterPetInventory character_pet_inventory_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_pet_inventory_e.char_id));
		insert_values.push_back(std::to_string(character_pet_inventory_e.pet));
		insert_values.push_back(std::to_string(character_pet_inventory_e.slot));
		insert_values.push_back(std::to_string(character_pet_inventory_e.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_pet_inventory_e.char_id = results.LastInsertedID();
			return character_pet_inventory_e;
		}

		character_pet_inventory_e = NewEntity();

		return character_pet_inventory_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterPetInventory> character_pet_inventory_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_pet_inventory_e: character_pet_inventory_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_pet_inventory_e.char_id));
			insert_values.push_back(std::to_string(character_pet_inventory_e.pet));
			insert_values.push_back(std::to_string(character_pet_inventory_e.slot));
			insert_values.push_back(std::to_string(character_pet_inventory_e.item_id));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<CharacterPetInventory> All(Database& db)
	{
		std::vector<CharacterPetInventory> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetInventory e{};

			e.char_id = atoi(row[0]);
			e.pet     = atoi(row[1]);
			e.slot    = atoi(row[2]);
			e.item_id = atoi(row[3]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterPetInventory> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterPetInventory> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetInventory e{};

			e.char_id = atoi(row[0]);
			e.pet     = atoi(row[1]);
			e.slot    = atoi(row[2]);
			e.item_id = atoi(row[3]);

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_CHARACTER_PET_INVENTORY_REPOSITORY_H
