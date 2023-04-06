#include "../../common/database/database_update.h"

void WorldserverCLI::DatabaseUpdates(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Runs database updates manually";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	DatabaseUpdate update;
	update.SetDatabase(&database)->CheckDbUpdates();
}
