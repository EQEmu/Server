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

#ifndef EQEMU_BASE_CHARACTER_LEADERSHIP_ABILITIES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_LEADERSHIP_ABILITIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterLeadershipAbilitiesRepository {
public:
	struct CharacterLeadershipAbilities {
		int id;
		int slot;
		int rank;
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
			"rank",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_leadership_abilities");
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

	static CharacterLeadershipAbilities NewEntity()
	{
		CharacterLeadershipAbilities entry{};

		entry.id   = 0;
		entry.slot = 0;
		entry.rank = 0;

		return entry;
	}

	static CharacterLeadershipAbilities GetCharacterLeadershipAbilitiesEntry(
		const std::vector<CharacterLeadershipAbilities> &character_leadership_abilitiess,
		int character_leadership_abilities_id
	)
	{
		for (auto &character_leadership_abilities : character_leadership_abilitiess) {
			if (character_leadership_abilities.id == character_leadership_abilities_id) {
				return character_leadership_abilities;
			}
		}

		return NewEntity();
	}

	static CharacterLeadershipAbilities FindOne(
		Database& db,
		int character_leadership_abilities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_leadership_abilities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterLeadershipAbilities entry{};

			entry.id   = atoi(row[0]);
			entry.slot = atoi(row[1]);
			entry.rank = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_leadership_abilities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_leadership_abilities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterLeadershipAbilities character_leadership_abilities_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_leadership_abilities_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_leadership_abilities_entry.slot));
		update_values.push_back(columns[2] + " = " + std::to_string(character_leadership_abilities_entry.rank));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_leadership_abilities_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterLeadershipAbilities InsertOne(
		Database& db,
		CharacterLeadershipAbilities character_leadership_abilities_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_leadership_abilities_entry.id));
		insert_values.push_back(std::to_string(character_leadership_abilities_entry.slot));
		insert_values.push_back(std::to_string(character_leadership_abilities_entry.rank));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_leadership_abilities_entry.id = results.LastInsertedID();
			return character_leadership_abilities_entry;
		}

		character_leadership_abilities_entry = NewEntity();

		return character_leadership_abilities_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterLeadershipAbilities> character_leadership_abilities_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_leadership_abilities_entry: character_leadership_abilities_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_leadership_abilities_entry.id));
			insert_values.push_back(std::to_string(character_leadership_abilities_entry.slot));
			insert_values.push_back(std::to_string(character_leadership_abilities_entry.rank));

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

	static std::vector<CharacterLeadershipAbilities> All(Database& db)
	{
		std::vector<CharacterLeadershipAbilities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterLeadershipAbilities entry{};

			entry.id   = atoi(row[0]);
			entry.slot = atoi(row[1]);
			entry.rank = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterLeadershipAbilities> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterLeadershipAbilities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterLeadershipAbilities entry{};

			entry.id   = atoi(row[0]);
			entry.slot = atoi(row[1]);
			entry.rank = atoi(row[2]);

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

#endif //EQEMU_BASE_CHARACTER_LEADERSHIP_ABILITIES_REPOSITORY_H
