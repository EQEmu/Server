#include "../client.h"
#include "../expedition.h"

void command_dzkickplayers(Client *c, const Seperator *sep)
{
	if (c) {
		auto expedition = c->GetExpedition();
		if (expedition) {
			expedition->DzKickPlayers(c);
		}
	}
}

