#include "../client.h"

void command_findrace(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;

	if (arguments == 0) {
		c->Message(Chat::White, "Command Syntax: #findrace [search criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		int         race_id   = std::stoi(sep->arg[1]);
		std::string race_name = GetRaceIDName(race_id);
		if (race_id >= RACE_HUMAN_1 && race_id <= RACE_PEGASUS_732) {
			c->Message(
				Chat::White,
				fmt::format(
					"Race {}: {}",
					race_id,
					race_name
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Race ID {} was not found.",
					race_id
				).c_str()
			);
		}
	}
	else {
		std::string search_criteria = str_tolower(sep->argplus[1]);
		int         found_count     = 0;
		for (int    race_id         = RACE_HUMAN_1; race_id <= RACE_PEGASUS_732; race_id++) {
			std::string race_name       = GetRaceIDName(race_id);
			std::string race_name_lower = str_tolower(race_name);
			if (search_criteria.length() > 0 && race_name_lower.find(search_criteria) == std::string::npos) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Race {}: {}",
					race_id,
					race_name
				).c_str()
			);
			found_count++;

			if (found_count == 20) {
				break;
			}
		}

		if (found_count == 20) {
			c->Message(Chat::White, "20 Races found... max reached.");
		}
		else {
			auto race_message = (
				found_count > 0 ?
					(
						found_count == 1 ?
							"A Race was" :
							fmt::format("{} Races were", found_count)
					) :
					"No Races were"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"{} found.",
					race_message
				).c_str()
			);
		}
	}
}

