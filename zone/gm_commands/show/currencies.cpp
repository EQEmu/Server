#include "../../client.h"
#include "../../dialogue_window.h"

void ShowCurrencies(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 platinum = (
		t->GetMoney(3, 0) +
		t->GetMoney(3, 1) +
		t->GetMoney(3, 2) +
		t->GetMoney(3, 3)
	);

	const uint32 gold = (
		t->GetMoney(2, 0) +
		t->GetMoney(2, 1) +
		t->GetMoney(2, 2)
	);

	const uint32 silver = (
		t->GetMoney(1, 0) +
		t->GetMoney(1, 1) +
		t->GetMoney(1, 2)
	);

	const uint32 copper = (
		t->GetMoney(0, 0) +
		t->GetMoney(0, 1) +
		t->GetMoney(0, 2)
	);

	std::string currency_table;

	bool has_currency = false;

	currency_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Currency")  +
		DialogueWindow::TableCell("Amount")
	);

	if (
		platinum ||
		gold ||
		silver ||
		copper
	) {
		currency_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("Money") +
			DialogueWindow::TableCell(Strings::Money(platinum, gold, silver, copper))
		);

		has_currency = true;
	}

	const uint32 ebon_crystals = t->GetEbonCrystals();
	if (ebon_crystals) {
		currency_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("Ebon Crystals") +
			DialogueWindow::TableCell(Strings::Commify(ebon_crystals))
		);

		has_currency = true;
	}

	const uint32 radiant_crystals = t->GetRadiantCrystals();
	if (radiant_crystals) {
		currency_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("Radiant Crystals") +
			DialogueWindow::TableCell(Strings::Commify(radiant_crystals))
		);

		has_currency = true;
	}

	for (const auto& a : zone->AlternateCurrencies) {
		const uint32 currency_value = t->GetAlternateCurrencyValue(a.id);
		if (currency_value) {
			const auto* d = database.GetItem(a.item_id);
			currency_table += DialogueWindow::TableRow(
				DialogueWindow::TableCell(d->Name) +
				DialogueWindow::TableCell(Strings::Commify(currency_value))
			);

			has_currency = true;
		}
	}

	for (const auto& l : EQ::constants::GetLDoNThemeMap()) {
		const uint32 ldon_currency_value = t->GetLDoNPointsTheme(l.first);
		if (ldon_currency_value) {
			currency_table += DialogueWindow::TableRow(
				DialogueWindow::TableCell(l.second) +
				DialogueWindow::TableCell(Strings::Commify(ldon_currency_value))
			);

			has_currency = true;
		}
	}

	const uint32 pvp_points = t->GetPVPPoints();
	if (pvp_points) {
		currency_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("PVP Points") +
			DialogueWindow::TableCell(Strings::Commify(pvp_points))
		);

		has_currency = true;
	}

	currency_table = DialogueWindow::Table(currency_table);

	if (!has_currency) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have any currencies.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "do" : "does"
			).c_str()
		);

		return;
	}

	c->SendPopupToClient(
		fmt::format(
			"Currency for {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf)
		).c_str(),
		currency_table.c_str()
	);
}
