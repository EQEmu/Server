#include "../client.h"

void command_level(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #level [Level]");
		return;
	}

	auto target = c->GetTarget();
	if (!target) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}
	
	auto level = static_cast<uint8>(std::stoul(sep->arg[1]));
	auto max_level = static_cast<uint8>(RuleI(Character, MaxLevel));

	if (c->Admin() < RuleI(GM, MinStatusToLevelTarget)) {
		c->Message(Chat::White, "Your status is not high enough to change another person's level.");
		return;
	}

	if (
		level > max_level &&
		c->Admin() < commandLevelAboveCap
	) {
		c->Message(
			Chat::White,
			fmt::format(
				"Level {} is above the Maximum Level of {} and your status is not high enough to go beyond the cap.",
				level,
				max_level
			).c_str()
		);
		return;
	}

	target->SetLevel(level, true);
	if (target->IsClient()) {
		target->CastToClient()->SendLevelAppearance();

#ifdef BOTS
		if (RuleB(Bots, BotLevelsWithOwner)) {
			Bot::LevelBotWithClient(target->CastToClient(), level, true);
		}
#endif

	}
}