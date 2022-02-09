#include "../client.h"

void command_flags(Client *c, const Seperator *sep)
{
	Client *target = c;

	if (
		c->GetTarget() &&
		c->GetTarget()->IsClient() &&
		c->Admin() >= minStatusToSeeOthersZoneFlags
	) {
		target =  c->GetTarget()->CastToClient();
	}

	target->SendZoneFlagInfo(c);
}

