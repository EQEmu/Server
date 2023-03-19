#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_motd(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_Motd, sizeof(ServerMotd_Struct));
	auto m = (ServerMotd_Struct *) pack->pBuffer;
	strn0cpy(m->myname, c->GetName(), sizeof(m->myname));
	strn0cpy(m->motd, sep->argplus[1], sizeof(m->motd));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

