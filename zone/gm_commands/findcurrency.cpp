#include "../client.h"

void command_findcurrency(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #findcurrency [Search Criteria]");
		return;
	}

	const auto can_summon_items = c->Admin() >= GetCommandStatus(c, "summonitem");

	if (sep->IsNumber(1)) {
		const auto item_id     = Strings::ToUnsignedInt(sep->arg[1]);
		const auto currency_id = zone->GetCurrencyID(item_id);

		if (!currency_id) {
			c->Message(
				Chat::White,
				fmt::format(
					"There is no currency with an item ID of {}.",
					item_id
				).c_str()
			);

			return;
		}

		const auto item_data = database.GetItem(item_id);
		if (!item_data) {
			c->Message(
				Chat::White,
				fmt::format(
					"Item ID {} does not exist.",
					item_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Currency {} | {} ({}){}",
				currency_id,
				database.CreateItemLink(item_id),
				item_id,
				(
					can_summon_items ?
						fmt::format(
							" | {}",
							Saylink::Silent(
								fmt::format(
									"#summonitem {} {}",
									item_id,
									item_data->StackSize
								),
								"Summon"
							)
						) :
						""
				)
			).c_str()
		);

		return;
	}

	const std::string search_criteria  = sep->argplus[1];

	uint32 found_count = 0;

	for (const auto& e : zone->AlternateCurrencies) {
		const auto item_data = database.GetItem(e.item_id);
		if (!item_data) {
			continue;
		}

		const std::string item_name = Strings::ToLower(item_data->Name);

		if (Strings::Contains(item_name, Strings::ToLower(search_criteria))) {
			c->Message(
				Chat::White,
				fmt::format(
					"Currency {} | {} ({}){}",
					e.id,
					database.CreateItemLink(e.item_id),
					e.item_id,
					(
						can_summon_items ?
						fmt::format(
							" | {}",
							Saylink::Silent(
								fmt::format(
									"#summonitem {} {}",
									e.item_id,
									item_data->StackSize
								),
								"Summon"
							)
						) :
						""
					)
				).c_str()
			);

			found_count++;
		}
	}

	if (!found_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"No currencies were found matching '{}'.",
				search_criteria
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} currenc{} found matching '{}'.",
			found_count,
			found_count != 1 ? "ies were" : "y was",
			search_criteria
		).c_str()
	);
}

