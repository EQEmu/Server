#include "../../client.h"

void command_setanim(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #setanim [Animation ID]");

		uint32 animation_number = 1;
		for (const auto& a : EQ::constants::GetSpawnAnimationMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Animation {} | ID: {} Name: {}",
					animation_number,
					a.first,
					a.second
				).c_str()
			);

			animation_number++;
		}
		return;
	}

	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	const auto animation_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1]));
	if (
		!EQ::ValueWithin(
			animation_id,
			static_cast<uint8>(eaStanding),
			static_cast<uint8>(eaLooting)
		)
	) {
		c->Message(Chat::White, "Usage: #setanim [Animation ID]");

		uint32 animation_number = 1;
		for (const auto& a : EQ::constants::GetSpawnAnimationMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Animation {} | ID: {} Name: {}",
					animation_number,
					a.first,
					a.second
				).c_str()
			);

			animation_number++;
		}
		return;
	}

	target->SetAppearance(static_cast<EmuAppearance>(animation_id), false);

	const auto animation_name = EQ::constants::GetSpawnAnimationName(animation_id);

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

