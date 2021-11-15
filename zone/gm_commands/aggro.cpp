#include "../client.h"

void command_aggro(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == nullptr || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "Error: you must have an NPC target.");
		return;
	}
	float d = atof(sep->arg[1]);
	if (d == 0.0f) {
		c->Message(Chat::Red, "Error: distance argument required.");
		return;
	}
	bool verbose = false;
	if (sep->arg[2][0] == '-' && sep->arg[2][1] == 'v' && sep->arg[2][2] == '\0') {
		verbose = true;
	}

	entity_list.DescribeAggro(c, c->GetTarget()->CastToNPC(), d, verbose);
}

