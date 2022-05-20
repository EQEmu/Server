#include "../client.h"

void command_sethp(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #sethp [Health]");
		return;
	}

	auto health = static_cast<int>(std::min(std::stoll(sep->arg[1]), (long long) 2000000000));
	bool set_to_max = false;
	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	if (health >= target->GetMaxHP()) {
		health = target->GetMaxHP();
		set_to_max = true;
	}

	target->SetHP(health);
	target->SendHPUpdate();

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to {} Health{}.",
			c->GetTargetDescription(target),
			(
				set_to_max ?
				"full" :
				std::to_string(health)
			),
			(
				set_to_max ?
				fmt::format(
					" ({})",
					health
				) :
				""
			)
		).c_str()
	);
}

