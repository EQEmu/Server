#include "../client.h"

void command_mana(Client *c, const Seperator *sep)
{
	auto target = c->GetTarget() ? c->GetTarget() : c;
	if (target->IsClient()) {
		target->CastToClient()->SetMana(target->CastToClient()->CalcMaxMana());
	}
	else {
		target->SetMana(target->CalcMaxMana());
	}

	if (c != target) {
		c->Message(
			Chat::White,
			fmt::format(
				"Set {} ({}) to full Mana.",
				target->GetCleanName(),
				target->GetID()
			).c_str()
		);
	}
	else {
		c->Message(Chat::White, "Restored your Mana to full.");
	}
}

