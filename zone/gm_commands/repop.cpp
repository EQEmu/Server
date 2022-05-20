#include "../client.h"

void command_repop(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Zone depopped, repopping now.");
		zone->Repop();
		zone->spawn2_timer.Trigger();
		return;
	}

	bool is_force = !strcasecmp(sep->arg[1], "force");

	if (is_force) {
		zone->ClearSpawnTimers();
		c->Message(Chat::White, "Zone depopped, forcefully repopping now.");
	} else {		
		c->Message(Chat::White, "Zone depopped, repopping now.");
	}

	zone->Repop();
	zone->spawn2_timer.Trigger();
}

