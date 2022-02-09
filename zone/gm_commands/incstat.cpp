#include "../client.h"

void command_incstat(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] && sep->arg[2][0] && c->GetTarget() != 0 && c->GetTarget()->IsClient()) {
		c->GetTarget()->CastToClient()->IncStats(atoi(sep->arg[1]), atoi(sep->arg[2]));
	}
	else {
		c->Message(Chat::White, "This command is used to permanently increase or decrease a players stats.");
		c->Message(Chat::White, "Usage: #setstat {type} {value by which to increase or decrease}");
		c->Message(
			Chat::White,
			"Note: The value is in increments of 2, so a value of 3 will actually increase the stat by 6"
		);
		c->Message(Chat::White, "Types: Str: 0, Sta: 1, Agi: 2, Dex: 3, Int: 4, Wis: 5, Cha: 6");
	}
}

