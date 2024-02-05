#include "../client.h"

void command_repop(Client *c, const Seperator *sep)
{
	const bool is_forced = !strcasecmp(sep->arg[1], "force");

	c->Message(
		Chat::White,
		fmt::format(
			"Zone depopped, {}repopping now.",
			is_forced ? "forcefully " : ""
		).c_str()
	);

	entity_list.ClearAreas();
	zone->Repop(is_forced);
}

