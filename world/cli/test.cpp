#include "../../common/zone_store.h"

void WorldserverCLI::TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	zone_store.GetZoneName(0, false);
}
