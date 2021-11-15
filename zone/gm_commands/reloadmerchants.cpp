#include "../client.h"

void command_reloadmerchants(Client *c, const Seperator *sep)
{
	entity_list.ReloadMerchants();
	c->Message(Chat::Yellow, "Reloading merchants.");
}

