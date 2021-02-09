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

#ifndef EQEMU_BASE_CHARACTER_PET_INFO_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_PET_INFO_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterPetInfoRepository {
public:
	struct CharacterPetInfo {
		int         char_id;
		int         pet;
		std::string petname;
		int         petpower;
		int         spell_id;
		int         hp;
		int         mana;
		float       size;
		int         taunting;
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
			"petname",
			"petpower",
			"spell_id",
			"hp",
			"mana",
			"size",
			"taunting",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_pet_info");
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
			ColumnsRaw()
		);
	}

	static CharacterPetInfo NewEntity()
	{
		CharacterPetInfo entry{};

		entry.char_id  = 0;
		entry.pet      = 0;
		entry.petname  = "";
		entry.petpower = 0;
		entry.spell_id = 0;
		entry.hp       = 0;
		entry.mana     = 0;
		entry.size     = 0;
		entry.taunting = 1;

		return entry;
	}

	static CharacterPetInfo GetCharacterPetInfoEntry(
		const std::vector<CharacterPetInfo> &character_pet_infos,
		int character_pet_info_id
	)
	{
		for (auto &character_pet_info : character_pet_infos) {
			if (character_pet_info.char_id == character_pet_info_id) {
				return character_pet_info;
			}
		}

		return NewEntity();
	}

	static CharacterPetInfo FindOne(
		Database& db,
		int character_pet_info_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_pet_info_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterPetInfo entry{};

			entry.char_id  = atoi(row[0]);
			entry.pet      = atoi(row[1]);
			entry.petname  = row[2] ? row[2] : "";
			entry.petpower = atoi(row[3]);
			entry.spell_id = atoi(row[4]);
			entry.hp       = atoi(row[5]);
			entry.mana     = atoi(row[6]);
			entry.size     = static_cast<float>(atof(row[7]));
			entry.taunting = atoi(row[8]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_pet_info_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_pet_info_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterPetInfo character_pet_info_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_pet_info_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_pet_info_entry.pet));
		update_values.push_back(columns[2] + " = '" + EscapeString(character_pet_info_entry.petname) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(character_pet_info_entry.petpower));
		update_values.push_back(columns[4] + " = " + std::to_string(character_pet_info_entry.spell_id));
		update_values.push_back(columns[5] + " = " + std::to_string(character_pet_info_entry.hp));
		update_values.push_back(columns[6] + " = " + std::to_string(character_pet_info_entry.mana));
		update_values.push_back(columns[7] + " = " + std::to_string(character_pet_info_entry.size));
		update_values.push_back(columns[8] + " = " + std::to_string(character_pet_info_entry.taunting));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_pet_info_entry.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterPetInfo InsertOne(
		Database& db,
		CharacterPetInfo character_pet_info_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_pet_info_entry.char_id));
		insert_values.push_back(std::to_string(character_pet_info_entry.pet));
		insert_values.push_back("'" + EscapeString(character_pet_info_entry.petname) + "'");
		insert_values.push_back(std::to_string(character_pet_info_entry.petpower));
		insert_values.push_back(std::to_string(character_pet_info_entry.spell_id));
		insert_values.push_back(std::to_string(character_pet_info_entry.hp));
		insert_values.push_back(std::to_string(character_pet_info_entry.mana));
		insert_values.push_back(std::to_string(character_pet_info_entry.size));
		insert_values.push_back(std::to_string(character_pet_info_entry.taunting));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_pet_info_entry.char_id = results.LastInsertedID();
			return character_pet_info_entry;
		}

		character_pet_info_entry = NewEntity();

		return character_pet_info_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterPetInfo> character_pet_info_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_pet_info_entry: character_pet_info_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_pet_info_entry.char_id));
			insert_values.push_back(std::to_string(character_pet_info_entry.pet));
			insert_values.push_back("'" + EscapeString(character_pet_info_entry.petname) + "'");
			insert_values.push_back(std::to_string(character_pet_info_entry.petpower));
			insert_values.push_back(std::to_string(character_pet_info_entry.spell_id));
			insert_values.push_back(std::to_string(character_pet_info_entry.hp));
			insert_values.push_back(std::to_string(character_pet_info_entry.mana));
			insert_values.push_back(std::to_string(character_pet_info_entry.size));
			insert_values.push_back(std::to_string(character_pet_info_entry.taunting));

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

	static std::vector<CharacterPetInfo> All(Database& db)
	{
		std::vector<CharacterPetInfo> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetInfo entry{};

			entry.char_id  = atoi(row[0]);
			entry.pet      = atoi(row[1]);
			entry.petname  = row[2] ? row[2] : "";
			entry.petpower = atoi(row[3]);
			entry.spell_id = atoi(row[4]);
			entry.hp       = atoi(row[5]);
			entry.mana     = atoi(row[6]);
			entry.size     = static_cast<float>(atof(row[7]));
			entry.taunting = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterPetInfo> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterPetInfo> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetInfo entry{};

			entry.char_id  = atoi(row[0]);
			entry.pet      = atoi(row[1]);
			entry.petname  = row[2] ? row[2] : "";
			entry.petpower = atoi(row[3]);
			entry.spell_id = atoi(row[4]);
			entry.hp       = atoi(row[5]);
			entry.mana     = atoi(row[6]);
			entry.size     = static_cast<float>(atof(row[7]));
			entry.taunting = atoi(row[8]);

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

#endif //EQEMU_BASE_CHARACTER_PET_INFO_REPOSITORY_H
