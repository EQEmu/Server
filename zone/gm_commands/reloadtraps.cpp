#include "../client.h"

void command_reloadtraps(Client *c, const Seperator *sep)
{
	entity_list.UpdateAllTraps(true, true);
	c->Message(Chat::Default, "Traps reloaded for %s.", zone->GetShortName());
}

