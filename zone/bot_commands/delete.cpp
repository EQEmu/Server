#include "../client.h"
#include "../bot_command.h"

void bot_command_delete(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_delete", sep->arg[0], "botdelete"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <target_bot> %s", sep->arg[0]);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must <target> a bot that you own to use this command");
		return;
	}

	if (!my_bot->DeleteBot()) {
		c->Message(Chat::White, "Failed to delete '%s' due to database error", my_bot->GetCleanName());
		return;
	}

	auto bid = my_bot->GetBotID();
	std::string bot_name = my_bot->GetCleanName();

	my_bot->Camp(false);

	c->Message(Chat::White, "Successfully deleted bot '%s' (id: %i)", bot_name.c_str(), bid);
}
