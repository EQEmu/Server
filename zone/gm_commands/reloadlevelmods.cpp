#include "../client.h"

void command_reloadlevelmods(Client *c, const Seperator *sep)
{
	if (!RuleB(Zone, LevelBasedEXPMods)) {
		c->Message(Chat::White, "Level based experience modifiers are disabled.");
		return;
	}
		
	c->Message(Chat::White, "Attempted to reload level based experience modifiers globally.");
	auto pack = new ServerPacket(ServerOP_ReloadLevelEXPMods, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

