#include "../client.h"
#include "../bot_command.h"

void bot_command_suffix(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must specify a [suffix] to use this command. (use _ to define spaces or -remove to clear.)");
		return;
	}
	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}
	if (strlen(sep->arg[1]) > 31) {
		c->Message(Chat::White, "Suffix must be 31 characters or less.");
		return;
	}
	std::string bot_suffix = sep->arg[1];
	bot_suffix = (bot_suffix == "-remove") ? "" : bot_suffix;
	std::replace(bot_suffix.begin(), bot_suffix.end(), '_', ' ');

	my_bot->SetSuffix(bot_suffix);
	if (!database.botdb.SaveBot(my_bot)) {
		c->Message(Chat::White, BotDatabase::fail::SaveBot());
	}
	else {
		c->Message(Chat::White, "Bot Suffix Saved.");
	}
}
