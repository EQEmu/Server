#include "../client.h"

void command_reloademote(Client *c, const Seperator *sep)
{
	zone->NPCEmoteList.Clear();
	zone->LoadNPCEmotes(&zone->NPCEmoteList);
	c->Message(Chat::White, "NPC emotes reloaded.");
}

