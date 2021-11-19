#include "../client.h"

void command_findfaction(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;

	if (arguments == 0) {
		c->Message(Chat::White, "Command Syntax: #findfaction [search criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		int  faction_id   = std::stoi(sep->arg[1]);
		auto faction_name = content_db.GetFactionName(faction_id);
		if (!faction_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Faction {}: {}",
					faction_id,
					faction_name
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Faction ID {} was not found.",
					faction_id
				).c_str()
			);
		}
	}
	else {
		std::string search_criteria = str_tolower(sep->argplus[1]);
		int         found_count     = 0;
		int         max_faction_id  = content_db.GetMaxFaction();
		for (int    faction_id      = 0; faction_id < max_faction_id; faction_id++) {
			std::string faction_name       = content_db.GetFactionName(faction_id);
			std::string faction_name_lower = str_tolower(faction_name);
			if (faction_name.empty()) {
				continue;
			}

			if (faction_name.find(search_criteria) == std::string::npos) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Faction {}: {}",
					faction_id,
					faction_name
				).c_str()
			);
			found_count++;

			if (found_count == 20) {
				break;
			}
		}

		if (found_count == 20) {
			c->Message(Chat::White, "20 Factions found... max reached.");
		}
		else {
			auto faction_message = (
				found_count > 0 ?
					(
						found_count == 1 ?
							"A Faction was" :
							fmt::format("{} Factions were", found_count)
					) :
					"No Factions were"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"{} found.",
					faction_message
				).c_str()
			);
		}
	}
}

