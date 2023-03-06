#include "../client.h"

void command_haste(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #haste [Percentage] - Set GM Bonus Haste (100 is 100% more Attack Speed)");
		return;
	}

	auto t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto extra_haste = Strings::ToInt(sep->arg[1]);

	t->SetExtraHaste(extra_haste);
	t->CalcBonuses();
	t->SetAttackTimer();

	c->Message(
		Chat::White,
		fmt::format(
			"GM Haste Bonus set to {}%% for {}.",
			Strings::Commify(extra_haste),
			c->GetTargetDescription(t)
		).c_str()
	);
}

