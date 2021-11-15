#include "../client.h"

void command_ipban(Client *c, const Seperator *sep)
{
	if (sep->arg[1] == 0) {
		c->Message(Chat::White, "Usage: #ipban [xxx.xxx.xxx.xxx]");
	}
	else {
		if (database.AddBannedIP(sep->arg[1], c->GetName())) {
			c->Message(
				Chat::White,
				"%s has been successfully added to the banned_ips table by %s",
				sep->arg[1],
				c->GetName());
		}
		else {
			c->Message(Chat::White, "IPBan Failed (IP address is possibly already in the table?)");
		}
	}
}

