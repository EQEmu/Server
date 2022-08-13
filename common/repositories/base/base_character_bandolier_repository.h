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
#include "../../strings.h"
#include <ctime>

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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("character_bandolier");
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

	static CharacterBandolier NewEntity()
	{
		CharacterBandolier e{};

		e.id             = 0;
		e.bandolier_id   = 0;
		e.bandolier_slot = 0;
		e.item_id        = 0;
		e.icon           = 0;
		e.bandolier_name = "0";

		return e;
	}

	static CharacterBandolier GetCharacterBandoliere(
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
			CharacterBandolier e{};

			e.id             = atoi(row[0]);
			e.bandolier_id   = atoi(row[1]);
			e.bandolier_slot = atoi(row[2]);
			e.item_id        = atoi(row[3]);
			e.icon           = atoi(row[4]);
			e.bandolier_name = row[5] ? row[5] : "";

			return e;
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
		CharacterBandolier character_bandolier_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_bandolier_e.id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_bandolier_e.bandolier_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_bandolier_e.bandolier_slot));
		update_values.push_back(columns[3] + " = " + std::to_string(character_bandolier_e.item_id));
		update_values.push_back(columns[4] + " = " + std::to_string(character_bandolier_e.icon));
		update_values.push_back(columns[5] + " = '" + Strings::Escape(character_bandolier_e.bandolier_name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				character_bandolier_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterBandolier InsertOne(
		Database& db,
		CharacterBandolier character_bandolier_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_bandolier_e.id));
		insert_values.push_back(std::to_string(character_bandolier_e.bandolier_id));
		insert_values.push_back(std::to_string(character_bandolier_e.bandolier_slot));
		insert_values.push_back(std::to_string(character_bandolier_e.item_id));
		insert_values.push_back(std::to_string(character_bandolier_e.icon));
		insert_values.push_back("'" + Strings::Escape(character_bandolier_e.bandolier_name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_bandolier_e.id = results.LastInsertedID();
			return character_bandolier_e;
		}

		character_bandolier_e = NewEntity();

		return character_bandolier_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterBandolier> character_bandolier_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_bandolier_e: character_bandolier_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_bandolier_e.id));
			insert_values.push_back(std::to_string(character_bandolier_e.bandolier_id));
			insert_values.push_back(std::to_string(character_bandolier_e.bandolier_slot));
			insert_values.push_back(std::to_string(character_bandolier_e.item_id));
			insert_values.push_back(std::to_string(character_bandolier_e.icon));
			insert_values.push_back("'" + Strings::Escape(character_bandolier_e.bandolier_name) + "'");

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
			CharacterBandolier e{};

			e.id             = atoi(row[0]);
			e.bandolier_id   = atoi(row[1]);
			e.bandolier_slot = atoi(row[2]);
			e.item_id        = atoi(row[3]);
			e.icon           = atoi(row[4]);
			e.bandolier_name = row[5] ? row[5] : "";

			all_entries.push_back(e);
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
			CharacterBandolier e{};

			e.id             = atoi(row[0]);
			e.bandolier_id   = atoi(row[1]);
			e.bandolier_slot = atoi(row[2]);
			e.item_id        = atoi(row[3]);
			e.icon           = atoi(row[4]);
			e.bandolier_name = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_CHARACTER_BANDOLIER_REPOSITORY_H
