#include "../client.h"

void command_movechar(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #movechar [Character ID|Character Name] [Zone ID|Zone Short Name]");
		return;
	}
	
	std::string character_name = (
		sep->IsNumber(1) ?
		database.GetCharNameByID(std::stoul(sep->arg[1])) :
		sep->arg[1]
	);
	auto character_id = database.GetCharacterID(character_name.c_str());
	if (!character_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character {} could not be found.",
				character_name
			).c_str()
		);
		return;
	}

	auto account_id = database.GetAccountIDByChar(character_name.c_str());

	std::string zone_short_name = str_tolower(
		sep->IsNumber(2) ?
		ZoneName(std::stoul(sep->arg[2]), true) :
		sep->arg[2]
	);

	bool is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
	if (is_unknown_zone) {
		c->Message(
			Chat::White,
			fmt::format(
				"Zone ID {} could not be found.",
				std::stoul(sep->arg[2])
			).c_str()
		);
		return;
	}

	auto zone_id = ZoneID(zone_short_name);
	std::string zone_long_name = ZoneLongName(zone_id);

	bool is_special_zone = (
		zone_short_name.find("cshome") != std::string::npos ||
		zone_short_name.find("load") != std::string::npos ||
		zone_short_name.find("load2") != std::string::npos
	);

	if (c->Admin() < commandMovecharToSpecials && is_special_zone) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) is a special zone and you cannot move someone there.",
				zone_long_name,
				zone_short_name
			).c_str()
		);
		return;
	}

	if (
		c->Admin() >= commandMovecharSelfOnly ||
		account_id == c->AccountID()
	) {
		bool moved = database.MoveCharacterToZone(character_name.c_str(), zone_id);
		std::string moved_string = (
			moved ?
			"Succeeded" :
			"Failed"
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Character Move {} | Character: {} ({})",
				moved_string,
				character_name,
				character_id
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Character Move {} | Zone: {} ({}) ID: {}",
				moved_string,
				zone_long_name,
				zone_short_name,
				zone_id
			).c_str()
		);
	} else {
		c->Message(Chat::White, "You cannot move characters that are not on your account.");
	}
}
