#include "../client.h"

void command_hp(Client *c, const Seperator *sep)
{
	c->SendHPUpdate();
	c->CheckManaEndUpdate();
}

