#include "../client.h"
#include "../guild_mgr.h"

void command_guildlist(Client *c, const Seperator *sep)
{
	GuildApproval *tmp = guild_mgr.FindGuildByIDApproval(atoi(sep->arg[1]));
	if (tmp) {
		tmp->ApprovedMembers(c);
	}
	else {
		c->Message(Chat::White, "Could not find reference id.");
	}
}

