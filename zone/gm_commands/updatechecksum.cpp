#include "../client.h"
#include "../worldserver.h"
#include "../../common/repositories/account_repository.h"

extern WorldServer worldserver;

void command_updatechecksum(Client *c, const Seperator *sep)
{
	if (c) {
		// if account found
		auto account = AccountRepository::FindOne(database, c->AccountID());
		if (account.id > 0) {
			database.SetVariable("crc_eqgame", account.crc_eqgame);
			database.SetVariable("crc_skillcaps", account.crc_skillcaps);
			database.SetVariable("crc_basedata", account.crc_basedata);

			// reload rules (world)
			auto pack = new ServerPacket(ServerOP_ReloadRulesWorld, 0);
			worldserver.SendPacket(pack);
			c->Message(Chat::Red, "Successfully sent the packet to world to reload rules. (only world)");
			safe_delete(pack);

			// reload variables (world)
			pack = new ServerPacket(ServerOP_ReloadVariablesWorld, 0);
			worldserver.SendPacket(pack);
			c->Message(Chat::Red, "Successfully sent the packet to world to reload variables. (only world)");
			safe_delete(pack);

			return;
		}

		// we should never see this
		c->Message(Chat::Red, "Error: Your account was not found!");
	}
}
