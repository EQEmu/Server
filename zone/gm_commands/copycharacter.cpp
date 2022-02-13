#include "../client.h"

void command_copycharacter(Client *c, const Seperator *sep)
{
	if (sep->argnum < 3) {
		c->Message(
			Chat::White,
			"Usage: [source_character_name] [destination_character_name] [destination_account_name]"
		);
		return;
	}

	std::string source_character_name      = sep->arg[1];
	std::string destination_character_name = sep->arg[2];
	std::string destination_account_name   = sep->arg[3];

	bool result = database.CopyCharacter(
		source_character_name,
		destination_character_name,
		destination_account_name
	);

	c->Message(
		Chat::Yellow,
		fmt::format(
			"Character Copy [{}] to [{}] via account [{}] [{}]",
			source_character_name,
			destination_character_name,
			destination_account_name,
			result ? "Success" : "Failed"
		).c_str()
	);
}

