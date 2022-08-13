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

#ifndef EQEMU_BASE_INVENTORY_SNAPSHOTS_REPOSITORY_H
#define EQEMU_BASE_INVENTORY_SNAPSHOTS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseInventorySnapshotsRepository {
public:
	struct InventorySnapshots {
		int         time_index;
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
		return std::string("time_index");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"time_index",
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
			"time_index",
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
		return std::string("inventory_snapshots");
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

	static InventorySnapshots NewEntity()
	{
		InventorySnapshots e{};

		e.time_index          = 0;
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

	static InventorySnapshots GetInventorySnapshotse(
		const std::vector<InventorySnapshots> &inventory_snapshotss,
		int inventory_snapshots_id
	)
	{
		for (auto &inventory_snapshots : inventory_snapshotss) {
			if (inventory_snapshots.time_index == inventory_snapshots_id) {
				return inventory_snapshots;
			}
		}

		return NewEntity();
	}

	static InventorySnapshots FindOne(
		Database& db,
		int inventory_snapshots_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				inventory_snapshots_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			InventorySnapshots e{};

			e.time_index          = atoi(row[0]);
			e.charid              = atoi(row[1]);
			e.slotid              = atoi(row[2]);
			e.itemid              = atoi(row[3]);
			e.charges             = atoi(row[4]);
			e.color               = atoi(row[5]);
			e.augslot1            = atoi(row[6]);
			e.augslot2            = atoi(row[7]);
			e.augslot3            = atoi(row[8]);
			e.augslot4            = atoi(row[9]);
			e.augslot5            = atoi(row[10]);
			e.augslot6            = atoi(row[11]);
			e.instnodrop          = atoi(row[12]);
			e.custom_data         = row[13] ? row[13] : "";
			e.ornamenticon        = atoi(row[14]);
			e.ornamentidfile      = atoi(row[15]);
			e.ornament_hero_model = atoi(row[16]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int inventory_snapshots_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				inventory_snapshots_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		InventorySnapshots inventory_snapshots_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(inventory_snapshots_e.time_index));
		update_values.push_back(columns[1] + " = " + std::to_string(inventory_snapshots_e.charid));
		update_values.push_back(columns[2] + " = " + std::to_string(inventory_snapshots_e.slotid));
		update_values.push_back(columns[3] + " = " + std::to_string(inventory_snapshots_e.itemid));
		update_values.push_back(columns[4] + " = " + std::to_string(inventory_snapshots_e.charges));
		update_values.push_back(columns[5] + " = " + std::to_string(inventory_snapshots_e.color));
		update_values.push_back(columns[6] + " = " + std::to_string(inventory_snapshots_e.augslot1));
		update_values.push_back(columns[7] + " = " + std::to_string(inventory_snapshots_e.augslot2));
		update_values.push_back(columns[8] + " = " + std::to_string(inventory_snapshots_e.augslot3));
		update_values.push_back(columns[9] + " = " + std::to_string(inventory_snapshots_e.augslot4));
		update_values.push_back(columns[10] + " = " + std::to_string(inventory_snapshots_e.augslot5));
		update_values.push_back(columns[11] + " = " + std::to_string(inventory_snapshots_e.augslot6));
		update_values.push_back(columns[12] + " = " + std::to_string(inventory_snapshots_e.instnodrop));
		update_values.push_back(columns[13] + " = '" + Strings::Escape(inventory_snapshots_e.custom_data) + "'");
		update_values.push_back(columns[14] + " = " + std::to_string(inventory_snapshots_e.ornamenticon));
		update_values.push_back(columns[15] + " = " + std::to_string(inventory_snapshots_e.ornamentidfile));
		update_values.push_back(columns[16] + " = " + std::to_string(inventory_snapshots_e.ornament_hero_model));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				inventory_snapshots_e.time_index
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static InventorySnapshots InsertOne(
		Database& db,
		InventorySnapshots inventory_snapshots_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(inventory_snapshots_e.time_index));
		insert_values.push_back(std::to_string(inventory_snapshots_e.charid));
		insert_values.push_back(std::to_string(inventory_snapshots_e.slotid));
		insert_values.push_back(std::to_string(inventory_snapshots_e.itemid));
		insert_values.push_back(std::to_string(inventory_snapshots_e.charges));
		insert_values.push_back(std::to_string(inventory_snapshots_e.color));
		insert_values.push_back(std::to_string(inventory_snapshots_e.augslot1));
		insert_values.push_back(std::to_string(inventory_snapshots_e.augslot2));
		insert_values.push_back(std::to_string(inventory_snapshots_e.augslot3));
		insert_values.push_back(std::to_string(inventory_snapshots_e.augslot4));
		insert_values.push_back(std::to_string(inventory_snapshots_e.augslot5));
		insert_values.push_back(std::to_string(inventory_snapshots_e.augslot6));
		insert_values.push_back(std::to_string(inventory_snapshots_e.instnodrop));
		insert_values.push_back("'" + Strings::Escape(inventory_snapshots_e.custom_data) + "'");
		insert_values.push_back(std::to_string(inventory_snapshots_e.ornamenticon));
		insert_values.push_back(std::to_string(inventory_snapshots_e.ornamentidfile));
		insert_values.push_back(std::to_string(inventory_snapshots_e.ornament_hero_model));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			inventory_snapshots_e.time_index = results.LastInsertedID();
			return inventory_snapshots_e;
		}

		inventory_snapshots_e = NewEntity();

		return inventory_snapshots_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<InventorySnapshots> inventory_snapshots_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &inventory_snapshots_e: inventory_snapshots_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(inventory_snapshots_e.time_index));
			insert_values.push_back(std::to_string(inventory_snapshots_e.charid));
			insert_values.push_back(std::to_string(inventory_snapshots_e.slotid));
			insert_values.push_back(std::to_string(inventory_snapshots_e.itemid));
			insert_values.push_back(std::to_string(inventory_snapshots_e.charges));
			insert_values.push_back(std::to_string(inventory_snapshots_e.color));
			insert_values.push_back(std::to_string(inventory_snapshots_e.augslot1));
			insert_values.push_back(std::to_string(inventory_snapshots_e.augslot2));
			insert_values.push_back(std::to_string(inventory_snapshots_e.augslot3));
			insert_values.push_back(std::to_string(inventory_snapshots_e.augslot4));
			insert_values.push_back(std::to_string(inventory_snapshots_e.augslot5));
			insert_values.push_back(std::to_string(inventory_snapshots_e.augslot6));
			insert_values.push_back(std::to_string(inventory_snapshots_e.instnodrop));
			insert_values.push_back("'" + Strings::Escape(inventory_snapshots_e.custom_data) + "'");
			insert_values.push_back(std::to_string(inventory_snapshots_e.ornamenticon));
			insert_values.push_back(std::to_string(inventory_snapshots_e.ornamentidfile));
			insert_values.push_back(std::to_string(inventory_snapshots_e.ornament_hero_model));

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

	static std::vector<InventorySnapshots> All(Database& db)
	{
		std::vector<InventorySnapshots> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InventorySnapshots e{};

			e.time_index          = atoi(row[0]);
			e.charid              = atoi(row[1]);
			e.slotid              = atoi(row[2]);
			e.itemid              = atoi(row[3]);
			e.charges             = atoi(row[4]);
			e.color               = atoi(row[5]);
			e.augslot1            = atoi(row[6]);
			e.augslot2            = atoi(row[7]);
			e.augslot3            = atoi(row[8]);
			e.augslot4            = atoi(row[9]);
			e.augslot5            = atoi(row[10]);
			e.augslot6            = atoi(row[11]);
			e.instnodrop          = atoi(row[12]);
			e.custom_data         = row[13] ? row[13] : "";
			e.ornamenticon        = atoi(row[14]);
			e.ornamentidfile      = atoi(row[15]);
			e.ornament_hero_model = atoi(row[16]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<InventorySnapshots> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<InventorySnapshots> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InventorySnapshots e{};

			e.time_index          = atoi(row[0]);
			e.charid              = atoi(row[1]);
			e.slotid              = atoi(row[2]);
			e.itemid              = atoi(row[3]);
			e.charges             = atoi(row[4]);
			e.color               = atoi(row[5]);
			e.augslot1            = atoi(row[6]);
			e.augslot2            = atoi(row[7]);
			e.augslot3            = atoi(row[8]);
			e.augslot4            = atoi(row[9]);
			e.augslot5            = atoi(row[10]);
			e.augslot6            = atoi(row[11]);
			e.instnodrop          = atoi(row[12]);
			e.custom_data         = row[13] ? row[13] : "";
			e.ornamenticon        = atoi(row[14]);
			e.ornamentidfile      = atoi(row[15]);
			e.ornament_hero_model = atoi(row[16]);

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

#endif //EQEMU_BASE_INVENTORY_SNAPSHOTS_REPOSITORY_H
