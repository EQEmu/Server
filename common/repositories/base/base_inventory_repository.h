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
		int         charid;
		int         slotid;
		int         itemid;
		int         charges;
		int         color;
		int         augslot1;
		int         augslot2;
		int         augslot3;
		int         augslot4;
		int         augslot5;
		int         augslot6;
		int         instnodrop;
		std::string custom_data;
		int         ornamenticon;
		int         ornamentidfile;
		int         ornament_hero_model;
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

	static Inventory GetInventorye(
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

			e.charid              = atoi(row[0]);
			e.slotid              = atoi(row[1]);
			e.itemid              = atoi(row[2]);
			e.charges             = atoi(row[3]);
			e.color               = atoi(row[4]);
			e.augslot1            = atoi(row[5]);
			e.augslot2            = atoi(row[6]);
			e.augslot3            = atoi(row[7]);
			e.augslot4            = atoi(row[8]);
			e.augslot5            = atoi(row[9]);
			e.augslot6            = atoi(row[10]);
			e.instnodrop          = atoi(row[11]);
			e.custom_data         = row[12] ? row[12] : "";
			e.ornamenticon        = atoi(row[13]);
			e.ornamentidfile      = atoi(row[14]);
			e.ornament_hero_model = atoi(row[15]);

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
		Inventory inventory_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(inventory_e.charid));
		update_values.push_back(columns[1] + " = " + std::to_string(inventory_e.slotid));
		update_values.push_back(columns[2] + " = " + std::to_string(inventory_e.itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(inventory_e.charges));
		update_values.push_back(columns[4] + " = " + std::to_string(inventory_e.color));
		update_values.push_back(columns[5] + " = " + std::to_string(inventory_e.augslot1));
		update_values.push_back(columns[6] + " = " + std::to_string(inventory_e.augslot2));
		update_values.push_back(columns[7] + " = " + std::to_string(inventory_e.augslot3));
		update_values.push_back(columns[8] + " = " + std::to_string(inventory_e.augslot4));
		update_values.push_back(columns[9] + " = " + std::to_string(inventory_e.augslot5));
		update_values.push_back(columns[10] + " = " + std::to_string(inventory_e.augslot6));
		update_values.push_back(columns[11] + " = " + std::to_string(inventory_e.instnodrop));
		update_values.push_back(columns[12] + " = '" + Strings::Escape(inventory_e.custom_data) + "'");
		update_values.push_back(columns[13] + " = " + std::to_string(inventory_e.ornamenticon));
		update_values.push_back(columns[14] + " = " + std::to_string(inventory_e.ornamentidfile));
		update_values.push_back(columns[15] + " = " + std::to_string(inventory_e.ornament_hero_model));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				inventory_e.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Inventory InsertOne(
		Database& db,
		Inventory inventory_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(inventory_e.charid));
		insert_values.push_back(std::to_string(inventory_e.slotid));
		insert_values.push_back(std::to_string(inventory_e.itemid));
		insert_values.push_back(std::to_string(inventory_e.charges));
		insert_values.push_back(std::to_string(inventory_e.color));
		insert_values.push_back(std::to_string(inventory_e.augslot1));
		insert_values.push_back(std::to_string(inventory_e.augslot2));
		insert_values.push_back(std::to_string(inventory_e.augslot3));
		insert_values.push_back(std::to_string(inventory_e.augslot4));
		insert_values.push_back(std::to_string(inventory_e.augslot5));
		insert_values.push_back(std::to_string(inventory_e.augslot6));
		insert_values.push_back(std::to_string(inventory_e.instnodrop));
		insert_values.push_back("'" + Strings::Escape(inventory_e.custom_data) + "'");
		insert_values.push_back(std::to_string(inventory_e.ornamenticon));
		insert_values.push_back(std::to_string(inventory_e.ornamentidfile));
		insert_values.push_back(std::to_string(inventory_e.ornament_hero_model));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			inventory_e.charid = results.LastInsertedID();
			return inventory_e;
		}

		inventory_e = NewEntity();

		return inventory_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Inventory> inventory_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &inventory_e: inventory_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(inventory_e.charid));
			insert_values.push_back(std::to_string(inventory_e.slotid));
			insert_values.push_back(std::to_string(inventory_e.itemid));
			insert_values.push_back(std::to_string(inventory_e.charges));
			insert_values.push_back(std::to_string(inventory_e.color));
			insert_values.push_back(std::to_string(inventory_e.augslot1));
			insert_values.push_back(std::to_string(inventory_e.augslot2));
			insert_values.push_back(std::to_string(inventory_e.augslot3));
			insert_values.push_back(std::to_string(inventory_e.augslot4));
			insert_values.push_back(std::to_string(inventory_e.augslot5));
			insert_values.push_back(std::to_string(inventory_e.augslot6));
			insert_values.push_back(std::to_string(inventory_e.instnodrop));
			insert_values.push_back("'" + Strings::Escape(inventory_e.custom_data) + "'");
			insert_values.push_back(std::to_string(inventory_e.ornamenticon));
			insert_values.push_back(std::to_string(inventory_e.ornamentidfile));
			insert_values.push_back(std::to_string(inventory_e.ornament_hero_model));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

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

			e.charid              = atoi(row[0]);
			e.slotid              = atoi(row[1]);
			e.itemid              = atoi(row[2]);
			e.charges             = atoi(row[3]);
			e.color               = atoi(row[4]);
			e.augslot1            = atoi(row[5]);
			e.augslot2            = atoi(row[6]);
			e.augslot3            = atoi(row[7]);
			e.augslot4            = atoi(row[8]);
			e.augslot5            = atoi(row[9]);
			e.augslot6            = atoi(row[10]);
			e.instnodrop          = atoi(row[11]);
			e.custom_data         = row[12] ? row[12] : "";
			e.ornamenticon        = atoi(row[13]);
			e.ornamentidfile      = atoi(row[14]);
			e.ornament_hero_model = atoi(row[15]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Inventory> GetWhere(Database& db, std::string where_filter)
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

			e.charid              = atoi(row[0]);
			e.slotid              = atoi(row[1]);
			e.itemid              = atoi(row[2]);
			e.charges             = atoi(row[3]);
			e.color               = atoi(row[4]);
			e.augslot1            = atoi(row[5]);
			e.augslot2            = atoi(row[6]);
			e.augslot3            = atoi(row[7]);
			e.augslot4            = atoi(row[8]);
			e.augslot5            = atoi(row[9]);
			e.augslot6            = atoi(row[10]);
			e.instnodrop          = atoi(row[11]);
			e.custom_data         = row[12] ? row[12] : "";
			e.ornamenticon        = atoi(row[13]);
			e.ornamentidfile      = atoi(row[14]);
			e.ornament_hero_model = atoi(row[15]);

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_INVENTORY_REPOSITORY_H
