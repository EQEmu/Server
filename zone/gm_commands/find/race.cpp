#include "../../client.h"

void FindRace(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint16 race_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& race_name = Race::GetName(race_id);
		if (Strings::EqualFold(race_name, "UNKNOWN RACE")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Race ID {} does not exist.",
					race_id
				).c_str()
			);

			return;
		}

		std::string bitmask_string;

		if (Race::IsPlayerRace(race_id)) {
			bitmask_string = fmt::format(
				" ({})",
				Strings::Commify(Race::GetPlayerBit(race_id))
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Race {} | {}{}",
				race_id,
				race_name,
				bitmask_string
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (uint16 race_id = Race::Human; race_id <= Race::Pegasus3 + 2; race_id++) {
		uint16 current_race_id = race_id;
		if (current_race_id > Race::Pegasus3) {
			if (current_race_id == Race::Pegasus3 + 1) {
				current_race_id = Race::InteractiveObject;
			} else if (current_race_id == Race::Pegasus3 + 2) {
				current_race_id = Race::Node;
			}
		}

		const std::string& race_name = Race::GetName(current_race_id);
		const std::string& race_name_lower = Strings::ToLower(race_name);
		if (!Strings::Contains(race_name_lower, search_criteria)) {
			continue;
		}

		std::string bitmask_string;

		if (Race::IsPlayerRace(race_id)) {
			bitmask_string = fmt::format(
				" ({})",
				Strings::Commify(Race::GetPlayerBit(race_id))
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Race {} | {}{}",
				race_id,
				race_name,
				bitmask_string
			).c_str()
		);

		found_count++;

		if (found_count == 100) {
			break;
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Race{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
