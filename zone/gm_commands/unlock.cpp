#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_unlock(Client *c, const Seperator *sep)
{
	auto              outpack = new ServerPacket(ServerOP_Lock, sizeof(ServerLock_Struct));
	ServerLock_Struct *lss    = (ServerLock_Struct *) outpack->pBuffer;
	strcpy(lss->myname, c->GetName());
	lss->mode = 0;
	worldserver.SendPacket(outpack);
	safe_delete(outpack);
}

