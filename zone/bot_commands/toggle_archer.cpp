#include "../client.h"
#include "../bot_command.h"

void bot_command_toggle_archer(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_toggle_archer", sep->arg[0], "bottogglearcher")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([option: on | off]) ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::string arg1 = sep->arg[1];

	bool archer_state = false;
	bool toggle_archer = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		archer_state = true;
		toggle_archer = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_archer = false;
		ab_arg = 2;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]) == ActionableBots::ABT_None) {
		return;
	}

	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		if (toggle_archer) {
			bot_iter->SetBotArcherySetting(!bot_iter->IsBotArcher(), true);
		}
		else {
			bot_iter->SetBotArcherySetting(archer_state, true);
		}
		bot_iter->ChangeBotArcherWeapons(bot_iter->IsBotArcher());

		if (bot_iter->GetClass() == Class::Ranger && bot_iter->GetLevel() >= 61) {
			bot_iter->SetRangerAutoWeaponSelect(bot_iter->IsBotArcher());
		}
	}
}
