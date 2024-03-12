#include "../../client.h"

void FindDeity(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto  deity_id   = static_cast<EQ::deity::DeityType>(Strings::ToInt(sep->arg[2]));
		const auto& deity_name = EQ::deity::GetDeityName(deity_id);
		if (!deity_name.empty()) {
			const auto deity_bit = EQ::deity::GetDeityBitmask(deity_id);

			c->Message(
				Chat::White,
				fmt::format(
					"Deity {} | {} ({})",
					deity_id,
					deity_name,
					Strings::Commify(deity_bit)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Deity ID {} was not found.",
				deity_id
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	for (const auto& d : EQ::deity::GetDeityMap()) {
		const auto& deity_name_lower = Strings::ToLower(d.second);
		if (!Strings::Contains(deity_name_lower, search_criteria)) {
			continue;
		}

		const auto deity_bit = EQ::deity::GetDeityBitmask(d.first);

		c->Message(
			Chat::White,
			fmt::format(
				"Deity {} | {} ({})",
				d.first,
				d.second,
				Strings::Commify(deity_bit)
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Deit{} found matching '{}'.",
			found_count,
			found_count != 1 ? "ies" : "y",
			sep->argplus[2]
		).c_str()
	);
}
