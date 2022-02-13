#include "../client.h"

void command_gassign(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(1) && c->GetTarget() && c->GetTarget()->IsNPC() &&
		c->GetTarget()->CastToNPC()->GetSpawnPointID() > 0) {
		int spawn2id = c->GetTarget()->CastToNPC()->GetSpawnPointID();
		database.AssignGrid(c, atoi(sep->arg[1]), spawn2id);
	}
	else {
		c->Message(Chat::White, "Usage: #gassign [num] - must have an npc target!");
	}
}

