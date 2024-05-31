#include "../../client.h"

void FindSkill(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(2)) {
		const uint16 skill_id = Strings::ToInt(sep->arg[2]);
		const std::string& skill_name = Skill::GetName(skill_id);
		if (Strings::EqualFold(skill_name, "UNKNOWN SKILL")) {
			c->Message(
				Chat::White,
				fmt::format(
					"Skill ID {} does not exist.",
					skill_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Skill {} | {}",
				skill_id,
				skill_name
			).c_str()
		);

		return;
	}

	const std::string& search_criteria = Strings::ToLower(sep->argplus[2]);

	uint32 found_count = 0;

	for (const auto& s : skill_names) {
		const auto& skill_name_lower = Strings::ToLower(s.second);
		if (!Strings::Contains(skill_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Skill {} | {}",
				s.first,
				s.second
			).c_str()
		);

		found_count++;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Skill{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
