#include "../client.h"

void command_aggro(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #aggro [Distance] [-v] (-v is verbose Faction Information)");
		return;
	}

	if (
		!c->GetTarget() ||
		(
			c->GetTarget() &&
			!c->GetTarget()->IsNPC()
		)
	) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	NPC* target = c->GetTarget()->CastToNPC();

	float distance = std::stof(sep->arg[1]);
	bool verbose = !strcasecmp("-v", sep->arg[2]);
	entity_list.DescribeAggro(c, target, distance, verbose);
}

