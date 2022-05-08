#include "../client.h"

void command_pvp(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #pvp [On|Off]");
		return;
	}

	bool pvp_state = atobool(sep->arg[1]);
	Client* target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	target->SetPVP(pvp_state);
	if (c != target) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} now follow{} the ways of {}.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c != target ? "s" : "",
				pvp_state ? "Discord" : "Order"
			).c_str()
		);
	}
}
