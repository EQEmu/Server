/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
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

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
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
			InsertColumnsRaw()
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
		int character_alt_currency_id
	)
	{
		auto results = database.QueryDatabase(
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
		int character_alt_currency_id
	)
	{
		auto results = database.QueryDatabase(
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
		CharacterAltCurrency character_alt_currency_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(character_alt_currency_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(character_alt_currency_entry.currency_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_alt_currency_entry.amount));

		auto results = database.QueryDatabase(
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
		CharacterAltCurrency character_alt_currency_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_alt_currency_entry.char_id));
		insert_values.push_back(std::to_string(character_alt_currency_entry.currency_id));
		insert_values.push_back(std::to_string(character_alt_currency_entry.amount));

		auto results = database.QueryDatabase(
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

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<CharacterAltCurrency> All()
	{
		std::vector<CharacterAltCurrency> all_entries;

		auto results = database.QueryDatabase(
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

	static std::vector<CharacterAltCurrency> GetWhere(std::string where_filter)
	{
		std::vector<CharacterAltCurrency> all_entries;

		auto results = database.QueryDatabase(
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

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_CHARACTER_ALT_CURRENCY_REPOSITORY_H
