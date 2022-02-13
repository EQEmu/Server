#include "../client.h"

void command_haste(Client *c, const Seperator *sep)
{
	// #haste command to set client attack speed. Takes a percentage (100 = twice normal attack speed)
	if (sep->arg[1][0] != 0) {
		uint16 Haste = atoi(sep->arg[1]);
		if (Haste > 85) {
			Haste = 85;
		}
		c->SetExtraHaste(Haste);
		// SetAttackTimer must be called to make this take effect, so player needs to change
		// the primary weapon.
		c->Message(Chat::White, "Haste set to %d%% - Need to re-equip primary weapon before it takes effect", Haste);
	}
	else {
		c->Message(Chat::White, "Usage: #haste [percentage]");
	}
}

