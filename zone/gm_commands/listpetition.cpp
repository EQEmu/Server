#include "../client.h"

void command_listpetition(Client *c, const Seperator *sep)
{
	std::string query   = "SELECT petid, charname, accountname FROM petitions ORDER BY petid";
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	LogInfo("Petition list requested by [{}]", c->GetName());

	if (results.RowCount() == 0) {
		return;
	}

	c->Message(Chat::Red, "	ID : Character Name , Account Name");

	for (auto row = results.begin(); row != results.end(); ++row)
		c->Message(Chat::Yellow, " %s:	%s , %s ", row[0], row[1], row[2]);
}

