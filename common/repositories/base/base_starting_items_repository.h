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

#ifndef EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H
#define EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseStartingItemsRepository {
public:
	struct StartingItems {
		uint32_t    id;
		std::string class_list;
		std::string race_list;
		std::string deity_list;
		std::string zone_id_list;
		uint32_t    item_id;
		uint8_t     item_charges;
		uint32_t    augment_one;
		uint32_t    augment_two;
		uint32_t    augment_three;
		uint32_t    augment_four;
		uint32_t    augment_five;
		uint32_t    augment_six;
		int32_t     status;
		int32_t     inventory_slot;
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
			"class_list",
			"race_list",
			"deity_list",
			"zone_id_list",
			"item_id",
			"item_charges",
			"augment_one",
			"augment_two",
			"augment_three",
			"augment_four",
			"augment_five",
			"augment_six",
			"status",
			"inventory_slot",
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
			"class_list",
			"race_list",
			"deity_list",
			"zone_id_list",
			"item_id",
			"item_charges",
			"augment_one",
			"augment_two",
			"augment_three",
			"augment_four",
			"augment_five",
			"augment_six",
			"status",
			"inventory_slot",
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
		e.class_list             = "";
		e.race_list              = "";
		e.deity_list             = "";
		e.zone_id_list           = "";
		e.item_id                = 0;
		e.item_charges           = 1;
		e.augment_one            = 0;
		e.augment_two            = 0;
		e.augment_three          = 0;
		e.augment_four           = 0;
		e.augment_five           = 0;
		e.augment_six            = 0;
		e.status                 = 0;
		e.inventory_slot         = -1;
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
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				starting_items_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			StartingItems e{};

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.class_list             = row[1] ? row[1] : "";
			e.race_list              = row[2] ? row[2] : "";
			e.deity_list             = row[3] ? row[3] : "";
			e.zone_id_list           = row[4] ? row[4] : "";
			e.item_id                = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.item_charges           = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;
			e.augment_one            = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_two            = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_three          = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_four           = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augment_five           = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_six            = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.status                 = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.inventory_slot         = row[14] ? static_cast<int32_t>(atoi(row[14])) : -1;
			e.min_expansion          = row[15] ? static_cast<int8_t>(atoi(row[15])) : -1;
			e.max_expansion          = row[16] ? static_cast<int8_t>(atoi(row[16])) : -1;
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

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

		v.push_back(columns[1] + " = '" + Strings::Escape(e.class_list) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.race_list) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.deity_list) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.zone_id_list) + "'");
		v.push_back(columns[5] + " = " + std::to_string(e.item_id));
		v.push_back(columns[6] + " = " + std::to_string(e.item_charges));
		v.push_back(columns[7] + " = " + std::to_string(e.augment_one));
		v.push_back(columns[8] + " = " + std::to_string(e.augment_two));
		v.push_back(columns[9] + " = " + std::to_string(e.augment_three));
		v.push_back(columns[10] + " = " + std::to_string(e.augment_four));
		v.push_back(columns[11] + " = " + std::to_string(e.augment_five));
		v.push_back(columns[12] + " = " + std::to_string(e.augment_six));
		v.push_back(columns[13] + " = " + std::to_string(e.status));
		v.push_back(columns[14] + " = " + std::to_string(e.inventory_slot));
		v.push_back(columns[15] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[16] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[17] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[18] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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
		v.push_back("'" + Strings::Escape(e.class_list) + "'");
		v.push_back("'" + Strings::Escape(e.race_list) + "'");
		v.push_back("'" + Strings::Escape(e.deity_list) + "'");
		v.push_back("'" + Strings::Escape(e.zone_id_list) + "'");
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.augment_one));
		v.push_back(std::to_string(e.augment_two));
		v.push_back(std::to_string(e.augment_three));
		v.push_back(std::to_string(e.augment_four));
		v.push_back(std::to_string(e.augment_five));
		v.push_back(std::to_string(e.augment_six));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.inventory_slot));
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
			v.push_back("'" + Strings::Escape(e.class_list) + "'");
			v.push_back("'" + Strings::Escape(e.race_list) + "'");
			v.push_back("'" + Strings::Escape(e.deity_list) + "'");
			v.push_back("'" + Strings::Escape(e.zone_id_list) + "'");
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.augment_one));
			v.push_back(std::to_string(e.augment_two));
			v.push_back(std::to_string(e.augment_three));
			v.push_back(std::to_string(e.augment_four));
			v.push_back(std::to_string(e.augment_five));
			v.push_back(std::to_string(e.augment_six));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.inventory_slot));
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

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.class_list             = row[1] ? row[1] : "";
			e.race_list              = row[2] ? row[2] : "";
			e.deity_list             = row[3] ? row[3] : "";
			e.zone_id_list           = row[4] ? row[4] : "";
			e.item_id                = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.item_charges           = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;
			e.augment_one            = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_two            = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_three          = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_four           = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augment_five           = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_six            = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.status                 = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.inventory_slot         = row[14] ? static_cast<int32_t>(atoi(row[14])) : -1;
			e.min_expansion          = row[15] ? static_cast<int8_t>(atoi(row[15])) : -1;
			e.max_expansion          = row[16] ? static_cast<int8_t>(atoi(row[16])) : -1;
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

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

			e.id                     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.class_list             = row[1] ? row[1] : "";
			e.race_list              = row[2] ? row[2] : "";
			e.deity_list             = row[3] ? row[3] : "";
			e.zone_id_list           = row[4] ? row[4] : "";
			e.item_id                = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.item_charges           = row[6] ? static_cast<uint8_t>(strtoul(row[6], nullptr, 10)) : 1;
			e.augment_one            = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_two            = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_three          = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_four           = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.augment_five           = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.augment_six            = row[12] ? static_cast<uint32_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.status                 = row[13] ? static_cast<int32_t>(atoi(row[13])) : 0;
			e.inventory_slot         = row[14] ? static_cast<int32_t>(atoi(row[14])) : -1;
			e.min_expansion          = row[15] ? static_cast<int8_t>(atoi(row[15])) : -1;
			e.max_expansion          = row[16] ? static_cast<int8_t>(atoi(row[16])) : -1;
			e.content_flags          = row[17] ? row[17] : "";
			e.content_flags_disabled = row[18] ? row[18] : "";

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
		const StartingItems &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.class_list) + "'");
		v.push_back("'" + Strings::Escape(e.race_list) + "'");
		v.push_back("'" + Strings::Escape(e.deity_list) + "'");
		v.push_back("'" + Strings::Escape(e.zone_id_list) + "'");
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.augment_one));
		v.push_back(std::to_string(e.augment_two));
		v.push_back(std::to_string(e.augment_three));
		v.push_back(std::to_string(e.augment_four));
		v.push_back(std::to_string(e.augment_five));
		v.push_back(std::to_string(e.augment_six));
		v.push_back(std::to_string(e.status));
		v.push_back(std::to_string(e.inventory_slot));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<StartingItems> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.class_list) + "'");
			v.push_back("'" + Strings::Escape(e.race_list) + "'");
			v.push_back("'" + Strings::Escape(e.deity_list) + "'");
			v.push_back("'" + Strings::Escape(e.zone_id_list) + "'");
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.augment_one));
			v.push_back(std::to_string(e.augment_two));
			v.push_back(std::to_string(e.augment_three));
			v.push_back(std::to_string(e.augment_four));
			v.push_back(std::to_string(e.augment_five));
			v.push_back(std::to_string(e.augment_six));
			v.push_back(std::to_string(e.status));
			v.push_back(std::to_string(e.inventory_slot));
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
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_BASE_STARTING_ITEMS_REPOSITORY_H
