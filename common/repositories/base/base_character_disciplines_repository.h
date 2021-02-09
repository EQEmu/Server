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

#ifndef EQEMU_BASE_CHARACTER_DISCIPLINES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_DISCIPLINES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterDisciplinesRepository {
public:
	struct CharacterDisciplines {
		int id;
		int slot_id;
		int disc_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"slot_id",
			"disc_id",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_disciplines");
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

	static CharacterDisciplines NewEntity()
	{
		CharacterDisciplines entry{};

		entry.id      = 0;
		entry.slot_id = 0;
		entry.disc_id = 0;

		return entry;
	}

	static CharacterDisciplines GetCharacterDisciplinesEntry(
		const std::vector<CharacterDisciplines> &character_discipliness,
		int character_disciplines_id
	)
	{
		for (auto &character_disciplines : character_discipliness) {
			if (character_disciplines.id == character_disciplines_id) {
				return character_disciplines;
			}
		}

		return NewEntity();
	}

	static CharacterDisciplines FindOne(
		Database& db,
		int character_disciplines_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_disciplines_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterDisciplines entry{};

			entry.id      = atoi(row[0]);
			entry.slot_id = atoi(row[1]);
			entry.disc_id = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_disciplines_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_disciplines_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterDisciplines character_disciplines_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_disciplines_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_disciplines_entry.slot_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_disciplines_entry.disc_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_disciplines_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterDisciplines InsertOne(
		Database& db,
		CharacterDisciplines character_disciplines_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_disciplines_entry.id));
		insert_values.push_back(std::to_string(character_disciplines_entry.slot_id));
		insert_values.push_back(std::to_string(character_disciplines_entry.disc_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_disciplines_entry.id = results.LastInsertedID();
			return character_disciplines_entry;
		}

		character_disciplines_entry = NewEntity();

		return character_disciplines_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterDisciplines> character_disciplines_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_disciplines_entry: character_disciplines_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_disciplines_entry.id));
			insert_values.push_back(std::to_string(character_disciplines_entry.slot_id));
			insert_values.push_back(std::to_string(character_disciplines_entry.disc_id));

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

	static std::vector<CharacterDisciplines> All(Database& db)
	{
		std::vector<CharacterDisciplines> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterDisciplines entry{};

			entry.id      = atoi(row[0]);
			entry.slot_id = atoi(row[1]);
			entry.disc_id = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterDisciplines> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterDisciplines> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterDisciplines entry{};

			entry.id      = atoi(row[0]);
			entry.slot_id = atoi(row[1]);
			entry.disc_id = atoi(row[2]);

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

#endif //EQEMU_BASE_CHARACTER_DISCIPLINES_REPOSITORY_H
