#include "../client.h"

void command_gmspeed(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #gmspeed [On|Off]");
		return;
	}

	bool gm_speed_flag = atobool(sep->arg[1]);
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	database.SetGMSpeed(
		target->AccountID(),
		gm_speed_flag ? 1 : 0
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Turning GM Speed {} for {}.",
			gm_speed_flag ? "on" : "off",
			c->GetTargetDescription(target)
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Note: {} must zone for it to take effect.",
			c->GetTargetDescription(target, TargetDescriptionType::UCYou)
		).c_str()
	);
}

