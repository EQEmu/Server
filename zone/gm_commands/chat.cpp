#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_chat(Client *c, const Seperator *sep)
{
	if (sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #chat [channum] [message]");
	}
	else if (!worldserver.SendChannelMessage(0, 0, (uint8) atoi(sep->arg[1]), 0, 0, 100, sep->argplus[2])) {
		c->Message(Chat::White, "Error: World server disconnected");
	}
}

