#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_chat(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #chat [Channel ID] [Message]");
		return;
	}

	auto channel_id = static_cast<uint8>(std::stoul(sep->arg[1]));
	std::string message = sep->argplus[2];
	if (!worldserver.SendChannelMessage(0, 0, channel_id, 0, 0, 100, message.c_str())) {
		c->Message(Chat::White, "World server is disconnected.");
	}
}

