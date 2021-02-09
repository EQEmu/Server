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

#ifndef EQEMU_BASE_CHARACTER_BANDOLIER_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_BANDOLIER_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterBandolierRepository {
public:
	struct CharacterBandolier {
		int         id;
		int         bandolier_id;
		int         bandolier_slot;
		int         item_id;
		int         icon;
		std::string bandolier_name;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"bandolier_id",
			"bandolier_slot",
			"item_id",
			"icon",
			"bandolier_name",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_bandolier");
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

	static CharacterBandolier NewEntity()
	{
		CharacterBandolier entry{};

		entry.id             = 0;
		entry.bandolier_id   = 0;
		entry.bandolier_slot = 0;
		entry.item_id        = 0;
		entry.icon           = 0;
		entry.bandolier_name = "0";

		return entry;
	}

	static CharacterBandolier GetCharacterBandolierEntry(
		const std::vector<CharacterBandolier> &character_bandoliers,
		int character_bandolier_id
	)
	{
		for (auto &character_bandolier : character_bandoliers) {
			if (character_bandolier.id == character_bandolier_id) {
				return character_bandolier;
			}
		}

		return NewEntity();
	}

	static CharacterBandolier FindOne(
		Database& db,
		int character_bandolier_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_bandolier_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterBandolier entry{};

			entry.id             = atoi(row[0]);
			entry.bandolier_id   = atoi(row[1]);
			entry.bandolier_slot = atoi(row[2]);
			entry.item_id        = atoi(row[3]);
			entry.icon           = atoi(row[4]);
			entry.bandolier_name = row[5] ? row[5] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_bandolier_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_bandolier_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterBandolier character_bandolier_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_bandolier_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_bandolier_entry.bandolier_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_bandolier_entry.bandolier_slot));
		update_values.push_back(columns[3] + " = " + std::to_string(character_bandolier_entry.item_id));
		update_values.push_back(columns[4] + " = " + std::to_string(character_bandolier_entry.icon));
		update_values.push_back(columns[5] + " = '" + EscapeString(character_bandolier_entry.bandolier_name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_bandolier_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterBandolier InsertOne(
		Database& db,
		CharacterBandolier character_bandolier_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_bandolier_entry.id));
		insert_values.push_back(std::to_string(character_bandolier_entry.bandolier_id));
		insert_values.push_back(std::to_string(character_bandolier_entry.bandolier_slot));
		insert_values.push_back(std::to_string(character_bandolier_entry.item_id));
		insert_values.push_back(std::to_string(character_bandolier_entry.icon));
		insert_values.push_back("'" + EscapeString(character_bandolier_entry.bandolier_name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_bandolier_entry.id = results.LastInsertedID();
			return character_bandolier_entry;
		}

		character_bandolier_entry = NewEntity();

		return character_bandolier_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterBandolier> character_bandolier_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_bandolier_entry: character_bandolier_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_bandolier_entry.id));
			insert_values.push_back(std::to_string(character_bandolier_entry.bandolier_id));
			insert_values.push_back(std::to_string(character_bandolier_entry.bandolier_slot));
			insert_values.push_back(std::to_string(character_bandolier_entry.item_id));
			insert_values.push_back(std::to_string(character_bandolier_entry.icon));
			insert_values.push_back("'" + EscapeString(character_bandolier_entry.bandolier_name) + "'");

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

	static std::vector<CharacterBandolier> All(Database& db)
	{
		std::vector<CharacterBandolier> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterBandolier entry{};

			entry.id             = atoi(row[0]);
			entry.bandolier_id   = atoi(row[1]);
			entry.bandolier_slot = atoi(row[2]);
			entry.item_id        = atoi(row[3]);
			entry.icon           = atoi(row[4]);
			entry.bandolier_name = row[5] ? row[5] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterBandolier> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterBandolier> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterBandolier entry{};

			entry.id             = atoi(row[0]);
			entry.bandolier_id   = atoi(row[1]);
			entry.bandolier_slot = atoi(row[2]);
			entry.item_id        = atoi(row[3]);
			entry.icon           = atoi(row[4]);
			entry.bandolier_name = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_CHARACTER_BANDOLIER_REPOSITORY_H
