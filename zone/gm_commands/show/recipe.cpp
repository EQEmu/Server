#include "../../client.h"
#include "../../command.h"
#include "../../common/repositories/tradeskill_recipe_repository.h"
#include "../../common/repositories/tradeskill_recipe_entries_repository.h"

void ShowRecipe(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(2)) {
		c->Message(Chat::White, "Command Syntax: #show recipe [Recipe ID]");
		return;
	}

	const uint16 recipe_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));

	const auto& re = TradeskillRecipeEntriesRepository::GetWhere(
		database,
		fmt::format("recipe_id = {} ORDER BY id ASC", recipe_id)
	);

	const auto& r = TradeskillRecipeRepository::GetWhere(
		database,
		fmt::format("id = {}", recipe_id)
	);

	if (re.empty() || r.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Recipe ID {} has no entries or could not be found.",
				Strings::Commify(recipe_id)
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Recipe {} | {}",
			Strings::Commify(recipe_id),
			r[0].name
		).c_str()
	);

	uint32 entry_number = 1;
	const bool can_summon_items = c->Admin() >= GetCommandStatus(c, "summonitem");

	for (const auto& e : re) {
		c->Message(
			Chat::White,
			fmt::format(
				"Entry {}{} | {}{}",
				entry_number,
				e.iscontainer > 0 ? " (Container)" : "",
				(
					e.item_id > 1000 ?
					database.CreateItemLink(e.item_id) :
					EQ::constants::GetObjectTypeName(e.item_id)
				),
				(
					can_summon_items && e.item_id > 1000 ?
					fmt::format(
						" | {}",
						Saylink::Silent(
							fmt::format("#si {}", e.item_id),
							"Summon"
						)
					) :
					""
				)
			).c_str()
		);

		std::vector<std::string> emv;
		bool has_message = false;

		if (e.componentcount) {
			emv.push_back(
				fmt::format(
					"Component: {}",
					e.componentcount
				)
			);

			has_message = true;
		}

		if (e.failcount) {
			emv.push_back(
				fmt::format(
					"Fail: {}",
					e.failcount
				)
			);

			has_message = true;
		}

		if (e.salvagecount) {
			emv.push_back(
				fmt::format(
					"Salvage: {}",
					e.salvagecount
				)
			);

			has_message = true;
		}

		if (e.successcount) {
			emv.push_back(
				fmt::format(
					"Success: {}",
					e.successcount
				)
			);

			has_message = true;
		}

		if (has_message) {
			c->Message(
				Chat::White,
				fmt::format(
					"Entry {} Counts | {}",
					entry_number,
					Strings::Implode(" | ", emv)
				).c_str()
			);
		}

		entry_number++;
	}
}
