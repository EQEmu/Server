#include "../client.h"

void command_emotesearch(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #emotesearch [search string or emoteid]");
	}
	else {
		const char *search_criteria = sep->argplus[1];
		int        count            = 0;

		if (Seperator::IsNumber(search_criteria)) {
			uint16                                 emoteid = atoi(search_criteria);
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
		else {
			char sText[64];
			char sCriteria[515];
			strn0cpy(sCriteria, search_criteria, sizeof(sCriteria));
			strupr(sCriteria);
			char *pdest;

			LinkedListIterator<NPC_Emote_Struct *> iterator(zone->NPCEmoteList);
			iterator.Reset();
			while (iterator.MoreElements()) {
				NPC_Emote_Struct *nes = iterator.GetData();
				strn0cpy(sText, nes->text, sizeof(sText));
				strupr(sText);
				pdest = strstr(sText, sCriteria);
				if (pdest != nullptr) {
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
				if (count == 50) {
					break;
				}

				iterator.Advance();
			}
			if (count == 50) {
				c->Message(Chat::White, "50 emotes shown...too many results.");
			}
			else {
				c->Message(Chat::White, "%i emote(s) found", count);
			}
		}
	}
}

