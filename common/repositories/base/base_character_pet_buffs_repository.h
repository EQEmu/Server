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

#ifndef EQEMU_BASE_CHARACTER_PET_BUFFS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_PET_BUFFS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterPetBuffsRepository {
public:
	struct CharacterPetBuffs {
		int         char_id;
		int         pet;
		int         slot;
		int         spell_id;
		int         caster_level;
		std::string castername;
		int         ticsremaining;
		int         counters;
		int         numhits;
		int         rune;
		int         instrument_mod;
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
			"spell_id",
			"caster_level",
			"castername",
			"ticsremaining",
			"counters",
			"numhits",
			"rune",
			"instrument_mod",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_pet_buffs");
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

	static CharacterPetBuffs NewEntity()
	{
		CharacterPetBuffs entry{};

		entry.char_id        = 0;
		entry.pet            = 0;
		entry.slot           = 0;
		entry.spell_id       = 0;
		entry.caster_level   = 0;
		entry.castername     = "";
		entry.ticsremaining  = 0;
		entry.counters       = 0;
		entry.numhits        = 0;
		entry.rune           = 0;
		entry.instrument_mod = 10;

		return entry;
	}

	static CharacterPetBuffs GetCharacterPetBuffsEntry(
		const std::vector<CharacterPetBuffs> &character_pet_buffss,
		int character_pet_buffs_id
	)
	{
		for (auto &character_pet_buffs : character_pet_buffss) {
			if (character_pet_buffs.char_id == character_pet_buffs_id) {
				return character_pet_buffs;
			}
		}

		return NewEntity();
	}

	static CharacterPetBuffs FindOne(
		Database& db,
		int character_pet_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_pet_buffs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterPetBuffs entry{};

			entry.char_id        = atoi(row[0]);
			entry.pet            = atoi(row[1]);
			entry.slot           = atoi(row[2]);
			entry.spell_id       = atoi(row[3]);
			entry.caster_level   = atoi(row[4]);
			entry.castername     = row[5] ? row[5] : "";
			entry.ticsremaining  = atoi(row[6]);
			entry.counters       = atoi(row[7]);
			entry.numhits        = atoi(row[8]);
			entry.rune           = atoi(row[9]);
			entry.instrument_mod = atoi(row[10]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_pet_buffs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_pet_buffs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterPetBuffs character_pet_buffs_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_pet_buffs_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_pet_buffs_entry.pet));
		update_values.push_back(columns[2] + " = " + std::to_string(character_pet_buffs_entry.slot));
		update_values.push_back(columns[3] + " = " + std::to_string(character_pet_buffs_entry.spell_id));
		update_values.push_back(columns[4] + " = " + std::to_string(character_pet_buffs_entry.caster_level));
		update_values.push_back(columns[5] + " = '" + EscapeString(character_pet_buffs_entry.castername) + "'");
		update_values.push_back(columns[6] + " = " + std::to_string(character_pet_buffs_entry.ticsremaining));
		update_values.push_back(columns[7] + " = " + std::to_string(character_pet_buffs_entry.counters));
		update_values.push_back(columns[8] + " = " + std::to_string(character_pet_buffs_entry.numhits));
		update_values.push_back(columns[9] + " = " + std::to_string(character_pet_buffs_entry.rune));
		update_values.push_back(columns[10] + " = " + std::to_string(character_pet_buffs_entry.instrument_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_pet_buffs_entry.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterPetBuffs InsertOne(
		Database& db,
		CharacterPetBuffs character_pet_buffs_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_pet_buffs_entry.char_id));
		insert_values.push_back(std::to_string(character_pet_buffs_entry.pet));
		insert_values.push_back(std::to_string(character_pet_buffs_entry.slot));
		insert_values.push_back(std::to_string(character_pet_buffs_entry.spell_id));
		insert_values.push_back(std::to_string(character_pet_buffs_entry.caster_level));
		insert_values.push_back("'" + EscapeString(character_pet_buffs_entry.castername) + "'");
		insert_values.push_back(std::to_string(character_pet_buffs_entry.ticsremaining));
		insert_values.push_back(std::to_string(character_pet_buffs_entry.counters));
		insert_values.push_back(std::to_string(character_pet_buffs_entry.numhits));
		insert_values.push_back(std::to_string(character_pet_buffs_entry.rune));
		insert_values.push_back(std::to_string(character_pet_buffs_entry.instrument_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_pet_buffs_entry.char_id = results.LastInsertedID();
			return character_pet_buffs_entry;
		}

		character_pet_buffs_entry = NewEntity();

		return character_pet_buffs_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterPetBuffs> character_pet_buffs_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_pet_buffs_entry: character_pet_buffs_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_pet_buffs_entry.char_id));
			insert_values.push_back(std::to_string(character_pet_buffs_entry.pet));
			insert_values.push_back(std::to_string(character_pet_buffs_entry.slot));
			insert_values.push_back(std::to_string(character_pet_buffs_entry.spell_id));
			insert_values.push_back(std::to_string(character_pet_buffs_entry.caster_level));
			insert_values.push_back("'" + EscapeString(character_pet_buffs_entry.castername) + "'");
			insert_values.push_back(std::to_string(character_pet_buffs_entry.ticsremaining));
			insert_values.push_back(std::to_string(character_pet_buffs_entry.counters));
			insert_values.push_back(std::to_string(character_pet_buffs_entry.numhits));
			insert_values.push_back(std::to_string(character_pet_buffs_entry.rune));
			insert_values.push_back(std::to_string(character_pet_buffs_entry.instrument_mod));

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

	static std::vector<CharacterPetBuffs> All(Database& db)
	{
		std::vector<CharacterPetBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetBuffs entry{};

			entry.char_id        = atoi(row[0]);
			entry.pet            = atoi(row[1]);
			entry.slot           = atoi(row[2]);
			entry.spell_id       = atoi(row[3]);
			entry.caster_level   = atoi(row[4]);
			entry.castername     = row[5] ? row[5] : "";
			entry.ticsremaining  = atoi(row[6]);
			entry.counters       = atoi(row[7]);
			entry.numhits        = atoi(row[8]);
			entry.rune           = atoi(row[9]);
			entry.instrument_mod = atoi(row[10]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterPetBuffs> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterPetBuffs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPetBuffs entry{};

			entry.char_id        = atoi(row[0]);
			entry.pet            = atoi(row[1]);
			entry.slot           = atoi(row[2]);
			entry.spell_id       = atoi(row[3]);
			entry.caster_level   = atoi(row[4]);
			entry.castername     = row[5] ? row[5] : "";
			entry.ticsremaining  = atoi(row[6]);
			entry.counters       = atoi(row[7]);
			entry.numhits        = atoi(row[8]);
			entry.rune           = atoi(row[9]);
			entry.instrument_mod = atoi(row[10]);

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

#endif //EQEMU_BASE_CHARACTER_PET_BUFFS_REPOSITORY_H
