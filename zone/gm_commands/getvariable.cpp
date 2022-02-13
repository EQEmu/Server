#include "../client.h"

void command_getvariable(Client *c, const Seperator *sep)
{
	std::string tmp;
	if (database.GetVariable(sep->argplus[1], tmp)) {
		c->Message(Chat::White, "%s = %s", sep->argplus[1], tmp.c_str());
	}
	else {
		c->Message(Chat::White, "GetVariable(%s) returned false", sep->argplus[1]);
	}
}

