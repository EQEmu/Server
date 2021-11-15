#include "../client.h"

void command_zuwcoords(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #zuwcoords <under world coords>");
	}
	else {
		zone->newzone_data.underworld = atof(sep->arg[1]);
		//float newdata = atof(sep->arg[1]);
		//memcpy(&zone->zone_header_data[130], &newdata, sizeof(float));
		auto outapp                   = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

