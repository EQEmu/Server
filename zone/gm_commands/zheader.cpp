#include "../client.h"

void command_zheader(Client *c, const Seperator *sep)
{
	// sends zhdr packet
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #zheader <zone name>");
	}
	else if (ZoneID(sep->argplus[1]) == 0) {
		c->Message(Chat::White, "Invalid Zone Name: %s", sep->argplus[1]);
	}
	else {

		if (zone->LoadZoneCFG(sep->argplus[1], 0)) {
			c->Message(Chat::White, "Successfully loaded zone header for %s from database.", sep->argplus[1]);
		}
		else {
			c->Message(Chat::White, "Failed to load zone header %s from database", sep->argplus[1]);
		}
		auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

