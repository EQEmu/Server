#include "../client.h"

void command_endurance(Client *c, const Seperator *sep)
{
	auto target = c->GetTarget() ? c->GetTarget() : c;
	if (target->IsClient()) {
		target->CastToClient()->SetEndurance(target->CastToClient()->GetMaxEndurance());
	}
	else {
		target->SetEndurance(target->GetMaxEndurance());
	}

	if (c != target) {
		c->Message(
			Chat::White,
			fmt::format(
				"Set {} ({}) to full Endurance.",
				target->GetCleanName(),
				target->GetID()
			).c_str()
		);
	}
	else {
		c->Message(Chat::White, "Restored your Endurance to full.");
	}
}

