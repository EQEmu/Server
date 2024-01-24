#include "../client.h"
#include "../bot_command.h"

void bot_command_title(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must specify a [title] to use this command. (use _ to define spaces or -remove to clear.)");
		return;
	}
	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}
	if (strlen(sep->arg[1]) > 31) {
		c->Message(Chat::White, "Title must be 31 characters or less.");
		return;
	}
	std::string bot_title = sep->arg[1];
	bot_title = (bot_title == "-remove") ? "" : bot_title;
	std::replace(bot_title.begin(), bot_title.end(), '_', ' ');

	my_bot->SetTitle(bot_title);
	if (!database.botdb.SaveBot(my_bot)) {
		c->Message(Chat::White, BotDatabase::fail::SaveBot());
	}
	else {
		c->Message(Chat::White, "Bot Title Saved.");
	}
}
