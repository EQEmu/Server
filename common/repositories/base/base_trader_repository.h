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
#include "../../strings.h"
#include <ctime>

class BaseTraderRepository {
public:
	struct Trader {
		uint32_t char_id;
		uint32_t item_id;
		uint32_t serialnumber;
		int32_t  charges;
		uint32_t item_cost;
		uint8_t  slot_id;
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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("trader");
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

	static Trader NewEntity()
	{
		Trader e{};

		e.char_id      = 0;
		e.item_id      = 0;
		e.serialnumber = 0;
		e.charges      = 0;
		e.item_cost    = 0;
		e.slot_id      = 0;

		return e;
	}

	static Trader GetTrader(
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
			Trader e{};

			e.char_id      = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.item_id      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.serialnumber = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.charges      = static_cast<int32_t>(atoi(row[3]));
			e.item_cost    = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.slot_id      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));

			return e;
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
		const Trader &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.char_id));
		v.push_back(columns[1] + " = " + std::to_string(e.item_id));
		v.push_back(columns[2] + " = " + std::to_string(e.serialnumber));
		v.push_back(columns[3] + " = " + std::to_string(e.charges));
		v.push_back(columns[4] + " = " + std::to_string(e.item_cost));
		v.push_back(columns[5] + " = " + std::to_string(e.slot_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Trader InsertOne(
		Database& db,
		Trader e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.serialnumber));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.item_cost));
		v.push_back(std::to_string(e.slot_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.char_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Trader> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.char_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.serialnumber));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.item_cost));
			v.push_back(std::to_string(e.slot_id));

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
			Trader e{};

			e.char_id      = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.item_id      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.serialnumber = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.charges      = static_cast<int32_t>(atoi(row[3]));
			e.item_cost    = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.slot_id      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Trader> GetWhere(Database& db, const std::string &where_filter)
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
			Trader e{};

			e.char_id      = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.item_id      = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.serialnumber = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.charges      = static_cast<int32_t>(atoi(row[3]));
			e.item_cost    = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.slot_id      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));

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

#endif //EQEMU_BASE_TRADER_REPOSITORY_H
