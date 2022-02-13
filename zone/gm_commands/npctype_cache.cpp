#include "../client.h"

void command_npctype_cache(Client *c, const Seperator *sep)
{
	if (sep->argnum > 0) {
		for (int i = 0; i < sep->argnum; ++i) {
			if (strcasecmp(sep->arg[i + 1], "all") == 0) {
				c->Message(Chat::White, "Clearing all npc types from the cache.");
				zone->ClearNPCTypeCache(-1);
			}
			else {
				int id = atoi(sep->arg[i + 1]);
				if (id > 0) {
					c->Message(Chat::White, "Clearing npc type %d from the cache.", id);
					zone->ClearNPCTypeCache(id);
					return;
				}
			}
		}
	}
	else {
		c->Message(Chat::White, "Usage:");
		c->Message(Chat::White, "#npctype_cache [npctype_id] ...");
		c->Message(Chat::White, "#npctype_cache all");
	}
}

