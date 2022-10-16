#include "../client.h"

void command_emoteview(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to view their emotes.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();

	auto emote_count = 0;
	auto emote_id = target->GetEmoteID();

	auto emote_number = 1;

	LinkedListIterator<NPC_Emote_Struct *> iterator(zone->NPCEmoteList);
	iterator.Reset();
	while (iterator.MoreElements()) {
		auto &e = iterator.GetData();
		if (emote_id == e->emoteid) {
			c->Message(
				Chat::White,
				fmt::format(
					"Emote {} | Event: {} ({}) Type: {} ({})",
					emote_number,
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
					emote_number,
					e->text
				).c_str()
			);

			emote_count++;
			emote_number++;
		}

		iterator.Advance();
	}

	if (!emote_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has no emotes on Emote ID {}.",
				c->GetTargetDescription(target),
				emote_id
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} has {} emote{} on Emote ID {}.",
			c->GetTargetDescription(target),
			emote_count,
			emote_count != 1 ? "s" : "",
			emote_id
		).c_str()
	);
}
