#include "../client.h"

void command_resetaa_timer(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #resetaa_timer all - Reset all Alternate Advancement timers");
		c->Message(Chat::White, "Usage: #resetaa_timer [Timer ID] - Reset Alternate Advancement timer by ID");
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	bool is_all = !strcasecmp(sep->arg[1], "all");

	if (is_all) {
		c->Message(
			Chat::White,
			fmt::format(
				"Reset all Alternate Advancement timers for {}.",
				c->GetTargetDescription(target)
			).c_str()
		);
		target->ResetAlternateAdvancementTimers();
		return;
	}

	if (sep->IsNumber(1)) {
		int timer_id = std::stoi(sep->arg[1]);
		c->Message(
			Chat::White,
			fmt::format(
				"Reset Alternate Advancement timer {} for {}.",
				timer_id,
				c->GetTargetDescription(target)
			).c_str()
		);
		target->ResetAlternateAdvancementTimer(timer_id);
	}
}

