#include "../client.h"

void command_randomfeatures(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();
	if (!target) {
		c->Message(Chat::White, "Error: This command requires a target");
	}
	else {
		if (target->RandomizeFeatures()) {
			c->Message(Chat::White, "Features Randomized");
		}
		else {
			c->Message(Chat::White, "This command requires a Playable Race as the target");
		}
	}
}

