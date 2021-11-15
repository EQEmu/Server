#include "../client.h"

void command_reloadlevelmods(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		if (RuleB(Zone, LevelBasedEXPMods)) {
			zone->LoadLevelEXPMods();
			c->Message(Chat::Yellow, "Level based EXP Mods have been reloaded zonewide");
		}
		else {
			c->Message(Chat::Yellow, "Level based EXP Mods are disabled in rules!");
		}
	}
}

