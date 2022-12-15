#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include "../../common/events/player_events.h"

void WorldserverCLI::TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

}
