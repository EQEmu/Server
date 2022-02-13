#include "../client.h"

void command_delacct(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Format: #delacct accountname");
	}
	else {
		std::string user;
		std::string loginserver;
		ParseAccountString(sep->arg[1], user, loginserver);

		if (database.DeleteAccount(user.c_str(), loginserver.c_str())) {
			c->Message(Chat::White, "The account was deleted.");
		}
		else {
			c->Message(Chat::White, "Unable to delete account.");
		}
	}
}

