#include "../client.h"
#include "../corpse.h"

void command_corpsefix(Client *c, const Seperator *sep)
{
	entity_list.CorpseFix(c);
}

