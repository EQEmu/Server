#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_serverlock(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #serverlock [0|1] - Lock or Unlock the World Server (0 = Unlocked, 1 = Locked)");
		return;
	}
		
	auto is_locked = std::stoi(sep->arg[1]) ? true : false;

	auto pack = new ServerPacket(ServerOP_Lock, sizeof(ServerLock_Struct));
	auto l = (ServerLock_Struct *) pack->pBuffer;
	strn0cpy(l->character_name, c->GetCleanName(), sizeof(l->character_name));
	l->is_locked = is_locked;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

