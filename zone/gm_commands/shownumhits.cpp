#include "../client.h"

void command_shownumhits(Client *c, const Seperator *sep)
{
	c->ShowNumHits();
	return;
}

