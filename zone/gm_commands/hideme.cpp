#include "../client.h"

void command_hideme(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #hideme [On|Off]");
		c->Message(Chat::White, "Usage: #hideme [0|1]");
		return;
	}

	auto t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto is_hidden = Strings::ToBool(sep->arg[1]);

	t->SetHideMe(is_hidden);

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} now {} to players below a status level of {}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "are" : "is",
			is_hidden ? "invisible" : "visible",
			t->Admin()
		).c_str()
	);
}
