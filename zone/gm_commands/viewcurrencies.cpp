#include "../client.h"

void command_viewcurrencies(Client *c, const Seperator *sep)
{	
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

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

	if (
		platinum ||
		gold ||
		silver ||
		copper
	) {
		c->Message(
			Chat::White,
			fmt::format(
				"Money for {} | {}",
				c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
				ConvertMoneyToString(
					platinum,
					gold,
					silver,
					copper
				)
			).c_str()
		);
	}

	auto ebon_crystals = target->GetEbonCrystals();
	if (ebon_crystals) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} for {} | {}",
				database.CreateItemLink(RuleI(Zone, EbonCrystalItemID)),
				c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
				ebon_crystals
			).c_str()
		);
	}

	auto radiant_crystals = target->GetRadiantCrystals();
	if (radiant_crystals) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} for {} | {}",
				database.CreateItemLink(RuleI(Zone, RadiantCrystalItemID)),
				c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
				radiant_crystals
			).c_str()
		);
	}

	for (const auto& ac : zone->AlternateCurrencies) {
		auto currency_value = target->GetAlternateCurrencyValue(ac.id);
		if (currency_value) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} for {} | {}",
					database.CreateItemLink(ac.item_id),
					c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
					currency_value
				).c_str()
			);
		}
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
					"{} for {} | {}",
					EQ::constants::GetLDoNThemeName(ldon_currency_id),
					c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
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
				c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
				pvp_points
			).c_str()
		);
	}
}
