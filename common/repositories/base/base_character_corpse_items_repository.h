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

#ifndef EQEMU_BASE_CHARACTER_CORPSE_ITEMS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_CORPSE_ITEMS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterCorpseItemsRepository {
public:
	struct CharacterCorpseItems {
		int corpse_id;
		int equip_slot;
		int item_id;
		int charges;
		int aug_1;
		int aug_2;
		int aug_3;
		int aug_4;
		int aug_5;
		int aug_6;
		int attuned;
	};

	static std::string PrimaryKey()
	{
		return std::string("corpse_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"corpse_id",
			"equip_slot",
			"item_id",
			"charges",
			"aug_1",
			"aug_2",
			"aug_3",
			"aug_4",
			"aug_5",
			"aug_6",
			"attuned",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"corpse_id",
			"equip_slot",
			"item_id",
			"charges",
			"aug_1",
			"aug_2",
			"aug_3",
			"aug_4",
			"aug_5",
			"aug_6",
			"attuned",
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
		return std::string("character_corpse_items");
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

	static CharacterCorpseItems NewEntity()
	{
		CharacterCorpseItems e{};

		e.corpse_id  = 0;
		e.equip_slot = 0;
		e.item_id    = 0;
		e.charges    = 0;
		e.aug_1      = 0;
		e.aug_2      = 0;
		e.aug_3      = 0;
		e.aug_4      = 0;
		e.aug_5      = 0;
		e.aug_6      = 0;
		e.attuned    = 0;

		return e;
	}

	static CharacterCorpseItems GetCharacterCorpseItems(
		const std::vector<CharacterCorpseItems> &character_corpse_itemss,
		int character_corpse_items_id
	)
	{
		for (auto &character_corpse_items : character_corpse_itemss) {
			if (character_corpse_items.corpse_id == character_corpse_items_id) {
				return character_corpse_items;
			}
		}

		return NewEntity();
	}

	static CharacterCorpseItems FindOne(
		Database& db,
		int character_corpse_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_corpse_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterCorpseItems e{};

			e.corpse_id  = atoi(row[0]);
			e.equip_slot = atoi(row[1]);
			e.item_id    = atoi(row[2]);
			e.charges    = atoi(row[3]);
			e.aug_1      = atoi(row[4]);
			e.aug_2      = atoi(row[5]);
			e.aug_3      = atoi(row[6]);
			e.aug_4      = atoi(row[7]);
			e.aug_5      = atoi(row[8]);
			e.aug_6      = atoi(row[9]);
			e.attuned    = atoi(row[10]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_corpse_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_corpse_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterCorpseItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.corpse_id));
		v.push_back(columns[1] + " = " + std::to_string(e.equip_slot));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.charges));
		v.push_back(columns[4] + " = " + std::to_string(e.aug_1));
		v.push_back(columns[5] + " = " + std::to_string(e.aug_2));
		v.push_back(columns[6] + " = " + std::to_string(e.aug_3));
		v.push_back(columns[7] + " = " + std::to_string(e.aug_4));
		v.push_back(columns[8] + " = " + std::to_string(e.aug_5));
		v.push_back(columns[9] + " = " + std::to_string(e.aug_6));
		v.push_back(columns[10] + " = " + std::to_string(e.attuned));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.corpse_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterCorpseItems InsertOne(
		Database& db,
		CharacterCorpseItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.corpse_id));
		v.push_back(std::to_string(e.equip_slot));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.aug_1));
		v.push_back(std::to_string(e.aug_2));
		v.push_back(std::to_string(e.aug_3));
		v.push_back(std::to_string(e.aug_4));
		v.push_back(std::to_string(e.aug_5));
		v.push_back(std::to_string(e.aug_6));
		v.push_back(std::to_string(e.attuned));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.corpse_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterCorpseItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.corpse_id));
			v.push_back(std::to_string(e.equip_slot));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.aug_1));
			v.push_back(std::to_string(e.aug_2));
			v.push_back(std::to_string(e.aug_3));
			v.push_back(std::to_string(e.aug_4));
			v.push_back(std::to_string(e.aug_5));
			v.push_back(std::to_string(e.aug_6));
			v.push_back(std::to_string(e.attuned));

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

	static std::vector<CharacterCorpseItems> All(Database& db)
	{
		std::vector<CharacterCorpseItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterCorpseItems e{};

			e.corpse_id  = atoi(row[0]);
			e.equip_slot = atoi(row[1]);
			e.item_id    = atoi(row[2]);
			e.charges    = atoi(row[3]);
			e.aug_1      = atoi(row[4]);
			e.aug_2      = atoi(row[5]);
			e.aug_3      = atoi(row[6]);
			e.aug_4      = atoi(row[7]);
			e.aug_5      = atoi(row[8]);
			e.aug_6      = atoi(row[9]);
			e.attuned    = atoi(row[10]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterCorpseItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterCorpseItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterCorpseItems e{};

			e.corpse_id  = atoi(row[0]);
			e.equip_slot = atoi(row[1]);
			e.item_id    = atoi(row[2]);
			e.charges    = atoi(row[3]);
			e.aug_1      = atoi(row[4]);
			e.aug_2      = atoi(row[5]);
			e.aug_3      = atoi(row[6]);
			e.aug_4      = atoi(row[7]);
			e.aug_5      = atoi(row[8]);
			e.aug_6      = atoi(row[9]);
			e.attuned    = atoi(row[10]);

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

#endif //EQEMU_BASE_CHARACTER_CORPSE_ITEMS_REPOSITORY_H
