#include "../client.h"
#include "../../common/repositories/account_repository.h"

void command_godmode(Client *c, const Seperator *sep)
{
	bool   state      = atobool(sep->arg[1]);
	uint32 account_id = c->AccountID();

	if (sep->arg[1][0] != 0) {
		auto a = AccountRepository::FindOne(database, c->AccountID());
		if (a.id > 0) {
			a.flymode      = state ? 1 : 0;
			a.gmspeed      = state ? 1 : 0;
			a.invulnerable = state ? 1 : 0;
			a.hideme       = state ? 1 : 0;
		}

		c->SetInvul(state);
		c->SendAppearancePacket(AT_Levitate, state);
		c->SetHideMe(state);
		c->Message(
			Chat::White,
			"Turning GodMode %s for %s (zone for gmspeed to take effect)",
			state ? "On" : "Off",
			c->GetName()
		);

		AccountRepository::UpdateOne(database, a);
	}
	else {
		c->Message(Chat::White, "Usage: #godmode [on/off]");
	}
}
