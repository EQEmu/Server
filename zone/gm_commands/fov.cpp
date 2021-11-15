#include "../client.h"

void command_fov(Client *c, const Seperator *sep)
{
	if (c->GetTarget()) {
		auto        target         = c->GetTarget();
		std::string behind_message = (
			c->BehindMob(
				target,
				c->GetX(),
				c->GetY()
			) ?
				"behind" :
				"not behind"
		);
		std::string gender_message = (
			target->GetGender() == MALE ?
				"he" :
				(
					target->GetGender() == FEMALE ?
						"she" :
						"it"
				)
		);

		c->Message(
			Chat::White,
			fmt::format(
				"You are {} {} ({}), {} has a heading of {}.",
				behind_message,
				target->GetCleanName(),
				target->GetID(),
				gender_message,
				target->GetHeading()
			).c_str()
		);
	}
	else {
		c->Message(Chat::White, "You must have a target to use this command.");
	}
}

