#include "../client.h"

void command_npcemote(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsNPC() && sep->arg[1][0]) {
		c->GetTarget()->Emote(sep->argplus[1]);
	}
	else {
		c->Message(Chat::White, "Usage: #npcemote message (requires NPC target");
	}
}

