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

#ifndef EQEMU_BASE_CHARACTER_ALT_CURRENCY_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_ALT_CURRENCY_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterAltCurrencyRepository {
public:
	struct CharacterAltCurrency {
		int char_id;
		int currency_id;
		int amount;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"currency_id",
			"amount",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_alt_currency");
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

	static CharacterAltCurrency NewEntity()
	{
		CharacterAltCurrency entry{};

		entry.char_id     = 0;
		entry.currency_id = 0;
		entry.amount      = 0;

		return entry;
	}

	static CharacterAltCurrency GetCharacterAltCurrencyEntry(
		const std::vector<CharacterAltCurrency> &character_alt_currencys,
		int character_alt_currency_id
	)
	{
		for (auto &character_alt_currency : character_alt_currencys) {
			if (character_alt_currency.char_id == character_alt_currency_id) {
				return character_alt_currency;
			}
		}

		return NewEntity();
	}

	static CharacterAltCurrency FindOne(
		Database& db,
		int character_alt_currency_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_alt_currency_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterAltCurrency entry{};

			entry.char_id     = atoi(row[0]);
			entry.currency_id = atoi(row[1]);
			entry.amount      = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_alt_currency_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_alt_currency_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterAltCurrency character_alt_currency_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_alt_currency_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_alt_currency_entry.currency_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_alt_currency_entry.amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_alt_currency_entry.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterAltCurrency InsertOne(
		Database& db,
		CharacterAltCurrency character_alt_currency_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_alt_currency_entry.char_id));
		insert_values.push_back(std::to_string(character_alt_currency_entry.currency_id));
		insert_values.push_back(std::to_string(character_alt_currency_entry.amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_alt_currency_entry.char_id = results.LastInsertedID();
			return character_alt_currency_entry;
		}

		character_alt_currency_entry = NewEntity();

		return character_alt_currency_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterAltCurrency> character_alt_currency_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_alt_currency_entry: character_alt_currency_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_alt_currency_entry.char_id));
			insert_values.push_back(std::to_string(character_alt_currency_entry.currency_id));
			insert_values.push_back(std::to_string(character_alt_currency_entry.amount));

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

	static std::vector<CharacterAltCurrency> All(Database& db)
	{
		std::vector<CharacterAltCurrency> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAltCurrency entry{};

			entry.char_id     = atoi(row[0]);
			entry.currency_id = atoi(row[1]);
			entry.amount      = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterAltCurrency> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterAltCurrency> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterAltCurrency entry{};

			entry.char_id     = atoi(row[0]);
			entry.currency_id = atoi(row[1]);
			entry.amount      = atoi(row[2]);

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

#endif //EQEMU_BASE_CHARACTER_ALT_CURRENCY_REPOSITORY_H
