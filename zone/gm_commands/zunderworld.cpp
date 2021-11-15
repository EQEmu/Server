#include "../client.h"

void command_zunderworld(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #zunderworld <zcoord>");
	}
	else {
		zone->newzone_data.underworld = atof(sep->arg[1]);
	}
}

