#include "../client.h"

void command_setanim(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && sep->IsNumber(1)) {
		int num = atoi(sep->arg[1]);
		if (num < 0 || num >= _eaMaxAppearance) {
			c->Message(Chat::White, "Invalid animation number, between 0 and %d", _eaMaxAppearance - 1);
		}
		c->GetTarget()->SetAppearance(EmuAppearance(num));
	}
	else {
		c->Message(Chat::White, "Usage: #setanim [animnum]");
	}
}

