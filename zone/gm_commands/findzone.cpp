#include "../client.h"

void command_findzone(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #findzone [search criteria]");
		c->Message(Chat::White, "Usage: #findzone expansion [expansion number]");
		return;
	}

	std::string query;
	int         id = atoi((const char *) sep->arg[1]);

	std::string arg1 = sep->arg[1];

	if (arg1 == "expansion") {
		query = fmt::format(
			"SELECT zoneidnumber, short_name, long_name, version FROM zone WHERE expansion = {}",
			sep->arg[2]
		);
	}
	else {

		/**
		 * If id evaluates to 0, then search as if user entered a string
		 */
		if (id == 0) {
			query = fmt::format(
				"SELECT zoneidnumber, short_name, long_name, version FROM zone WHERE long_name LIKE '%{}%' OR `short_name` LIKE '%{}%'",
				EscapeString(sep->arg[1]),
				EscapeString(sep->arg[1])
			);
		}
		else {
			query = fmt::format(
				"SELECT zoneidnumber, short_name, long_name, version FROM zone WHERE zoneidnumber = {}",
				id
			);
		}
	}

	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		c->Message(Chat::White, "Error querying database.");
		c->Message(Chat::White, query.c_str());
		return;
	}

	int       count   = 0;
	const int maxrows = 100;

	for (auto row = results.begin(); row != results.end(); ++row) {
		std::string zone_id    = row[0];
		std::string short_name = row[1];
		std::string long_name  = row[2];
		int         version    = atoi(row[3]);

		if (++count > maxrows) {
			c->Message(Chat::White, "%i zones shown. Too many results.", maxrows);
			break;
		}

		std::string command_zone   = EQ::SayLinkEngine::GenerateQuestSaylink("#zone " + short_name, false, "zone");
		std::string command_gmzone = EQ::SayLinkEngine::GenerateQuestSaylink(
			fmt::format("#gmzone {} {}", short_name, version),
			false,
			"gmzone"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"[{}] [{}] [{}] ID ({}) Version ({}) [{}]",
				(version == 0 ? command_zone : "zone"),
				command_gmzone,
				short_name,
				zone_id,
				version,
				long_name
			).c_str()
		);
	}

	if (count <= maxrows) {
		c->Message(
			Chat::White,
			"Query complete. %i rows shown. %s",
			count,
			(arg1 == "expansion" ? "(expansion search)" : ""));
	}
	else if (count == 0) {
		c->Message(Chat::White, "No matches found for %s.", sep->arg[1]);
	}
}

