#include "../client.h"

void command_illusion_block(Client* c, const Seperator* sep)
{
	int arguments = sep->argnum;
	if (!arguments || !strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "usage: #illusionblock [help | current | value].");
		c->Message(Chat::White, "note: Used to control whether or not illusion effects will land on you.");
		c->Message(Chat::White, "note: A value of 0 is disabled (Allow Illusions), 1 is enabled (Block Illusions).");
		c->Message(Chat::White, "note: Use [current] to check the current setting.");
		return;
	}

	if (sep->IsNumber(1)) {
		int set_status = atoi(sep->arg[1]);
		if (set_status == 0 || set_status == 1) {
			c->SetIllusionBlock(set_status);
			c->Message(Chat::White, "Your Illusion Block has been %s.", (set_status ? "enabled" : "disabled"));
		}
		else {
			c->Message(Chat::White, "You must enter 0 for disabled or 1 for enabled.");
			return;
		}
	}
	else if (!strcasecmp(sep->arg[1], "current")) {
		c->Message(Chat::White, "You're currently %s illusions.", (c->GetIllusionBlock() ? "blocking" : "allowing"));
	}
	else {
		c->Message(Chat::White, "Incorrect argument, use %s help for a list of options.", sep->arg[0]);
	}
}
