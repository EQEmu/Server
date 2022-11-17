#include "../client.h"
#include "../data_bucket.h"

void command_devtools(Client *c, const Seperator *sep)
{
	bool is_disable = !strcasecmp(sep->arg[1], "disable");
	bool is_enable = !strcasecmp(sep->arg[1], "enable");

	if (is_disable || is_enable) {
		c->SetDevToolsEnabled(is_enable);
	}

	c->ShowDevToolsMenu();
}

