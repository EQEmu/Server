#include "../client.h"

void command_viewcurrencies(Client *c, const Seperator *sep)
{	
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}
	
	auto target_string = (
		c == target ?
		"Yourself" :
		fmt::format(
			"{} ({})",
			target->GetCleanName(),
			target->GetID()
		)
	);

	auto platinum = (
		target->GetMoney(3, 0) +
		target->GetMoney(3, 1) +
		target->GetMoney(3, 2) +
		target->GetMoney(3, 3)
	);
	
	auto gold = (
		target->GetMoney(2, 0) +
		target->GetMoney(2, 1) +
		target->GetMoney(2, 2)
	);
	
	auto silver = (
		target->GetMoney(1, 0) +
		target->GetMoney(1, 1) +
		target->GetMoney(1, 2)
	);
	
	auto copper = (
		target->GetMoney(0, 0) +
		target->GetMoney(0, 1) +
		target->GetMoney(0, 2)
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Money for {} | {}",
			target_string,
			ConvertMoneyToString(
				platinum,
				gold,
				silver,
				copper
			)
		).c_str()
	);

	if (
		target->GetEbonCrystals() ||
		target->GetRadiantCrystals()
	) {
		c->Message(
			Chat::White,
			fmt::format(
				"Crystals for {} | {}: {} {}: {}",
				target_string,
				database.CreateItemLink(RuleI(Zone, EbonCrystalItemID)),
				target->GetEbonCrystals(),
				database.CreateItemLink(RuleI(Zone, RadiantCrystalItemID)),
				target->GetRadiantCrystals()
			).c_str()
		);
	}

	auto alternate_currency = zone->AlternateCurrencies.begin();
	while (alternate_currency != zone->AlternateCurrencies.end()) {
		auto currency_id = alternate_currency->id;
		auto currency_value = target->GetAlternateCurrencyValue(currency_id);
		if (currency_value) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) for {} | {}",
					database.CreateItemLink(alternate_currency->item_id),
					alternate_currency->id,
					target_string,
					currency_value
				).c_str()
			);
		}
		++alternate_currency;
	}

	for (
		uint32 ldon_currency_id = LDoNThemes::GUK;
		ldon_currency_id <= LDoNThemes::TAK;
		ldon_currency_id++
	) {
		auto ldon_currency_value = target->GetLDoNPointsTheme(ldon_currency_id);
		if (ldon_currency_value) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} ({}) for {} | {}",
					EQ::constants::GetLDoNThemeName(ldon_currency_id),
					ldon_currency_id,
					target_string,
					ldon_currency_value
				).c_str()
			);
		}
	}

	auto pvp_points = target->GetPVPPoints();
	if (pvp_points) {
		c->Message(
			Chat::White,
			fmt::format(
				"PVP Points for {} | {}",
				target_string,
				pvp_points
			).c_str()
		);
	}
}
