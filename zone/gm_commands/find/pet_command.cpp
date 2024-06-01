#include "../../client.h"

void FindPetCommand(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint8 pet_command = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& pet_command_name = Pets::Command::GetName(pet_command);
		if (Strings::EqualFold(pet_command_name, "UNKNOWN PET COMMAND")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Pet Command {} does not exist.",
					pet_command
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Pet Command {} | {}",
				pet_command,
				pet_command_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : pet_types) {
		const std::string& pet_command_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(pet_command_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Pet Command {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Pet Command{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

