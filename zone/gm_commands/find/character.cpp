#include "../../client.h"
#include "../../common/repositories/character_data_repository.h"

void FindCharacter(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const auto character_id = Strings::ToUnsignedInt(sep->arg[2]);

		const auto& e = CharacterDataRepository::FindOne(content_db, character_id);
		if (!e.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character ID {} does not exist or is invalid.",
					Strings::Commify(character_id)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Character ID {} | {}",
				Strings::Commify(character_id),
				e.name
			).c_str()
		);

		return;
	}

	const auto search_criteria = Strings::ToLower(sep->argplus[2]);

	const auto& l = CharacterDataRepository::GetWhere(
		content_db,
		fmt::format(
			"LOWER(`name`) LIKE '%{}%'",
			search_criteria
		)
	);

	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"No characters found matching '{}'.",
				sep->argplus[2]
			).c_str()
		);
	}

	auto found_count = 0;

	for (const auto& e : l) {
		c->Message(
			Chat::White,
			fmt::format(
				"Character ID {} | {}",
				Strings::Commify(e.id),
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
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Character{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}

