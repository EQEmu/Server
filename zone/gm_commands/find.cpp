#include "../client.h"
#include "find/aa.cpp"

void command_find(Client *c, const Seperator *sep)
{
	struct FindCommand {
		std::string command{};
		std::string usage{};
		void (*function)(Client *c, const Seperator *sep) = nullptr;
	};

	std::vector<FindCommand> commands = {
		FindCommand{.command = "aa", .usage = "aa [Search Criteria]", .function = FindAA},
	};

	// check for arguments
	auto arguments = sep->argnum;
	if (!arguments) {
		for (auto &cmd: commands) {
			c->Message(Chat::White, fmt::format("Usage: #find {}", cmd.usage).c_str());
		}
		return;
	}

	// execute the command
	for (auto &cmd: commands) {
		if (cmd.command == Strings::ToLower(sep->arg[1])) {
			cmd.function(c, sep);
			return;
		}
	}

	// command not found
	c->Message(Chat::White, "Command not found. Usage: #find [command]");
	for (auto &cmd: commands) {
		c->Message(Chat::White, fmt::format("Usage: #find {}", cmd.usage).c_str());
	}
}
