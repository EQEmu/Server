#include "../client.h"
#include "../command.h"
#include "../../common/repositories/tradeskill_recipe_repository.h"
#include "../../common/repositories/tradeskill_recipe_entries_repository.h"

void command_viewrecipe(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Command Syntax: #viewrecipe [Recipe ID]");
		return;
	}

	auto recipe_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	auto re = TradeskillRecipeEntriesRepository::GetWhere(
		database,
		fmt::format("recipe_id = {} ORDER BY id ASC", recipe_id)
	);
	auto r = TradeskillRecipeRepository::GetWhere(
		database,
		fmt::format("id = {}", recipe_id)
	);

	if (re.empty() || r.empty() || !re[0].id || !r[0].id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Recipe ID {} has no entries or could not be found.",
				Strings::Commify(std::to_string(recipe_id))
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Recipe {} | {}",
			Strings::Commify(std::to_string(recipe_id)),
			r[0].name
		).c_str()
	);

	auto entry_number = 1;
	bool can_summon_items = c->Admin() >= GetCommandStatus(c, "summonitem");

	for (const auto& e : re) {
		c->Message(
			Chat::White,
			fmt::format(
				"Entry {}{} | {}{}",
				entry_number,
				e.iscontainer > 0 ? " (Container)" : "",
				e.item_id > 1000 ? database.CreateItemLink(e.item_id) : EQ::constants::GetObjectTypeName(e.item_id),
				can_summon_items && e.item_id > 1000 ? fmt::format(" | {}", Saylink::Silent(fmt::format("#si {}", e.item_id), "Summon")) : ""
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

