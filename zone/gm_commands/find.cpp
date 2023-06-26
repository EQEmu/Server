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
	auto      request_cmd = Strings::ToLower(sep->arg[1]);
	for (auto &cmd: commands) {
		if (cmd.command == request_cmd) {
			cmd.function(c, sep);
			return;
		}
	}
}
