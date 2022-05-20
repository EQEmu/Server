#include "../client.h"

void command_setpass(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #setpass [Account Name] [Password]");
		return;
	}

	std::string account_name;
	std::string loginserver;
	ParseAccountString(sep->arg[1], account_name, loginserver);
	int16 status = 0;
	auto account_id = database.GetAccountIDByName(account_name, loginserver, &status);
	if (!account_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Account {} not found.",
				account_name
			).c_str()
		);
		return;
	}

	if (status > c->Admin()) {
		c->Message(
			Chat::White,
			fmt::format(
				"You cannot change the password for Account {} as its status is higher than yours.",
				account_name
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Password {} changed for Account {}.",
			(
				database.SetLocalPassword(account_id, sep->arg[2]) ?
				"successfully" :
				"failed"
			),
			account_name
		).c_str()
	);
}

