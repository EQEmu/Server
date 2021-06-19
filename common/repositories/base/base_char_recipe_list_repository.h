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

#ifndef EQEMU_BASE_CHAR_RECIPE_LIST_REPOSITORY_H
#define EQEMU_BASE_CHAR_RECIPE_LIST_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharRecipeListRepository {
public:
	struct CharRecipeList {
		int char_id;
		int recipe_id;
		int madecount;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"recipe_id",
			"madecount",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("char_recipe_list");
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

	static CharRecipeList NewEntity()
	{
		CharRecipeList entry{};

		entry.char_id   = 0;
		entry.recipe_id = 0;
		entry.madecount = 0;

		return entry;
	}

	static CharRecipeList GetCharRecipeListEntry(
		const std::vector<CharRecipeList> &char_recipe_lists,
		int char_recipe_list_id
	)
	{
		for (auto &char_recipe_list : char_recipe_lists) {
			if (char_recipe_list.char_id == char_recipe_list_id) {
				return char_recipe_list;
			}
		}

		return NewEntity();
	}

	static CharRecipeList FindOne(
		Database& db,
		int char_recipe_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				char_recipe_list_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharRecipeList entry{};

			entry.char_id   = atoi(row[0]);
			entry.recipe_id = atoi(row[1]);
			entry.madecount = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int char_recipe_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				char_recipe_list_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharRecipeList char_recipe_list_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(char_recipe_list_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(char_recipe_list_entry.recipe_id));
		update_values.push_back(columns[2] + " = " + std::to_string(char_recipe_list_entry.madecount));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				char_recipe_list_entry.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharRecipeList InsertOne(
		Database& db,
		CharRecipeList char_recipe_list_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(char_recipe_list_entry.char_id));
		insert_values.push_back(std::to_string(char_recipe_list_entry.recipe_id));
		insert_values.push_back(std::to_string(char_recipe_list_entry.madecount));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			char_recipe_list_entry.char_id = results.LastInsertedID();
			return char_recipe_list_entry;
		}

		char_recipe_list_entry = NewEntity();

		return char_recipe_list_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharRecipeList> char_recipe_list_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &char_recipe_list_entry: char_recipe_list_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(char_recipe_list_entry.char_id));
			insert_values.push_back(std::to_string(char_recipe_list_entry.recipe_id));
			insert_values.push_back(std::to_string(char_recipe_list_entry.madecount));

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

	static std::vector<CharRecipeList> All(Database& db)
	{
		std::vector<CharRecipeList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharRecipeList entry{};

			entry.char_id   = atoi(row[0]);
			entry.recipe_id = atoi(row[1]);
			entry.madecount = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharRecipeList> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharRecipeList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharRecipeList entry{};

			entry.char_id   = atoi(row[0]);
			entry.recipe_id = atoi(row[1]);
			entry.madecount = atoi(row[2]);

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

#endif //EQEMU_BASE_CHAR_RECIPE_LIST_REPOSITORY_H
