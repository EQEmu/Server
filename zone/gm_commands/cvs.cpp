#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_cvs(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_ClientVersionSummary, sizeof(ServerRequestClientVersionSummary_Struct));
	auto srcvss = (ServerRequestClientVersionSummary_Struct *) pack->pBuffer;
	strn0cpy(srcvss->Name, c->GetName(), sizeof(srcvss->Name));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

