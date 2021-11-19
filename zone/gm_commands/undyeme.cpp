#include "../client.h"

void command_undyeme(Client *c, const Seperator *sep)
{
	if (c) {
		c->Undye();
		c->Message(Chat::Red, "Dye removed from all slots. Please zone for the process to complete.");
	}
}

