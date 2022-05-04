#include "../client.h"

void command_gassign(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #gassign [Grid ID]");
		return;
	}

	auto grid_id = std::stoul(sep->arg[1]);

	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();
	if (target->GetSpawnPointID() > 0) {
		auto spawn2_id = target->GetSpawnPointID();
		database.AssignGrid(c, grid_id, target->GetID());
	}
}

