#include "../client.h"

void command_mana(Client *c, const Seperator *sep)
{
	auto target = c->GetTarget() ? c->GetTarget() : c;
	int mana = 0;
	if (target->IsClient()) {
		mana = target->CastToClient()->CalcMaxMana();
		target->CastToClient()->SetMana(mana);
	}
	else {
		mana = target->CalcMaxMana();
		target->SetMana(mana);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to full Mana ({}).",
			c->GetTargetDescription(target),
			mana
		).c_str()
	);
}

