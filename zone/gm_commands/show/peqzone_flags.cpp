#include "../../client.h"

void ShowPEQZoneFlags(Client *c, const Seperator *sep)
{
	auto t = c;
	if (
		c->GetTarget() &&
		c->GetTarget()->IsClient() &&
		c->Admin() >= minStatusToSeeOthersZoneFlags
	) {
		t =  c->GetTarget()->CastToClient();
	}

	t->SendPEQZoneFlagInfo(c);
}
