#include "../client.h"

void command_findaa(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Command Syntax: #findaa [Search Criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		int aa_id = std::stoi(sep->arg[1]);
		auto aa_name = zone->GetAAName(aa_id);
		if (!aa_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"AA {}: {}",
					aa_id,
					aa_name
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"AA ID {} was not found.",
					aa_id
				).c_str()
			);
		}
	} else {
		const auto search_criteria = Strings::ToLower(sep->argplus[1]);
		if (!search_criteria.empty()) {
			std::map<int, std::string> ordered_aas;

			for (const auto& a : zone->aa_abilities) {
				ordered_aas[a.second.get()->first->id] = a.second.get()->name;
			}

			int found_count = 0;
			for (const auto& a : ordered_aas) {
				auto aa_name = zone->GetAAName(a.first);
				if (!aa_name.empty()) {
					auto aa_name_lower = Strings::ToLower(aa_name);
					if (aa_name_lower.find(search_criteria) == std::string::npos) {
						continue;
					}

					c->Message(
						Chat::White,
						fmt::format(
							"AA {}: {}",
							a.first,
							aa_name
						).c_str()
					);
					found_count++;

					if (found_count == 50) {
						break;
					}
				}
			}

			if (!found_count) {
				c->Message(
					Chat::White,
					fmt::format(
						"No AAs were found matching '{}'.",
						search_criteria
					).c_str()
				);
				return;
			}

			if (found_count == 50) {
				c->Message(
					Chat::White,
					fmt::format(
						"50 AAs were found matching '{}', max reached.",
						search_criteria
					).c_str()
				);
			} else {
				auto skill_message = found_count == 1 ? "An AA was" : fmt::format("{} AAs were", found_count);

				c->Message(
					Chat::White,
					fmt::format(
						"{} found matching '{}'.",
						skill_message,
						search_criteria
					).c_str()
				);
			}
		}
	}
}

