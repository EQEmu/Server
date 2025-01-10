#include "../command.h"

void command_spell_max_thresholds(Client* c, const Seperator* sep)
{
	const int arguments = sep->argnum;
	if (arguments) {
		const bool is_help = !strcasecmp(sep->arg[1], "help");

		if (is_help) {
			std::vector<std::string> description =
			{
				"Threshold of your own health when bots will start casting the chosen spell type"
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
				"To set Complete Heals to start at 90% health:",
				fmt::format(
					"{} {} 90",
					sep->arg[0],
					c->GetSpellTypeShortNameByID(BotSpellTypes::CompleteHeal)
				),
				fmt::format(
					"{} {} 90",
					sep->arg[0],
					BotSpellTypes::CompleteHeal
				)
			};
			std::vector<std::string> examples_two =
			{
				"To check your current HoT Heal settings:",
				fmt::format(
					"{} {} current",
					sep->arg[0],
					c->GetSpellTypeShortNameByID(BotSpellTypes::HoTHeals)
				),
				fmt::format(
					"{} {} current",
					sep->arg[0],
					BotSpellTypes::HoTHeals
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
	uint16 spell_type = 0;
	uint32 type_value = 0;

	if (sep->IsNumber(1)) {
		spell_type = atoi(sep->arg[1]);

		if (!IsClientBotSpellType(spell_type)) {
			c->Message(Chat::Yellow, "Invalid spell type.");
			c->SendSpellTypePrompts(false, true);

			return;
		}
	}
	else {
		if (c->GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
			spell_type = c->GetSpellTypeIDByShortName(arg1);

			if (!IsClientBotSpellType(spell_type)) {
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
		type_value = atoi(sep->arg[2]);
		++ab_arg;
		if (type_value < 0 || type_value > 100) {
			c->Message(Chat::Yellow, "You must enter a value between 0-100 (0%% to 100%% of your health).");

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
				"Your [{}] maximum hold is currently [{}%%].'",
				c->GetSpellTypeNameByID(spell_type),
				c->GetSpellMaxThreshold(spell_type)
			).c_str()
		);
	}
	else {
		c->SetSpellMaxThreshold(spell_type, type_value);
		c->Message(
			Chat::Green,
			fmt::format(
				"Your [{}] maximum hold was set to [{}%%].'",
				c->GetSpellTypeNameByID(spell_type),
				c->GetSpellMaxThreshold(spell_type)
			).c_str()
		);
	}
}
