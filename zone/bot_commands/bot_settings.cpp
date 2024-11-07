#include "../bot_command.h"

void bot_command_bot_settings(Client* c, const Seperator* sep)
{
	std::list<const char*> subcommand_list;
	subcommand_list.push_back("behindmob");
	subcommand_list.push_back("distanceranged");
	subcommand_list.push_back("copysettings");
	subcommand_list.push_back("defaultsettings");
	subcommand_list.push_back("enforcespelllist");
	subcommand_list.push_back("follow");
	subcommand_list.push_back("followdistance");
	subcommand_list.push_back("illusionblock");
	subcommand_list.push_back("maxmeleerange");
	subcommand_list.push_back("owneroption");
	subcommand_list.push_back("petsettype");
	subcommand_list.push_back("sithppercent");
	subcommand_list.push_back("sitincombat");
	subcommand_list.push_back("sitmanapercent");
	subcommand_list.push_back("sithppercent");
	subcommand_list.push_back("spellaggrocheck");
	subcommand_list.push_back("spelldelays");
	subcommand_list.push_back("spellengagedpriority");
	subcommand_list.push_back("spellholds");
	subcommand_list.push_back("spellidlepriority");
	subcommand_list.push_back("spellmaxhppct");
	subcommand_list.push_back("spellmaxmanapct");
	subcommand_list.push_back("spellmaxthresholds");
	subcommand_list.push_back("spellminhppct");
	subcommand_list.push_back("spellminmanapct");
	subcommand_list.push_back("spellminthresholds");
	subcommand_list.push_back("spellpursuepriority");
	subcommand_list.push_back("spelltargetcount");
	subcommand_list.push_back("spelllist");
	subcommand_list.push_back("stance");
	subcommand_list.push_back("togglehelm");
	subcommand_list.push_back("bottoggleranged");

	if (helper_command_alias_fail(c, "bot_command_bot_settings", sep->arg[0], "botsettings"))
		return;

	helper_send_available_subcommands(c, "botsettings", subcommand_list);
}
