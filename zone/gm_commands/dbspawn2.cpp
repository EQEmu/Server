#include "../client.h"

void command_dbspawn2(Client *c, const Seperator *sep)
{

	if (sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3)) {
		LogInfo("Spawning database spawn");
		uint16 cond     = 0;
		int16  cond_min = 0;
		if (sep->IsNumber(4)) {
			cond = atoi(sep->arg[4]);
			if (sep->IsNumber(5)) {
				cond_min = atoi(sep->arg[5]);
			}
		}
		database.CreateSpawn2(
			c,
			atoi(sep->arg[1]),
			zone->GetShortName(),
			c->GetPosition(),
			atoi(sep->arg[2]),
			atoi(sep->arg[3]),
			cond,
			cond_min
		);
	}
	else {
		c->Message(Chat::White, "Usage: #dbspawn2 spawngroup respawn variance [condition_id] [condition_min]");
	}
}

