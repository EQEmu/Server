#include "../bot_command.h"

void bot_command_caster_range(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_caster_range", sep->arg[0], "casterrange")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [current | value: 0 - 300] ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationtargets | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "note: Can only be used for Casters or Hybrids.");
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
	uint32 crange = 0;

	if (sep->IsNumber(1)) {
		ab_arg = 2;
		crange = atoi(sep->arg[1]);
		if (crange < 0 || crange > 300) {
			c->Message(Chat::White, "You must enter a value within the range of 0 - 300.");
			return;
		}
	}
	else if (!arg1.compare("current")) {
		ab_arg = 2;
		current_check = true;
	}
	else {
		c->Message(Chat::White, "Incorrect argument, use %s help for a list of options.", sep->arg[0]);
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
		if (!IsCasterClass(my_bot->GetClass()) && !IsHybridClass(my_bot->GetClass())) {
			continue;
		}

		if (!first_found) {
			first_found = my_bot;
		}

		if (current_check) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} says, 'My current Caster Range is {}.'",
					my_bot->GetCleanName(),
					my_bot->GetBotCasterRange()
				).c_str()
			);
		}
		else {
			my_bot->SetBotCasterRange(crange);
			++success_count;

			database.botdb.SaveBotCasterRange(my_bot->GetBotID(), crange);
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} says, 'My Caster Range was set to {}.'",
					first_found->GetCleanName(),
					crange
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} of your bots set their Caster Range to {}.",
					success_count,
					crange
				).c_str()
			);
		}
	}
}
