#include "../client.h"

void command_setanim(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #setanim [Animation ID (IDs are 0 to 4)]");
		return;
	}

	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}


	int animation_id = std::stoi(sep->arg[1]);
	if (
		animation_id < 0 ||
		animation_id > eaLooting
	) {
		c->Message(Chat::White, "Usage: #setanim [Animation ID (IDs are 0 to 4)]");
		return;
	}

	target->SetAppearance(static_cast<EmuAppearance>(animation_id), false);
	std::string animation_name;
	if (animation_id == eaStanding) {
		animation_name = "Standing";
	} else if (animation_id == eaSitting) {
		animation_name = "Sitting";
	} else if (animation_id == eaCrouching) {
		animation_name = "Crouching";
	} else if (animation_id == eaDead) {
		animation_name = "Dead";
	} else if (animation_id == eaLooting) {
		animation_name = "Looting";
	}
	
	c->Message(
		Chat::White,
		fmt::format(
			"Set animation to {} ({}) for {}.",
			animation_name,
			animation_id,
			c->GetTargetDescription(target)
		).c_str()
	);
}

