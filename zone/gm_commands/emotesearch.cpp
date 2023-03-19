#include "../client.h"

void command_emotesearch(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #emotesearch [Emote ID]");
		c->Message(Chat::White, "Usage: #emotesearch [Search Crteria]");
		return;
	}

	auto emote_count = 0;
	auto emote_number = 1;

	std::string search_criteria = sep->argplus[1];
	bool found_by_id = false;

	if (!sep->IsNumber(1)) {
		LinkedListIterator<NPC_Emote_Struct *> iterator(zone->NPCEmoteList);
		iterator.Reset();
		while (iterator.MoreElements()) {
			auto &e = iterator.GetData();
			auto current_text = Strings::ToLower(e->text);
			
			if (Strings::Contains(current_text, Strings::ToLower(search_criteria))) {
				c->Message(
					Chat::White,
					fmt::format(
						"Emote {} | Emote ID: {}",
						emote_number,
						e->emoteid
					).c_str()
				);

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

			if (emote_count == 50) {
				break;
			}

			iterator.Advance();
		}
	} else {
		auto emote_id = std::stoul(search_criteria);
		
		LinkedListIterator<NPC_Emote_Struct *> iterator(zone->NPCEmoteList);
		iterator.Reset();
		while (iterator.MoreElements()) {
			auto &e = iterator.GetData();
			if (emote_id == e->emoteid) {
				found_by_id = true;

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

			if (emote_count == 50) {
				break;
			}

			iterator.Advance();
		}
	}

	auto found_string = (
		found_by_id ?
		fmt::format("ID {}", search_criteria) :
		search_criteria
	);

	if (!emote_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"No Emotes found matching {}.",
				found_string
			).c_str()
		);
	} else if (emote_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Emotes shown matching {}, too many results.",
				found_string
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} Emote{} found matching {}.",
				emote_count,
				emote_count != 1 ? "s" : "",
				found_string
			).c_str()
		);
	}
}
