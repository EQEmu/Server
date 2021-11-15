#include "../client.h"

void command_version(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Current version information.");
	c->Message(Chat::White, "	%s", CURRENT_VERSION);
	c->Message(Chat::White, "	Compiled on: %s at %s", COMPILE_DATE, COMPILE_TIME);
	c->Message(Chat::White, "	Last modified on: %s", LAST_MODIFIED);
}

