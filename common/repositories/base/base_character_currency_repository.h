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

#ifndef EQEMU_BASE_CHARACTER_CURRENCY_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_CURRENCY_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterCurrencyRepository {
public:
	struct CharacterCurrency {
		int id;
		int platinum;
		int gold;
		int silver;
		int copper;
		int platinum_bank;
		int gold_bank;
		int silver_bank;
		int copper_bank;
		int platinum_cursor;
		int gold_cursor;
		int silver_cursor;
		int copper_cursor;
		int radiant_crystals;
		int career_radiant_crystals;
		int ebon_crystals;
		int career_ebon_crystals;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"platinum",
			"gold",
			"silver",
			"copper",
			"platinum_bank",
			"gold_bank",
			"silver_bank",
			"copper_bank",
			"platinum_cursor",
			"gold_cursor",
			"silver_cursor",
			"copper_cursor",
			"radiant_crystals",
			"career_radiant_crystals",
			"ebon_crystals",
			"career_ebon_crystals",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"platinum",
			"gold",
			"silver",
			"copper",
			"platinum_bank",
			"gold_bank",
			"silver_bank",
			"copper_bank",
			"platinum_cursor",
			"gold_cursor",
			"silver_cursor",
			"copper_cursor",
			"radiant_crystals",
			"career_radiant_crystals",
			"ebon_crystals",
			"career_ebon_crystals",
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
		return std::string("character_currency");
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

	static CharacterCurrency NewEntity()
	{
		CharacterCurrency e{};

		e.id                      = 0;
		e.platinum                = 0;
		e.gold                    = 0;
		e.silver                  = 0;
		e.copper                  = 0;
		e.platinum_bank           = 0;
		e.gold_bank               = 0;
		e.silver_bank             = 0;
		e.copper_bank             = 0;
		e.platinum_cursor         = 0;
		e.gold_cursor             = 0;
		e.silver_cursor           = 0;
		e.copper_cursor           = 0;
		e.radiant_crystals        = 0;
		e.career_radiant_crystals = 0;
		e.ebon_crystals           = 0;
		e.career_ebon_crystals    = 0;

		return e;
	}

	static CharacterCurrency GetCharacterCurrency(
		const std::vector<CharacterCurrency> &character_currencys,
		int character_currency_id
	)
	{
		for (auto &character_currency : character_currencys) {
			if (character_currency.id == character_currency_id) {
				return character_currency;
			}
		}

		return NewEntity();
	}

	static CharacterCurrency FindOne(
		Database& db,
		int character_currency_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_currency_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterCurrency e{};

			e.id                      = atoi(row[0]);
			e.platinum                = atoi(row[1]);
			e.gold                    = atoi(row[2]);
			e.silver                  = atoi(row[3]);
			e.copper                  = atoi(row[4]);
			e.platinum_bank           = atoi(row[5]);
			e.gold_bank               = atoi(row[6]);
			e.silver_bank             = atoi(row[7]);
			e.copper_bank             = atoi(row[8]);
			e.platinum_cursor         = atoi(row[9]);
			e.gold_cursor             = atoi(row[10]);
			e.silver_cursor           = atoi(row[11]);
			e.copper_cursor           = atoi(row[12]);
			e.radiant_crystals        = atoi(row[13]);
			e.career_radiant_crystals = atoi(row[14]);
			e.ebon_crystals           = atoi(row[15]);
			e.career_ebon_crystals    = atoi(row[16]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_currency_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_currency_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterCurrency &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.id));
		v.push_back(columns[1] + " = " + std::to_string(e.platinum));
		v.push_back(columns[2] + " = " + std::to_string(e.gold));
		v.push_back(columns[3] + " = " + std::to_string(e.silver));
		v.push_back(columns[4] + " = " + std::to_string(e.copper));
		v.push_back(columns[5] + " = " + std::to_string(e.platinum_bank));
		v.push_back(columns[6] + " = " + std::to_string(e.gold_bank));
		v.push_back(columns[7] + " = " + std::to_string(e.silver_bank));
		v.push_back(columns[8] + " = " + std::to_string(e.copper_bank));
		v.push_back(columns[9] + " = " + std::to_string(e.platinum_cursor));
		v.push_back(columns[10] + " = " + std::to_string(e.gold_cursor));
		v.push_back(columns[11] + " = " + std::to_string(e.silver_cursor));
		v.push_back(columns[12] + " = " + std::to_string(e.copper_cursor));
		v.push_back(columns[13] + " = " + std::to_string(e.radiant_crystals));
		v.push_back(columns[14] + " = " + std::to_string(e.career_radiant_crystals));
		v.push_back(columns[15] + " = " + std::to_string(e.ebon_crystals));
		v.push_back(columns[16] + " = " + std::to_string(e.career_ebon_crystals));

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

	static CharacterCurrency InsertOne(
		Database& db,
		CharacterCurrency e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.platinum));
		v.push_back(std::to_string(e.gold));
		v.push_back(std::to_string(e.silver));
		v.push_back(std::to_string(e.copper));
		v.push_back(std::to_string(e.platinum_bank));
		v.push_back(std::to_string(e.gold_bank));
		v.push_back(std::to_string(e.silver_bank));
		v.push_back(std::to_string(e.copper_bank));
		v.push_back(std::to_string(e.platinum_cursor));
		v.push_back(std::to_string(e.gold_cursor));
		v.push_back(std::to_string(e.silver_cursor));
		v.push_back(std::to_string(e.copper_cursor));
		v.push_back(std::to_string(e.radiant_crystals));
		v.push_back(std::to_string(e.career_radiant_crystals));
		v.push_back(std::to_string(e.ebon_crystals));
		v.push_back(std::to_string(e.career_ebon_crystals));

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
		const std::vector<CharacterCurrency> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.platinum));
			v.push_back(std::to_string(e.gold));
			v.push_back(std::to_string(e.silver));
			v.push_back(std::to_string(e.copper));
			v.push_back(std::to_string(e.platinum_bank));
			v.push_back(std::to_string(e.gold_bank));
			v.push_back(std::to_string(e.silver_bank));
			v.push_back(std::to_string(e.copper_bank));
			v.push_back(std::to_string(e.platinum_cursor));
			v.push_back(std::to_string(e.gold_cursor));
			v.push_back(std::to_string(e.silver_cursor));
			v.push_back(std::to_string(e.copper_cursor));
			v.push_back(std::to_string(e.radiant_crystals));
			v.push_back(std::to_string(e.career_radiant_crystals));
			v.push_back(std::to_string(e.ebon_crystals));
			v.push_back(std::to_string(e.career_ebon_crystals));

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

	static std::vector<CharacterCurrency> All(Database& db)
	{
		std::vector<CharacterCurrency> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterCurrency e{};

			e.id                      = atoi(row[0]);
			e.platinum                = atoi(row[1]);
			e.gold                    = atoi(row[2]);
			e.silver                  = atoi(row[3]);
			e.copper                  = atoi(row[4]);
			e.platinum_bank           = atoi(row[5]);
			e.gold_bank               = atoi(row[6]);
			e.silver_bank             = atoi(row[7]);
			e.copper_bank             = atoi(row[8]);
			e.platinum_cursor         = atoi(row[9]);
			e.gold_cursor             = atoi(row[10]);
			e.silver_cursor           = atoi(row[11]);
			e.copper_cursor           = atoi(row[12]);
			e.radiant_crystals        = atoi(row[13]);
			e.career_radiant_crystals = atoi(row[14]);
			e.ebon_crystals           = atoi(row[15]);
			e.career_ebon_crystals    = atoi(row[16]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterCurrency> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterCurrency> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterCurrency e{};

			e.id                      = atoi(row[0]);
			e.platinum                = atoi(row[1]);
			e.gold                    = atoi(row[2]);
			e.silver                  = atoi(row[3]);
			e.copper                  = atoi(row[4]);
			e.platinum_bank           = atoi(row[5]);
			e.gold_bank               = atoi(row[6]);
			e.silver_bank             = atoi(row[7]);
			e.copper_bank             = atoi(row[8]);
			e.platinum_cursor         = atoi(row[9]);
			e.gold_cursor             = atoi(row[10]);
			e.silver_cursor           = atoi(row[11]);
			e.copper_cursor           = atoi(row[12]);
			e.radiant_crystals        = atoi(row[13]);
			e.career_radiant_crystals = atoi(row[14]);
			e.ebon_crystals           = atoi(row[15]);
			e.career_ebon_crystals    = atoi(row[16]);

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

#endif //EQEMU_BASE_CHARACTER_CURRENCY_REPOSITORY_H
