#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_flag(Client *c, const Seperator *sep)
{
	if (sep->arg[2][0] == 0) {
		if (!c->GetTarget() || (c->GetTarget() && c->GetTarget() == c)) {
			c->UpdateAdmin();
			c->Message(Chat::White, "Refreshed your admin flag from DB.");
		}
		else if (c->GetTarget() && c->GetTarget() != c && c->GetTarget()->IsClient()) {
			c->GetTarget()->CastToClient()->UpdateAdmin();
			c->Message(Chat::White, "%s's admin flag has been refreshed.", c->GetTarget()->GetName());
			c->GetTarget()->Message(Chat::White, "%s refreshed your admin flag.", c->GetName());
		}
	}
	else if (!sep->IsNumber(1) || atoi(sep->arg[1]) < -2 || atoi(sep->arg[1]) > 255 || strlen(sep->arg[2]) == 0) {
		c->Message(Chat::White, "Usage: #flag [status] [acctname]");
	}

	else if (c->Admin() < commandChangeFlags) {
		//this check makes banning players by less than this level
		//impossible, but i'll leave it in anyways
		c->Message(Chat::White, "You may only refresh your own flag, doing so now.");
		c->UpdateAdmin();
	}
	else {
		if (atoi(sep->arg[1]) > c->Admin()) {
			c->Message(Chat::White, "You cannot set people's status to higher than your own");
		}
		else if (atoi(sep->arg[1]) < 0 && c->Admin() < commandBanPlayers) {
			c->Message(Chat::White, "You have too low of status to suspend/ban");
		}
		else if (!database.SetAccountStatus(sep->argplus[2], atoi(sep->arg[1]))) {
			c->Message(Chat::White, "Unable to set GM Flag.");
		}
		else {
			c->Message(Chat::White, "Set GM Flag on account.");

			std::string user;
			std::string loginserver;
			ParseAccountString(sep->argplus[2], user, loginserver);

			ServerPacket pack(ServerOP_FlagUpdate, 6);
			*((uint32 *) pack.pBuffer)    = database.GetAccountIDByName(user.c_str(), loginserver.c_str());
			*((int16 *) &pack.pBuffer[4]) = atoi(sep->arg[1]);
			worldserver.SendPacket(&pack);
		}
	}
}

