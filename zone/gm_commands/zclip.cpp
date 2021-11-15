#include "../client.h"

void command_zclip(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if (sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #zclip <min clip> <max clip>");
	}
	else if (atoi(sep->arg[1]) <= 0) {
		c->Message(Chat::White, "ERROR: Min clip can not be zero or less!");
	}
	else if (atoi(sep->arg[2]) <= 0) {
		c->Message(Chat::White, "ERROR: Max clip can not be zero or less!");
	}
	else if (atoi(sep->arg[1]) > atoi(sep->arg[2])) {
		c->Message(Chat::White, "ERROR: Min clip is greater than max clip!");
	}
	else {
		zone->newzone_data.minclip = atof(sep->arg[1]);
		zone->newzone_data.maxclip = atof(sep->arg[2]);
		if (sep->arg[3][0] != 0) {
			zone->newzone_data.fog_minclip[0] = atof(sep->arg[3]);
		}
		if (sep->arg[4][0] != 0) {
			zone->newzone_data.fog_minclip[1] = atof(sep->arg[4]);
		}
		if (sep->arg[5][0] != 0) {
			zone->newzone_data.fog_maxclip[0] = atof(sep->arg[5]);
		}
		if (sep->arg[6][0] != 0) {
			zone->newzone_data.fog_maxclip[1] = atof(sep->arg[6]);
		}
		auto outapp                = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

