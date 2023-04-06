#include "../../common/database/database_update.h"

void WorldserverCLI::BotsBootstrap(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Bootstraps bot tables and enables bots";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {};

	if (cmd[{"-h", "--help"}]) {
		return;
	}


}
