#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void ShowIPLookup(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(
		ServerOP_IPLookup,
		sizeof(ServerGenericWorldQuery_Struct) + strlen(sep->argplus[2]) + 1
	);

	auto s = (ServerGenericWorldQuery_Struct *) pack->pBuffer;
	strn0cpy(s->from, c->GetName(), sizeof(s->from));
	s->admin = c->Admin();

	if (strlen(sep->argplus[2])) {
		strn0cpy(s->query, sep->argplus[2], sizeof(s->query));
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

