/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EQEMU_TRADESKILL_RECIPE_REPOSITORY_H
#define EQEMU_TRADESKILL_RECIPE_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class TradeskillRecipeRepository {
public:
	struct TradeskillRecipe {
		int         id;
		std::string name;
		int16       tradeskill;
		int16       skillneeded;
		int16       trivial;
		int8        nofail;
		int8        replace_container;
		std::string notes;
		int8        must_learn;
		int8        quest;
		int8        enabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"tradeskill",
			"skillneeded",
			"trivial",
			"nofail",
			"replace_container",
			"notes",
			"must_learn",
			"quest",
			"enabled",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("tradeskill_recipe");
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
			InsertColumnsRaw()
		);
	}

	static TradeskillRecipe NewEntity()
	{
		TradeskillRecipe entry{};

		entry.id                = 0;
		entry.name              = "";
		entry.tradeskill        = 0;
		entry.skillneeded       = 0;
		entry.trivial           = 0;
		entry.nofail            = 0;
		entry.replace_container = 0;
		entry.notes             = 0;
		entry.must_learn        = 0;
		entry.quest             = 0;
		entry.enabled           = 1;

		return entry;
	}

	static TradeskillRecipe GetTradeskillRecipeEntry(
		const std::vector<TradeskillRecipe> &tradeskill_recipes,
		int tradeskill_recipe_id
	)
	{
		for (auto &tradeskill_recipe : tradeskill_recipes) {
			if (tradeskill_recipe.id == tradeskill_recipe_id) {
				return tradeskill_recipe;
			}
		}

		return NewEntity();
	}

	static TradeskillRecipe FindOne(
		int tradeskill_recipe_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tradeskill_recipe_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			TradeskillRecipe entry{};

			entry.id                = atoi(row[0]);
			entry.name              = row[1];
			entry.tradeskill        = atoi(row[2]);
			entry.skillneeded       = atoi(row[3]);
			entry.trivial           = atoi(row[4]);
			entry.nofail            = atoi(row[5]);
			entry.replace_container = atoi(row[6]);
			entry.notes             = row[7];
			entry.must_learn        = atoi(row[8]);
			entry.quest             = atoi(row[9]);
			entry.enabled           = atoi(row[10]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int tradeskill_recipe_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tradeskill_recipe_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		TradeskillRecipe tradeskill_recipe_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(tradeskill_recipe_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(tradeskill_recipe_entry.tradeskill));
		update_values.push_back(columns[3] + " = " + std::to_string(tradeskill_recipe_entry.skillneeded));
		update_values.push_back(columns[4] + " = " + std::to_string(tradeskill_recipe_entry.trivial));
		update_values.push_back(columns[5] + " = " + std::to_string(tradeskill_recipe_entry.nofail));
		update_values.push_back(columns[6] + " = " + std::to_string(tradeskill_recipe_entry.replace_container));
		update_values.push_back(columns[7] + " = '" + EscapeString(tradeskill_recipe_entry.notes) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(tradeskill_recipe_entry.must_learn));
		update_values.push_back(columns[9] + " = " + std::to_string(tradeskill_recipe_entry.quest));
		update_values.push_back(columns[10] + " = " + std::to_string(tradeskill_recipe_entry.enabled));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				tradeskill_recipe_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static TradeskillRecipe InsertOne(
		TradeskillRecipe tradeskill_recipe_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.name) + "'");
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.tradeskill));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.skillneeded));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.trivial));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.nofail));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.replace_container));
		insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.notes) + "'");
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.must_learn));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.quest));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.enabled));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			tradeskill_recipe_entry.id = results.LastInsertedID();
			return tradeskill_recipe_entry;
		}

		tradeskill_recipe_entry = InstanceListRepository::NewEntity();

		return tradeskill_recipe_entry;
	}

	static int InsertMany(
		std::vector<TradeskillRecipe> tradeskill_recipe_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &tradeskill_recipe_entry: tradeskill_recipe_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.name) + "'");
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.tradeskill));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.skillneeded));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.trivial));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.nofail));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.replace_container));
			insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.notes) + "'");
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.must_learn));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.quest));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.enabled));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<TradeskillRecipe> All()
	{
		std::vector<TradeskillRecipe> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TradeskillRecipe entry{};

			entry.id                = atoi(row[0]);
			entry.name              = row[1];
			entry.tradeskill        = atoi(row[2]);
			entry.skillneeded       = atoi(row[3]);
			entry.trivial           = atoi(row[4]);
			entry.nofail            = atoi(row[5]);
			entry.replace_container = atoi(row[6]);
			entry.notes             = row[7];
			entry.must_learn        = atoi(row[8]);
			entry.quest             = atoi(row[9]);
			entry.enabled           = atoi(row[10]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_TRADESKILL_RECIPE_REPOSITORY_H
