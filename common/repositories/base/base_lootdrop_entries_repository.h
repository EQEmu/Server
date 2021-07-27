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

#ifndef EQEMU_BASE_LOOTDROP_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_LOOTDROP_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLootdropEntriesRepository {
public:
	struct LootdropEntries {
		int   lootdrop_id;
		int   item_id;
		int   item_charges;
		int   equip_item;
		float chance;
		float disabled_chance;
		int   trivial_min_level;
		int   trivial_max_level;
		int   multiplier;
		int   npc_min_level;
		int   npc_max_level;
	};

	static std::string PrimaryKey()
	{
		return std::string("lootdrop_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"lootdrop_id",
			"item_id",
			"item_charges",
			"equip_item",
			"chance",
			"disabled_chance",
			"trivial_min_level",
			"trivial_max_level",
			"multiplier",
			"npc_min_level",
			"npc_max_level",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("lootdrop_entries");
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

	static LootdropEntries NewEntity()
	{
		LootdropEntries entry{};

		entry.lootdrop_id       = 0;
		entry.item_id           = 0;
		entry.item_charges      = 1;
		entry.equip_item        = 0;
		entry.chance            = 1;
		entry.disabled_chance   = 0;
		entry.trivial_min_level = 0;
		entry.trivial_max_level = 0;
		entry.multiplier        = 1;
		entry.npc_min_level     = 0;
		entry.npc_max_level     = 0;

		return entry;
	}

	static LootdropEntries GetLootdropEntriesEntry(
		const std::vector<LootdropEntries> &lootdrop_entriess,
		int lootdrop_entries_id
	)
	{
		for (auto &lootdrop_entries : lootdrop_entriess) {
			if (lootdrop_entries.lootdrop_id == lootdrop_entries_id) {
				return lootdrop_entries;
			}
		}

		return NewEntity();
	}

	static LootdropEntries FindOne(
		Database& db,
		int lootdrop_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				lootdrop_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LootdropEntries entry{};

			entry.lootdrop_id       = atoi(row[0]);
			entry.item_id           = atoi(row[1]);
			entry.item_charges      = atoi(row[2]);
			entry.equip_item        = atoi(row[3]);
			entry.chance            = static_cast<float>(atof(row[4]));
			entry.disabled_chance   = static_cast<float>(atof(row[5]));
			entry.trivial_min_level = atoi(row[6]);
			entry.trivial_max_level = atoi(row[7]);
			entry.multiplier        = atoi(row[8]);
			entry.npc_min_level     = atoi(row[9]);
			entry.npc_max_level     = atoi(row[10]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int lootdrop_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				lootdrop_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LootdropEntries lootdrop_entries_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(lootdrop_entries_entry.lootdrop_id));
		update_values.push_back(columns[1] + " = " + std::to_string(lootdrop_entries_entry.item_id));
		update_values.push_back(columns[2] + " = " + std::to_string(lootdrop_entries_entry.item_charges));
		update_values.push_back(columns[3] + " = " + std::to_string(lootdrop_entries_entry.equip_item));
		update_values.push_back(columns[4] + " = " + std::to_string(lootdrop_entries_entry.chance));
		update_values.push_back(columns[5] + " = " + std::to_string(lootdrop_entries_entry.disabled_chance));
		update_values.push_back(columns[6] + " = " + std::to_string(lootdrop_entries_entry.trivial_min_level));
		update_values.push_back(columns[7] + " = " + std::to_string(lootdrop_entries_entry.trivial_max_level));
		update_values.push_back(columns[8] + " = " + std::to_string(lootdrop_entries_entry.multiplier));
		update_values.push_back(columns[9] + " = " + std::to_string(lootdrop_entries_entry.npc_min_level));
		update_values.push_back(columns[10] + " = " + std::to_string(lootdrop_entries_entry.npc_max_level));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				lootdrop_entries_entry.lootdrop_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LootdropEntries InsertOne(
		Database& db,
		LootdropEntries lootdrop_entries_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(lootdrop_entries_entry.lootdrop_id));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.item_id));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.item_charges));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.equip_item));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.chance));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.disabled_chance));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.trivial_min_level));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.trivial_max_level));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.multiplier));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.npc_min_level));
		insert_values.push_back(std::to_string(lootdrop_entries_entry.npc_max_level));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			lootdrop_entries_entry.lootdrop_id = results.LastInsertedID();
			return lootdrop_entries_entry;
		}

		lootdrop_entries_entry = NewEntity();

		return lootdrop_entries_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LootdropEntries> lootdrop_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &lootdrop_entries_entry: lootdrop_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(lootdrop_entries_entry.lootdrop_id));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.item_id));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.item_charges));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.equip_item));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.chance));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.disabled_chance));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.trivial_min_level));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.trivial_max_level));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.multiplier));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.npc_min_level));
			insert_values.push_back(std::to_string(lootdrop_entries_entry.npc_max_level));

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

	static std::vector<LootdropEntries> All(Database& db)
	{
		std::vector<LootdropEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LootdropEntries entry{};

			entry.lootdrop_id       = atoi(row[0]);
			entry.item_id           = atoi(row[1]);
			entry.item_charges      = atoi(row[2]);
			entry.equip_item        = atoi(row[3]);
			entry.chance            = static_cast<float>(atof(row[4]));
			entry.disabled_chance   = static_cast<float>(atof(row[5]));
			entry.trivial_min_level = atoi(row[6]);
			entry.trivial_max_level = atoi(row[7]);
			entry.multiplier        = atoi(row[8]);
			entry.npc_min_level     = atoi(row[9]);
			entry.npc_max_level     = atoi(row[10]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LootdropEntries> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LootdropEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LootdropEntries entry{};

			entry.lootdrop_id       = atoi(row[0]);
			entry.item_id           = atoi(row[1]);
			entry.item_charges      = atoi(row[2]);
			entry.equip_item        = atoi(row[3]);
			entry.chance            = static_cast<float>(atof(row[4]));
			entry.disabled_chance   = static_cast<float>(atof(row[5]));
			entry.trivial_min_level = atoi(row[6]);
			entry.trivial_max_level = atoi(row[7]);
			entry.multiplier        = atoi(row[8]);
			entry.npc_min_level     = atoi(row[9]);
			entry.npc_max_level     = atoi(row[10]);

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

#endif //EQEMU_BASE_LOOTDROP_ENTRIES_REPOSITORY_H
