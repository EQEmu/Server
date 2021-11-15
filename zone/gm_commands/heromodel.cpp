#include "../client.h"

void command_heromodel(Client *c, const Seperator *sep)
{
	if (sep->argnum < 1) {
		c->Message(Chat::White, "Usage: #heromodel [hero forge model] [ [slot] ] (example: #heromodel 63)");
	}
	else if (c->GetTarget() == nullptr) {
		c->Message(Chat::Red, "You must have a target to do a wear change for Hero's Forge Models.");
	}
	else {
		uint32 hero_forge_model = atoi(sep->arg[1]);

		if (sep->argnum > 1) {
			uint8 wearslot = (uint8) atoi(sep->arg[2]);
			c->GetTarget()->SendTextureWC(wearslot, 0, hero_forge_model, 0, 0, 0);
		}
		else {
			if (hero_forge_model > 0) {
				// Conversion to simplify the command arguments
				// Hero's Forge model is actually model * 1000 + texture * 100 + wearslot
				// Hero's Forge Model slot 7 is actually for Robes, but it still needs to use wearslot 1 in the packet
				hero_forge_model *= 100;

				for (uint8 wearslot = 0; wearslot < 7; wearslot++) {
					c->GetTarget()->SendTextureWC(wearslot, 0, (hero_forge_model + wearslot), 0, 0, 0);
				}
			}
			else {
				c->Message(Chat::Red, "Hero's Forge Model must be greater than 0.");
			}
		}
	}
}

