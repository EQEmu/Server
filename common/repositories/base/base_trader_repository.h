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

#ifndef EQEMU_BASE_TRADER_REPOSITORY_H
#define EQEMU_BASE_TRADER_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTraderRepository {
public:
	struct Trader {
		int char_id;
		int item_id;
		int serialnumber;
		int charges;
		int item_cost;
		int slot_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"item_id",
			"serialnumber",
			"charges",
			"item_cost",
			"slot_id",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("trader");
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

	static Trader NewEntity()
	{
		Trader entry{};

		entry.char_id      = 0;
		entry.item_id      = 0;
		entry.serialnumber = 0;
		entry.charges      = 0;
		entry.item_cost    = 0;
		entry.slot_id      = 0;

		return entry;
	}

	static Trader GetTraderEntry(
		const std::vector<Trader> &traders,
		int trader_id
	)
	{
		for (auto &trader : traders) {
			if (trader.char_id == trader_id) {
				return trader;
			}
		}

		return NewEntity();
	}

	static Trader FindOne(
		Database& db,
		int trader_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				trader_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Trader entry{};

			entry.char_id      = atoi(row[0]);
			entry.item_id      = atoi(row[1]);
			entry.serialnumber = atoi(row[2]);
			entry.charges      = atoi(row[3]);
			entry.item_cost    = atoi(row[4]);
			entry.slot_id      = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int trader_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				trader_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Trader trader_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(trader_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(trader_entry.item_id));
		update_values.push_back(columns[2] + " = " + std::to_string(trader_entry.serialnumber));
		update_values.push_back(columns[3] + " = " + std::to_string(trader_entry.charges));
		update_values.push_back(columns[4] + " = " + std::to_string(trader_entry.item_cost));
		update_values.push_back(columns[5] + " = " + std::to_string(trader_entry.slot_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				trader_entry.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Trader InsertOne(
		Database& db,
		Trader trader_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(trader_entry.char_id));
		insert_values.push_back(std::to_string(trader_entry.item_id));
		insert_values.push_back(std::to_string(trader_entry.serialnumber));
		insert_values.push_back(std::to_string(trader_entry.charges));
		insert_values.push_back(std::to_string(trader_entry.item_cost));
		insert_values.push_back(std::to_string(trader_entry.slot_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			trader_entry.char_id = results.LastInsertedID();
			return trader_entry;
		}

		trader_entry = NewEntity();

		return trader_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Trader> trader_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &trader_entry: trader_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(trader_entry.char_id));
			insert_values.push_back(std::to_string(trader_entry.item_id));
			insert_values.push_back(std::to_string(trader_entry.serialnumber));
			insert_values.push_back(std::to_string(trader_entry.charges));
			insert_values.push_back(std::to_string(trader_entry.item_cost));
			insert_values.push_back(std::to_string(trader_entry.slot_id));

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

	static std::vector<Trader> All(Database& db)
	{
		std::vector<Trader> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Trader entry{};

			entry.char_id      = atoi(row[0]);
			entry.item_id      = atoi(row[1]);
			entry.serialnumber = atoi(row[2]);
			entry.charges      = atoi(row[3]);
			entry.item_cost    = atoi(row[4]);
			entry.slot_id      = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Trader> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Trader> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Trader entry{};

			entry.char_id      = atoi(row[0]);
			entry.item_id      = atoi(row[1]);
			entry.serialnumber = atoi(row[2]);
			entry.charges      = atoi(row[3]);
			entry.item_cost    = atoi(row[4]);
			entry.slot_id      = atoi(row[5]);

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

#endif //EQEMU_BASE_TRADER_REPOSITORY_H
