#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_iplookup(Client *c, const Seperator *sep)
{
	auto                           pack =
									   new ServerPacket(
										   ServerOP_IPLookup,
										   sizeof(ServerGenericWorldQuery_Struct) +
										   strlen(sep->argplus[1]) + 1
									   );
	ServerGenericWorldQuery_Struct *s   = (ServerGenericWorldQuery_Struct *) pack->pBuffer;
	strcpy(s->from, c->GetName());
	s->admin = c->Admin();
	if (sep->argplus[1][0] != 0) {
		strcpy(s->query, sep->argplus[1]);
	}
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

