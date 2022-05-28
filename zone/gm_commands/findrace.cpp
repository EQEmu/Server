#include "../client.h"

void command_findrace(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Command Syntax: #findrace [Search Criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		auto race_id = static_cast<uint16>(std::stoul(sep->arg[1]));
		std::string race_name = GetRaceIDName(race_id);
		if (
			race_id >= RACE_HUMAN_1 &&
			race_id <= RACE_PEGASUS_732
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Race {} | {}{}",
					race_id,
					race_name,
					(
						c->IsPlayerRace(race_id) ?
						fmt::format(
							" ({})",
							GetPlayerRaceBit(race_id)
						) :
						""
					)
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Race ID {} was not found.",
					race_id
				).c_str()
			);
		}
	} else {
		auto search_criteria = str_tolower(sep->argplus[1]);
		int found_count = 0;
		for (uint16 race_id = RACE_HUMAN_1; race_id <= RACE_PEGASUS_732; race_id++) {
			std::string race_name = GetRaceIDName(race_id);
			auto race_name_lower = str_tolower(race_name);
			if (
				search_criteria.length() && 
				race_name_lower.find(search_criteria) == std::string::npos
			) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Race {} | {}{}",
					race_id,
					race_name,
					(
						c->IsPlayerRace(race_id) ?
						fmt::format(
							" ({})",
							GetPlayerRaceBit(race_id)
						) :
						""
					)
				).c_str()
			);

			found_count++;

			if (found_count == 50) {
				break;
			}
		}

		if (found_count == 50) {
			c->Message(Chat::White, "50 Races found, max reached.");
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} Race{} found.",
					found_count,
					found_count != 1 ? "s" : ""
				).c_str()
			);
		}
	}
}

