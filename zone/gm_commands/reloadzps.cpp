#include "../client.h"

void command_reloadzps(Client *c, const Seperator *sep)
{
	content_db.LoadStaticZonePoints(&zone->zone_point_list, zone->GetShortName(), zone->GetInstanceVersion());
	c->Message(Chat::White, "Reloading server zone_points.");
}

