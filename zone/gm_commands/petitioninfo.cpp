#include "../client.h"

void command_petitioninfo(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #petitioninfo (petition number) Type #listpetition for a list");
		return;
	}

	std::string query   = "SELECT petid, charname, accountname, zone, charclass, charrace, charlevel FROM petitions ORDER BY petid";
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	LogInfo("Petition information request from [{}], petition number:", c->GetName(), atoi(sep->argplus[1]));

	if (results.RowCount() == 0) {
		c->Message(Chat::Red, "There was an error in your request: ID not found! Please check the Id and try again.");
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
		if (strcasecmp(row[0], sep->argplus[1]) == 0) {
			c->Message(
				Chat::Red,
				"	ID : %s Character Name: %s Account Name: %s Zone: %s Character Class: %s Character Race: %s Character Level: %s",
				row[0],
				row[1],
				row[2],
				row[3],
				row[4],
				row[5],
				row[6]
			);
		}

}

