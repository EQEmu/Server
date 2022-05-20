#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_flag(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		auto target = c->GetTarget() && c->GetTarget()->IsClient() ? c->GetTarget()->CastToClient() : c;
		if (target != c) {
			c->Message(
				Chat::White,
				fmt::format(
					"Status level has been refreshed for {}.",
					target->GetCleanName()
				).c_str()
			);

			target->Message(
				Chat::White,
				fmt::format(
					"Your status level has been refreshed by {}.",
					c->GetCleanName()
				).c_str()
			);
		} else {
			c->Message(Chat::White, "Your status level has been refreshed.");
		}
		target->UpdateAdmin();
		return;
	}
	
	
	if (
		!sep->IsNumber(1) ||
		strlen(sep->arg[2]) == 0
	) {
		c->Message(Chat::White, "Usage: #flag [Status] [Account Name]");
		return;
	}

	auto status = std::stoi(sep->arg[1]);
	if (status < -2 || status > 255) {
		c->Message(Chat::White, "The lowest a status level can go is -2 and the highest a status level can go is 255.");
		return;
	}

	std::string account_name = sep->argplus[2];
	auto account_id = database.GetAccountIDByChar(account_name.c_str());
	
	if (c->Admin() < commandChangeFlags) { //this check makes banning players by less than this level impossible, but i'll leave it in anyways
		c->Message(Chat::White, "You may only refresh your own flag, doing so now.");
		c->UpdateAdmin();
	} else {
		if (status > c->Admin()) {
			c->Message(
				Chat::White,
				fmt::format(
					"You cannot set someone's status level to {} because your status level is only {}.",
					status,
					c->Admin()
				).c_str()
			);
		} else if (status < 0 && c->Admin() < commandBanPlayers) {
			c->Message(Chat::White, "Your status level is not high enough to ban or suspend.");
		} 	else if (!database.SetAccountStatus(account_name, status)) {
			c->Message(Chat::White, "Failed to set status level.");
		}
		else {
			c->Message(Chat::White, "Set GM Flag on account.");

			std::string user;
			std::string loginserver;
			ParseAccountString(account_name, user, loginserver);

			account_id = database.GetAccountIDByName(account_name, loginserver);

			ServerPacket pack(ServerOP_FlagUpdate, sizeof(ServerFlagUpdate_Struct));
			ServerFlagUpdate_Struct *sfus = (ServerFlagUpdate_Struct *) pack.pBuffer;
			sfus->account_id = account_id;
			sfus->admin = status;
			worldserver.SendPacket(&pack);
		}
	}
}

