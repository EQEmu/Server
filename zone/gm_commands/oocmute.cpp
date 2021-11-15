#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_oocmute(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || !(sep->arg[1][0] == '1' || sep->arg[1][0] == '0')) {
		c->Message(Chat::White, "Usage: #oocmute [1/0]");
	}
	else {
		auto outapp = new ServerPacket(ServerOP_OOCMute, 1);
		*(outapp->pBuffer) = atoi(sep->arg[1]);
		worldserver.SendPacket(outapp);
		safe_delete(outapp);
	}
}

