#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void SetLoginserverInfo(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3) {
		c->Message(Chat::White, "Usage: #set lsinfo [Email] [Password]");
		return;
	}

	const std::string& email    = sep->arg[2];
	const std::string& password = sep->arg[2];

	auto pack = new ServerPacket(ServerOP_LSAccountUpdate, sizeof(ServerLSAccountUpdate_Struct));

	auto s = (ServerLSAccountUpdate_Struct *) pack->pBuffer;

	s->useraccountid = c->LSAccountID();
	strn0cpy(s->useraccount, c->AccountName(), 30);
	strn0cpy(s->user_email, email.c_str(), 100);
	strn0cpy(s->userpassword, password.c_str(), 50);

	worldserver.SendPacket(pack);
	safe_delete(pack);

	c->Message(Chat::White, "Your email and local loginserver password have been set.");
}
