#include "../client.h"

void command_findskill(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;

	if (arguments == 0) {
		c->Message(Chat::White, "Command Syntax: #findskill [search criteria]");
		return;
	}

	std::map<EQ::skills::SkillType, std::string> skills = EQ::skills::GetSkillTypeMap();
	if (sep->IsNumber(1)) {
		int skill_id = std::stoi(sep->arg[1]);
		if (skill_id >= EQ::skills::Skill1HBlunt && skill_id < EQ::skills::SkillCount) {
			for (auto skill : skills) {
				if (skill_id == skill.first) {
					c->Message(
						Chat::White,
						fmt::format(
							"Skill {}: {}",
							skill.first,
							skill.second
						).c_str()
					);
					break;
				}
			}
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Skill ID {} was not found.",
					skill_id
				).c_str()
			);
		}
	}
	else {
		std::string search_criteria = str_tolower(sep->argplus[1]);
		if (!search_criteria.empty()) {
			int       found_count = 0;
			for (auto skill : skills) {
				std::string skill_name_lower = str_tolower(skill.second);
				if (skill_name_lower.find(search_criteria) == std::string::npos) {
					continue;
				}

				c->Message(
					Chat::White,
					fmt::format(
						"Skill {}: {}",
						skill.first,
						skill.second
					).c_str()
				);
				found_count++;

				if (found_count == 20) {
					break;
				}
			}

			if (found_count == 20) {
				c->Message(Chat::White, "20 Skills were found, max reached.");
			}
			else {
				auto skill_message = (
					found_count > 0 ?
						(
							found_count == 1 ?
								"A Skill was" :
								fmt::format("{} Skills were", found_count)
						) :
						"No Skills were"
				);

				c->Message(
					Chat::White,
					fmt::format(
						"{} found.",
						skill_message
					).c_str()
				);
			}
		}
	}
}

