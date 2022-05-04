#include "../client.h"

void command_npcemote(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #npcemote [Message]");
		return;
	}


	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	std::string message = sep->argplus[1];
	c->GetTarget()->Emote(message.c_str());
}

