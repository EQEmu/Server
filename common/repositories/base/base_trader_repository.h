/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_TRADER_REPOSITORY_H
#define EQEMU_BASE_TRADER_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseTraderRepository {
public:
	struct Trader {
		uint64_t    id;
		uint32_t    character_id;
		uint32_t    item_id;
		std::string item_unique_id;
		uint32_t    augment_one;
		uint32_t    augment_two;
		uint32_t    augment_three;
		uint32_t    augment_four;
		uint32_t    augment_five;
		uint32_t    augment_six;
		int32_t     item_charges;
		uint32_t    item_cost;
		uint8_t     slot_id;
		uint32_t    char_entity_id;
		uint32_t    char_zone_id;
		int32_t     char_zone_instance_id;
		uint8_t     active_transaction;
		time_t      listing_date;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"character_id",
			"item_id",
			"item_unique_id",
			"augment_one",
			"augment_two",
			"augment_three",
			"augment_four",
			"augment_five",
			"augment_six",
			"item_charges",
			"item_cost",
			"slot_id",
			"char_entity_id",
			"char_zone_id",
			"char_zone_instance_id",
			"active_transaction",
			"listing_date",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"character_id",
			"item_id",
			"item_unique_id",
			"augment_one",
			"augment_two",
			"augment_three",
			"augment_four",
			"augment_five",
			"augment_six",
			"item_charges",
			"item_cost",
			"slot_id",
			"char_entity_id",
			"char_zone_id",
			"char_zone_instance_id",
			"active_transaction",
			"UNIX_TIMESTAMP(listing_date)",
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

		e.id                    = 0;
		e.character_id          = 0;
		e.item_id               = 0;
		e.item_unique_id        = "";
		e.augment_one           = 0;
		e.augment_two           = 0;
		e.augment_three         = 0;
		e.augment_four          = 0;
		e.augment_five          = 0;
		e.augment_six           = 0;
		e.item_charges          = 0;
		e.item_cost             = 0;
		e.slot_id               = 0;
		e.char_entity_id        = 0;
		e.char_zone_id          = 0;
		e.char_zone_instance_id = 0;
		e.active_transaction    = 0;
		e.listing_date          = 0;

		return e;
	}

	static Trader GetTrader(
		const std::vector<Trader> &traders,
		int trader_id
	)
	{
		for (auto &trader : traders) {
			if (trader.id == trader_id) {
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
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				trader_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Trader e{};

			e.id                    = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id               = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_unique_id        = row[3] ? row[3] : "";
			e.augment_one           = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.augment_two           = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_three         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_four          = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_five          = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_six           = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.item_charges          = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.item_cost             = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.slot_id               = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.char_entity_id        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.char_zone_id          = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.char_zone_instance_id = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.active_transaction    = row[16] ? static_cast<uint8_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.listing_date          = strtoll(row[17] ? row[17] : "-1", nullptr, 10);

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

		v.push_back(columns[1] + " = " + std::to_string(e.character_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.item_unique_id) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.augment_one));
		v.push_back(columns[5] + " = " + std::to_string(e.augment_two));
		v.push_back(columns[6] + " = " + std::to_string(e.augment_three));
		v.push_back(columns[7] + " = " + std::to_string(e.augment_four));
		v.push_back(columns[8] + " = " + std::to_string(e.augment_five));
		v.push_back(columns[9] + " = " + std::to_string(e.augment_six));
		v.push_back(columns[10] + " = " + std::to_string(e.item_charges));
		v.push_back(columns[11] + " = " + std::to_string(e.item_cost));
		v.push_back(columns[12] + " = " + std::to_string(e.slot_id));
		v.push_back(columns[13] + " = " + std::to_string(e.char_entity_id));
		v.push_back(columns[14] + " = " + std::to_string(e.char_zone_id));
		v.push_back(columns[15] + " = " + std::to_string(e.char_zone_instance_id));
		v.push_back(columns[16] + " = " + std::to_string(e.active_transaction));
		v.push_back(columns[17] + " = FROM_UNIXTIME(" + (e.listing_date > 0 ? std::to_string(e.listing_date) : "null") + ")");

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

	static Trader InsertOne(
		Database& db,
		Trader e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back("'" + Strings::Escape(e.item_unique_id) + "'");
		v.push_back(std::to_string(e.augment_one));
		v.push_back(std::to_string(e.augment_two));
		v.push_back(std::to_string(e.augment_three));
		v.push_back(std::to_string(e.augment_four));
		v.push_back(std::to_string(e.augment_five));
		v.push_back(std::to_string(e.augment_six));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.item_cost));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.char_entity_id));
		v.push_back(std::to_string(e.char_zone_id));
		v.push_back(std::to_string(e.char_zone_instance_id));
		v.push_back(std::to_string(e.active_transaction));
		v.push_back("FROM_UNIXTIME(" + (e.listing_date > 0 ? std::to_string(e.listing_date) : "null") + ")");

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
		const std::vector<Trader> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.item_unique_id) + "'");
			v.push_back(std::to_string(e.augment_one));
			v.push_back(std::to_string(e.augment_two));
			v.push_back(std::to_string(e.augment_three));
			v.push_back(std::to_string(e.augment_four));
			v.push_back(std::to_string(e.augment_five));
			v.push_back(std::to_string(e.augment_six));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.item_cost));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.char_entity_id));
			v.push_back(std::to_string(e.char_zone_id));
			v.push_back(std::to_string(e.char_zone_instance_id));
			v.push_back(std::to_string(e.active_transaction));
			v.push_back("FROM_UNIXTIME(" + (e.listing_date > 0 ? std::to_string(e.listing_date) : "null") + ")");

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

			e.id                    = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id               = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_unique_id        = row[3] ? row[3] : "";
			e.augment_one           = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.augment_two           = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_three         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_four          = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_five          = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_six           = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.item_charges          = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.item_cost             = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.slot_id               = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.char_entity_id        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.char_zone_id          = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.char_zone_instance_id = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.active_transaction    = row[16] ? static_cast<uint8_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.listing_date          = strtoll(row[17] ? row[17] : "-1", nullptr, 10);

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

			e.id                    = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id               = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_unique_id        = row[3] ? row[3] : "";
			e.augment_one           = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.augment_two           = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_three         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_four          = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_five          = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_six           = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.item_charges          = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.item_cost             = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.slot_id               = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.char_entity_id        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.char_zone_id          = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.char_zone_instance_id = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.active_transaction    = row[16] ? static_cast<uint8_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.listing_date          = strtoll(row[17] ? row[17] : "-1", nullptr, 10);

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

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const Trader &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back("'" + Strings::Escape(e.item_unique_id) + "'");
		v.push_back(std::to_string(e.augment_one));
		v.push_back(std::to_string(e.augment_two));
		v.push_back(std::to_string(e.augment_three));
		v.push_back(std::to_string(e.augment_four));
		v.push_back(std::to_string(e.augment_five));
		v.push_back(std::to_string(e.augment_six));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.item_cost));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.char_entity_id));
		v.push_back(std::to_string(e.char_zone_id));
		v.push_back(std::to_string(e.char_zone_instance_id));
		v.push_back(std::to_string(e.active_transaction));
		v.push_back("FROM_UNIXTIME(" + (e.listing_date > 0 ? std::to_string(e.listing_date) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<Trader> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.item_unique_id) + "'");
			v.push_back(std::to_string(e.augment_one));
			v.push_back(std::to_string(e.augment_two));
			v.push_back(std::to_string(e.augment_three));
			v.push_back(std::to_string(e.augment_four));
			v.push_back(std::to_string(e.augment_five));
			v.push_back(std::to_string(e.augment_six));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.item_cost));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.char_entity_id));
			v.push_back(std::to_string(e.char_zone_id));
			v.push_back(std::to_string(e.char_zone_instance_id));
			v.push_back(std::to_string(e.active_transaction));
			v.push_back("FROM_UNIXTIME(" + (e.listing_date > 0 ? std::to_string(e.listing_date) : "null") + ")");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_BASE_TRADER_REPOSITORY_H
