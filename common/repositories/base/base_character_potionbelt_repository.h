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

#ifndef EQEMU_BASE_CHARACTER_POTIONBELT_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_POTIONBELT_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterPotionbeltRepository {
public:
	struct CharacterPotionbelt {
		uint32_t id;
		uint8_t  potion_id;
		uint32_t item_id;
		uint32_t icon;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"potion_id",
			"item_id",
			"icon",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"potion_id",
			"item_id",
			"icon",
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
		return std::string("character_potionbelt");
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

	static CharacterPotionbelt NewEntity()
	{
		CharacterPotionbelt e{};

		e.id        = 0;
		e.potion_id = 0;
		e.item_id   = 0;
		e.icon      = 0;

		return e;
	}

	static CharacterPotionbelt GetCharacterPotionbelt(
		const std::vector<CharacterPotionbelt> &character_potionbelts,
		int character_potionbelt_id
	)
	{
		for (auto &character_potionbelt : character_potionbelts) {
			if (character_potionbelt.id == character_potionbelt_id) {
				return character_potionbelt;
			}
		}

		return NewEntity();
	}

	static CharacterPotionbelt FindOne(
		Database& db,
		int character_potionbelt_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_potionbelt_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterPotionbelt e{};

			e.id        = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.potion_id = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.item_id   = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.icon      = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_potionbelt_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_potionbelt_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterPotionbelt &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.potion_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.icon));

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

	static CharacterPotionbelt InsertOne(
		Database& db,
		CharacterPotionbelt e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.potion_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.icon));

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
		const std::vector<CharacterPotionbelt> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.potion_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.icon));

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

	static std::vector<CharacterPotionbelt> All(Database& db)
	{
		std::vector<CharacterPotionbelt> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPotionbelt e{};

			e.id        = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.potion_id = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.item_id   = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.icon      = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterPotionbelt> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterPotionbelt> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterPotionbelt e{};

			e.id        = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.potion_id = static_cast<uint8_t>(strtoul(row[1], nullptr, 10));
			e.item_id   = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.icon      = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));

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

#endif //EQEMU_BASE_CHARACTER_POTIONBELT_REPOSITORY_H
