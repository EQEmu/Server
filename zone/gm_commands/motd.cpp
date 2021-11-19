#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_motd(Client *c, const Seperator *sep)
{
	auto              outpack = new ServerPacket(ServerOP_Motd, sizeof(ServerMotd_Struct));
	ServerMotd_Struct *mss    = (ServerMotd_Struct *) outpack->pBuffer;
	strn0cpy(mss->myname, c->GetName(), 64);
	strn0cpy(mss->motd, sep->argplus[1], 512);
	worldserver.SendPacket(outpack);
	safe_delete(outpack);
}

