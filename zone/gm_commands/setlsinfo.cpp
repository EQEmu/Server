#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_setlsinfo(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #setlsinfo [Email] [Password]");
		return;
	}

	auto pack = new ServerPacket(ServerOP_LSAccountUpdate, sizeof(ServerLSAccountUpdate_Struct));
	auto s = (ServerLSAccountUpdate_Struct *) pack->pBuffer;
	s->useraccountid = c->LSAccountID();
	strn0cpy(s->useraccount, c->AccountName(), 30);
	strn0cpy(s->user_email, sep->arg[1], 100);
	strn0cpy(s->userpassword, sep->arg[2], 50);
	worldserver.SendPacket(pack);
	c->Message(Chat::White, "Your email and local loginserver password have been set.");
}

