#include "../client.h"
#include "../../common/file_util.h"

void command_reloadaa(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Reloading Alternate Advancement Data...");
	zone->LoadAlternateAdvancement();
	c->Message(Chat::White, "Alternate Advancement Data Reloaded");
	entity_list.SendAlternateAdvancementStats();
}

inline bool file_exists(const std::string &name)
{
	std::ifstream f(name.c_str());
	return f.good();
}

