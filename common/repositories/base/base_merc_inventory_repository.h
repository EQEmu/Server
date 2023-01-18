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

#ifndef EQEMU_BASE_MERC_INVENTORY_REPOSITORY_H
#define EQEMU_BASE_MERC_INVENTORY_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>


class BaseMercInventoryRepository {
public:
	struct MercInventory {
		uint32_t merc_inventory_id;
		uint32_t merc_subtype_id;
		uint32_t item_id;
		uint32_t min_level;
		uint32_t max_level;
	};

	static std::string PrimaryKey()
	{
		return std::string("merc_inventory_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merc_inventory_id",
			"merc_subtype_id",
			"item_id",
			"min_level",
			"max_level",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merc_inventory_id",
			"merc_subtype_id",
			"item_id",
			"min_level",
			"max_level",
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
		return std::string("merc_inventory");
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

	static MercInventory NewEntity()
	{
		MercInventory e{};

		e.merc_inventory_id = 0;
		e.merc_subtype_id   = 0;
		e.item_id           = 0;
		e.min_level         = 0;
		e.max_level         = 0;

		return e;
	}

	static MercInventory GetMercInventory(
		const std::vector<MercInventory> &merc_inventorys,
		int merc_inventory_id
	)
	{
		for (auto &merc_inventory : merc_inventorys) {
			if (merc_inventory.merc_inventory_id == merc_inventory_id) {
				return merc_inventory;
			}
		}

		return NewEntity();
	}

	static MercInventory FindOne(
		Database& db,
		int merc_inventory_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_inventory_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercInventory e{};

			e.merc_inventory_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_subtype_id   = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.item_id           = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.min_level         = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.max_level         = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_inventory_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_inventory_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercInventory &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.merc_subtype_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.min_level));
		v.push_back(columns[4] + " = " + std::to_string(e.max_level));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merc_inventory_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercInventory InsertOne(
		Database& db,
		MercInventory e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_inventory_id));
		v.push_back(std::to_string(e.merc_subtype_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.min_level));
		v.push_back(std::to_string(e.max_level));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merc_inventory_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercInventory> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_inventory_id));
			v.push_back(std::to_string(e.merc_subtype_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.min_level));
			v.push_back(std::to_string(e.max_level));

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

	static std::vector<MercInventory> All(Database& db)
	{
		std::vector<MercInventory> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercInventory e{};

			e.merc_inventory_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_subtype_id   = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.item_id           = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.min_level         = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.max_level         = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercInventory> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercInventory> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercInventory e{};

			e.merc_inventory_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_subtype_id   = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.item_id           = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.min_level         = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.max_level         = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

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

#endif //EQEMU_BASE_MERC_INVENTORY_REPOSITORY_H
