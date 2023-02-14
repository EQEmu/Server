#include "../client.h"
#include "../command.h"
#include "../../common/repositories/tradeskill_recipe_repository.h"

void command_findrecipe(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Command Syntax: #findrecipe [Search Criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		auto recipe_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
		auto r = TradeskillRecipeRepository::GetWhere(
			database,
			fmt::format("id = {}", recipe_id)
		);

		if (r.empty() || !r[0].id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Recipe ID {} could not be found.",
					Strings::Commify(std::to_string(recipe_id))
				).c_str()
			);
			return;
		}

		bool can_view_recipes = c->Admin() >= GetCommandStatus(c, "viewrecipe");

		c->Message(
			Chat::White,
			fmt::format(
				"Recipe {} | {}{}",
				Strings::Commify(std::to_string(recipe_id)),
				r[0].name,
				can_view_recipes ? fmt::format(" | {}", Saylink::Silent(fmt::format("#viewrecipe {}", r[0].id), "View")) : ""
			).c_str()
		);
	} else {
		auto search_criteria = Strings::ToLower(sep->argplus[1]);
		int found_count = 0;

		auto rl = TradeskillRecipeRepository::GetWhere(
			database,
			fmt::format("`name` LIKE '%{}%' ORDER BY `id` ASC", search_criteria)
		);

		if (rl.empty() || !rl[0].id) {
			c->Message(
				Chat::White,
				fmt::format(
					"No recipes were found matching '{}'.",
					search_criteria
				).c_str()
			);
			return;
		}

		bool can_view_recipes = c->Admin() >= GetCommandStatus(c, "viewrecipe");

		for (const auto& r : rl) {
			c->Message(
				Chat::White,
				fmt::format(
					"Recipe {} | {}{}",
					Strings::Commify(std::to_string(r.id)),
					r.name,
					can_view_recipes ? fmt::format(" | {}", Saylink::Silent(fmt::format("#viewrecipe {}", r.id), "View")) : ""
				).c_str()
			);

			if (found_count == 50) {
				break;
			}

			found_count++;
		}

		if (found_count == 50) {
			c->Message(Chat::White, "50 Recipes found, max reached.");
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} Recipe{} found.",
					found_count,
					found_count != 1 ? "s" : ""
				).c_str()
			);
		}
	}
}

