#include "../client.h"

void command_zsky(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #zsky <sky type>");
	}
	else if (atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > 255) {
		c->Message(Chat::White, "ERROR: Sky type can not be less than 0 or greater than 255!");
	}
	else {
		zone->newzone_data.sky = atoi(sep->arg[1]);
		auto outapp            = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

