#include "../command.h"

void command_spell_holds(Client *c, const Seperator *sep)
{
	//unused for clients
	c->Message(Chat::Yellow, "Spell Holds for players is currently unused.");
	return;

	const int arguments = sep->argnum;
	if (arguments) {
		const bool is_help = !strcasecmp(sep->arg[1], "help");

		if (is_help) {
			BotCommandHelpParams p;

			p.description = { "Toggles whether or not bots can cast certain spell types on you" };
			p.notes =
			{
				"- All pet types are control your how your pet will be affected"
			};
			p.example_format =
			{
				fmt::format(
					"{} [Type Shortname] [value]", 
					sep->arg[0]
				),
				fmt::format(
					"{} [Type ID] [value]",
					sep->arg[0]
				)
			};
			p.examples_one =
			{
				"To set DoTs to be held:",
				fmt::format(
					"{} {} 1",
					sep->arg[0],
					c->GetSpellTypeShortNameByID(BotSpellTypes::DOT)
				),
				fmt::format(
					"{} {} 1",
					sep->arg[0],
					BotSpellTypes::DOT
				)
			};
			p.examples_two =
			{
				"To check your current DoT settings:",
				fmt::format(
					"{} {} current",
					sep->arg[0],
					c->GetSpellTypeShortNameByID(BotSpellTypes::DOT)
				),
				fmt::format(
					"{} {} current",
					sep->arg[0],
					BotSpellTypes::DOT
				)
			};

			std::string popup_text = c->SendBotCommandHelpWindow(p);
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
		if (type_value < 0 || type_value > 1) {
			c->Message(Chat::Yellow, "You must enter either 0 for disabled or 1 for enabled.");

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
				"Your [{}] spell hold is currently [{}].'",
				c->GetSpellTypeNameByID(spell_type),
				c->GetSpellHold(spell_type) ? "enabled" : "disabled"
			).c_str()
		);
	}
	else {
		c->SetSpellHold(spell_type, type_value);
		c->Message(
			Chat::Green,
			fmt::format(
				"Your [{}] spell hold was [{}].'",
				c->GetSpellTypeNameByID(spell_type),
				c->GetSpellHold(spell_type) ? "enabled" : "disabled"
			).c_str()
		);
	}
}
