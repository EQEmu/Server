#include "../client.h"
#include "../groups.h"

void command_refreshgroup(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	Group *g = c->GetGroup();

	if (!g) {
		return;
	}

	database.RefreshGroupFromDB(c);
	//g->SendUpdate(7, c);
}

