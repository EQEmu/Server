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

		auto e = PlayerEvent::TradeEvent{};

		// player a
		auto t = PlayerEvent::TradeEntry{};
		t.character_id   = 1;
		t.character_name = "PlayerA";
		t.coin           = 100;
		t.items.push_back(PlayerEvent::TradeItem{1001, "Cloth Cap", 1});
		t.items.push_back(PlayerEvent::TradeItem{1002, "Cloth Veil", 2});
		t.items.push_back(PlayerEvent::TradeItem{1003, "Cloth Choker", 3});
		e.entries.push_back(t);

		// player b
		t = PlayerEvent::TradeEntry{};
		t.character_id   = 2;
		t.character_name = "PlayerB";
		t.coin           = 101;
		t.items.push_back(PlayerEvent::TradeItem{1001, "Cloth Cap", 1});
		t.items.push_back(PlayerEvent::TradeItem{1002, "Cloth Veil", 2});
		t.items.push_back(PlayerEvent::TradeItem{1003, "Cloth Choker", 3});
		e.entries.push_back(t);

		e.serialize(ar);

//		std::cout << e.serialize(ar) << std::endl;
	}

	std::string output = ss.str();
	output = Strings::Replace(output, "	", "");
	output = Strings::Replace(output, " ", "");
	output = Strings::Replace(output, "\n", "");

	std::cout << output << std::endl;

/*

	{
		cereal::JSONInputArchive ar(ss);
		A                        a;
		ar(a);

		std::ostringstream        oss;
		cereal::JSONOutputArchive ar2(oss);
		ar2(a);

		std::string output = oss.str();
		output = Strings::Replace(output, "	", "");
		output = Strings::Replace(output, " ", "");
		output = Strings::Replace(output, "\n", "");

		LogInfo("{}", output);
	}
*/

}
