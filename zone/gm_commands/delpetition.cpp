#include "../client.h"

void command_delpetition(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || strcasecmp(sep->arg[1], "*") == 0) {
		c->Message(Chat::White, "Usage: #delpetition (petition number) Type #listpetition for a list");
		return;
	}

	c->Message(Chat::Red, "Attempting to delete petition number: %i", atoi(sep->argplus[1]));
	std::string query   = StringFormat("DELETE FROM petitions WHERE petid = %i", atoi(sep->argplus[1]));
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	LogInfo("Delete petition request from [{}], petition number:", c->GetName(), atoi(sep->argplus[1]));

}

