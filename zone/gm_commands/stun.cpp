#include "../client.h"

void command_stun(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #stun [Duration]");
		return;
	}

	Mob *target  = c;
	int duration = static_cast<int>(std::min(std::stoll(sep->arg[1]), (long long) 2000000000));

	if (duration < 0) {
		duration = 0;
	}

	if (c->GetTarget()) {
		target = c->GetTarget();
		if (target->IsClient()) {
			target->CastToClient()->Stun(duration);
		} else if (target->IsNPC()) {
			target->CastToNPC()->Stun(duration);
		}
	} else {
		c->Stun(duration);
	}

	std::string stun_message = (
		duration ?
			fmt::format(
				"You stunned {} for {}.",
				c->GetTargetDescription(target),
				ConvertMillisecondsToTime(duration)
			) :
			fmt::format(
				"You unstunned {}.",
				c->GetTargetDescription(target)
			)
	);
	c->Message(
		Chat::White,
		stun_message.c_str()
	);
}


