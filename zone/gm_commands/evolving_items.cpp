#include "../client.h"
#include "../command.h"
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
		if (arguments > 1) {
			c->Message(Chat::White, "Usage: #evolve test");
		}
		else {
			for (auto const& i : *t->GetEvolvingItems()) {
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Evolving Items", ":", i.second.item_id).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Activated", ":", i.second.activated ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Current Amount", ":", i.second.current_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Required Amount", ":", i.second.required_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Progression %", ":", i.second.progression).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Type", ":", i.second.type).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "SubType", ":", i.second.subtype).c_str());
			}
		}
	}
	else if (is_target) {
		if (arguments > 1) {
			c->Message(Chat::White, "Usage: #evolve target");
		}
		else {
			for (auto const& i : *t->GetEvolvingItems()) {
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Evolving Items", ":", i.second.item_id).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Activated", ":", i.second.activated ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Current Amount", ":", i.second.current_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Required Amount", ":", i.second.required_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Progression %", ":", i.second.progression).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Type", ":", i.second.type).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "SubType", ":", i.second.subtype).c_str());
			}
		}
	}
	else if (is_client) {
		if (arguments > 1) {
			c->Message(Chat::White, "Usage: #evolve client");
		}
		else {
			for (auto const& i : *c->GetEvolvingItems()) {
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Evolving Items", ":", i.second.item_id).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Activated", ":", i.second.activated ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Current Amount", ":", i.second.current_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Required Amount", ":", i.second.required_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Progression %", ":", i.second.progression).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Type", ":", i.second.type).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "SubType", ":", i.second.subtype).c_str());
			}
		}
	}
	else if (is_item) {
		if (arguments > 2) {
			c->Message(Chat::White, "Usage: #evolve item item_id");
		}
		else {
			std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(Strings::ToUnsignedInt(sep->arg[2])));
			if (inst->GetItem()) {
				auto i = inst->GetEvolvingInfo();
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Evolving Items", ":", inst->GetID()).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Activated", ":", i->activated ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Current Amount", ":", i->current_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Required Amount", ":", i->required_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Progression %", ":", i->progression).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Type", ":", i->type).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "SubType", ":", i->sub_type).c_str());
			}
		}
	}
	else if (is_zone) {
		if (arguments > 2) {
			c->Message(Chat::White, "Usage: #evolve zone [item_id]");
		}
		else if (sep->IsNumber(2)){

			for (auto const& i : *t->GetEvolvingItems()) {
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Evolving Items", ":", i.second.item_id).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Activated", ":", i.second.activated ? "Yes" : "No").c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Current Amount", ":", i.second.current_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Required Amount", ":", i.second.required_amount).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Progression %", ":", i.second.progression).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "Type", ":", i.second.type).c_str());
				c->Message(Chat::Yellow, fmt::format("{: <16}{: <1}{}", "SubType", ":", i.second.subtype).c_str());
			}
		}
	}
}

void SendEvolvingItemsSubCommands(Client* c)
{
	c->Message(Chat::White, "#evolve test");
}
