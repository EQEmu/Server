#include "../client.h"

void command_reloadstatic(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Reloading zone static data...");
	zone->ReloadStaticData();
}

