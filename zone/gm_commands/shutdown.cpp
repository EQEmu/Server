#include "../client.h"
#include "../../world/main.h"

void command_shutdown(Client *c, const Seperator *sep)
{
	CatchSignal(2);
}

