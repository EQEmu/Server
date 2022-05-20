#include "../client.h"

void command_setendurance(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #setendurance [Endurance]");
		return;
	}

	auto endurance = static_cast<int>(std::min(std::stoll(sep->arg[1]), (long long) 2000000000));
	bool set_to_max = false;
	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}
	
	if (target->IsClient()) {
		if (endurance >= target->CastToClient()->GetMaxEndurance()) {
			endurance = target->CastToClient()->GetMaxEndurance();
			set_to_max = true;
		}

		target->CastToClient()->SetEndurance(endurance);
	} else {
		if (endurance >= target->GetMaxEndurance()) {
			endurance = target->GetMaxEndurance();
			set_to_max = true;
		}

		target->SetEndurance(endurance);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to {} Endurance{}.",
			c->GetTargetDescription(target),
			(
				set_to_max ?
				"full" :
				std::to_string(endurance)
			),
			(
				set_to_max ?
				fmt::format(
					" ({})",
					endurance
				) :
				""
			)
		).c_str()
	);
}