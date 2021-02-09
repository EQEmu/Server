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

#ifndef EQEMU_BASE_CHARACTER_SKILLS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_SKILLS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterSkillsRepository {
public:
	struct CharacterSkills {
		int id;
		int skill_id;
		int value;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"skill_id",
			"value",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_skills");
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

	static CharacterSkills NewEntity()
	{
		CharacterSkills entry{};

		entry.id       = 0;
		entry.skill_id = 0;
		entry.value    = 0;

		return entry;
	}

	static CharacterSkills GetCharacterSkillsEntry(
		const std::vector<CharacterSkills> &character_skillss,
		int character_skills_id
	)
	{
		for (auto &character_skills : character_skillss) {
			if (character_skills.id == character_skills_id) {
				return character_skills;
			}
		}

		return NewEntity();
	}

	static CharacterSkills FindOne(
		Database& db,
		int character_skills_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_skills_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterSkills entry{};

			entry.id       = atoi(row[0]);
			entry.skill_id = atoi(row[1]);
			entry.value    = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_skills_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_skills_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterSkills character_skills_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(character_skills_entry.skill_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_skills_entry.value));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_skills_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterSkills InsertOne(
		Database& db,
		CharacterSkills character_skills_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_skills_entry.id));
		insert_values.push_back(std::to_string(character_skills_entry.skill_id));
		insert_values.push_back(std::to_string(character_skills_entry.value));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_skills_entry.id = results.LastInsertedID();
			return character_skills_entry;
		}

		character_skills_entry = NewEntity();

		return character_skills_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterSkills> character_skills_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_skills_entry: character_skills_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_skills_entry.id));
			insert_values.push_back(std::to_string(character_skills_entry.skill_id));
			insert_values.push_back(std::to_string(character_skills_entry.value));

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

	static std::vector<CharacterSkills> All(Database& db)
	{
		std::vector<CharacterSkills> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterSkills entry{};

			entry.id       = atoi(row[0]);
			entry.skill_id = atoi(row[1]);
			entry.value    = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterSkills> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterSkills> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterSkills entry{};

			entry.id       = atoi(row[0]);
			entry.skill_id = atoi(row[1]);
			entry.value    = atoi(row[2]);

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

#endif //EQEMU_BASE_CHARACTER_SKILLS_REPOSITORY_H
