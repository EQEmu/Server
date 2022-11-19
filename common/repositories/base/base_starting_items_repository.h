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

#ifndef EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H
#define EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseStartingItemsRepository {
public:
	struct StartingItems {
		uint32_t    id;
		int32_t     race;
		int32_t     class_;
		int32_t     deityid;
		int32_t     zoneid;
		int32_t     itemid;
		uint8_t     item_charges;
		int8_t      gm;
		int32_t     slot;
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
			"race",
			"`class`",
			"deityid",
			"zoneid",
			"itemid",
			"item_charges",
			"gm",
			"slot",
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
			"race",
			"`class`",
			"deityid",
			"zoneid",
			"itemid",
			"item_charges",
			"gm",
			"slot",
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
		return std::string("starting_items");
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

	static StartingItems NewEntity()
	{
		StartingItems e{};

		e.id                     = 0;
		e.race                   = 0;
		e.class_                 = 0;
		e.deityid                = 0;
		e.zoneid                 = 0;
		e.itemid                 = 0;
		e.item_charges           = 1;
		e.gm                     = 0;
		e.slot                   = -1;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static StartingItems GetStartingItems(
		const std::vector<StartingItems> &starting_itemss,
		int starting_items_id
	)
	{
		for (auto &starting_items : starting_itemss) {
			if (starting_items.id == starting_items_id) {
				return starting_items;
			}
		}

		return NewEntity();
	}

	static StartingItems FindOne(
		Database& db,
		int starting_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				starting_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			StartingItems e{};

			e.id                     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.race                   = static_cast<int32_t>(atoi(row[1]));
			e.class_                 = static_cast<int32_t>(atoi(row[2]));
			e.deityid                = static_cast<int32_t>(atoi(row[3]));
			e.zoneid                 = static_cast<int32_t>(atoi(row[4]));
			e.itemid                 = static_cast<int32_t>(atoi(row[5]));
			e.item_charges           = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.gm                     = static_cast<int8_t>(atoi(row[7]));
			e.slot                   = static_cast<int32_t>(atoi(row[8]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[9]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[10]));
			e.content_flags          = row[11] ? row[11] : "";
			e.content_flags_disabled = row[12] ? row[12] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int starting_items_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				starting_items_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const StartingItems &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.race));
		v.push_back(columns[2] + " = " + std::to_string(e.class_));
		v.push_back(columns[3] + " = " + std::to_string(e.deityid));
		v.push_back(columns[4] + " = " + std::to_string(e.zoneid));
		v.push_back(columns[5] + " = " + std::to_string(e.itemid));
		v.push_back(columns[6] + " = " + std::to_string(e.item_charges));
		v.push_back(columns[7] + " = " + std::to_string(e.gm));
		v.push_back(columns[8] + " = " + std::to_string(e.slot));
		v.push_back(columns[9] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[10] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[11] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[12] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static StartingItems InsertOne(
		Database& db,
		StartingItems e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.class_));
		v.push_back(std::to_string(e.deityid));
		v.push_back(std::to_string(e.zoneid));
		v.push_back(std::to_string(e.itemid));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.gm));
		v.push_back(std::to_string(e.slot));
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
		const std::vector<StartingItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.class_));
			v.push_back(std::to_string(e.deityid));
			v.push_back(std::to_string(e.zoneid));
			v.push_back(std::to_string(e.itemid));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.gm));
			v.push_back(std::to_string(e.slot));
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

	static std::vector<StartingItems> All(Database& db)
	{
		std::vector<StartingItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			StartingItems e{};

			e.id                     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.race                   = static_cast<int32_t>(atoi(row[1]));
			e.class_                 = static_cast<int32_t>(atoi(row[2]));
			e.deityid                = static_cast<int32_t>(atoi(row[3]));
			e.zoneid                 = static_cast<int32_t>(atoi(row[4]));
			e.itemid                 = static_cast<int32_t>(atoi(row[5]));
			e.item_charges           = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.gm                     = static_cast<int8_t>(atoi(row[7]));
			e.slot                   = static_cast<int32_t>(atoi(row[8]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[9]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[10]));
			e.content_flags          = row[11] ? row[11] : "";
			e.content_flags_disabled = row[12] ? row[12] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<StartingItems> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<StartingItems> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			StartingItems e{};

			e.id                     = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.race                   = static_cast<int32_t>(atoi(row[1]));
			e.class_                 = static_cast<int32_t>(atoi(row[2]));
			e.deityid                = static_cast<int32_t>(atoi(row[3]));
			e.zoneid                 = static_cast<int32_t>(atoi(row[4]));
			e.itemid                 = static_cast<int32_t>(atoi(row[5]));
			e.item_charges           = static_cast<uint8_t>(strtoul(row[6], nullptr, 10));
			e.gm                     = static_cast<int8_t>(atoi(row[7]));
			e.slot                   = static_cast<int32_t>(atoi(row[8]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[9]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[10]));
			e.content_flags          = row[11] ? row[11] : "";
			e.content_flags_disabled = row[12] ? row[12] : "";

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

#endif //EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H
