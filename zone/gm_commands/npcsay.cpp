#include "../client.h"

void command_npcsay(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #npcsay [Message]");
		return;
	}


	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	std::string message = sep->argplus[1];
	c->GetTarget()->Say(message.c_str());
}

