#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_oocmute(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #oocmute [0|1] - Enable or Disable Server OOC");
		return;
	}

	bool is_muted = std::stoi(sep->arg[1]) ? true : false;

	ServerPacket pack(ServerOP_OOCMute, sizeof(ServerOOCMute_Struct));
	auto o = (ServerOOCMute_Struct*) pack.pBuffer;
	o->is_muted = is_muted;
	worldserver.SendPacket(&pack);

	c->Message(
		Chat::White,
		fmt::format(
			"Server OOC is {} muted.",
			is_muted ? "now" : "no longer"
		).c_str()
	);
}

