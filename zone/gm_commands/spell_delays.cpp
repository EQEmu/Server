#include "../command.h"

void command_spell_delays(Client* c, const Seperator* sep)
{
	const int arguments = sep->argnum;
	if (arguments) {
		const bool is_help = !strcasecmp(sep->arg[1], "help");

		if (is_help) {
			std::vector<std::string> description =
			{
				"Controls how often bots can cast certain spell types on you"
			};

			std::vector<std::string> notes =
			{
				"- All pet types are control your how your pet will be affected"
			};

			std::vector<std::string> example_format =
			{
				fmt::format(
					"{} [Type Shortname] [value]"
					, sep->arg[0]
				),
				fmt::format(
					"{} [Type ID] [value]"
					, sep->arg[0]
				)
			};
			std::vector<std::string> examples_one =
			{
				"To set Very Fast Heals to be received every 1 second:",
				fmt::format(
					"{} {} 1000",
					sep->arg[0],
					c->GetSpellTypeShortNameByID(BotSpellTypes::VeryFastHeals)
				),
				fmt::format(
					"{} {} 1000",
					sep->arg[0],
					BotSpellTypes::VeryFastHeals
				)
			};
			std::vector<std::string> examples_two =
			{
				"To check your current Regular Heal delay:",
				fmt::format(
					"{} {} current",
					sep->arg[0],
					c->GetSpellTypeShortNameByID(BotSpellTypes::RegularHeal)
				),
				fmt::format(
					"{} {} current",
					sep->arg[0],
					BotSpellTypes::RegularHeal
				)
			};
			std::vector<std::string> examples_three = { };

			std::vector<std::string> actionables = { };

			std::vector<std::string> options = { };
			std::vector<std::string> options_one = { };
			std::vector<std::string> options_two = { };
			std::vector<std::string> options_three = { };

			std::string popup_text = c->SendCommandHelpWindow(
				c,
				description,
				notes,
				example_format,
				examples_one, examples_two, examples_three,
				actionables,
				options,
				options_one, options_two, options_three
			);

			popup_text = DialogueWindow::Table(popup_text);

			c->SendPopupToClient(sep->arg[0], popup_text.c_str());
			c->SendSpellTypePrompts(false, true);

			return;
		}
	}

	std::string arg1 = sep->arg[1];
	std::string arg2 = sep->arg[2];
	int ab_arg = 2;
	bool current_check = false;
	uint16 spellType = 0;
	uint32 typeValue = 0;

	if (sep->IsNumber(1)) {
		spellType = atoi(sep->arg[1]);

		if (!IsClientBotSpellType(spellType)) {
			c->Message(Chat::Yellow, "Invalid spell type.");
			c->SendSpellTypePrompts(false, true);

			return;
		}
	}
	else {
		if (c->GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
			spellType = c->GetSpellTypeIDByShortName(arg1);

			if (!IsClientBotSpellType(spellType)) {
				c->Message(Chat::Yellow, "Invalid spell type.");
				c->SendSpellTypePrompts(false, true);
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

	// Enable/Disable/Current checks
	if (sep->IsNumber(2)) {
		typeValue = atoi(sep->arg[2]);
		++ab_arg;
		if (typeValue < 0 || typeValue > 60000) {
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
				"Your [{}] delay is currently {} seconds.'",
				c->GetSpellTypeNameByID(spellType),
				c->GetSpellDelay(spellType) / 1000.00
			).c_str()
		);
	}
	else {
		c->SetSpellHold(spellType, typeValue);
		c->Message(
			Chat::Green,
			fmt::format(
				"Your [{}] delay was set to {} seconds.'",
				c->GetSpellTypeNameByID(spellType),
				c->GetSpellDelay(spellType) / 1000.00
			).c_str()
		);
	}
}
