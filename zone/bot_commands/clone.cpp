#include "../client.h"
#include "../bot_command.h"

void bot_command_clone(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_clone", sep->arg[0], "botclone")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: <target_bot> {} [clone_name]",
				sep->arg[0]
			).c_str()
		);
		return;
	}

	auto my_bot = ActionableBots::AsTarget_ByBot(c);
	if (!my_bot) {
		c->Message(Chat::White, "You must target a bot that you own to use this command!");
		return;
	}

	if (!my_bot->GetBotID()) {
		c->Message(
			Chat::White,
			fmt::format(
				"An unknown error has occured with {} (Bot ID {}).",
				my_bot->GetCleanName(),
				my_bot->GetBotID()
			).c_str()
		);
		LogCommands(
			"bot_command_clone(): - Error: Active bot reported invalid ID (BotName: [{}], BotID: [{}], OwnerName: [{}], OwnerID: [{}], AcctName: [{}], AcctID: [{}])",
			my_bot->GetCleanName(),
			my_bot->GetBotID(),
			c->GetCleanName(),
			c->CharacterID(),
			c->AccountName(),
			c->AccountID()
		);
		return;
	}

	if (sep->arg[1][0] == '\0' || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must name your bot clone.");
		return;
	}

	std::string bot_name = sep->arg[1];

	if (!Bot::IsValidName(bot_name)) {
		c->Message(
			Chat::White,
			fmt::format(
				"'{}' is an invalid name. You may only use characters 'A-Z', 'a-z' and '_'.",
				bot_name
			).c_str()
		);
		return;
	}

	bool available_flag = false;
	if (!database.botdb.QueryNameAvailablity(bot_name, available_flag)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to query name availability for '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	if (!available_flag) {
		c->Message(
			Chat::White,
			fmt::format(
				"The name '{}' is already being used. Please choose a different name.",
				bot_name
			).c_str()
		);
		return;
	}

	auto bot_creation_limit = c->GetBotCreationLimit();
	auto bot_creation_limit_class = c->GetBotCreationLimit(my_bot->GetClass());

	uint32 bot_count = 0;
	uint32 bot_class_count = 0;
	if (!database.botdb.QueryBotCount(c->CharacterID(), my_bot->GetClass(), bot_count, bot_class_count)) {
		c->Message(Chat::White, "Failed to query bot count.");
		return;
	}

	if (bot_creation_limit >= 0 && bot_count >= bot_creation_limit) {
		std::string message;

		if (bot_creation_limit) {
			message =  fmt::format(
				"You have reached the maximum limit of {} bot{}.",
				bot_creation_limit,
				bot_creation_limit != 1 ? "s" : ""
			);
		} else {
			message = "You cannot create any bots.";
		}

		c->Message(Chat::White, message.c_str());
		return;
	}

	if (bot_creation_limit_class >= 0 && bot_class_count >= bot_creation_limit_class) {
		std::string message;

		if (bot_creation_limit_class) {
			message = fmt::format(
				"You cannot create anymore than {} {} bot{}.",
				bot_creation_limit_class,
				GetClassIDName(my_bot->GetClass()),
				bot_creation_limit_class != 1 ? "s" : ""
			);
		} else {
			message = fmt::format(
				"You cannot create any {} bots.",
				GetClassIDName(my_bot->GetClass())
			);
		}

		c->Message(Chat::White, message.c_str());
		return;
	}

	uint32 clone_id = 0;
	if (!database.botdb.CreateCloneBot(c->CharacterID(), my_bot->GetBotID(), bot_name, clone_id) || !clone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to create clone bot '{}'.",
				bot_name
			).c_str()
		);
		return;
	}

	int clone_stance = EQ::constants::stancePassive;
	if (!database.botdb.LoadStance(my_bot->GetBotID(), clone_stance)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to load stance from '{}'.",
				my_bot->GetCleanName()
			).c_str()
		);
	}

	if (!database.botdb.SaveStance(clone_id, clone_stance)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to save stance for clone '{}'.",
				bot_name
			).c_str()
		);
	}

	if (!database.botdb.CreateCloneBotInventory(c->CharacterID(), my_bot->GetBotID(), clone_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to create clone bot inventory for clone '{}'.",
				bot_name
			).c_str()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Bot Cloned | From: {} To: {}",
			my_bot->GetCleanName(),
			bot_name
		).c_str()
	);
}
