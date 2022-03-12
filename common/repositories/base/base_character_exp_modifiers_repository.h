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

#ifndef EQEMU_BASE_CHARACTER_EXP_MODIFIERS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_EXP_MODIFIERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseCharacterExpModifiersRepository {
public:
	struct CharacterExpModifiers {
		int   character_id;
		int   zone_id;
		float aa_modifier;
		float exp_modifier;
	};

	static std::string PrimaryKey()
	{
		return std::string("character_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"character_id",
			"zone_id",
			"aa_modifier",
			"exp_modifier",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"character_id",
			"zone_id",
			"aa_modifier",
			"exp_modifier",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("character_exp_modifiers");
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

	static CharacterExpModifiers NewEntity()
	{
		CharacterExpModifiers entry{};

		entry.character_id = 0;
		entry.zone_id      = 0;
		entry.aa_modifier  = 0;
		entry.exp_modifier = 0;

		return entry;
	}

	static CharacterExpModifiers GetCharacterExpModifiersEntry(
		const std::vector<CharacterExpModifiers> &character_exp_modifierss,
		int character_exp_modifiers_id
	)
	{
		for (auto &character_exp_modifiers : character_exp_modifierss) {
			if (character_exp_modifiers.character_id == character_exp_modifiers_id) {
				return character_exp_modifiers;
			}
		}

		return NewEntity();
	}

	static CharacterExpModifiers FindOne(
		Database& db,
		int character_exp_modifiers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_exp_modifiers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterExpModifiers entry{};

			entry.character_id = atoi(row[0]);
			entry.zone_id      = atoi(row[1]);
			entry.aa_modifier  = static_cast<float>(atof(row[2]));
			entry.exp_modifier = static_cast<float>(atof(row[3]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_exp_modifiers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_exp_modifiers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterExpModifiers character_exp_modifiers_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_exp_modifiers_entry.character_id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_exp_modifiers_entry.zone_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_exp_modifiers_entry.aa_modifier));
		update_values.push_back(columns[3] + " = " + std::to_string(character_exp_modifiers_entry.exp_modifier));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_exp_modifiers_entry.character_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterExpModifiers InsertOne(
		Database& db,
		CharacterExpModifiers character_exp_modifiers_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_exp_modifiers_entry.character_id));
		insert_values.push_back(std::to_string(character_exp_modifiers_entry.zone_id));
		insert_values.push_back(std::to_string(character_exp_modifiers_entry.aa_modifier));
		insert_values.push_back(std::to_string(character_exp_modifiers_entry.exp_modifier));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_exp_modifiers_entry.character_id = results.LastInsertedID();
			return character_exp_modifiers_entry;
		}

		character_exp_modifiers_entry = NewEntity();

		return character_exp_modifiers_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterExpModifiers> character_exp_modifiers_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_exp_modifiers_entry: character_exp_modifiers_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_exp_modifiers_entry.character_id));
			insert_values.push_back(std::to_string(character_exp_modifiers_entry.zone_id));
			insert_values.push_back(std::to_string(character_exp_modifiers_entry.aa_modifier));
			insert_values.push_back(std::to_string(character_exp_modifiers_entry.exp_modifier));

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

	static std::vector<CharacterExpModifiers> All(Database& db)
	{
		std::vector<CharacterExpModifiers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterExpModifiers entry{};

			entry.character_id = atoi(row[0]);
			entry.zone_id      = atoi(row[1]);
			entry.aa_modifier  = static_cast<float>(atof(row[2]));
			entry.exp_modifier = static_cast<float>(atof(row[3]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterExpModifiers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterExpModifiers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterExpModifiers entry{};

			entry.character_id = atoi(row[0]);
			entry.zone_id      = atoi(row[1]);
			entry.aa_modifier  = static_cast<float>(atof(row[2]));
			entry.exp_modifier = static_cast<float>(atof(row[3]));

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

#endif //EQEMU_BASE_CHARACTER_EXP_MODIFIERS_REPOSITORY_H
