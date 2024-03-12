#include "../../client.h"

void SetHaste(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set haste [Percentage] - Set GM Bonus Haste (100 is 100% more Attack Speed)");
		return;
	}

	auto t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const int extra_haste = Strings::ToInt(sep->arg[2]);

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
