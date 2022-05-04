#include "../client.h"

void command_npctype_cache(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #npctype_cache [NPC ID] [NPC ID 2] - Remove specific NPCs from the NPC cache (can add spaces to do more)");
		c->Message(Chat::White, "Usage: #npctype_cache all - Remove all NPCs from the NPC cache");
		return;
	}

	for (int i = 0; i < arguments; ++i) {
		if (!strcasecmp(sep->arg[i + 1], "all")) {
			c->Message(Chat::White, "Clearing all npc types from the cache.");
			zone->ClearNPCTypeCache(-1);
			return;
		} else {
			auto npc_id = std::stoi(sep->arg[i + 1]);
			if (npc_id) {
				c->Message(
					Chat::White,
					fmt::format(
						"Clearing NPC ID {} from the cache.",
						npc_id
					).c_str()
				);
				zone->ClearNPCTypeCache(npc_id);
			}
		}
	}
}

