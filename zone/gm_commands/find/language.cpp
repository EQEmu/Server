#include "../../client.h"

void FindLanguage(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint8 language_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		const std::string& language_name = Language::GetName(language_id);
		if (Strings::EqualFold(language_name, "UNKNOWN LANGUAGE")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Language ID {} does not exist.",
					language_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Language {} | {}",
				language_id,
				language_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& l : language_names) {
		const std::string& language_name_lower = Strings::ToLower(l.second);
		if (!Strings::Contains(language_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Language {} | {}",
				l.first,
				l.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Language{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

