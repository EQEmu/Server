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

#ifndef EQEMU_BASE_GLOBAL_LOOT_REPOSITORY_H
#define EQEMU_BASE_GLOBAL_LOOT_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGlobalLootRepository {
public:
	struct GlobalLoot {
		int         id;
		std::string description;
		int         loottable_id;
		int         enabled;
		int         min_level;
		int         max_level;
		int         rare;
		int         raid;
		std::string race;
		std::string class_;
		std::string bodytype;
		std::string zone;
		int         hot_zone;
		int         min_expansion;
		int         max_expansion;
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
			"description",
			"loottable_id",
			"enabled",
			"min_level",
			"max_level",
			"rare",
			"raid",
			"race",
			"`class`",
			"bodytype",
			"zone",
			"hot_zone",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("global_loot");
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

	static GlobalLoot NewEntity()
	{
		GlobalLoot entry{};

		entry.id                     = 0;
		entry.description            = "";
		entry.loottable_id           = 0;
		entry.enabled                = 1;
		entry.min_level              = 0;
		entry.max_level              = 0;
		entry.rare                   = 0;
		entry.raid                   = 0;
		entry.race                   = "";
		entry.class_                 = "";
		entry.bodytype               = "";
		entry.zone                   = "";
		entry.hot_zone               = 0;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static GlobalLoot GetGlobalLootEntry(
		const std::vector<GlobalLoot> &global_loots,
		int global_loot_id
	)
	{
		for (auto &global_loot : global_loots) {
			if (global_loot.id == global_loot_id) {
				return global_loot;
			}
		}

		return NewEntity();
	}

	static GlobalLoot FindOne(
		Database& db,
		int global_loot_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				global_loot_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GlobalLoot entry{};

			entry.id                     = atoi(row[0]);
			entry.description            = row[1] ? row[1] : "";
			entry.loottable_id           = atoi(row[2]);
			entry.enabled                = atoi(row[3]);
			entry.min_level              = atoi(row[4]);
			entry.max_level              = atoi(row[5]);
			entry.rare                   = atoi(row[6]);
			entry.raid                   = atoi(row[7]);
			entry.race                   = row[8] ? row[8] : "";
			entry.class_                 = row[9] ? row[9] : "";
			entry.bodytype               = row[10] ? row[10] : "";
			entry.zone                   = row[11] ? row[11] : "";
			entry.hot_zone               = atoi(row[12]);
			entry.min_expansion          = atoi(row[13]);
			entry.max_expansion          = atoi(row[14]);
			entry.content_flags          = row[15] ? row[15] : "";
			entry.content_flags_disabled = row[16] ? row[16] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int global_loot_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				global_loot_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		GlobalLoot global_loot_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(global_loot_entry.description) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(global_loot_entry.loottable_id));
		update_values.push_back(columns[3] + " = " + std::to_string(global_loot_entry.enabled));
		update_values.push_back(columns[4] + " = " + std::to_string(global_loot_entry.min_level));
		update_values.push_back(columns[5] + " = " + std::to_string(global_loot_entry.max_level));
		update_values.push_back(columns[6] + " = " + std::to_string(global_loot_entry.rare));
		update_values.push_back(columns[7] + " = " + std::to_string(global_loot_entry.raid));
		update_values.push_back(columns[8] + " = '" + EscapeString(global_loot_entry.race) + "'");
		update_values.push_back(columns[9] + " = '" + EscapeString(global_loot_entry.class_) + "'");
		update_values.push_back(columns[10] + " = '" + EscapeString(global_loot_entry.bodytype) + "'");
		update_values.push_back(columns[11] + " = '" + EscapeString(global_loot_entry.zone) + "'");
		update_values.push_back(columns[12] + " = " + std::to_string(global_loot_entry.hot_zone));
		update_values.push_back(columns[13] + " = " + std::to_string(global_loot_entry.min_expansion));
		update_values.push_back(columns[14] + " = " + std::to_string(global_loot_entry.max_expansion));
		update_values.push_back(columns[15] + " = '" + EscapeString(global_loot_entry.content_flags) + "'");
		update_values.push_back(columns[16] + " = '" + EscapeString(global_loot_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				global_loot_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GlobalLoot InsertOne(
		Database& db,
		GlobalLoot global_loot_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(global_loot_entry.id));
		insert_values.push_back("'" + EscapeString(global_loot_entry.description) + "'");
		insert_values.push_back(std::to_string(global_loot_entry.loottable_id));
		insert_values.push_back(std::to_string(global_loot_entry.enabled));
		insert_values.push_back(std::to_string(global_loot_entry.min_level));
		insert_values.push_back(std::to_string(global_loot_entry.max_level));
		insert_values.push_back(std::to_string(global_loot_entry.rare));
		insert_values.push_back(std::to_string(global_loot_entry.raid));
		insert_values.push_back("'" + EscapeString(global_loot_entry.race) + "'");
		insert_values.push_back("'" + EscapeString(global_loot_entry.class_) + "'");
		insert_values.push_back("'" + EscapeString(global_loot_entry.bodytype) + "'");
		insert_values.push_back("'" + EscapeString(global_loot_entry.zone) + "'");
		insert_values.push_back(std::to_string(global_loot_entry.hot_zone));
		insert_values.push_back(std::to_string(global_loot_entry.min_expansion));
		insert_values.push_back(std::to_string(global_loot_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(global_loot_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(global_loot_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			global_loot_entry.id = results.LastInsertedID();
			return global_loot_entry;
		}

		global_loot_entry = NewEntity();

		return global_loot_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<GlobalLoot> global_loot_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &global_loot_entry: global_loot_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(global_loot_entry.id));
			insert_values.push_back("'" + EscapeString(global_loot_entry.description) + "'");
			insert_values.push_back(std::to_string(global_loot_entry.loottable_id));
			insert_values.push_back(std::to_string(global_loot_entry.enabled));
			insert_values.push_back(std::to_string(global_loot_entry.min_level));
			insert_values.push_back(std::to_string(global_loot_entry.max_level));
			insert_values.push_back(std::to_string(global_loot_entry.rare));
			insert_values.push_back(std::to_string(global_loot_entry.raid));
			insert_values.push_back("'" + EscapeString(global_loot_entry.race) + "'");
			insert_values.push_back("'" + EscapeString(global_loot_entry.class_) + "'");
			insert_values.push_back("'" + EscapeString(global_loot_entry.bodytype) + "'");
			insert_values.push_back("'" + EscapeString(global_loot_entry.zone) + "'");
			insert_values.push_back(std::to_string(global_loot_entry.hot_zone));
			insert_values.push_back(std::to_string(global_loot_entry.min_expansion));
			insert_values.push_back(std::to_string(global_loot_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(global_loot_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(global_loot_entry.content_flags_disabled) + "'");

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

	static std::vector<GlobalLoot> All(Database& db)
	{
		std::vector<GlobalLoot> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GlobalLoot entry{};

			entry.id                     = atoi(row[0]);
			entry.description            = row[1] ? row[1] : "";
			entry.loottable_id           = atoi(row[2]);
			entry.enabled                = atoi(row[3]);
			entry.min_level              = atoi(row[4]);
			entry.max_level              = atoi(row[5]);
			entry.rare                   = atoi(row[6]);
			entry.raid                   = atoi(row[7]);
			entry.race                   = row[8] ? row[8] : "";
			entry.class_                 = row[9] ? row[9] : "";
			entry.bodytype               = row[10] ? row[10] : "";
			entry.zone                   = row[11] ? row[11] : "";
			entry.hot_zone               = atoi(row[12]);
			entry.min_expansion          = atoi(row[13]);
			entry.max_expansion          = atoi(row[14]);
			entry.content_flags          = row[15] ? row[15] : "";
			entry.content_flags_disabled = row[16] ? row[16] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GlobalLoot> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<GlobalLoot> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GlobalLoot entry{};

			entry.id                     = atoi(row[0]);
			entry.description            = row[1] ? row[1] : "";
			entry.loottable_id           = atoi(row[2]);
			entry.enabled                = atoi(row[3]);
			entry.min_level              = atoi(row[4]);
			entry.max_level              = atoi(row[5]);
			entry.rare                   = atoi(row[6]);
			entry.raid                   = atoi(row[7]);
			entry.race                   = row[8] ? row[8] : "";
			entry.class_                 = row[9] ? row[9] : "";
			entry.bodytype               = row[10] ? row[10] : "";
			entry.zone                   = row[11] ? row[11] : "";
			entry.hot_zone               = atoi(row[12]);
			entry.min_expansion          = atoi(row[13]);
			entry.max_expansion          = atoi(row[14]);
			entry.content_flags          = row[15] ? row[15] : "";
			entry.content_flags_disabled = row[16] ? row[16] : "";

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

#endif //EQEMU_BASE_GLOBAL_LOOT_REPOSITORY_H
