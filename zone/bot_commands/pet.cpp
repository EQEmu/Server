#include "../bot_command.h"
#include "../bot.h"

void bot_command_pet(Client *c, const Seperator *sep)
{
	std::vector<const char*> subcommand_list = {
		"petgetlost",
		"petremove",
		"petsettype"
	};

	if (helper_command_alias_fail(c, "bot_command_pet", sep->arg[0], "pet"))
		return;

	helper_send_available_subcommands(c, "bot pet", subcommand_list);
}

void bot_command_pet_get_lost(Client *c, const Seperator *sep)
{
}

void bot_command_pet_remove(Client *c, const Seperator *sep)
{
}

void bot_command_pet_set_type(Client *c, const Seperator *sep)
{
}
