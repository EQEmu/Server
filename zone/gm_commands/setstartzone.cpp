#include "../client.h"

void command_setstartzone(Client *c, const Seperator *sep)
{
	uint32 startzone = 0;
	Client *target   = nullptr;
	if (c->GetTarget() && c->GetTarget()->IsClient() && sep->arg[1][0] != 0) {
		target = c->GetTarget()->CastToClient();
	}
	else {
		c->Message(Chat::White, "Usage: (needs PC target) #setstartzone zonename");
		c->Message(
			Chat::White,
			"Optional Usage: Use '#setstartzone reset' or '#setstartzone 0' to clear a starting zone. Player can select a starting zone using /setstartcity"
		);
		return;
	}

	if (sep->IsNumber(1)) {
		startzone = atoi(sep->arg[1]);
	}
	else if (strcasecmp(sep->arg[1], "reset") == 0) {
		startzone = 0;
	}
	else {
		startzone = ZoneID(sep->arg[1]);
		if (startzone == 0) {
			c->Message(Chat::White, "Unable to locate zone '%s'", sep->arg[1]);
			return;
		}
	}

	target->SetStartZone(startzone);
}

