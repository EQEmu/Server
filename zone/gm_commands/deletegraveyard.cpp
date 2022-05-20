#include "../client.h"

void command_deletegraveyard(Client *c, const Seperator *sep)
{
	uint32 zoneid       = 0;
	uint32 graveyard_id = 0;

	if (!sep->arg[1][0]) {
		c->Message(Chat::White, "Usage: #deletegraveyard [zonename]");
		return;
	}

	zoneid       = ZoneID(sep->arg[1]);
	graveyard_id = content_db.GetZoneGraveyardID(zoneid, 0);

	if (zoneid > 0 && graveyard_id > 0) {
		if (content_db.DeleteGraveyard(zoneid, graveyard_id)) {
			c->Message(Chat::White, "Successfuly deleted graveyard %u for zone %s.", graveyard_id, sep->arg[1]);
		}
		else {
			c->Message(Chat::White, "Unable to delete graveyard %u for zone %s.", graveyard_id, sep->arg[1]);
		}
	}
	else {
		if (zoneid <= 0) {
			c->Message(Chat::White, "Unable to retrieve a ZoneID for the zone: %s", sep->arg[1]);
		}
		else if (graveyard_id <= 0) {
			c->Message(Chat::White, "Unable to retrieve a valid GraveyardID for the zone: %s", sep->arg[1]);
		}
	}

	return;
}

