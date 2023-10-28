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

#ifndef EQEMU_BASE_BOT_STARTING_ITEMS_REPOSITORY_H
#define EQEMU_BASE_BOT_STARTING_ITEMS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>


class BaseBotStartingItemsRepository {
public:
	struct BotStartingItems {
		uint32_t    id;
		uint32_t    races;
		uint32_t    classes;
		uint32_t    item_id;
		uint8_t     item_charges;
		int32_t     slot_id;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"races",
			"classes",
			"item_id",
			"item_charges",
			"slot_id",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"races",
			"classes",
			"item_id",
			"item_charges",
			"slot_id",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("bot_starting_items");
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

	static BotStartingItems NewEntity()
	{
		BotStartingItems e{};

		e.id                     = 0;
		e.races                  = 0;
		e.classes                = 0;
		e.item_id                = 0;
		e.item_charges           = 1;
		e.slot_id                = -1;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static BotStartingItems GetBotStartingItems(
		const std::vector<BotStartingItems> &bot_starting_itemss,
		int bot_starting_items_id
	)
	{
		for (auto &bot_starting_items : bot_starting_itemss) {
			if (bot_starting_items.id == bot_starting_items_id) {
				return bot_starting_items;
			}
		}

		return NewEntity();
	}

	static BotStartingItems FindOne(
		Database& db,
		int bot_starting_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				bot_starting_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BotStartingItems e{};

			e.id                     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.races                  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.classes                = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.item_id                = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.item_charges           = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.slot_id                = static_cast<int32_t>(atoi(row[5]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[6]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[7]));
			e.content_flags          = row[8] ? row[8] : "";
			e.content_flags_disabled = row[9] ? row[9] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bot_starting_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bot_starting_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const BotStartingItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.races));
		v.push_back(columns[2] + " = " + std::to_string(e.classes));
		v.push_back(columns[3] + " = " + std::to_string(e.item_id));
		v.push_back(columns[4] + " = " + std::to_string(e.item_charges));
		v.push_back(columns[5] + " = " + std::to_string(e.slot_id));
		v.push_back(columns[6] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[7] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[8] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[9] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static BotStartingItems InsertOne(
		Database& db,
		BotStartingItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.races));
		v.push_back(std::to_string(e.classes));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<BotStartingItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.races));
			v.push_back(std::to_string(e.classes));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static std::vector<BotStartingItems> All(Database& db)
	{
		std::vector<BotStartingItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotStartingItems e{};

			e.id                     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.races                  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.classes                = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.item_id                = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.item_charges           = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.slot_id                = static_cast<int32_t>(atoi(row[5]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[6]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[7]));
			e.content_flags          = row[8] ? row[8] : "";
			e.content_flags_disabled = row[9] ? row[9] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<BotStartingItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<BotStartingItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BotStartingItems e{};

			e.id                     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.races                  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.classes                = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.item_id                = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.item_charges           = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.slot_id                = static_cast<int32_t>(atoi(row[5]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[6]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[7]));
			e.content_flags          = row[8] ? row[8] : "";
			e.content_flags_disabled = row[9] ? row[9] : "";

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

#endif //EQEMU_BASE_BOT_STARTING_ITEMS_REPOSITORY_H
