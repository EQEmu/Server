#include "../client.h"

void command_viewpetition(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #viewpetition (petition number) Type #listpetition for a list");
		return;
	}

	c->Message(Chat::Red, "	ID : Character Name , Petition Text");

	std::string query   = "SELECT petid, charname, petitiontext FROM petitions ORDER BY petid";
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	LogInfo("View petition request from [{}], petition number: [{}]", c->GetName(), atoi(sep->argplus[1]));

	if (results.RowCount() == 0) {
		c->Message(Chat::Red, "There was an error in your request: ID not found! Please check the Id and try again.");
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
		if (strcasecmp(row[0], sep->argplus[1]) == 0) {
			c->Message(Chat::Yellow, " %s:	%s , %s ", row[0], row[1], row[2]);
		}

}

