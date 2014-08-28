#ifndef BOT_STRUCTS
#define BOT_STRUCTS

#ifdef BOTS

#include "../common/types.h"

#include <sstream>

struct BotsAvailableList {
	uint32 BotID;
	char BotName[64];
	uint16 BotClass;
	uint8 BotLevel;
	uint16 BotRace;
};

struct BotGroup {
	uint32 BotGroupID;
	uint32 BotID;
};

struct BotGroupList {
	std::string BotGroupName;
	std::string BotGroupLeaderName;
};

struct SpawnedBotsList {
	char BotName[64];
	char ZoneName[64];
	uint32 BotLeaderCharID;
};

struct BotSpell {
	uint16 SpellId;
	int SpellIndex;
	int16 ManaCost;
};

struct BotAA {
	uint32 aa_id;
	uint8 req_level;
	uint8 total_levels;
};

#endif // BOTS

#endif // BOT_STRUCTS
