#include "../bot_command.h"

void bot_command_set_assistee(Client* c, const Seperator* sep)
{
	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Sets your bots to assist your target in addition to yourself"
		};

		std::vector<std::string> notes =
		{
			"- Your target must be another player in your group or raid.",
			"- This needs to be set on every zone/camp you do."
		};

		std::vector<std::string> example_format = { };
		std::vector<std::string> examples_one = { };
		std::vector<std::string> examples_two = { };
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

		return;
	}

	Mob* assistee = c->GetTarget();

	if (assistee && assistee->IsClient() && c->IsInGroupOrRaid(assistee)) {
		c->SetAssistee(assistee->CastToClient()->CharacterID());
		c->Message(Chat::Green, "Your bots will now assist %s.", assistee->GetCleanName());
		
		return;
	}

	c->Message(Chat::Yellow, "You can only set your bots to assist clients that are in your group or raid.");

	return;
}
