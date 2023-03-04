#include "../client.h"
#include "../../common/repositories/character_data_repository.h"

void command_findcharacter(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #findcharacter [Search Criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		const auto character_id = Strings::ToUnsignedInt(sep->arg[1]);

		const auto& e = CharacterDataRepository::FindOne(content_db, character_id);
		if (!e.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character ID {} does not exist or is invalid.",
					character_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Character ID {} | {}",
				character_id,
				e.name
			).c_str()
		);
	} else {
		const auto search_criteria = Strings::ToLower(sep->argplus[1]);
		const auto& l = CharacterDataRepository::GetWhere(
			content_db,
			fmt::format(
				"LOWER(`name`) LIKE '%%{}%%'",
				search_criteria
			).c_str()
		);
		if (l.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"No characters found matching '{}'.",
					sep->argplus[1]
				).c_str()
			);
		}

		auto found_count = 0;
		for (const auto& e : l) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character ID {} | {}",
					e.id,
					e.name
				).c_str()
			);

			found_count++;

			if (found_count == 50) {
				break;
			}
		}

		if (found_count == 50) {
			c->Message(
				Chat::White,
				fmt::format(
					"50 Characters found matching '{}', max reached.",
					sep->argplus[1]
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} Character{} found matching '{}'.",
					found_count,
					found_count != 1 ? "s" : "",
					sep->argplus[1]
				).c_str()
			);
		}
	}
}

