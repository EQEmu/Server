#include "../client.h"

void command_merchantshop(Client *c, const Seperator *sep)
{
	const auto m = c->GetTarget();
	if (
		!m ||
		!m->IsNPC() ||
		(
			m->GetClass() != MERCHANT &&
			m->GetClass() != DISCORD_MERCHANT &&
			m->GetClass() != ADVENTURE_MERCHANT &&
			m->GetClass() != NORRATHS_KEEPERS_MERCHANT &&
			m->GetClass() != DARK_REIGN_MERCHANT &&
			m->GetClass() != ALT_CURRENCY_MERCHANT
		)
	) {
		c->Message(Chat::White, "You must target a merchant to close or open their shop.");
		return;
	}

	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "#merchantshop close - Close your targeted merchant's shop");
		c->Message(Chat::White, "#merchantshop open - Open your targeted merchant's shop");
		return;
	}

	const bool is_close = !strcasecmp(sep->arg[1], "close");
	const bool is_open  = !strcasecmp(sep->arg[1], "open");
	if (!is_close && !is_open) {
		c->Message(Chat::White, "#merchantshop close - Close your targeted merchant's shop");
		c->Message(Chat::White, "#merchantshop open - Open your targeted merchant's shop");
		return;
	}

	if (is_close) {
		m->CastToNPC()->MerchantCloseShop();
	} else if (is_open) {
		m->CastToNPC()->MerchantOpenShop();
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} shop for {}.",
			is_close ? "Closed" : "Opened",
			c->GetTargetDescription(m)
		).c_str()
	);
}

