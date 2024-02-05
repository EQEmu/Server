#include "../client.h"

void command_repop(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Zone depopped, repopping now.");
		zone->Repop();
		return;
	}

	const bool is_forced = !strcasecmp(sep->arg[1], "force");

	c->Message(
		Chat::White,
		fmt::format(
			"Zone depopped, {}repopping now.",
			is_forced ? "forcefully " : ""
		).c_str()
	);

	zone->Repop(is_forced);
}

