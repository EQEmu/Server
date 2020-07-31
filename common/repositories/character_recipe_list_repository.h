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

#ifndef EQEMU_CHARACTER_RECIPE_LIST_REPOSITORY_H
#define EQEMU_CHARACTER_RECIPE_LIST_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class CharacterRecipeListRepository {
public:
	struct CharacterRecipeList {
		int character_id;
		int recipe_id;
		int made_count;
	};

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
		return std::string(
			fmt::format(
				"SELECT {} FROM {}",
				ColumnsRaw(),
				TableName()
			)
		);
	}

	static CharacterRecipeList NewEntity()
	{
		CharacterRecipeList character_recipe_list_entry;

		character_recipe_list_entry.character_id = 0;
		character_recipe_list_entry.made_count   = 0;
		character_recipe_list_entry.recipe_id    = 0;

		return character_recipe_list_entry;
	}

	static std::vector<CharacterRecipeList> GetLearnedRecipeList(int character_id)
	{
		std::vector<CharacterRecipeList> character_recipe_list;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE char_id = {}",
				BaseSelect(),
				character_id
			)
		);

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterRecipeList character_recipe_list_entry;

			character_recipe_list_entry.character_id = character_id;
			character_recipe_list_entry.recipe_id    = atoi(row[1]);
			character_recipe_list_entry.made_count   = atoi(row[2]);

			character_recipe_list.push_back(character_recipe_list_entry);
		}

		return character_recipe_list;
	}

	static CharacterRecipeList GetRecipe(
		std::vector<CharacterRecipeList> character_recipe_list,
		int recipe_id
	)
	{
		for (auto &row : character_recipe_list) {
			if (row.recipe_id == recipe_id) {
				return row;
			}
		}

		return NewEntity();
	}

};

#endif //EQEMU_CHARACTER_RECIPE_LIST_REPOSITORY_H
