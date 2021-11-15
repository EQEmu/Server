#include "../client.h"

void command_zsave(Client *c, const Seperator *sep)
{
	if (zone->SaveZoneCFG()) {
		c->Message(Chat::Red, "Zone header saved successfully.");
	}
	else {
		c->Message(Chat::Red, "ERROR: Zone header data was NOT saved.");
	}
}

