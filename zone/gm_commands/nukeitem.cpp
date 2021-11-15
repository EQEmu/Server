#include "../client.h"

void command_nukeitem(Client *c, const Seperator *sep)
{
	int numitems, itemid;

	if (c->GetTarget() && c->GetTarget()->IsClient() && (sep->IsNumber(1) || sep->IsHexNumber(1))) {
		itemid   = sep->IsNumber(1) ? atoi(sep->arg[1]) : hextoi(sep->arg[1]);
		numitems = c->GetTarget()->CastToClient()->NukeItem(itemid);
		c->Message(Chat::White, " %u items deleted", numitems);
	}
	else {
		c->Message(Chat::White, "Usage: (targted) #nukeitem itemnum - removes the item from the player's inventory");
	}
}

