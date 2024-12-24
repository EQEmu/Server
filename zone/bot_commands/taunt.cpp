#include "../bot_command.h"

void bot_command_taunt(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_taunt", sep->arg[0], "taunt")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Allows you to turn on/off the taunting state of your bots and/or their pets"
		};

		std::vector<std::string> notes = { };

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [on / off / pet] [optional: pet]  [actionable, default: target]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To turn off taunt on all bots:",
			fmt::format(
				"{} off spawned",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_two =
		{
			"To turn on taunt on all bots' pets:",
			fmt::format(
				"{} on pet spawned",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_three =
		{
			"To turn on taunt for all ShadowKnights:",
			fmt::format(
				"{} on byclass {}",
				sep->arg[0],
				Class::ShadowKnight
			)
		};

		std::vector<std::string> actionables =
		{
			"target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned"
		};

		std::vector<std::string> options = { };
		std::vector<std::string> options_one = { };
		std::vector<std::string> options_two = { };
		std::vector<std::string> options_three = { };

		std::string popup_text = c->SendCommandHelpWindow(
			c,
			description,
			notes,
			example_format,
			examples_one, examples_two, examples_three,
			actionables,
			options,
			options_one, options_two, options_three
		);

		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

		if (RuleB(Bots, SendClassRaceOnHelp)) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Use {} for information about race/class IDs.",
					Saylink::Silent("^classracelist")
				).c_str()
			);
		}

		return;
	}

	std::string arg1 = sep->arg[1];
	std::string arg2 = sep->arg[2];

	bool tauntState = false;
	bool petTaunt = false;
	bool validOption = false;

	int ab_arg = 1;

	if (!arg1.compare("on")) {
		tauntState = true;
		validOption = true;
		++ab_arg;
	}
	else if (!arg1.compare("off")) {
		validOption = true;
		++ab_arg;
	}

	if (!arg2.compare("pet")) {
		petTaunt = true;
		validOption = true;
		++ab_arg;
	}

	if (!validOption) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Incorrect argument, use {} for information regarding this command.",
				Saylink::Silent(
					fmt::format("{} help", sep->arg[0])
				)
			).c_str()
		);

		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string actionableArg = sep->arg[ab_arg];

	if (actionableArg.empty()) {
		actionableArg = "target";
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, actionableArg, sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	int botTauntingCount = 0;
	int petTauntingCount = 0;

	if (!petTaunt) {
		for (auto bot_iter : sbl) {
			if (!bot_iter->GetSkill(EQ::skills::SkillTaunt)) {
				continue;
			}

			bot_iter->SetTaunting(tauntState);

			Bot::BotGroupSay(
				bot_iter,
				fmt::format(
					"I am {} taunting.",
					bot_iter->IsTaunting() ? "now" : "no longer"
				).c_str()
			);

			++botTauntingCount;
		}
	}

	if (petTaunt) {
		for (auto bot_iter : sbl) {
			if (!bot_iter->HasPet()) {
				continue;
			}

			if (!bot_iter->GetPet()->GetSkill(EQ::skills::SkillTaunt)) {
				continue;
			}

			bot_iter->GetPet()->CastToNPC()->SetTaunting(tauntState);

			Bot::BotGroupSay(
				bot_iter,
				fmt::format(
					"My Pet is {} taunting.",
					bot_iter->GetPet()->CastToNPC()->IsTaunting() ? "now" : "no longer"
				).c_str()
			);

			++petTauntingCount;
		}
	}

	if (botTauntingCount || petTauntingCount) {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} of your {} are {} taunting.",
				(botTauntingCount ? botTauntingCount : petTauntingCount),
				(botTauntingCount ? "bots" : "bots' pets"),
				tauntState ? "now" : "no longer"
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"None of your {} are capable of taunting.",
				!petTaunt ? "bots" : "bots' pets"
			).c_str()
		);
	}
}
