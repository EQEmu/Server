#include "../client.h"

void command_setmana(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #setmana [Mana]");
		return;
	}

	auto mana = static_cast<int>(std::min(std::stoll(sep->arg[1]), (long long) 2000000000));
	bool set_to_max = false;
	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	if (target->IsClient()) {
		if (mana >= target->CastToClient()->CalcMaxMana()) {
			mana = target->CastToClient()->CalcMaxMana();
			set_to_max = true;
		}

		target->CastToClient()->SetMana(mana);
	} else {
		if (mana >= target->CalcMaxMana()) {
			mana = target->CalcMaxMana();
			set_to_max = true;
		}

		target->SetMana(mana);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to {} Mana{}.",
			c->GetTargetDescription(target),
			(
				set_to_max ?
				"full" :
				std::to_string(mana)
			),
			(
				set_to_max ?
				fmt::format(
					" ({})",
					mana
				) :
				""
			)
		).c_str()
	);
}

