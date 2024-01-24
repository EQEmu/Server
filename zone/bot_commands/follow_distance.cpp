#include "../client.h"
#include "../bot_command.h"

void bot_command_follow_distance(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_follow_distance", sep->arg[0], "botfollowdistance"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [set] [distance] ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "usage: %s [clear] ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	uint32 bfd = BOT_FOLLOW_DISTANCE_DEFAULT;
	bool set_flag = false;
	int ab_arg = 2;

	if (!strcasecmp(sep->arg[1], "set")) {
		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "A numeric [distance] is required to use this command");
			return;
		}

		bfd = Strings::ToInt(sep->arg[2]);
		if (bfd < 1)
			bfd = 1;
		if (bfd > BOT_FOLLOW_DISTANCE_DEFAULT_MAX)
			bfd = BOT_FOLLOW_DISTANCE_DEFAULT_MAX;
		set_flag = true;
		ab_arg = 3;
	}
	else if (strcasecmp(sep->arg[1], "clear")) {
		c->Message(Chat::White, "This command requires a [set | clear] argument");
		return;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, sep->arg[(ab_arg + 1)]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		bot_iter->SetFollowDistance(bfd);
		if (ab_type != ActionableBots::ABT_All && !database.botdb.SaveFollowDistance(c->CharacterID(), bot_iter->GetBotID(), bfd)) {
			c->Message(Chat::White, "%s for '%s'", BotDatabase::fail::SaveFollowDistance(), bot_iter->GetCleanName());
			return;
		}

		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		if (!database.botdb.SaveAllFollowDistances(c->CharacterID(), bfd)) {
			c->Message(Chat::White, "%s", BotDatabase::fail::SaveAllFollowDistances());
			return;
		}

		c->Message(Chat::White, "%s all of your bot follow distances", set_flag ? "Set" : "Cleared");
	}
	else {
		c->Message(Chat::White, "%s %i of your spawned bot follow distances", (set_flag ? "Set" : "Cleared"), bot_count);
	}
}
