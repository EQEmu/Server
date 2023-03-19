#include "../../common/zone_store.h"

void WorldserverCLI::TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	zone_store.LoadZones(database);

	const char* zonename = ZoneName(0);
	if (zonename == 0) {
		LogInfo("Zone name is 0");
	}
	if (zonename == nullptr) {
		LogInfo("Zone name is nullptr");
	}

}
