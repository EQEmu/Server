#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void SetServerLocked(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set server_locked [on|off]");
		return;
	}

	const bool is_locked = Strings::ToBool(sep->arg[1]);

	auto pack = new ServerPacket(ServerOP_Lock, sizeof(ServerLock_Struct));

	auto l = (ServerLock_Struct *) pack->pBuffer;
	strn0cpy(l->character_name, c->GetCleanName(), sizeof(l->character_name));
	l->is_locked = is_locked;

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
