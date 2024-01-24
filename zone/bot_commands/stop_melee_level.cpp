#include "../client.h"
#include "../bot_command.h"

void bot_command_stop_melee_level(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_stop_melee_level", sep->arg[0], "botstopmeleelevel"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s [current | reset | sync | value: 0-255]", sep->arg[0]);
		c->Message(Chat::White, "note: Only caster or hybrid class bots may be modified");
		c->Message(Chat::White, "note: Use [reset] to set stop melee level to server rule");
		c->Message(Chat::White, "note: Use [sync] to set stop melee level to current bot level");
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}
	if (!IsCasterClass(my_bot->GetClass()) && !IsHybridClass(my_bot->GetClass())) {
		c->Message(Chat::White, "You must <target> a caster or hybrid class bot to use this command");
		return;
	}

	uint8 sml = RuleI(Bots, CasterStopMeleeLevel);

	if (sep->IsNumber(1)) {
		sml = Strings::ToInt(sep->arg[1]);
	}
	else if (!strcasecmp(sep->arg[1], "sync")) {
		sml = my_bot->GetLevel();
	}
	else if (!strcasecmp(sep->arg[1], "current")) {
		c->Message(Chat::White, "My current melee stop level is %u", my_bot->GetStopMeleeLevel());
		return;
	}
	else if (strcasecmp(sep->arg[1], "reset")) {
		c->Message(Chat::White, "A [current] or [reset] argument, or numeric [value] is required to use this command");
		return;
	}
	// [reset] falls through with initialization value

	my_bot->SetStopMeleeLevel(sml);
	if (!database.botdb.SaveStopMeleeLevel(c->CharacterID(), my_bot->GetBotID(), sml))
		c->Message(Chat::White, "%s for '%s'", BotDatabase::fail::SaveStopMeleeLevel(), my_bot->GetCleanName());

	c->Message(Chat::White, "Successfully set stop melee level for %s to %u", my_bot->GetCleanName(), sml);
}
