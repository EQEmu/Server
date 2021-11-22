#include "../client.h"

void command_zsave(Client *c, const Seperator *sep)
{
	c->Message(
		Chat::Red,
		fmt::format(
			"Zone header {}.",
			(
				zone->SaveZoneCFG() ?
				"saved successfully" :
				"failed to save"
			)
		).c_str()
	);
}
