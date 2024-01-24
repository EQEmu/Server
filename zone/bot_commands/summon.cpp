#include "../client.h"
#include "../bot_command.h"

void bot_command_summon(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_summon", sep->arg[0], "botsummon")) {
		return;
	}

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

	sbl.remove(nullptr);

	for (auto bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}

		bot_iter->WipeHateList();
		bot_iter->SetTarget(nullptr);
		bot_iter->Teleport(c->GetPosition());
		bot_iter->DoAnim(0);

		if (!bot_iter->HasPet()) {
			continue;
		}

		bot_iter->GetPet()->WipeHateList();
		bot_iter->GetPet()->SetTarget(nullptr);
		bot_iter->GetPet()->Teleport(c->GetPosition());
	}

	if (sbl.size() == 1) {
		c->Message(
			Chat::White,
			fmt::format(
				"Summoned {} to you.",
				sbl.front() ? sbl.front()->GetCleanName() : "no one"
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::White,
			fmt::format(
				"Summoned {} bots to you.",
				sbl.size()
			).c_str()
		);
	}
}
