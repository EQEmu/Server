#include "../../client.h"

void SetFaction(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set faction [Faction ID]");
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

	NPC* t = c->GetTarget()->CastToNPC();

	const uint32 npc_id     = t->GetNPCTypeID();
	const int    faction_id = Strings::ToInt(sep->arg[2]);

	const std::string& faction_name = content_db.GetFactionName(faction_id);

	if (faction_name.empty()) {
		c->Message(
			Chat::White,
			"Invalid Faction ID, please specify a valid Faction ID."
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Faction Changed | Name: {} ({}) Faction: {} ({}).",
			t->GetCleanName(),
			npc_id,
			content_db.GetFactionName(faction_id),
			faction_id
		).c_str()
	);

	const std::string &query = fmt::format(
		"UPDATE npc_types SET npc_faction_id = {} WHERE id = {}",
		faction_id,
		npc_id
	);
	content_db.QueryDatabase(query);
}
