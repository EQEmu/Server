#include "../bot_command.h"

void bot_command_follow(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_follow", sep->arg[0], "follow")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Sets bots of your choosing to follow your target, view their current following state or reset their following state."
		};

		std::vector<std::string> notes =
		{
			"- You can only follow players, bots or mercenaries belonging to your group or raid."
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [optional] [actionable]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To set all Clerics to follow your target:",
			fmt::format(
				"{} byclass {}",
				sep->arg[0],
				Class::Cleric
			)
		};
		std::vector<std::string> examples_two =
		{
			"To check the current state of all bots:",
			fmt::format(
				"{} current spawned",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_three =
		{
			"To reset all bots:",
			fmt::format(
				"{} reset spawned",
				sep->arg[0]
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

		return;
	}

	const int ab_mask = ActionableBots::ABM_Type2;

	bool chain = false;
	bool reset = false;
	bool currentCheck = false;
	int ab_arg = 1;
	Mob* target_mob = nullptr;

	std::string optional_arg = sep->arg[1];
	
	if (!optional_arg.compare("reset")) {
		target_mob = c;
		reset = true;
		++ab_arg;
	}
	else if (!optional_arg.compare("current")) {
		currentCheck = true;
		++ab_arg;
	}
	else {
		target_mob = c->GetTarget();

		if (!target_mob || !target_mob->IsOfClientBotMerc() || !c->IsInGroupOrRaid(target_mob)) {
			c->Message(Chat::Yellow, "You must <target> a friendly player, bot or merc within your group or raid to use this command");
			return;
		}

		if (!optional_arg.compare("chain")) {
			chain = true;
			++ab_arg;
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

	auto botCount = sbl.size();
	Mob* follow_mob = nullptr;
	std::list<Bot*> chainList;
	std::list<Bot*>::const_iterator it = chainList.begin();
	uint16 count = 0;
	for (auto bot_iter : sbl) {
		if (currentCheck) {
			follow_mob = entity_list.GetMob(bot_iter->GetFollowID());

			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I am currently following {}.'",
					bot_iter->GetCleanName(),
					follow_mob ? follow_mob->GetCleanName() : "no one"
				).c_str()
			);

			if (!follow_mob && RuleB(Bots, DoResponseAnimations)) {
					bot_iter->DoAnim(28);
			}

			continue;
		}

		if (bot_iter == target_mob) {
			if (botCount == 1) {
				c->Message(
					Chat::Yellow,
					fmt::format(
						"{} says, 'I cannot follow myself, you want me to run circles?",
						bot_iter->GetCleanName()
					).c_str()
				);

				if (RuleB(Bots, DoResponseAnimations)) {
					bot_iter->DoAnim(60);
				}

				return;
			}

			bot_iter->WipeHateList();
			--botCount;

			continue;
		}

		if (!bot_iter->IsInGroupOrRaid(target_mob)) {
			--botCount;

			continue;
		}

		bot_iter->WipeHateList();

		if (!bot_iter->GetGroup() && !bot_iter->GetRaid()) {
			bot_iter->SetFollowID(0);
			bot_iter->SetManualFollow(false);
		}
		else {
			if (reset) {
				bot_iter->SetFollowID(c->GetID());
				bot_iter->SetManualFollow(false);
			}
			else {
				if (chain) {
					Mob* nextTar = target_mob;

					if (count > 0) {
						nextTar = *it;

						if (!nextTar) {
							nextTar = target_mob;
						}
					}

					chainList.push_back(bot_iter);
					++it;
					++count;
					bot_iter->SetFollowID(nextTar->GetID());
				}
				else {
					bot_iter->SetFollowID(target_mob->GetID());
				}

				bot_iter->SetManualFollow(true);
			}
		}

		if (!bot_iter->GetPet()) {
			continue;
		}

		bot_iter->GetPet()->WipeHateList();
		bot_iter->GetPet()->SetFollowID(bot_iter->GetID());
	}

	if (currentCheck || !botCount) {
		return;
	}

	follow_mob = target_mob;

	if (botCount == 1) {
		follow_mob = entity_list.GetMob(sbl.front()->GetFollowID());

		c->Message(
			Chat::Green,
			fmt::format(
				"{} says, 'Following {}.'",
				sbl.front()->GetCleanName(),
				follow_mob ? follow_mob->GetCleanName() : "you"
			).c_str()
		);
	}
	else {
		if (reset) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots are following you.",
					botCount
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots are {} {}.",
					botCount,
					chain ? "chain following" : "following",
					follow_mob ? follow_mob->GetCleanName() : "you"
				).c_str()
			);
		}
	}
}
