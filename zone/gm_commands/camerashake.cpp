#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_camerashake(Client *c, const Seperator *sep)
{
	if (c) {
		if (sep->arg[1][0] && sep->arg[2][0]) {
			auto                     pack  = new ServerPacket(ServerOP_CameraShake, sizeof(ServerCameraShake_Struct));
			ServerCameraShake_Struct *scss = (ServerCameraShake_Struct *) pack->pBuffer;
			scss->duration  = atoi(sep->arg[1]);
			scss->intensity = atoi(sep->arg[2]);
			worldserver.SendPacket(pack);
			c->Message(Chat::Red, "Successfully sent the packet to world! Shake it, world, shake it!");
			safe_delete(pack);
		}
		else {
			c->Message(Chat::Red, "Usage -- #camerashake [duration], [intensity [1-10])");
		}
	}
	return;
}

