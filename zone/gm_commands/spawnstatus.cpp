#include "../client.h"

void command_spawnstatus(Client *c, const Seperator *sep)
{
	if ((sep->arg[1][0] == 'e') | (sep->arg[1][0] == 'E')) {
		// show only enabled spawns
		zone->ShowEnabledSpawnStatus(c);
	}
	else if ((sep->arg[1][0] == 'd') | (sep->arg[1][0] == 'D')) {
		// show only disabled spawns
		zone->ShowDisabledSpawnStatus(c);
	}
	else if ((sep->arg[1][0] == 'a') | (sep->arg[1][0] == 'A')) {
		// show all spawn staus with no filters
		zone->SpawnStatus(c);
	}
	else if (sep->IsNumber(1)) {
		// show spawn status by spawn2 id
		zone->ShowSpawnStatusByID(c, atoi(sep->arg[1]));
	}
	else if (strcmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "Usage: #spawnstatus <[a]ll | [d]isabled | [e]nabled | {Spawn2 ID}>");
	}
	else {
		zone->SpawnStatus(c);
	}
}

