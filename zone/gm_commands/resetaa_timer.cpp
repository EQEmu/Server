#include "../client.h"

void command_resetaa_timer(Client *c, const Seperator *sep)
{
	Client *target = nullptr;
	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		target = c;
	}
	else {
		target = c->GetTarget()->CastToClient();
	}

	if (sep->IsNumber(1)) {
		int timer_id = atoi(sep->arg[1]);
		c->Message(Chat::White, "Reset of timer %i for %s", timer_id, c->GetName());
		c->ResetAlternateAdvancementTimer(timer_id);
	}
	else if (!strcasecmp(sep->arg[1], "all")) {
		c->Message(Chat::White, "Reset all timers for %s", c->GetName());
		c->ResetAlternateAdvancementTimers();
	}
	else {
		c->Message(Chat::White, "usage: #resetaa_timer [all | timer_id]");
	}
}

