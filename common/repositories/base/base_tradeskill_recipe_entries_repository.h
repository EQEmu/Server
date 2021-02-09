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

#ifndef EQEMU_BASE_TRADESKILL_RECIPE_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_TRADESKILL_RECIPE_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTradeskillRecipeEntriesRepository {
public:
	struct TradeskillRecipeEntries {
		int id;
		int recipe_id;
		int item_id;
		int successcount;
		int failcount;
		int componentcount;
		int salvagecount;
		int iscontainer;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"recipe_id",
			"item_id",
			"successcount",
			"failcount",
			"componentcount",
			"salvagecount",
			"iscontainer",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("tradeskill_recipe_entries");
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

	static TradeskillRecipeEntries NewEntity()
	{
		TradeskillRecipeEntries entry{};

		entry.id             = 0;
		entry.recipe_id      = 0;
		entry.item_id        = 0;
		entry.successcount   = 0;
		entry.failcount      = 0;
		entry.componentcount = 1;
		entry.salvagecount   = 0;
		entry.iscontainer    = 0;

		return entry;
	}

	static TradeskillRecipeEntries GetTradeskillRecipeEntriesEntry(
		const std::vector<TradeskillRecipeEntries> &tradeskill_recipe_entriess,
		int tradeskill_recipe_entries_id
	)
	{
		for (auto &tradeskill_recipe_entries : tradeskill_recipe_entriess) {
			if (tradeskill_recipe_entries.id == tradeskill_recipe_entries_id) {
				return tradeskill_recipe_entries;
			}
		}

		return NewEntity();
	}

	static TradeskillRecipeEntries FindOne(
		Database& db,
		int tradeskill_recipe_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tradeskill_recipe_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			TradeskillRecipeEntries entry{};

			entry.id             = atoi(row[0]);
			entry.recipe_id      = atoi(row[1]);
			entry.item_id        = atoi(row[2]);
			entry.successcount   = atoi(row[3]);
			entry.failcount      = atoi(row[4]);
			entry.componentcount = atoi(row[5]);
			entry.salvagecount   = atoi(row[6]);
			entry.iscontainer    = atoi(row[7]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tradeskill_recipe_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tradeskill_recipe_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		TradeskillRecipeEntries tradeskill_recipe_entries_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(tradeskill_recipe_entries_entry.recipe_id));
		update_values.push_back(columns[2] + " = " + std::to_string(tradeskill_recipe_entries_entry.item_id));
		update_values.push_back(columns[3] + " = " + std::to_string(tradeskill_recipe_entries_entry.successcount));
		update_values.push_back(columns[4] + " = " + std::to_string(tradeskill_recipe_entries_entry.failcount));
		update_values.push_back(columns[5] + " = " + std::to_string(tradeskill_recipe_entries_entry.componentcount));
		update_values.push_back(columns[6] + " = " + std::to_string(tradeskill_recipe_entries_entry.salvagecount));
		update_values.push_back(columns[7] + " = " + std::to_string(tradeskill_recipe_entries_entry.iscontainer));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				tradeskill_recipe_entries_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static TradeskillRecipeEntries InsertOne(
		Database& db,
		TradeskillRecipeEntries tradeskill_recipe_entries_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.id));
		insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.recipe_id));
		insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.item_id));
		insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.successcount));
		insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.failcount));
		insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.componentcount));
		insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.salvagecount));
		insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.iscontainer));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			tradeskill_recipe_entries_entry.id = results.LastInsertedID();
			return tradeskill_recipe_entries_entry;
		}

		tradeskill_recipe_entries_entry = NewEntity();

		return tradeskill_recipe_entries_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<TradeskillRecipeEntries> tradeskill_recipe_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &tradeskill_recipe_entries_entry: tradeskill_recipe_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.id));
			insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.recipe_id));
			insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.item_id));
			insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.successcount));
			insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.failcount));
			insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.componentcount));
			insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.salvagecount));
			insert_values.push_back(std::to_string(tradeskill_recipe_entries_entry.iscontainer));

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

	static std::vector<TradeskillRecipeEntries> All(Database& db)
	{
		std::vector<TradeskillRecipeEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TradeskillRecipeEntries entry{};

			entry.id             = atoi(row[0]);
			entry.recipe_id      = atoi(row[1]);
			entry.item_id        = atoi(row[2]);
			entry.successcount   = atoi(row[3]);
			entry.failcount      = atoi(row[4]);
			entry.componentcount = atoi(row[5]);
			entry.salvagecount   = atoi(row[6]);
			entry.iscontainer    = atoi(row[7]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<TradeskillRecipeEntries> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<TradeskillRecipeEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TradeskillRecipeEntries entry{};

			entry.id             = atoi(row[0]);
			entry.recipe_id      = atoi(row[1]);
			entry.item_id        = atoi(row[2]);
			entry.successcount   = atoi(row[3]);
			entry.failcount      = atoi(row[4]);
			entry.componentcount = atoi(row[5]);
			entry.salvagecount   = atoi(row[6]);
			entry.iscontainer    = atoi(row[7]);

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

#endif //EQEMU_BASE_TRADESKILL_RECIPE_ENTRIES_REPOSITORY_H
