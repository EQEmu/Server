#include "../client.h"
#include "../guild_mgr.h"

void command_guildcreate(Client *c, const Seperator *sep)
{
	if (strlen(sep->argplus[1]) > 4 && strlen(sep->argplus[1]) < 16) {
		guild_mgr.AddGuildApproval(sep->argplus[1], c);
	}
	else {
		c->Message(Chat::White, "Guild name must be more than 4 characters and less than 16.");
	}
}

