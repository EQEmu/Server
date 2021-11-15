#include "../client.h"

void command_wc(Client *c, const Seperator *sep)
{
	if (sep->argnum < 2) {
		c->Message(
			0,
			"Usage: #wc [wear slot] [material] [ [hero_forge_model] [elite_material] [unknown06] [unknown18] ]"
		);
	}
	else if (c->GetTarget() == nullptr) {
		c->Message(Chat::Red, "You must have a target to do a wear change.");
	}
	else {
		uint32 hero_forge_model = 0;
		uint32 wearslot         = atoi(sep->arg[1]);

		// Hero Forge
		if (sep->argnum > 2) {
			hero_forge_model = atoi(sep->arg[3]);

			if (hero_forge_model != 0 && hero_forge_model < 1000) {
				// Shorthand Hero Forge ID. Otherwise use the value the user entered.
				hero_forge_model = (hero_forge_model * 100) + wearslot;
			}
		}
		/*
		// Leaving here to add color option to the #wc command eventually
		uint32 Color;
		if (c->GetTarget()->IsClient())
			Color = c->GetTarget()->GetEquipmentColor(atoi(sep->arg[1]));
		else
			Color = c->GetTarget()->GetArmorTint(atoi(sep->arg[1]));
		*/
		c->GetTarget()->SendTextureWC(
			wearslot,
			atoi(sep->arg[2]),
			hero_forge_model,
			atoi(sep->arg[4]),
			atoi(sep->arg[5]),
			atoi(sep->arg[6]));
	}
}

