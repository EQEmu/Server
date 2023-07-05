#include "../../client.h"

void ShowEmotes(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to view their emotes.");
		return;
	}

	const auto t = c->GetTarget()->CastToNPC();

	uint32       emote_count = 0;
	const uint32 emote_id    = t->GetEmoteID();

	LinkedListIterator<NPC_Emote_Struct *> iterator(zone->NPCEmoteList);
	iterator.Reset();
	while (iterator.MoreElements()) {
		const auto& e = iterator.GetData();
		if (emote_id == e->emoteid) {
			c->Message(
				Chat::White,
				fmt::format(
					"Emote {} | Event: {} ({}) Type: {} ({})",
					e->emoteid,
					EQ::constants::GetEmoteEventTypeName(e->event_),
					e->event_,
					EQ::constants::GetEmoteTypeName(e->type),
					e->type
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Emote {} | Text: {}",
					e->emoteid,
					e->text
				).c_str()
			);

			emote_count++;
		}

		iterator.Advance();
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} has {} emote{} on Emote ID {}.",
			c->GetTargetDescription(t),
			emote_count,
			emote_count != 1 ? "s" : "",
			emote_id
		).c_str()
	);
}
