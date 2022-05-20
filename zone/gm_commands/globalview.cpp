#include "../client.h"

void command_globalview(Client *c, const Seperator *sep)
{
	NPC *npcmob = nullptr;

	if (c->GetTarget() && c->GetTarget()->IsNPC()) {
		npcmob = c->GetTarget()->CastToNPC();
		QGlobalCache *npc_c  = nullptr;
		QGlobalCache *char_c = nullptr;
		QGlobalCache *zone_c = nullptr;

		if (npcmob) {
			npc_c = npcmob->GetQGlobals();
		}

		char_c                   = c->GetQGlobals();
		zone_c                   = zone->GetQGlobals();

		std::list<QGlobal> globalMap;
		uint32             ntype = 0;

		if (npcmob) {
			ntype = npcmob->GetNPCTypeID();
		}

		if (npc_c) {
			QGlobalCache::Combine(globalMap, npc_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		if (char_c) {
			QGlobalCache::Combine(globalMap, char_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		if (zone_c) {
			QGlobalCache::Combine(globalMap, zone_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		auto   iter   = globalMap.begin();
		uint32 gcount = 0;

		c->Message(Chat::White, "Name, Value");
		while (iter != globalMap.end()) {
			c->Message(Chat::White, "%s %s", (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
			++gcount;
		}
		c->Message(Chat::White, "%u globals loaded.", gcount);
	}
	else {
		QGlobalCache *char_c = nullptr;
		QGlobalCache *zone_c = nullptr;

		char_c                   = c->GetQGlobals();
		zone_c                   = zone->GetQGlobals();

		std::list<QGlobal> globalMap;
		uint32             ntype = 0;

		if (char_c) {
			QGlobalCache::Combine(globalMap, char_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		if (zone_c) {
			QGlobalCache::Combine(globalMap, zone_c->GetBucket(), ntype, c->CharacterID(), zone->GetZoneID());
		}

		auto   iter   = globalMap.begin();
		uint32 gcount = 0;

		c->Message(Chat::White, "Name, Value");
		while (iter != globalMap.end()) {
			c->Message(Chat::White, "%s %s", (*iter).name.c_str(), (*iter).value.c_str());
			++iter;
			++gcount;
		}
		c->Message(Chat::White, "%u globals loaded.", gcount);
	}
}

