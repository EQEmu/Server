#include "../client.h"

void command_givemoney(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) { //as long as the first one is a number, we'll just let atoi convert the rest to 0 or a number
		c->Message(Chat::Red, "Usage: #Usage: #givemoney [Platinum] [Gold] [Silver] [Copper]");
		return;
	}

	
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 platinum = std::stoi(sep->arg[1]);
	uint32 gold = std::stoi(sep->arg[2]);
	uint32 silver = std::stoi(sep->arg[3]);
	uint32 copper = std::stoi(sep->arg[4]);
	if (!platinum && !gold && !silver && !copper) {
		c->Message(Chat::Red, "Usage: #Usage: #givemoney [Platinum] [Gold] [Silver] [Copper]");
		return;
	}

	target->AddMoneyToPP(
		copper,
		silver,
		gold,
		platinum,
		true
	);
	
	c->Message(
		Chat::White,
		fmt::format(
			"Added {} to {}.",
			ConvertMoneyToString(platinum, gold, silver, copper),
			c == target ? "yourself" : target->GetCleanName()
		).c_str()
	);
}
