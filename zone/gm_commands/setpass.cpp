#include "../client.h"

void command_setpass(Client *c, const Seperator *sep)
{
	if (sep->argnum != 2) {
		c->Message(Chat::White, "Format: #setpass accountname password");
	}
	else {
		std::string user;
		std::string loginserver;
		ParseAccountString(sep->arg[1], user, loginserver);

		int16  tmpstatus = 0;
		uint32 tmpid     = database.GetAccountIDByName(user.c_str(), loginserver.c_str(), &tmpstatus);
		if (!tmpid) {
			c->Message(Chat::White, "Error: Account not found");
		}
		else if (tmpstatus > c->Admin()) {
			c->Message(Chat::White, "Cannot change password: Account's status is higher than yours");
		}
		else if (database.SetLocalPassword(tmpid, sep->arg[2])) {
			c->Message(Chat::White, "Password changed.");
		}
		else {
			c->Message(Chat::White, "Error changing password.");
		}
	}
}

