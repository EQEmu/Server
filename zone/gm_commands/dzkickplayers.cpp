#include "../client.h"

void command_dzkickplayers(Client *c, const Seperator *sep)
{
	if (c) {
		auto dz = c->GetExpedition();
		if (dz) {
			dz->DzKickPlayers(c);
		}
	}
}
