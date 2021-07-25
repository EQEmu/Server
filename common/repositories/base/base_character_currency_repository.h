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
#include "../../string_util.h"

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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_currency");
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

	static CharacterCurrency NewEntity()
	{
		CharacterCurrency entry{};

		entry.id                      = 0;
		entry.platinum                = 0;
		entry.gold                    = 0;
		entry.silver                  = 0;
		entry.copper                  = 0;
		entry.platinum_bank           = 0;
		entry.gold_bank               = 0;
		entry.silver_bank             = 0;
		entry.copper_bank             = 0;
		entry.platinum_cursor         = 0;
		entry.gold_cursor             = 0;
		entry.silver_cursor           = 0;
		entry.copper_cursor           = 0;
		entry.radiant_crystals        = 0;
		entry.career_radiant_crystals = 0;
		entry.ebon_crystals           = 0;
		entry.career_ebon_crystals    = 0;

		return entry;
	}

	static CharacterCurrency GetCharacterCurrencyEntry(
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
			CharacterCurrency entry{};

			entry.id                      = atoi(row[0]);
			entry.platinum                = atoi(row[1]);
			entry.gold                    = atoi(row[2]);
			entry.silver                  = atoi(row[3]);
			entry.copper                  = atoi(row[4]);
			entry.platinum_bank           = atoi(row[5]);
			entry.gold_bank               = atoi(row[6]);
			entry.silver_bank             = atoi(row[7]);
			entry.copper_bank             = atoi(row[8]);
			entry.platinum_cursor         = atoi(row[9]);
			entry.gold_cursor             = atoi(row[10]);
			entry.silver_cursor           = atoi(row[11]);
			entry.copper_cursor           = atoi(row[12]);
			entry.radiant_crystals        = atoi(row[13]);
			entry.career_radiant_crystals = atoi(row[14]);
			entry.ebon_crystals           = atoi(row[15]);
			entry.career_ebon_crystals    = atoi(row[16]);

			return entry;
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
		CharacterCurrency character_currency_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_currency_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_currency_entry.platinum));
		update_values.push_back(columns[2] + " = " + std::to_string(character_currency_entry.gold));
		update_values.push_back(columns[3] + " = " + std::to_string(character_currency_entry.silver));
		update_values.push_back(columns[4] + " = " + std::to_string(character_currency_entry.copper));
		update_values.push_back(columns[5] + " = " + std::to_string(character_currency_entry.platinum_bank));
		update_values.push_back(columns[6] + " = " + std::to_string(character_currency_entry.gold_bank));
		update_values.push_back(columns[7] + " = " + std::to_string(character_currency_entry.silver_bank));
		update_values.push_back(columns[8] + " = " + std::to_string(character_currency_entry.copper_bank));
		update_values.push_back(columns[9] + " = " + std::to_string(character_currency_entry.platinum_cursor));
		update_values.push_back(columns[10] + " = " + std::to_string(character_currency_entry.gold_cursor));
		update_values.push_back(columns[11] + " = " + std::to_string(character_currency_entry.silver_cursor));
		update_values.push_back(columns[12] + " = " + std::to_string(character_currency_entry.copper_cursor));
		update_values.push_back(columns[13] + " = " + std::to_string(character_currency_entry.radiant_crystals));
		update_values.push_back(columns[14] + " = " + std::to_string(character_currency_entry.career_radiant_crystals));
		update_values.push_back(columns[15] + " = " + std::to_string(character_currency_entry.ebon_crystals));
		update_values.push_back(columns[16] + " = " + std::to_string(character_currency_entry.career_ebon_crystals));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_currency_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterCurrency InsertOne(
		Database& db,
		CharacterCurrency character_currency_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_currency_entry.id));
		insert_values.push_back(std::to_string(character_currency_entry.platinum));
		insert_values.push_back(std::to_string(character_currency_entry.gold));
		insert_values.push_back(std::to_string(character_currency_entry.silver));
		insert_values.push_back(std::to_string(character_currency_entry.copper));
		insert_values.push_back(std::to_string(character_currency_entry.platinum_bank));
		insert_values.push_back(std::to_string(character_currency_entry.gold_bank));
		insert_values.push_back(std::to_string(character_currency_entry.silver_bank));
		insert_values.push_back(std::to_string(character_currency_entry.copper_bank));
		insert_values.push_back(std::to_string(character_currency_entry.platinum_cursor));
		insert_values.push_back(std::to_string(character_currency_entry.gold_cursor));
		insert_values.push_back(std::to_string(character_currency_entry.silver_cursor));
		insert_values.push_back(std::to_string(character_currency_entry.copper_cursor));
		insert_values.push_back(std::to_string(character_currency_entry.radiant_crystals));
		insert_values.push_back(std::to_string(character_currency_entry.career_radiant_crystals));
		insert_values.push_back(std::to_string(character_currency_entry.ebon_crystals));
		insert_values.push_back(std::to_string(character_currency_entry.career_ebon_crystals));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_currency_entry.id = results.LastInsertedID();
			return character_currency_entry;
		}

		character_currency_entry = NewEntity();

		return character_currency_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterCurrency> character_currency_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_currency_entry: character_currency_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_currency_entry.id));
			insert_values.push_back(std::to_string(character_currency_entry.platinum));
			insert_values.push_back(std::to_string(character_currency_entry.gold));
			insert_values.push_back(std::to_string(character_currency_entry.silver));
			insert_values.push_back(std::to_string(character_currency_entry.copper));
			insert_values.push_back(std::to_string(character_currency_entry.platinum_bank));
			insert_values.push_back(std::to_string(character_currency_entry.gold_bank));
			insert_values.push_back(std::to_string(character_currency_entry.silver_bank));
			insert_values.push_back(std::to_string(character_currency_entry.copper_bank));
			insert_values.push_back(std::to_string(character_currency_entry.platinum_cursor));
			insert_values.push_back(std::to_string(character_currency_entry.gold_cursor));
			insert_values.push_back(std::to_string(character_currency_entry.silver_cursor));
			insert_values.push_back(std::to_string(character_currency_entry.copper_cursor));
			insert_values.push_back(std::to_string(character_currency_entry.radiant_crystals));
			insert_values.push_back(std::to_string(character_currency_entry.career_radiant_crystals));
			insert_values.push_back(std::to_string(character_currency_entry.ebon_crystals));
			insert_values.push_back(std::to_string(character_currency_entry.career_ebon_crystals));

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
			CharacterCurrency entry{};

			entry.id                      = atoi(row[0]);
			entry.platinum                = atoi(row[1]);
			entry.gold                    = atoi(row[2]);
			entry.silver                  = atoi(row[3]);
			entry.copper                  = atoi(row[4]);
			entry.platinum_bank           = atoi(row[5]);
			entry.gold_bank               = atoi(row[6]);
			entry.silver_bank             = atoi(row[7]);
			entry.copper_bank             = atoi(row[8]);
			entry.platinum_cursor         = atoi(row[9]);
			entry.gold_cursor             = atoi(row[10]);
			entry.silver_cursor           = atoi(row[11]);
			entry.copper_cursor           = atoi(row[12]);
			entry.radiant_crystals        = atoi(row[13]);
			entry.career_radiant_crystals = atoi(row[14]);
			entry.ebon_crystals           = atoi(row[15]);
			entry.career_ebon_crystals    = atoi(row[16]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterCurrency> GetWhere(Database& db, std::string where_filter)
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
			CharacterCurrency entry{};

			entry.id                      = atoi(row[0]);
			entry.platinum                = atoi(row[1]);
			entry.gold                    = atoi(row[2]);
			entry.silver                  = atoi(row[3]);
			entry.copper                  = atoi(row[4]);
			entry.platinum_bank           = atoi(row[5]);
			entry.gold_bank               = atoi(row[6]);
			entry.silver_bank             = atoi(row[7]);
			entry.copper_bank             = atoi(row[8]);
			entry.platinum_cursor         = atoi(row[9]);
			entry.gold_cursor             = atoi(row[10]);
			entry.silver_cursor           = atoi(row[11]);
			entry.copper_cursor           = atoi(row[12]);
			entry.radiant_crystals        = atoi(row[13]);
			entry.career_radiant_crystals = atoi(row[14]);
			entry.ebon_crystals           = atoi(row[15]);
			entry.career_ebon_crystals    = atoi(row[16]);

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

#endif //EQEMU_BASE_CHARACTER_CURRENCY_REPOSITORY_H
