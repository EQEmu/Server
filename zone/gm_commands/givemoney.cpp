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

	c->GetTarget()->CastToClient()->AddMoneyToPP(
		copper,
		silver,
		gold,
		platinum,
		true
	);
	std::string money_string;
	if (copper && silver && gold && platinum) { // CSGP
		money_string = fmt::format(
			"{} Platinum, {} Gold, {} Silver, and {} Copper",
			platinum,
			gold,
			silver,
			copper
		);
	} else if (copper && silver && gold && !platinum) { // CSG
		money_string = fmt::format(
			"{} Gold, {} Silver, and {} Copper",
			gold,
			silver,
			copper
		);
	} else if (copper && silver && !gold && !platinum) { // CS
		money_string = fmt::format(
			"{} Silver and {} Copper",
			silver,
			copper
		);
	} else if (copper && !silver && !gold && !platinum) { // C
		money_string = fmt::format(
			"{} Copper",
			copper
		);
	} else if (!copper && silver && gold && platinum) { // SGP
		money_string = fmt::format(
			"{} Platinum, {} Gold, and {} Silver",
			platinum,
			gold,
			silver
		);
	} else if (!copper && silver && gold && !platinum) { // SG
		money_string = fmt::format(
			"{} Gold and {} Silver",
			gold,
			silver
		);
	} else if (!copper && silver && !gold && !platinum) { // S
		money_string = fmt::format(
			"{} Silver",
			silver
		);
	} else if (copper && !silver && gold && platinum) { // CGP
		money_string = fmt::format(
			"{} Platinum, {} Gold, and {} Copper",
			platinum,
			gold,
			copper
		);
	} else if (copper && !silver && gold && !platinum) { // CG
		money_string = fmt::format(
			"{} Gold and {} Copper",
			gold,
			copper
		);
	} else if (!copper && !silver && gold && platinum) { // GP
		money_string = fmt::format(
			"{} Platinum and {} Gold",
			platinum,
			gold
		);
	} else if (!copper && !silver && gold && !platinum) { // G
		money_string = fmt::format(
			"{} Gold",
			gold
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Added {} to {}.",
			money_string,
			c == target ? "yourself" : target->GetCleanName()
		).c_str()
	);
}
