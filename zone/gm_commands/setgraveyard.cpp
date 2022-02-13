#include "../client.h"

void command_setgraveyard(Client *c, const Seperator *sep)
{
	uint32 zoneid       = 0;
	uint32 graveyard_id = 0;
	Client *t           = c;

	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	if (!sep->arg[1][0]) {
		c->Message(Chat::White, "Usage: #setgraveyard [zonename]");
		return;
	}

	zoneid = ZoneID(sep->arg[1]);

	if (zoneid > 0) {
		graveyard_id = content_db.CreateGraveyardRecord(zoneid, t->GetPosition());

		if (graveyard_id > 0) {
			c->Message(Chat::White, "Successfuly added a new record for this graveyard!");
			if (content_db.AddGraveyardIDToZone(zoneid, graveyard_id) > 0) {
				c->Message(Chat::White, "Successfuly added this new graveyard for the zone %s.", sep->arg[1]);
				// TODO: Set graveyard data to the running zone process.
				c->Message(Chat::White, "Done!");
			}
			else {
				c->Message(Chat::White, "Unable to add this new graveyard to the zone %s.", sep->arg[1]);
			}
		}
		else {
			c->Message(Chat::White, "Unable to create a new graveyard record in the database.");
		}
	}
	else {
		c->Message(Chat::White, "Unable to retrieve a ZoneID for the zone: %s", sep->arg[1]);
	}

	return;
}

