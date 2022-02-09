#include "../client.h"

void command_untraindisc(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	for (int i = 0; i < MAX_PP_DISCIPLINES; i++) {
		if (t->GetPP().disciplines.values[i] == atoi(sep->arg[1])) {
			t->UntrainDisc(i, 1);
			return;
		}
	}
}

