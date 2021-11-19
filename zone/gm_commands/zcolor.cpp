#include "../client.h"

void command_zcolor(Client *c, const Seperator *sep)
{
	// modifys and resends zhdr packet
	if (sep->arg[3][0] == 0) {
		c->Message(Chat::White, "Usage: #zcolor <red> <green> <blue>");
	}
	else if (atoi(sep->arg[1]) < 0 || atoi(sep->arg[1]) > 255) {
		c->Message(Chat::White, "ERROR: Red can not be less than 0 or greater than 255!");
	}
	else if (atoi(sep->arg[2]) < 0 || atoi(sep->arg[2]) > 255) {
		c->Message(Chat::White, "ERROR: Green can not be less than 0 or greater than 255!");
	}
	else if (atoi(sep->arg[3]) < 0 || atoi(sep->arg[3]) > 255) {
		c->Message(Chat::White, "ERROR: Blue can not be less than 0 or greater than 255!");
	}
	else {
		for (int z      = 0; z < 4; z++) {
			zone->newzone_data.fog_red[z]   = atoi(sep->arg[1]);
			zone->newzone_data.fog_green[z] = atoi(sep->arg[2]);
			zone->newzone_data.fog_blue[z]  = atoi(sep->arg[3]);
		}
		auto     outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
		memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

