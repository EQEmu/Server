#include "../client.h"

void command_flags(Client *c, const Seperator *sep)
{
	Client *t = c;

	if (c->Admin() >= minStatusToSeeOthersZoneFlags) {
		Mob *tgt = c->GetTarget();
		if (tgt != nullptr && tgt->IsClient()) {
			t = tgt->CastToClient();
		}
	}

	t->SendZoneFlagInfo(c);
}

