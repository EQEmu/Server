#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_updatechecksum(Client* c, const Seperator* sep) {
	if (c)
	{
		database.SetVariable("checksum_crc1_eqgame", std::to_string(database.GetAccountCRC1EQGame(c->AccountID())));
		database.SetVariable("checksum_crc2_skillcaps", std::to_string(database.GetAccountCRC2SkillCaps(c->AccountID())));
		database.SetVariable("checksum_crc3_basedata", std::to_string(database.GetAccountCRC3BaseData(c->AccountID())));

		auto pack = new ServerPacket(ServerOP_ReloadRulesWorld, 0);
		worldserver.SendPacket(pack);
		c->Message(Chat::Red, "Successfully sent the packet to world to reload rules. (only world)");
		safe_delete(pack);
	}
}
