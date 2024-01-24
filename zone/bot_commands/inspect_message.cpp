#include "../client.h"
#include "../bot_command.h"

void bot_command_inspect_message(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_inspect_message", sep->arg[0], "botinspectmessage"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [set | clear] ([actionable: target | byname | ownergroup | targetgroup | namesgroup | healrotation | spawned] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "Notes:");
		if (c->ClientVersion() >= EQ::versions::ClientVersion::SoF) {
			c->Message(Chat::White, "- Self-inspect and type your bot's inspect message");
			c->Message(Chat::White, "- Close the self-inspect window to update the server");
			c->Message(Chat::White, "- Type '%s set' to set the bot's inspect message", sep->arg[0]);
		}
		else {
			c->Message(Chat::White, "- Self-inspect and type your bot's inspect message");
			c->Message(Chat::White, "- Close the self-inspect window");
			c->Message(Chat::White, "- Self-inspect again to update the server");
			c->Message(Chat::White, "- Type '%s set' to set the bot's inspect message", sep->arg[0]);
		}
		return;
	}
	const int ab_mask = ActionableBots::ABM_NoFilter;

	bool set_flag = false;
	if (!strcasecmp(sep->arg[1], "set")) {
		set_flag = true;
	}
	else if (strcasecmp(sep->arg[1], "clear")) {
		c->Message(Chat::White, "This command requires a [set | clear] argument");
		return;
	}

	std::list<Bot*> sbl;
	auto ab_type = ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]);
	if (ab_type == ActionableBots::ABT_None)
		return;

	const auto client_message_struct = &c->GetInspectMessage();

	int bot_count = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		auto bot_message_struct = &bot_iter->GetInspectMessage();
		memset(bot_message_struct, 0, sizeof(InspectMessage_Struct));
		if (set_flag)
			memcpy(bot_message_struct, client_message_struct, sizeof(InspectMessage_Struct));

		if (ab_type != ActionableBots::ABT_All && !database.botdb.SaveInspectMessage(bot_iter->GetBotID(), *bot_message_struct)) {
			c->Message(Chat::White, "%s for '%s'", BotDatabase::fail::SaveInspectMessage(), bot_iter->GetCleanName());
			return;
		}

		++bot_count;
	}

	if (ab_type == ActionableBots::ABT_All) {
		InspectMessage_Struct bot_message_struct;
		memset(&bot_message_struct, 0, sizeof(InspectMessage_Struct));
		if (set_flag)
			memcpy(&bot_message_struct, client_message_struct, sizeof(InspectMessage_Struct));

		if (!database.botdb.SaveAllInspectMessages(c->CharacterID(), bot_message_struct)) {
			c->Message(Chat::White, "%s", BotDatabase::fail::SaveAllInspectMessages());
			return;
		}

		c->Message(Chat::White, "%s all of your bot inspect messages", set_flag ? "Set" : "Cleared");
	}
	else {
		c->Message(Chat::White, "%s %i of your spawned bot inspect messages", set_flag ? "Set" : "Cleared", bot_count);
	}
}
