#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_setlsinfo(Client *c, const Seperator *sep)
{
	if (sep->argnum != 2) {
		c->Message(Chat::White, "Format: #setlsinfo email password");
	}
	else {
		auto                         pack = new ServerPacket(
			ServerOP_LSAccountUpdate,
			sizeof(ServerLSAccountUpdate_Struct));
		ServerLSAccountUpdate_Struct *s   = (ServerLSAccountUpdate_Struct *) pack->pBuffer;
		s->useraccountid = c->LSAccountID();
		strn0cpy(s->useraccount, c->AccountName(), 30);
		strn0cpy(s->user_email, sep->arg[1], 100);
		strn0cpy(s->userpassword, sep->arg[2], 50);
		worldserver.SendPacket(pack);
		c->Message(Chat::White, "Login Server update packet sent.");
	}
}

