#include "../client.h"

void command_findnpctype(Client *c, const Seperator *sep)
{
	int arguments               = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #findnpctype [Search Criteria]");
		return;
	}

	std::string query;
	std::string search_criteria = sep->arg[1];
	if (sep->IsNumber(1)) {
		query = fmt::format(
			"SELECT id, name FROM npc_types WHERE id = {}",
			search_criteria
		);
	}
	else {
		query = fmt::format(
			"SELECT id, name FROM npc_types WHERE name LIKE '%%{}%%'",
			search_criteria
		);
	}

	auto results = content_db.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		c->Message(
			Chat::White,
			fmt::format(
				"No matches found for '{}'.",
				search_criteria
			).c_str()
		);
		return;
	}

	int found_count = 0;

	for (auto row : results) {
		int found_number = (found_count + 1);
		if (found_count == 20) {
			break;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"NPC {} | {} ({})",
				found_number,
				row[1],
				row[0]
			).c_str()
		);
		found_count++;
	}

	if (found_count == 20) {
		c->Message(Chat::White, "20 NPCs were found, max reached.");
	}
	else {
		auto npc_message = (
			found_count == 1 ?
				"An NPC was" :
				fmt::format("{} NPCs were", found_count)
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} found.",
				npc_message
			).c_str()
		);
	}
}

