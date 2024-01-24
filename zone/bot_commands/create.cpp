#include "../client.h"
#include "../bot_command.h"

void bot_command_create(Client *c, const Seperator *sep)
{
	const std::string class_substrs[17] = {
		"",
		"WAR", "CLR", "PAL", "RNG",
		"SHD", "DRU", "MNK", "BRD",
		"ROG", "SHM", "NEC", "WIZ",
		"MAG", "ENC", "BST", "BER"
	};

	const std::string race_substrs[17] = {
		"",
		"HUM", "BAR", "ERU", "ELF",
		"HIE", "DEF", "HEF", "DWF",
		"TRL", "OGR", "HFL", "GNM",
		"IKS", "VAH", "FRG", "DRK"
	};

	const uint16 race_values[17] = {
		Race::Doug,
		Race::Human, Race::Barbarian, Race::Erudite, Race::WoodElf,
		Race::HighElf, Race::DarkElf, Race::HalfElf, Race::Dwarf,
		Race::Troll, Race::Ogre, Race::Halfling, Race::Gnome,
		Race::Iksar, Race::VahShir, Race::Froglok2, Race::Drakkin
	};

	const std::string gender_substrs[2] = {
		"Male", "Female",
	};

	if (helper_command_alias_fail(c, "bot_command_create", sep->arg[0], "botcreate")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} [Name] [Class] [Race] [Gender]",
				sep->arg[0]
			).c_str()
		);

		std::string window_text;
		std::string message_separator;
		int object_count = 0;
		const int object_max = 4;

		window_text.append(
			fmt::format(
				"Classes{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		object_count = 0;
		for (int i = 0; i <= 15; ++i) {
			window_text.append(message_separator);

			if (object_count >= object_max) {
				window_text.append(DialogueWindow::Break());
				object_count = 0;
			}

			window_text.append(
				fmt::format("{} ({})",
							class_substrs[i + 1],
							(i + 1)
				)
			);

			++object_count;
			message_separator = ", ";
		}

		window_text.append(DialogueWindow::Break(2));

		window_text.append(
			fmt::format(
				"<c \"#FFFFFF\">Races{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		object_count = 0;
		for (int i = 0; i <= 15; ++i) {
			window_text.append(message_separator);

			if (object_count >= object_max) {
				window_text.append(DialogueWindow::Break());
				object_count = 0;
			}

			window_text.append(
				fmt::format("{} ({})",
							race_substrs[i + 1],
							race_values[i + 1]
				)
			);

			++object_count;
			message_separator = ", ";
		}

		window_text.append(DialogueWindow::Break(2));

		window_text.append(
			fmt::format(
				"<c \"#FFFFFF\">Genders{}<c \"#FFFF\">",
				DialogueWindow::Break()
			)
		);

		message_separator = " ";
		for (int i = 0; i <= 1; ++i) {
			window_text.append(message_separator);

			window_text.append(
				fmt::format("{} ({})",
							gender_substrs[i],
							i
				)
			);

			message_separator = ", ";
		}

		c->SendPopupToClient("Bot Creation Options", window_text.c_str());

		return;
	}

	const auto arguments = sep->argnum;
	if (!arguments || sep->IsNumber(1)) {
		c->Message(Chat::White, "You must name your bot!");
		return;
	}

	std::string bot_name = sep->arg[1];
	bot_name = Strings::UcFirst(bot_name);

	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Invalid class!");
		return;
	}

	auto bot_class = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));

	if (arguments < 3 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Invalid race!");
		return;
	}

	auto bot_race = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[3]));

	if (arguments < 4) {
		c->Message(Chat::White, "Invalid gender!");
		return;
	}

	auto bot_gender = Gender::Male;

	if (sep->IsNumber(4)) {
		bot_gender = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[4]));
		if (bot_gender == Gender::Neuter) {
			bot_gender = Gender::Male;
		}
	} else {
		if (!strcasecmp(sep->arg[4], "m") || !strcasecmp(sep->arg[4], "male")) {
			bot_gender = Gender::Male;
		} else if (!strcasecmp(sep->arg[4], "f") || !strcasecmp(sep->arg[4], "female")) {
			bot_gender = Gender::Female;
		}
	}

	helper_bot_create(c, bot_name, bot_class, bot_race, bot_gender);
}
