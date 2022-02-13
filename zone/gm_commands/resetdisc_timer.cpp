#include "../client.h"

void command_resetdisc_timer(Client *c, const Seperator *sep)
{
	Client *target = c->GetTarget()->CastToClient();
	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		target = c;
	}

	if (sep->IsNumber(1)) {
		int timer_id = atoi(sep->arg[1]);
		c->Message(Chat::White, "Reset of disc timer %i for %s", timer_id, c->GetName());
		c->ResetDisciplineTimer(timer_id);
	}
	else if (!strcasecmp(sep->arg[1], "all")) {
		c->Message(Chat::White, "Reset all disc timers for %s", c->GetName());
		c->ResetAllDisciplineTimers();
	}
	else {
		c->Message(Chat::White, "usage: #resetdisc_timer [all | timer_id]");
	}
}

