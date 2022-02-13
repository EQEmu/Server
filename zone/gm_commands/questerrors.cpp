#include "../client.h"
#include "../quest_parser_collection.h"

void command_questerrors(Client *c, const Seperator *sep)
{
	std::list<std::string> err;
	parse->GetErrors(err);
	c->Message(Chat::White, "Current Quest Errors:");

	auto iter = err.begin();
	int  i    = 0;
	while (iter != err.end()) {
		if (i >= 30) {
			c->Message(Chat::White, "Maximum of 30 Errors shown...");
			break;
		}

		c->Message(Chat::White, iter->c_str());
		++i;
		++iter;
	}
}

