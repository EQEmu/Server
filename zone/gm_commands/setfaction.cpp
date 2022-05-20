#include "../client.h"

void command_setfaction(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #setfaction [Faction ID]");
		return;
	}
	
	if (
		!c->GetTarget() ||
		(
			c->GetTarget() &&
			c->GetTarget()->IsClient()
		)
	) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	NPC* target = c->GetTarget()->CastToNPC();
	auto npc_id = target->GetNPCTypeID();
	auto faction_id = std::stoi(sep->arg[1]);
	auto faction_name = content_db.GetFactionName(faction_id);
	if (!faction_name.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Faction Changed | Name: {} ({}) Faction: {} ({}).",
				target->GetCleanName(),
				npc_id,
				content_db.GetFactionName(faction_id),
				faction_id
			).c_str()
		);

		std::string query = fmt::format(
			"UPDATE npc_types SET npc_faction_id = {} WHERE id = {}",
			faction_id,
			npc_id
		);
		content_db.QueryDatabase(query);
	} else {
		c->Message(
			Chat::White,
			"Invalid Faction ID, please specify a valid Faction ID."
		);
	}
}

