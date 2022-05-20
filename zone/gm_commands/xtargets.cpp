#include "../client.h"

void command_xtargets(Client *c, const Seperator *sep)
{
	Client *t;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}
	else {
		t = c;
	}

	if (sep->arg[1][0]) {
		uint8 NewMax = atoi(sep->arg[1]);

		if ((NewMax < 5) || (NewMax > XTARGET_HARDCAP)) {
			c->Message(Chat::Red, "Number of XTargets must be between 5 and %i", XTARGET_HARDCAP);
			return;
		}
		t->SetMaxXTargets(NewMax);
		c->Message(Chat::White, "Max number of XTargets set to %i", NewMax);
	}
	else {
		t->ShowXTargets(c);
	}
}

