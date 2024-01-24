#include "../client.h"
#include "../bot_command.h"

void bot_command_update(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_update", sep->arg[0], "botupdate"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s", sep->arg[0]);
		return;
	}

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);
	if (sbl.empty()) {
		c->Message(Chat::White, "You currently have no spawned bots");
		return;
	}

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter || bot_iter->IsEngaged() || bot_iter->GetLevel() == c->GetLevel())
			continue;

		bot_iter->SetPetChooser(false);
		bot_iter->CalcBotStats(c->GetBotOption(Client::booStatsUpdate));
		bot_iter->SendAppearancePacket(AppearanceType::WhoLevel, bot_iter->GetLevel(), true, true);
		++bot_count;
	}

	c->Message(Chat::White, "Updated %i of your %i spawned bots", bot_count, sbl.size());
}
