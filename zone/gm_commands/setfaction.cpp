#include "../client.h"

void command_setfaction(Client *c, const Seperator *sep)
{
	if ((sep->arg[1][0] == 0 || strcasecmp(sep->arg[1], "*") == 0) ||
		((c->GetTarget() == 0) || (c->GetTarget()->IsClient()))) {
		c->Message(Chat::White, "Usage: #setfaction [faction number]");
		return;
	}

	auto npcTypeID = c->GetTarget()->CastToNPC()->GetNPCTypeID();
	c->Message(Chat::Yellow, "Setting NPC %u to faction %i", npcTypeID, atoi(sep->argplus[1]));

	std::string query = StringFormat(
		"UPDATE npc_types SET npc_faction_id = %i WHERE id = %i",
		atoi(sep->argplus[1]), npcTypeID
	);
	content_db.QueryDatabase(query);
}

