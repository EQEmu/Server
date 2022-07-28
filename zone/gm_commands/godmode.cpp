#include "../client.h"

void command_godmode(Client *c, const Seperator *sep) {
	bool state = atobool(sep->arg[1]);
	uint32 account = c->AccountID();

	if (sep->arg[1][0] != 0) {
		c->SetInvul(state);
		database.SetGMInvul(account, state);
		database.SetGMSpeed(account, state ? 1 : 0);
		c->SendAppearancePacket(AT_Levitate, state);
		database.SetGMFlymode(account, state);
		c->SetHideMe(state);
		c->Message(Chat::White, "Turning GodMode %s for %s (zone for gmspeed to take effect)", state ? "On" : "Off", c->GetName());
	} else {
		c->Message(Chat::White, "Usage: #godmode [on/off]");
	}
}
