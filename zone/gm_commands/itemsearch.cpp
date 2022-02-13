#include "../client.h"

void command_itemsearch(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #itemsearch [search string]");
	}
	else {
		const char *search_criteria = sep->argplus[1];

		const EQ::ItemData *item = nullptr;
		EQ::SayLinkEngine  linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemData);

		if (Seperator::IsNumber(search_criteria)) {
			item = database.GetItem(atoi(search_criteria));
			if (item) {
				linker.SetItemData(item);
				std::string item_id          = std::to_string(item->ID);
				std::string saylink_commands =
								"[" +
								EQ::SayLinkEngine::GenerateQuestSaylink(
									"#si " + item_id,
									false,
									"X"
								) +
								"] ";

				if (item->Stackable && item->StackSize > 1) {
					std::string stack_size = std::to_string(item->StackSize);
					saylink_commands +=
						"[" +
						EQ::SayLinkEngine::GenerateQuestSaylink(
							"#si " + item_id + " " + stack_size,
							false,
							stack_size
						) +
						"]";
				}

				c->Message(
					Chat::White,
					fmt::format(
						" Summon {} [{}] [{}]",
						saylink_commands,
						linker.GenerateLink(),
						item->ID
					).c_str()
				);
			}
			else {
				c->Message(
					Chat::White,
					fmt::format(
						"Item {} not found",
						search_criteria
					).c_str()
				);
			}

			return;
		}

		int  count = 0;
		char sName[64];
		char sCriteria[255];
		strn0cpy(sCriteria, search_criteria, sizeof(sCriteria));
		strupr(sCriteria);
		char   *pdest;
		uint32 it  = 0;
		while ((item = database.IterateItems(&it))) {
			strn0cpy(sName, item->Name, sizeof(sName));
			strupr(sName);
			pdest = strstr(sName, sCriteria);
			if (pdest != nullptr) {
				linker.SetItemData(item);
				std::string item_id          = std::to_string(item->ID);
				std::string saylink_commands =
								"[" +
								EQ::SayLinkEngine::GenerateQuestSaylink(
									"#si " + item_id,
									false,
									"X"
								) +
								"] ";
				if (item->Stackable && item->StackSize > 1) {
					std::string stack_size = std::to_string(item->StackSize);
					saylink_commands +=
						"[" +
						EQ::SayLinkEngine::GenerateQuestSaylink(
							"#si " + item_id + " " + stack_size,
							false,
							stack_size
						) +
						"]";
				}

				c->Message(
					Chat::White,
					fmt::format(
						" Summon {} [{}] [{}]",
						saylink_commands,
						linker.GenerateLink(),
						item->ID
					).c_str()
				);

				++count;
			}

			if (count == 50) {
				break;
			}
		}

		if (count == 50) {
			c->Message(Chat::White, "50 items shown...too many results.");
		}
		else {
			c->Message(Chat::White, "%i items found", count);
		}

	}
}

