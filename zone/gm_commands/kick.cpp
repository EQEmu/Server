#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_kick(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #kick [charname]");
	}
	else {
		Client *client = entity_list.GetClientByName(sep->arg[1]);
		if (client != 0) {
			if (client->Admin() <= c->Admin()) {
				client->Message(Chat::White, "You have been kicked by %s", c->GetName());
				auto outapp = new EQApplicationPacket(OP_GMKick, 0);
				client->QueuePacket(outapp);
				client->Kick("Ordered kicked by command");
				c->Message(Chat::White, "Kick: local: kicking %s", sep->arg[1]);
			}
		}
		else if (!worldserver.Connected()) {
			c->Message(Chat::White, "Error: World server disconnected");
		}
		else {
			auto                    pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
			ServerKickPlayer_Struct *skp = (ServerKickPlayer_Struct *) pack->pBuffer;
			strcpy(skp->adminname, c->GetName());
			strcpy(skp->name, sep->arg[1]);
			skp->adminrank = c->Admin();
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
	}
}

