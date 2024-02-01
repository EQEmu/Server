#include "../bot_command.h"

void bot_command_actionable(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_actionable", sep->arg[0], "actionable")) {
		return;
	}

	c->Message(Chat::White, "Actionable command arguments:");
	c->Message(Chat::White, "target - selects target as single bot .. use ^command [target] or imply by empty actionable argument");
	c->Message(Chat::White, "byname [name] - selects single bot by name");
	c->Message(Chat::White, "ownergroup - selects all bots in the owner's group");
	c->Message(Chat::White, "ownerraid - selects all bots in the owner's raid");
	c->Message(Chat::White, "targetgroup - selects all bots in target's group");
	c->Message(Chat::White, "namesgroup [name] - selects all bots in name's group");
	c->Message(Chat::White, "healrotation [name] - selects all member and target bots of a heal rotation where name is a member");
	c->Message(Chat::White, "healrotationmembers [name] - selects all member bots of a heal rotation where name is a member");
	c->Message(Chat::White, "healrotationtargets [name] - selects all target bots of a heal rotation where name is a member");
	c->Message(Chat::White, "byclass - selects all bots of the chosen class");
	c->Message(Chat::White, "byrace - selects all bots of the chosen rsce");
	c->Message(Chat::White, "spawned - selects all spawned bots");
	c->Message(Chat::White, "all - selects all spawned bots .. argument use indicates en masse database updating");
	c->Message(Chat::White, "You may only select your bots as actionable");
}
