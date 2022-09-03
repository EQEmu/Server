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

#ifndef EQEMU_BASE_CHARACTER_ALTERNATE_ABILITIES_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_ALTERNATE_ABILITIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterAlternateAbilitiesRepository {
public:
	struct CharacterAlternateAbilities {
		uint32_t id;
		uint16_t aa_id;
		uint16_t aa_value;
		uint16_t charges;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"aa_id",
			"aa_value",
			"charges",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"aa_id",
			"aa_value",
			"charges",
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
		return std::string("character_alternate_abilities");
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

	static CharacterAlternateAbilities NewEntity()
	{
		CharacterAlternateAbilities e{};

		e.id       = 0;
		e.aa_id    = 0;
		e.aa_value = 0;
		e.charges  = 0;

		return e;
	}

	static CharacterAlternateAbilities GetCharacterAlternateAbilities(
		const std::vector<CharacterAlternateAbilities> &character_alternate_abilitiess,
		int character_alternate_abilities_id
	)
	{
		for (auto &character_alternate_abilities : character_alternate_abilitiess) {
			if (character_alternate_abilities.id == character_alternate_abilities_id) {
				return character_alternate_abilities;
			}
		}

		return NewEntity();
	}

	static CharacterAlternateAbilities FindOne(
		Database& db,
		int character_alternate_abilities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_alternate_abilities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterAlternateAbilities e{};

			e.id       = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.aa_id    = static_cast<uint16_t>(strtoul(row[1], nullptr, 10));
			e.aa_value = static_cast<uint16_t>(strtoul(row[2], nullptr, 10));
			e.charges  = static_cast<uint16_t>(strtoul(row[3], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_alternate_abilities_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_alternate_abilities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterAlternateAbilities &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.aa_id));
		v.push_back(columns[2] + " = " + std::to_string(e.aa_value));
		v.push_back(columns[3] + " = " + std::to_string(e.charges));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterAlternateAbilities InsertOne(
		Database& db,
		CharacterAlternateAbilities e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.aa_id));
		v.push_back(std::to_string(e.aa_value));
		v.push_back(std::to_string(e.charges));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterAlternateAbilities> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.aa_id));
			v.push_back(std::to_string(e.aa_value));
			v.push_back(std::to_string(e.charges));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<CharacterAlternateAbilities> All(Database& db)
	{
		std::vector<CharacterAlternateAbilities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAlternateAbilities e{};

			e.id       = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.aa_id    = static_cast<uint16_t>(strtoul(row[1], nullptr, 10));
			e.aa_value = static_cast<uint16_t>(strtoul(row[2], nullptr, 10));
			e.charges  = static_cast<uint16_t>(strtoul(row[3], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterAlternateAbilities> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterAlternateAbilities> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAlternateAbilities e{};

			e.id       = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.aa_id    = static_cast<uint16_t>(strtoul(row[1], nullptr, 10));
			e.aa_value = static_cast<uint16_t>(strtoul(row[2], nullptr, 10));
			e.charges  = static_cast<uint16_t>(strtoul(row[3], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_CHARACTER_ALTERNATE_ABILITIES_REPOSITORY_H
