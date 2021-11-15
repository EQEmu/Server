#include "../client.h"

void command_emoteview(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target a NPC to view their emotes.");
		return;
	}

	if (c->GetTarget() && c->GetTarget()->IsNPC()) {
		int count   = 0;
		int emoteid = c->GetTarget()->CastToNPC()->GetEmoteID();

		LinkedListIterator<NPC_Emote_Struct *> iterator(zone->NPCEmoteList);
		iterator.Reset();
		while (iterator.MoreElements()) {
			NPC_Emote_Struct *nes = iterator.GetData();
			if (emoteid == nes->emoteid) {
				c->Message(
					Chat::White,
					"EmoteID: %i Event: %i Type: %i Text: %s",
					nes->emoteid,
					nes->event_,
					nes->type,
					nes->text
				);
				count++;
			}
			iterator.Advance();
		}
		if (count == 0) {
			c->Message(Chat::White, "No emotes found.");
		}
		else {
			c->Message(Chat::White, "%i emote(s) found", count);
		}
	}
}

