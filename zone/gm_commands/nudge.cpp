#include "../client.h"

void command_nudge(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #nudge [x=f] [y=f] [z=f] [h=f] (partial/mixed arguments allowed)");
	}
	else {

		auto target     = c->GetTarget();
		if (!target) {

			c->Message(Chat::Yellow, "This command requires a target.");
			return;
		}
		if (target->IsMoving()) {

			c->Message(Chat::Yellow, "This command requires a stationary target.");
			return;
		}

		glm::vec4 position_offset(0.0f, 0.0f, 0.0f, 0.0f);
		for (auto index = 1; index <= 4; ++index) {

			if (!sep->arg[index]) {
				continue;
			}

			Seperator argsep(sep->arg[index], '=');
			if (!argsep.arg[1][0]) {
				continue;
			}

			switch (argsep.arg[0][0]) {
				case 'x':
					position_offset.x = atof(argsep.arg[1]);
					break;
				case 'y':
					position_offset.y = atof(argsep.arg[1]);
					break;
				case 'z':
					position_offset.z = atof(argsep.arg[1]);
					break;
				case 'h':
					position_offset.w = atof(argsep.arg[1]);
					break;
				default:
					break;
			}
		}

		const auto &current_position = target->GetPosition();
		glm::vec4  new_position(
			(current_position.x + position_offset.x),
			(current_position.y + position_offset.y),
			(current_position.z + position_offset.z),
			(current_position.w + position_offset.w)
		);

		target->GMMove(new_position.x, new_position.y, new_position.z, new_position.w);

		c->Message(
			Chat::White,
			"Nudging '%s' to {%1.3f, %1.3f, %1.3f, %1.2f} (adjustment: {%1.3f, %1.3f, %1.3f, %1.2f})",
			target->GetName(),
			new_position.x,
			new_position.y,
			new_position.z,
			new_position.w,
			position_offset.x,
			position_offset.y,
			position_offset.z,
			position_offset.w
		);
	}
}

