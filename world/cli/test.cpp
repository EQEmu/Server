#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include "../../common/events/player_events.h"

void WorldserverCLI::TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	std::stringstream ss;
	{
		cereal::JSONOutputArchive ar(ss);
		PlayerEvent::SayEvent n;
		n.message = "Test";
		n.serialize(ar);
	}

	std::cout << ss.str() << std::endl;

	{
		cereal::JSONInputArchive ar(ss);
		PlayerEvent::SayEvent n2;
		n2.message = "Test2";
		cereal::JSONOutputArchive ar2(std::cout);
		n2.serialize(ar2);
	}

}
