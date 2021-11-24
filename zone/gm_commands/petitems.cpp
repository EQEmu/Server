#include "../client.h"

void command_petitems(Client *c, const Seperator *sep)
{
	if (!c->GetPet()) {
		c->Message(Chat::White, "You must have a pet to use this command.");
		return;
	}
	
	auto pet = c->GetPet()->CastToNPC();
	auto loot_list = pet->GetLootList();
	if (!loot_list.empty()) {
		pet->QueryLoot(c, true);
		c->Message(
			Chat::White,
			fmt::format(
				"Your pet has {} item{}.",
				loot_list.size(),
				loot_list.size() != 1 ? "s" : ""
			).c_str()
		);
	} else {
		c->Message(Chat::White, "Your pet has no items.");
	}
}
