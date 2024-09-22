#include "../client.h"
#include "../command.h"
#include "../../common/evolving.h"
//#include "../worldserver.h"

//extern WorldServer worldserver;

void command_evolvingitems(Client* c, const Seperator* sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		SendEvolvingItemsSubCommands(c);
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	bool const is_test   = !strcasecmp(sep->arg[1], "test");
	bool const is_target = !strcasecmp(sep->arg[1], "target");
	bool const is_client = !strcasecmp(sep->arg[1], "client");
	bool const is_zone   = !strcasecmp(sep->arg[1], "zone");
	bool const is_item   = !strcasecmp(sep->arg[1], "item");

	if (
		!is_test &&
		!is_target &&
		!is_client &&
		!is_item &&
		!is_zone
		) {
		SendEvolvingItemsSubCommands(c);
		return;
	}

	if (is_test) {
		if (arguments > 3) {
			c->Message(Chat::White, "Usage: #evolve test");
		}
		else {
			auto item_id_1 = Strings::ToUnsignedInt(sep->arg[2]);
			auto item_id_2 = Strings::ToUnsignedInt(sep->arg[3]);
			//c->SendEvolveXPWindowDetails(item_id_1, item_id_2);
		}
	}
	else if (is_target) {
		if (arguments > 1) {
			c->Message(Chat::White, "Usage: #evolve target");
		}
		else {
			c->Message(Chat::Red, "Worn Items");
			for (auto const& [key, value] : t->GetInv().GetWorn()) {
				if (!value->IsEvolving()) {
					continue;
				}

				auto item = evolving_items_manager.GetEvolvingItemsCache().at(value->GetID());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Evolving Items", ":", value->GetID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Unique ID", ":", value->GetEvolveUniqueID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Final Item ID", ":", value->GetEvolveFinalItemID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Activated", ":", value->GetEvolveActivated() ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Equiped", ":", value->GetEvolveEquiped() ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Current Amount", ":", value->GetEvolveCurrentAmount()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Required Amount", ":", item.required_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:.{}f}", "Progression %", ":", value->GetEvolveProgression(), 2).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Type", ":", item.type).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "SubType", ":", item.sub_type).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Timer", ":", value->GetTimers().at("evolve").Enabled()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Timer Remaining", ":", value->GetTimers().at("evolve").GetRemainingTime()).c_str());
				c->Message(Chat::Green, fmt::format("Evolving Item Details:ID:{: >6} UID:{: >6} A:{} E:{} T:{} ST:{} C:{} R:{} P:{:.{}f}",
													value->GetID(),
													value->GetEvolveUniqueID(),
													value->GetEvolveActivated() ? "Y" : "N",
													value->GetEvolveEquiped() ? "Y" : "N",
													item.type,
													item.sub_type,
													value->GetEvolveCurrentAmount(),
													item.required_amount,
													value->GetEvolveProgression(),
													2).c_str()
				);
			}

			c->Message(Chat::Red, "Personal Items");
			for (auto const& [key, value] : t->GetInv().GetPersonal()) {
				if (!value->IsEvolving()) {
					continue;
				}

				auto item = evolving_items_manager.GetEvolvingItemsCache().at(value->GetID());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Evolving Items", ":", value->GetID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Unique ID", ":", value->GetEvolveUniqueID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Final Item ID", ":", value->GetEvolveFinalItemID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Activated", ":", value->GetEvolveActivated() ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Equiped", ":", value->GetEvolveEquiped() ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Current Amount", ":", value->GetEvolveCurrentAmount()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Required Amount", ":", item.required_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:.{}f}", "Progression %", ":", value->GetEvolveProgression(), 2).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Type", ":", item.type).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "SubType", ":", item.sub_type).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Timer", ":", value->GetTimers().at("evolve").Enabled()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Timer Remaining", ":", value->GetTimers().at("evolve").GetRemainingTime()).c_str());
				c->Message(Chat::Green, fmt::format("Evolving Item Details:ID:{: >6} UID:{: >6} A:{} E:{} T:{} ST:{} C:{} R:{} P:{:.{}f}",
													value->GetID(),
													value->GetEvolveUniqueID(),
													value->GetEvolveActivated() ? "Y" : "N",
													value->GetEvolveEquiped() ? "Y" : "N",
													item.type,
													item.sub_type,
													value->GetEvolveCurrentAmount(),
													item.required_amount,
													value->GetEvolveProgression(),
													2).c_str()
				);
			}


		}
	}
	else if (is_client) {
		if (arguments > 1) {
			c->Message(Chat::White, "Usage: #evolve client");
		}
		else {
			// for (auto const& [key, value] : c->GetEvolvingItems()) {
			// 	auto item = evolving_items_manager.GetEvolvingItemsCache().at(value.item_id);
			// 	c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Evolving Items", ":", value.item_id).c_str());
			// 	c->Message(Chat::Yellow, fmt::format("{}{}{}", "Activated", ":", value.activated ? "Yes" : "No").c_str());
			// 	c->Message(Chat::Yellow, fmt::format("{}{}{}", "Equiped", ":", value.equiped ? "Yes" : "No").c_str());
			// 	c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Current Amount", ":", value.current_amount).c_str());
			// 	c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Required Amount", ":", item.required_amount).c_str());
			// 	c->Message(Chat::Yellow, fmt::format("{}{}{:.{}f}", "Progression %", ":", value.progression, 2).c_str());
			// 	c->Message(Chat::Yellow, fmt::format("{}{}{}", "Type", ":", item.type).c_str());
			// 	c->Message(Chat::Yellow, fmt::format("{}{}{}", "SubType", ":", item.sub_type).c_str());
			// 	c->Message(Chat::Green, fmt::format("Evolving Item Details:ID:{: >6} A:{} E:{} T:{} ST:{} C:{} R:{} P:{:.{}f}",
			// 										value.item_id,
			// 										value.activated ? "Y" : "N",
			// 										value.equiped ? "Y" : "N",
			// 										item.type,
			// 										item.sub_type,
			// 										value.current_amount,
			// 										item.required_amount,
			// 										value.progression,
			// 										2).c_str()
			// 	);
			// }
		}
	}
	else if (is_item) {
		if (arguments > 2) {
			c->Message(Chat::White, "Usage: #evolve item item_id");
		}
		else if(sep->IsNumber(2)){
			auto item_id = Strings::ToUnsignedInt(sep->arg[2]);
			auto item = c->GetInv().GetItem(c->GetInv().HasItem(item_id));
			if (item) {
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Evolving Items", ":", item->GetID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Slot", ":", c->GetInv().HasItem(item_id)).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Activated", ":", item->GetEvolveActivated() ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Equiped", ":", item->GetEvolveEquiped() ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Unique ID", ":", item->GetEvolveUniqueID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:.{}f}", "Progression %", ":", item->GetEvolveProgression(), 2).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Timer", ":", item->GetTimers().at("evolve").Enabled()).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Timer Remaining", ":", item->GetTimers().at("evolve").GetRemainingTime()).c_str());
			}
			else {
				c->Message(Chat::Red, fmt::format("Item {} could not be found in your inventory.", item_id).c_str());
			}
		}
		else {
			SendEvolvingItemsSubCommands(c);
		}
	}
	else if (is_zone) {
		if (arguments > 2) {
			c->Message(Chat::White, "Usage: #evolve zone item_id");
		}
		else if (sep->IsNumber(2)){
			auto item_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (evolving_items_manager.GetEvolvingItemsCache().contains(item_id)) {
				auto item = evolving_items_manager.GetEvolvingItemsCache().at(item_id);
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Evolving Items", ":", item.item_id).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Evolve Lore ID", ":", item.item_evo_id).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Evolve Level", ":", item.item_evolve_level).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{:0d}", "Required Amount", ":", item.required_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "Type", ":", item.type).c_str());
				c->Message(Chat::Yellow, fmt::format("{}{}{}", "SubType", ":", item.sub_type).c_str());
			}
			else {
				c->Message(Chat::Red, fmt::format("Item {} could not be found or is not an evolving item.", item_id).c_str());
			}
		}
		else {
			SendEvolvingItemsSubCommands(c);
		}
	}
}

void SendEvolvingItemsSubCommands(Client* c)
{
	c->Message(Chat::White, "#evolve test");
	c->Message(Chat::White, "#evolve client (Shows evolve values within the client cache)");
	c->Message(Chat::White, "#evolve target (Shows evolve values within the target's cache - Must have a player target selected.)");
	c->Message(Chat::White, "#evolve item item_id (Shows evolve values within the iteminstance");
	c->Message(Chat::White, "#evolve zone item_id (Shows evolve values within the zone cache)");
}
