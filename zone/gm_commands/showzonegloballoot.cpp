#include "../client.h"

void command_showzonegloballoot(Client *c, const Seperator *sep)
{
	c->Message(
		Chat::White,
		"GlobalLoot for %s (%d:%d)",
		zone->GetShortName(),
		zone->GetZoneID(),
		zone->GetInstanceVersion());
	zone->ShowZoneGlobalLoot(c);
}

