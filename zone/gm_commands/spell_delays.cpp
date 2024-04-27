#include "../command.h"

void command_spell_delays(Client* c, const Seperator* sep)
{
	const int arguments = sep->argnum;
	if (arguments) {
		const bool is_help = !strcasecmp(sep->arg[1], "help");

		if (is_help) {
			c->Message(Chat::White, "usage: %s [spelltype ID | spelltype Shortname] [current | value: 0-1].", sep->arg[0]);
			c->Message(Chat::White, "example: [%s 15 4000] or [%s cures 4000] would allow bots to cast cures on you every 4 seconds.", sep->arg[0], sep->arg[0]);
			c->Message(Chat::White, "note: Use [current] to check your current setting.");
			c->Message(
				Chat::White,
				fmt::format(
					"note: Use {} for a list of spell types by ID or {} for a list of spell types by short name.",
					Saylink::Silent(
						fmt::format("{} listid", sep->arg[0])
					),
					Saylink::Silent(
						fmt::format("{} listname", sep->arg[0])
					)
				).c_str()
			);

			return;
		}
	}

	std::string arg1 = sep->arg[1];

	if (!arg1.compare("listid") || !arg1.compare("listname")) {
		const std::string& color_red = "red_1";
		const std::string& color_blue = "royal_blue";
		const std::string& color_green = "forest_green";
		const std::string& bright_green = "green";
		const std::string& bright_red = "red";
		const std::string& heroic_color = "gold";

		std::string fillerLine = "-----------";
		std::string spellTypeField = "Spell Type";
		std::string pluralS = "s";
		std::string idField = "ID";
		std::string shortnameField = "Short Name";

		std::string popup_text = DialogueWindow::TableRow(
			DialogueWindow::TableCell(
				fmt::format(
					"{}",
					DialogueWindow::ColorMessage(bright_green, spellTypeField)
				)
			) +
			DialogueWindow::TableCell(
				fmt::format(
					"{}",
					(!arg1.compare("listid") ? DialogueWindow::ColorMessage(bright_green, idField) : DialogueWindow::ColorMessage(bright_green, shortnameField))
				)
			)
		);

		popup_text += DialogueWindow::TableRow(
			DialogueWindow::TableCell(
				fmt::format(
					"{}",
					DialogueWindow::ColorMessage(heroic_color, fillerLine)
				)
			) +
			DialogueWindow::TableCell(
				fmt::format(
					"{}",
					DialogueWindow::ColorMessage(heroic_color, fillerLine)
				)
			)
		);

		for (int i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
			if (!IsClientBotSpellType(i)) {
				continue;
			}

			popup_text += DialogueWindow::TableRow(
				DialogueWindow::TableCell(
					fmt::format(
						"{}{}",
						DialogueWindow::ColorMessage(color_green, c->GetSpellTypeNameByID(i)),
						DialogueWindow::ColorMessage(color_green, pluralS)
					)
				) +
				DialogueWindow::TableCell(
					fmt::format(
						"{}",
						(!arg1.compare("listid") ? DialogueWindow::ColorMessage(color_blue, std::to_string(i)) : DialogueWindow::ColorMessage(color_blue, c->GetSpellTypeShortNameByID(i)))
					)
				)
			);
		}

		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient("Spell Types", popup_text.c_str());

		return;
	}

	std::string arg2 = sep->arg[2];
	int ab_arg = 2;
	bool current_check = false;
	uint16 spellType = 0;
	uint32 typeValue = 0;

	if (sep->IsNumber(1)) {
		spellType = atoi(sep->arg[1]);

		if (!IsClientBotSpellType(spellType)) {
			c->Message(
				Chat::White,
				fmt::format(
					"You must choose a valid spell type. Use {} for a list of spell types by ID or {} for a list of spell types by short name.",
					Saylink::Silent(
						fmt::format("{} listid", sep->arg[0])
					),
					Saylink::Silent(
						fmt::format("{} listname", sep->arg[0])
					)
				).c_str()
			);

			return;
		}
	}
	else {
		if (c->GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
			spellType = c->GetSpellTypeIDByShortName(arg1);

			if (!IsClientBotSpellType(spellType)) {
				c->Message(
					Chat::White,
					fmt::format(
						"You must choose a valid spell type. Use {} for a list of spell types by ID or {} for a list of spell types by short name.",
						Saylink::Silent(
							fmt::format("{} listid", sep->arg[0])
						),
						Saylink::Silent(
							fmt::format("{} listname", sep->arg[0])
						)
					).c_str()
				);

				return;
			}
		}
		else {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Incorrect argument, use {} for information regarding this command.",
					Saylink::Silent(
						fmt::format("{} help", sep->arg[0])
					)
				).c_str()
			);

			return;
		}
	}

	if (sep->IsNumber(2)) {
		typeValue = atoi(sep->arg[2]);
		++ab_arg;
		if (typeValue < 1 || typeValue > 60000) {
			c->Message(Chat::Yellow, "You must enter a value between 1-60000 (1ms to 60s).");

			return;
		}
	}
	else if (!arg2.compare("current")) {
		++ab_arg;
		current_check = true;
	}
	else {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Incorrect argument, use {} for information regarding this command.",
				Saylink::Silent(
					fmt::format("{} help", sep->arg[0])
				)
			).c_str()
		);

		return;
	}

	if (current_check) {
		c->Message(
			Chat::Green,
			fmt::format(
				"Your current {} delay is {} seconds.",
				c->GetSpellTypeNameByID(spellType),
				c->GetSpellDelay(spellType) / 1000.00
			).c_str()
		);
	}
	else {
		c->SetSpellDelay(spellType, typeValue);
		c->Message(
			Chat::Green,
			fmt::format(
				"Your {} delay was set to {} seconds.",
				c->GetSpellTypeNameByID(spellType),
				c->GetSpellDelay(spellType) / 1000.00
			).c_str()
		);
	}
}
