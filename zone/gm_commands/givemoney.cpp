#include "../client.h"

void command_givemoney(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {    //as long as the first one is a number, we'll just let atoi convert the rest to 0 or a number
		c->Message(Chat::Red, "Usage: #Usage: #givemoney [pp] [gp] [sp] [cp]");
	}
	else if (c->GetTarget() == nullptr) {
		c->Message(Chat::Red, "You must target a player to give money to.");
	}
	else if (!c->GetTarget()->IsClient()) {
		c->Message(Chat::Red, "You can only give money to players with this command.");
	}
	else {
		//TODO: update this to the client, otherwise the client doesn't show any weight change until you zone, move an item, etc
		c->GetTarget()->CastToClient()->AddMoneyToPP(
			atoi(sep->arg[4]),
			atoi(sep->arg[3]),
			atoi(sep->arg[2]),
			atoi(sep->arg[1]),
			true
		);
		c->Message(
			Chat::White,
			"Added %i Platinum, %i Gold, %i Silver, and %i Copper to %s's inventory.",
			atoi(sep->arg[1]),
			atoi(sep->arg[2]),
			atoi(sep->arg[3]),
			atoi(sep->arg[4]),
			c->GetTarget()->GetName());
	}
}

