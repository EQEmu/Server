#include "../client.h"
#include "../quest_parser_collection.h"

void command_reloadqst(Client *c, const Seperator *sep)
{
	bool stop_timers = false;

	if (sep->IsNumber(1)) {
		stop_timers = std::stoi(sep->arg[1]) != 0 ? true : false;
	}

	std::string stop_timers_message = stop_timers ? " and stopping timers" : "";
	c->Message(
		Chat::White,
		fmt::format(
			"Clearing quest memory cache{}.",
			stop_timers_message
		).c_str()
	);
	entity_list.ClearAreas();
	parse->ReloadQuests(stop_timers);
}

