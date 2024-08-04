#include "../bot_command.h"

void bot_command_click_item(Client* c, const Seperator* sep)
{
	if (!RuleB(Bots, BotsCanClickItems)) {
		c->Message(Chat::White, "The ability for bots to click equipped items is currently disabled.");
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <slot id> %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "This will cause the selected bots to click the item in the given slot ID.");
		c->Message(Chat::White, "Use ^invlist to see their items along with slot IDs.");
		return;
	}

	if (!sep->IsNumber(1)) {
		c->Message(Chat::Yellow, "You must specify a slot ID. Use %s help for more information.", sep->arg[0]);
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	int ab_arg = 1;
	uint32 slot_id = 0;

	if (sep->IsNumber(1)) {
		ab_arg = 2;
		slot_id = atoi(sep->arg[1]);
		if (slot_id < EQ::invslot::EQUIPMENT_BEGIN || slot_id > EQ::invslot::EQUIPMENT_END) {
			c->Message(Chat::Yellow, "You must specify a valid inventory slot from 0 to 22. Use %s help for more information", sep->arg[0]);
			return;
		}
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}
	sbl.remove(nullptr);

	for (auto my_bot : sbl) {
		if (RuleI(Bots, BotsClickItemsMinLvl) > my_bot->GetLevel()) {
			c->Message(Chat::White, "%s must be level %i to use clickable items.", my_bot->GetCleanName(), RuleI(Bots, BotsClickItemsMinLvl));
			continue;
		}

		my_bot->TryItemClick(slot_id);
	}
}
