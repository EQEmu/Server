#include "../client.h"
#include "../worldserver.h"
#include "../../common/repositories/account_repository.h"

extern WorldServer worldserver;

void command_updatechecksum(Client *c, const Seperator *sep)
{
	auto account = AccountRepository::FindOne(database, c->AccountID());
	if (!account.id) {
		c->Message(Chat::White, "Your account was not found!");
		return;
	}

	database.SetVariable("crc_eqgame", account.crc_eqgame);
	database.SetVariable("crc_skillcaps", account.crc_skillcaps);
	database.SetVariable("crc_basedata", account.crc_basedata);

	c->Message(Chat::White, "Attempting to reload Rules globally.");
	auto pack = new ServerPacket(ServerOP_ReloadRules, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);

	c->Message(Chat::White, "Attempting to reload Variables globally.");
	pack = new ServerPacket(ServerOP_ReloadVariables, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}
