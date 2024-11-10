#include "../bot_command.h"

void bot_command_distance_ranged(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_distance_ranged", sep->arg[0], "distanceranged")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [current | value: 0 - 300] ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "note: Use [current] to check the current setting.");
		c->Message(Chat::White, "note: Set the value to the minimum distance you want your bot to try to remain from its target.");
		c->Message(Chat::White, "note: If they are too far for a spell, it will be skipped.");
		c->Message(Chat::White, "note: This is set to (90) units by default.");
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;

	std::string arg1 = sep->arg[1];
	int ab_arg = 1;
	bool current_check = false;
	uint32 value = 0;

	if (sep->IsNumber(1)) {
		++ab_arg;
		value = atoi(sep->arg[1]);
		if (value < 0 || value > RuleI(Bots, MaxDistanceRanged)) {
			c->Message(Chat::Yellow, "You must enter a value within the range of 0 - 300.");
			return;
		}
	}
	else if (!arg1.compare("current")) {
		++ab_arg;
		current_check = true;
	}
	else {
		c->Message(Chat::Yellow, "Incorrect argument, use %s help for a list of options.", sep->arg[0]);
		return;
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

	Bot* first_found = nullptr;
	int success_count = 0;

	for (auto my_bot : sbl) {
		if (my_bot->BotPassiveCheck()) {
			continue;
		}

		if (!first_found) {
			first_found = my_bot;
		}

		if (current_check) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My current Distance Ranged is {}.'",
					my_bot->GetCleanName(),
					my_bot->GetBotDistanceRanged()
				).c_str()
			);
		}
		else {
			my_bot->SetBotDistanceRanged(value);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My Distance Ranged was set to {}.'",
					first_found->GetCleanName(),
					value
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots set their Distance Ranged to {}.",
					success_count,
					value
				).c_str()
			);
		}
	}
}
