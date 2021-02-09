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

#ifndef EQEMU_BASE_ALTERNATE_CURRENCY_REPOSITORY_H
#define EQEMU_BASE_ALTERNATE_CURRENCY_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAlternateCurrencyRepository {
public:
	struct AlternateCurrency {
		int id;
		int item_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"item_id",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("alternate_currency");
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

	static AlternateCurrency NewEntity()
	{
		AlternateCurrency entry{};

		entry.id      = 0;
		entry.item_id = 0;

		return entry;
	}

	static AlternateCurrency GetAlternateCurrencyEntry(
		const std::vector<AlternateCurrency> &alternate_currencys,
		int alternate_currency_id
	)
	{
		for (auto &alternate_currency : alternate_currencys) {
			if (alternate_currency.id == alternate_currency_id) {
				return alternate_currency;
			}
		}

		return NewEntity();
	}

	static AlternateCurrency FindOne(
		Database& db,
		int alternate_currency_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				alternate_currency_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AlternateCurrency entry{};

			entry.id      = atoi(row[0]);
			entry.item_id = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int alternate_currency_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				alternate_currency_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		AlternateCurrency alternate_currency_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(alternate_currency_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(alternate_currency_entry.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				alternate_currency_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AlternateCurrency InsertOne(
		Database& db,
		AlternateCurrency alternate_currency_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(alternate_currency_entry.id));
		insert_values.push_back(std::to_string(alternate_currency_entry.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			alternate_currency_entry.id = results.LastInsertedID();
			return alternate_currency_entry;
		}

		alternate_currency_entry = NewEntity();

		return alternate_currency_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AlternateCurrency> alternate_currency_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &alternate_currency_entry: alternate_currency_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(alternate_currency_entry.id));
			insert_values.push_back(std::to_string(alternate_currency_entry.item_id));

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

	static std::vector<AlternateCurrency> All(Database& db)
	{
		std::vector<AlternateCurrency> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AlternateCurrency entry{};

			entry.id      = atoi(row[0]);
			entry.item_id = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AlternateCurrency> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<AlternateCurrency> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AlternateCurrency entry{};

			entry.id      = atoi(row[0]);
			entry.item_id = atoi(row[1]);

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

#endif //EQEMU_BASE_ALTERNATE_CURRENCY_REPOSITORY_H
