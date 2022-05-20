#include "../client.h"

void command_invul(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #invul [On|Off]");
		return;
	}

	bool invul_flag = atobool(sep->arg[1]);
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	target->SetInvul(invul_flag);
	c->Message(
		Chat::White,
		fmt::format(
			"{} {} now {}.",
			c->GetTargetDescription(target, TargetDescriptionType::UCYou),
			c == target ? "are" : "is",
			invul_flag ? "invulnerable" : "vulnerable"
		).c_str()
	);
}

