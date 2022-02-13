#include "../client.h"

void command_merchantcloseshop(Client *c, const Seperator *sep)
{
	Mob *merchant = c->GetTarget();
	if (!merchant || merchant->GetClass() != MERCHANT) {
		c->Message(Chat::White, "You must target a merchant to close their shop.");
		return;
	}

	merchant->CastToNPC()->MerchantCloseShop();
}

