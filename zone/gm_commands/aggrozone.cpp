#include "../client.h"

void command_aggrozone(Client *c, const Seperator *sep)
{
	if (!c) {
		return;
	}

	Mob *m = c->CastToMob();

	if (!m) {
		return;
	}

	uint32 hate = atoi(sep->arg[1]); //should default to 0 if we don't enter anything
	entity_list.AggroZone(m, hate);
	c->Message(Chat::White, "Train to you! Last chance to go invulnerable...");
}

