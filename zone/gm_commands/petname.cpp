#include "../client.h"

void command_petname(Client *c, const Seperator *sep)
{
	Mob *target;
	target = c->GetTarget();

	if (!target || !target->IsPet()) {
		c->Message(Chat::White, "Usage: #petname newname (requires a pet target)");
	}
	else if (target->GetOwnerID() == c->GetID() && strlen(sep->arg[1]) > 0) {
		char *oldname = strdup(target->GetName());
		target->TempName(sep->arg[1]);
		c->Message(Chat::White, "Renamed %s to %s", oldname, sep->arg[1]);
		free(oldname);
	}
	else {
		target->TempName();
		c->Message(Chat::White, "Restored the original name");
	}
}

