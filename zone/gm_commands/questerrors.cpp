#include "../client.h"
#include "../quest_parser_collection.h"

void command_questerrors(Client *c, const Seperator *sep)
{
	std::list<std::string> quest_errors;
	parse->GetErrors(quest_errors);

	if (quest_errors.size()) {
		c->Message(Chat::White, "Quest errors currently are as follows:");

		int error_index = 0;
		for (auto quest_error : quest_errors) {
			if (error_index >= RuleI(World, MaximumQuestErrors)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Maximum of {} error{} shown.",
						RuleI(World, MaximumQuestErrors),
						RuleI(World, MaximumQuestErrors) != 1 ? "s" : ""
					).c_str()
				);
				break;
			}

			c->Message(Chat::White, quest_error.c_str());
			error_index++;
		}
	} else {
		c->Message(Chat::White, "There are no Quest errors currently.");
	}
}

