#include "../client.h"
#include "../bot_command.h"

void bot_command_camp(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_camp", sep->arg[0], "botcamp"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type1;

	std::string class_race_arg = sep->arg[1];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, !class_race_check ? sep->arg[2] : nullptr, class_race_check ? atoi(sep->arg[2]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	for (auto bot_iter : sbl)
		bot_iter->Camp();
}
