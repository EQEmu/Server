#include "../client.h"
#include "../bot_command.h"

void bot_command_out_of_combat(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_out_of_combat", sep->arg[0], "botoutofcombat"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([option: on | off]) ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::string arg1 = sep->arg[1];

	bool behavior_state = false;
	bool toggle_behavior = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		behavior_state = true;
		toggle_behavior = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_behavior = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]) == ActionableBots::ABT_None)
		return;

	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (toggle_behavior)
			bot_iter->SetAltOutOfCombatBehavior(!bot_iter->GetAltOutOfCombatBehavior());
		else
			bot_iter->SetAltOutOfCombatBehavior(behavior_state);

		helper_bot_out_of_combat(c, bot_iter);
	}
}
