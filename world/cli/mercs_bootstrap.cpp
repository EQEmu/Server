#include "../../common/database/database_update.h"

void WorldserverCLI::MercsBootstrap(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Bootstraps merc tables";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {};

	if (cmd[{"-h", "--help"}]) {
		return;
	}


}
