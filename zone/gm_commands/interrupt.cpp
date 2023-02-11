#include "../client.h"

void command_interrupt(Client *c, const Seperator *sep)
{
	uint16 ci_message = 0x01b7, ci_color = 0x0121;

	if (sep->arg[1][0]) {
		ci_message = Strings::ToInt(sep->arg[1]);
	}
	if (sep->arg[2][0]) {
		ci_color = Strings::ToInt(sep->arg[2]);
	}

	c->InterruptSpell(ci_message, ci_color);
}

