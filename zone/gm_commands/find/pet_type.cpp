#include "../../client.h"

void FindPetType(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint8 pet_type = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& pet_type_name = Pets::Type::GetName(pet_type);
		if (Strings::EqualFold(pet_type_name, "UNKNOWN PET TYPE")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Pet Type {} does not exist.",
					pet_type
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Pet Type {} | {}",
				pet_type,
				pet_type_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& e : pet_types) {
		const std::string& pet_type_name_lower = Strings::ToLower(e.second);
		if (!Strings::Contains(pet_type_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Pet Type {} | {}",
				e.first,
				e.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Pet Type{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

