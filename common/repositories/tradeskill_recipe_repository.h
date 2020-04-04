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
		int         tradeskill;
		int         skillneeded;
		int         trivial;
		uint8       nofail;
		int         replace_container;
		std::string notes;
		uint8       must_learn;
		uint8       quest;
		uint8       enabled;
	};

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

	static std::string TableName()
	{
		return std::string("tradeskill_recipe");
	}

	static std::string BaseSelect()
	{
		return std::string(
			fmt::format(
				"SELECT {} FROM {}",
				ColumnsRaw(),
				TableName()
			)
		);
	}

	static TradeskillRecipe NewEntity()
	{
		TradeskillRecipe entry;

		entry.id                = 0;
		entry.name              = "";
		entry.tradeskill        = 0;
		entry.skillneeded       = 0;
		entry.trivial           = 0;
		entry.nofail            = 0;
		entry.replace_container = 0;
		entry.notes             = "";
		entry.must_learn        = 0;
		entry.quest             = 0;
		entry.enabled           = 0;

		return entry;
	}

	static TradeskillRecipe GetRecipe(int recipe_id)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {}",
				BaseSelect(),
				recipe_id
			)
		);

		TradeskillRecipe tradeskill_recipe = NewEntity();

		auto row = results.begin();
		if (results.RowCount() == 0) {
			return tradeskill_recipe;
		}

		tradeskill_recipe.id                = atoi(row[0]);
		tradeskill_recipe.name              = (row[1] ? row[1] : "");
		tradeskill_recipe.tradeskill        = atoi(row[2]);
		tradeskill_recipe.skillneeded       = atoi(row[3]);
		tradeskill_recipe.trivial           = atoi(row[4]);
		tradeskill_recipe.nofail            = atoi(row[5]);
		tradeskill_recipe.replace_container = atoi(row[6]);
		tradeskill_recipe.notes             = (row[7] ? row[7] : "");
		tradeskill_recipe.must_learn        = atoi(row[8]);
		tradeskill_recipe.quest             = atoi(row[9]);
		tradeskill_recipe.enabled           = atoi(row[10]);

		return tradeskill_recipe;
	}

};

#endif
