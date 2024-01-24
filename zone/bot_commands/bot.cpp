#include "../client.h"
#include "../bot_command.h"

void bot_command_bot(Client *c, const Seperator *sep)
{

	std::list<const char*> subcommand_list;
	subcommand_list.push_back("botappearance");
	subcommand_list.push_back("botcamp");
	subcommand_list.push_back("botclone");
	subcommand_list.push_back("botcreate");
	subcommand_list.push_back("botdelete");
	subcommand_list.push_back("botdetails");
	subcommand_list.push_back("botdyearmor");
	subcommand_list.push_back("botinspectmessage");
	subcommand_list.push_back("botfollowdistance");
	subcommand_list.push_back("botlist");
	subcommand_list.push_back("botoutofcombat");
	subcommand_list.push_back("botreport");
	subcommand_list.push_back("botspawn");
	subcommand_list.push_back("botstance");
	subcommand_list.push_back("botstopmeleelevel");
	subcommand_list.push_back("botsummon");
	subcommand_list.push_back("bottogglearcher");
	subcommand_list.push_back("bottogglehelm");
	subcommand_list.push_back("botupdate");

	if (helper_command_alias_fail(c, "bot_command_bot", sep->arg[0], "bot"))
		return;

	helper_send_available_subcommands(c, "bot", subcommand_list);
}
