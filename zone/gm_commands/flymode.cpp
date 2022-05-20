#include "../client.h"

void command_flymode(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		return;
	}

	Mob *target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	auto flymode_id = std::stoul(sep->arg[1]);
	if (
		flymode_id < EQ::constants::GravityBehavior::Ground &&
		flymode_id > EQ::constants::GravityBehavior::LevitateWhileRunning
	) {		
		c->Message(Chat::White, "Usage:: #flymode [Flymode ID]");
		c->Message(Chat::White, "0 = Ground, 1 = Flying, 2 = Levitating, 3 = Water, 4 = Floating, 5 = Levitating While Running");
		return;
	}

	target->SetFlyMode(static_cast<GravityBehavior>(flymode_id));
	target->SendAppearancePacket(AT_Levitate, flymode_id);
	c->Message(
		Chat::White,
		fmt::format(
			"Fly Mode for {} is now {} ({}).",
			c->GetTargetDescription(target),
			EQ::constants::GetFlyModeName(flymode_id),
			flymode_id
		).c_str()
	);
}


