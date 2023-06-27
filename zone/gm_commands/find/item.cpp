#include "../../client.h"
#include "../../common/repositories/items_repository.h"

void FindItem(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto  item_id = Strings::ToUnsignedInt(sep->arg[2]);
		const auto* item    = database.GetItem(item_id);
		if (item) {
			auto summon_links = Saylink::Silent(
				fmt::format(
					"#si {}",
					item_id
				),
				"X"
			);

			if (item->Stackable && item->StackSize > 1) {
				summon_links += fmt::format(
					" | {}",
					Saylink::Silent(
						fmt::format(
							"#si {} {}",
							item_id,
							item->StackSize
						),
						std::to_string(item->StackSize)
					)
				);
			}

			c->Message(
				Chat::White,
				fmt::format(
					"[{}] [{}]",
					summon_links,
					database.CreateItemLink(item_id)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Item ID {} not found",
				item_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	const auto& l = ItemsRepository::GetWhere(
		content_db,
		fmt::format(
			"LOWER(`name`) LIKE '%{}%'",
			search_criteria
		)
	);

	if (l.empty() || !l[0].id) {
		c->Message(
			Chat::White,
			fmt::format(
				"No items were found matching '{}'.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	auto found_count = 0;

	for (const auto& e : l) {
		const auto item_id = e.id;
		const auto *item = database.GetItem(item_id);
		auto summon_links = Saylink::Silent(
			fmt::format(
				"#si {}",
				item_id
			),
			"X"
		);

		if (item->Stackable && item->StackSize > 1) {
			summon_links += fmt::format(
				" | {}",
				Saylink::Silent(
					fmt::format(
						"#si {} {}",
						item_id,
						item->StackSize
					),
					std::to_string(item->StackSize)
				)
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"[{}] [{}]",
				summon_links,
				database.CreateItemLink(item_id)
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Items found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Item{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" :"",
			sep->argplus[2]
		).c_str()
	);
}

