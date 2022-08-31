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

#ifndef EQEMU_BASE_INVENTORY_REPOSITORY_H
#define EQEMU_BASE_INVENTORY_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseInventoryRepository {
public:
	struct Inventory {
		uint32_t    charid;
		uint32_t    slotid;
		uint32_t    itemid;
		uint16_t    charges;
		uint32_t    color;
		uint32_t    augslot1;
		uint32_t    augslot2;
		uint32_t    augslot3;
		uint32_t    augslot4;
		uint32_t    augslot5;
		int32_t     augslot6;
		uint8_t     instnodrop;
		std::string custom_data;
		uint32_t    ornamenticon;
		uint32_t    ornamentidfile;
		int32_t     ornament_hero_model;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"slotid",
			"itemid",
			"charges",
			"color",
			"augslot1",
			"augslot2",
			"augslot3",
			"augslot4",
			"augslot5",
			"augslot6",
			"instnodrop",
			"custom_data",
			"ornamenticon",
			"ornamentidfile",
			"ornament_hero_model",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"charid",
			"slotid",
			"itemid",
			"charges",
			"color",
			"augslot1",
			"augslot2",
			"augslot3",
			"augslot4",
			"augslot5",
			"augslot6",
			"instnodrop",
			"custom_data",
			"ornamenticon",
			"ornamentidfile",
			"ornament_hero_model",
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
		return std::string("inventory");
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

	static Inventory NewEntity()
	{
		Inventory e{};

		e.charid              = 0;
		e.slotid              = 0;
		e.itemid              = 0;
		e.charges             = 0;
		e.color               = 0;
		e.augslot1            = 0;
		e.augslot2            = 0;
		e.augslot3            = 0;
		e.augslot4            = 0;
		e.augslot5            = 0;
		e.augslot6            = 0;
		e.instnodrop          = 0;
		e.custom_data         = "";
		e.ornamenticon        = 0;
		e.ornamentidfile      = 0;
		e.ornament_hero_model = 0;

		return e;
	}

	static Inventory GetInventory(
		const std::vector<Inventory> &inventorys,
		int inventory_id
	)
	{
		for (auto &inventory : inventorys) {
			if (inventory.charid == inventory_id) {
				return inventory;
			}
		}

		return NewEntity();
	}

	static Inventory FindOne(
		Database& db,
		int inventory_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				inventory_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Inventory e{};

			e.charid              = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.slotid              = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.itemid              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.charges             = static_cast<uint16_t>(strtoul(row[3], nullptr, 10));
			e.color               = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.augslot1            = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.augslot2            = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.augslot3            = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.augslot4            = static_cast<uint32_t>(strtoul(row[8], nullptr, 10));
			e.augslot5            = static_cast<uint32_t>(strtoul(row[9], nullptr, 10));
			e.augslot6            = static_cast<int32_t>(atoi(row[10]));
			e.instnodrop          = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.custom_data         = row[12] ? row[12] : "";
			e.ornamenticon        = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.ornamentidfile      = static_cast<uint32_t>(strtoul(row[14], nullptr, 10));
			e.ornament_hero_model = static_cast<int32_t>(atoi(row[15]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int inventory_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				inventory_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Inventory &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.charid));
		v.push_back(columns[1] + " = " + std::to_string(e.slotid));
		v.push_back(columns[2] + " = " + std::to_string(e.itemid));
		v.push_back(columns[3] + " = " + std::to_string(e.charges));
		v.push_back(columns[4] + " = " + std::to_string(e.color));
		v.push_back(columns[5] + " = " + std::to_string(e.augslot1));
		v.push_back(columns[6] + " = " + std::to_string(e.augslot2));
		v.push_back(columns[7] + " = " + std::to_string(e.augslot3));
		v.push_back(columns[8] + " = " + std::to_string(e.augslot4));
		v.push_back(columns[9] + " = " + std::to_string(e.augslot5));
		v.push_back(columns[10] + " = " + std::to_string(e.augslot6));
		v.push_back(columns[11] + " = " + std::to_string(e.instnodrop));
		v.push_back(columns[12] + " = '" + Strings::Escape(e.custom_data) + "'");
		v.push_back(columns[13] + " = " + std::to_string(e.ornamenticon));
		v.push_back(columns[14] + " = " + std::to_string(e.ornamentidfile));
		v.push_back(columns[15] + " = " + std::to_string(e.ornament_hero_model));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Inventory InsertOne(
		Database& db,
		Inventory e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.slotid));
		v.push_back(std::to_string(e.itemid));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.color));
		v.push_back(std::to_string(e.augslot1));
		v.push_back(std::to_string(e.augslot2));
		v.push_back(std::to_string(e.augslot3));
		v.push_back(std::to_string(e.augslot4));
		v.push_back(std::to_string(e.augslot5));
		v.push_back(std::to_string(e.augslot6));
		v.push_back(std::to_string(e.instnodrop));
		v.push_back("'" + Strings::Escape(e.custom_data) + "'");
		v.push_back(std::to_string(e.ornamenticon));
		v.push_back(std::to_string(e.ornamentidfile));
		v.push_back(std::to_string(e.ornament_hero_model));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.charid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Inventory> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.slotid));
			v.push_back(std::to_string(e.itemid));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.color));
			v.push_back(std::to_string(e.augslot1));
			v.push_back(std::to_string(e.augslot2));
			v.push_back(std::to_string(e.augslot3));
			v.push_back(std::to_string(e.augslot4));
			v.push_back(std::to_string(e.augslot5));
			v.push_back(std::to_string(e.augslot6));
			v.push_back(std::to_string(e.instnodrop));
			v.push_back("'" + Strings::Escape(e.custom_data) + "'");
			v.push_back(std::to_string(e.ornamenticon));
			v.push_back(std::to_string(e.ornamentidfile));
			v.push_back(std::to_string(e.ornament_hero_model));

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

	static std::vector<Inventory> All(Database& db)
	{
		std::vector<Inventory> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Inventory e{};

			e.charid              = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.slotid              = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.itemid              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.charges             = static_cast<uint16_t>(strtoul(row[3], nullptr, 10));
			e.color               = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.augslot1            = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.augslot2            = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.augslot3            = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.augslot4            = static_cast<uint32_t>(strtoul(row[8], nullptr, 10));
			e.augslot5            = static_cast<uint32_t>(strtoul(row[9], nullptr, 10));
			e.augslot6            = static_cast<int32_t>(atoi(row[10]));
			e.instnodrop          = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.custom_data         = row[12] ? row[12] : "";
			e.ornamenticon        = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.ornamentidfile      = static_cast<uint32_t>(strtoul(row[14], nullptr, 10));
			e.ornament_hero_model = static_cast<int32_t>(atoi(row[15]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Inventory> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Inventory> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Inventory e{};

			e.charid              = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.slotid              = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.itemid              = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.charges             = static_cast<uint16_t>(strtoul(row[3], nullptr, 10));
			e.color               = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			e.augslot1            = static_cast<uint32_t>(strtoul(row[5], nullptr, 10));
			e.augslot2            = static_cast<uint32_t>(strtoul(row[6], nullptr, 10));
			e.augslot3            = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			e.augslot4            = static_cast<uint32_t>(strtoul(row[8], nullptr, 10));
			e.augslot5            = static_cast<uint32_t>(strtoul(row[9], nullptr, 10));
			e.augslot6            = static_cast<int32_t>(atoi(row[10]));
			e.instnodrop          = static_cast<uint8_t>(strtoul(row[11], nullptr, 10));
			e.custom_data         = row[12] ? row[12] : "";
			e.ornamenticon        = static_cast<uint32_t>(strtoul(row[13], nullptr, 10));
			e.ornamentidfile      = static_cast<uint32_t>(strtoul(row[14], nullptr, 10));
			e.ornament_hero_model = static_cast<int32_t>(atoi(row[15]));

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

#endif //EQEMU_BASE_INVENTORY_REPOSITORY_H
