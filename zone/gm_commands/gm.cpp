#include "../client.h"

void command_gm(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #gm [On|Off]");
		return;
	}

	bool gm_flag = atobool(sep->arg[1]);
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	target->SetGM(gm_flag);
	if (c != target) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} is {} flagged as a GM.",
				c->GetTargetDescription(target),
				gm_flag ? "now" : "no longer"
			).c_str()
		);
	}
}
