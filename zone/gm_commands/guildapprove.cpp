#include "../client.h"
#include "../guild_mgr.h"

void command_guildapprove(Client *c, const Seperator *sep)
{
	guild_mgr.AddMemberApproval(atoi(sep->arg[1]), c);
}

