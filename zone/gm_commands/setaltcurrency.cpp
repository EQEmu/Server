#include "../client.h"

void command_setaltcurrency(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (
		arguments < 2 ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2)
	) {
		c->Message(Chat::White, "Command Syntax: #setaltcurrency [Currency ID] [Amount]");
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto currency_id = std::stoul(sep->arg[1]);
	auto amount = static_cast<int>(std::min(std::stoll(sep->arg[2]), (long long) 2000000000));
	uint32 currency_item_id = zone->GetCurrencyItemID(currency_id);
	if (!currency_item_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Currency ID {} could not be found.",
				currency_id
			).c_str()
		);
		return;
	}

	target->SetAlternateCurrencyValue(currency_id, amount);
	
	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}.",
			c->GetTargetDescription(target, TargetDescriptionType::UCYou),
			c == target ? "have" : "has",
			(
				amount ?
				std::to_string(amount) :
				"no"
			),
			database.CreateItemLink(currency_item_id)
		).c_str()
	);
}

