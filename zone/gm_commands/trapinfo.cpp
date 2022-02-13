#include "../client.h"

void command_trapinfo(Client *c, const Seperator *sep)
{
	entity_list.GetTrapInfo(c);
}

