#include "../client.h"

void command_merchantopenshop(Client *c, const Seperator *sep)
{
	Mob *merchant = c->GetTarget();
	if (!merchant || merchant->GetClass() != MERCHANT) {
		c->Message(Chat::White, "You must target a merchant to open their shop.");
		return;
	}

	merchant->CastToNPC()->MerchantOpenShop();
}

