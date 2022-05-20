#include "../client.h"

void command_depopzone(Client *c, const Seperator *sep)
{
	zone->Depop();
	c->Message(Chat::White, "Zone depoped.");
}

