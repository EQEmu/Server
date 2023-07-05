#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void ShowIPLookup(Client *c, const Seperator *sep)
{
	const uint32 ip_length = sep->argnum == 2 ? strlen(sep->argplus[2]) : 0;

	auto pack = new ServerPacket(
		ServerOP_IPLookup,
		sizeof(ServerGenericWorldQuery_Struct) + ip_length + 1
	);

	auto s = (ServerGenericWorldQuery_Struct *) pack->pBuffer;
	strn0cpy(s->from, c->GetName(), sizeof(s->from));
	s->admin = c->Admin();

	if (ip_length) {
		strn0cpy(s->query, sep->argplus[2], sizeof(s->query));
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

