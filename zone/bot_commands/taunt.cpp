#include "../bot_command.h"

void bot_command_taunt(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_taunt", sep->arg[0], "taunt"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([option: on | off]) ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_Type1;

	std::string arg1 = sep->arg[1];

	bool taunt_state = false;
	bool toggle_taunt = true;
	int ab_arg = 1;
	if (!arg1.compare("on")) {
		taunt_state = true;
		toggle_taunt = false;
		ab_arg = 2;
	}
	else if (!arg1.compare("off")) {
		toggle_taunt = false;
		ab_arg = 2;
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[(ab_arg + 1)] : nullptr, class_race_check ? atoi(sep->arg[(ab_arg + 1)]) : 0) == ActionableBots::ABT_None) {
		return;
	}
	sbl.remove(nullptr);

	int taunting_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter->GetSkill(EQ::skills::SkillTaunt)) {
			continue;
		}

		if (toggle_taunt) {
			bot_iter->SetTaunting(!bot_iter->IsTaunting());
		} else {
			bot_iter->SetTaunting(taunt_state);
		}

		if (sbl.size() == 1) {
			Bot::BotGroupSay(
				bot_iter,
				fmt::format(
					"I am {} taunting.",
					bot_iter->IsTaunting() ? "now" : "no longer"
				).c_str()
			);
		}

		++taunting_count;
	}

	for (auto bot_iter : sbl) {
		if (!bot_iter->HasPet()) {
			continue;
		}

		if (!bot_iter->GetPet()->GetSkill(EQ::skills::SkillTaunt)) {
			continue;
		}

		if (toggle_taunt) {
			bot_iter->GetPet()->CastToNPC()->SetTaunting(!bot_iter->GetPet()->CastToNPC()->IsTaunting());
		} else {
			bot_iter->GetPet()->CastToNPC()->SetTaunting(taunt_state);
		}

		if (sbl.size() == 1) {
			Bot::BotGroupSay(
				bot_iter,
				fmt::format(
					"My Pet is {} taunting.",
					bot_iter->GetPet()->CastToNPC()->IsTaunting() ? "now" : "no longer"
				).c_str()
			);
		}

		++taunting_count;
	}

	if (taunting_count) {
		if (toggle_taunt) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} of your bots and their pets {} toggled their taunting state",
					taunting_count,
					taunting_count != 1 ? "have" : "has"
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} of your bots and their pets {} {} taunting.",
					taunting_count,
					taunting_count != 1 ? "have" : "has",
					taunt_state ? "started" : "stopped"
				).c_str()
			);
		}
	}
	else {
		c->Message(Chat::White, "None of your bots are capable of taunting");
	}
}
