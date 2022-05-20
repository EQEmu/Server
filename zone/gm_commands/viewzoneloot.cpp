#include "../client.h"

void command_viewzoneloot(Client *c, const Seperator *sep)
{
	std::map<uint32, ItemList> zone_loot_list;
	auto                       npc_list    = entity_list.GetNPCList();
	uint32                     loot_amount = 0, loot_id = 1, search_item_id = 0;
	if (sep->argnum == 1 && sep->IsNumber(1)) {
		search_item_id = atoi(sep->arg[1]);
	}
	else if (sep->argnum == 1 && !sep->IsNumber(1)) {
		c->Message(
			Chat::Yellow,
			"Usage: #viewzoneloot [item id]"
		);
		return;
	}

	for (auto npc_entity : npc_list) {
		auto current_npc_item_list = npc_entity.second->GetItemList();
		zone_loot_list.insert({npc_entity.second->GetID(), current_npc_item_list});
	}

	for (auto loot_item : zone_loot_list) {
		uint32      current_entity_id = loot_item.first;
		auto        current_item_list = loot_item.second;
		auto        current_npc       = entity_list.GetNPCByID(current_entity_id);
		std::string npc_link;
		if (current_npc) {
			std::string npc_name     = current_npc->GetCleanName();
			uint32      instance_id  = zone->GetInstanceID();
			uint32      zone_id      = zone->GetZoneID();
			std::string command_link = EQ::SayLinkEngine::GenerateQuestSaylink(
				fmt::format(
					"#{} {} {} {} {}",
					(instance_id != 0 ? "zoneinstance" : "zone"),
					(instance_id != 0 ? instance_id : zone_id),
					current_npc->GetX(),
					current_npc->GetY(),
					current_npc->GetZ()
				),
				false,
				"Goto"
			);
			npc_link = fmt::format(
				" NPC: {} (ID {}) [{}]",
				npc_name,
				current_entity_id,
				command_link
			);
		}

		for (auto current_item : current_item_list) {
			if (search_item_id == 0 || current_item->item_id == search_item_id) {
				EQ::SayLinkEngine linker;
				linker.SetLinkType(EQ::saylink::SayLinkLootItem);
				linker.SetLootData(current_item);
				c->Message(
					Chat::White,
					fmt::format(
						"{}. {} ({}){}",
						loot_id,
						linker.GenerateLink(),
						current_item->item_id,
						npc_link
					).c_str()
				);
				loot_id++;
				loot_amount++;
			}
		}
	}


	if (search_item_id != 0) {
		std::string drop_string = (
			loot_amount > 0 ?
				fmt::format(
					"dropping in {} {}",
					loot_amount,
					(loot_amount > 1 ? "places" : "place")
				) :
				"not dropping"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) is {}.",
				database.CreateItemLink(search_item_id),
				search_item_id,
				drop_string
			).c_str()
		);
	}
	else {
		std::string drop_string = (
			loot_amount > 0 ?
				fmt::format(
					"{} {} dropping",
					(loot_amount > 1 ? "items" : "item"),
					(loot_amount > 1 ? "are" : "is")
				) :
				"items are dropping"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} {}.",
				loot_amount,
				drop_string
			).c_str()
		);
	}
}

