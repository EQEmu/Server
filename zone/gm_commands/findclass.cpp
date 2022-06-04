#include "../client.h"

void command_findclass(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Command Syntax: #findclass [Search Criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		int class_id = std::stoi(sep->arg[1]);
		if (class_id >= WARRIOR && class_id <= MERCENARY_MASTER) {
			std::string class_name = GetClassIDName(class_id);
			c->Message(
				Chat::White,
				fmt::format(
					"Class {} | {}{}",
					class_id,
					class_name,
					(
						c->IsPlayerClass(class_id) ?
						fmt::format(
							" ({})",
							GetPlayerClassBit(class_id)
						) :
						""
					)
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Class ID {} was not found.",
					class_id
				).c_str()
			);
		}
	} else {
		auto search_criteria = str_tolower(sep->argplus[1]);
		int found_count = 0;
		for (uint16 class_id = WARRIOR; class_id <= MERCENARY_MASTER; class_id++) {
			std::string class_name = GetClassIDName(class_id);
			auto class_name_lower = str_tolower(class_name);
			if (
				search_criteria.length() &&
				class_name_lower.find(search_criteria) == std::string::npos
			) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Class {} | {}{}",
					class_id,
					class_name,
					(
						c->IsPlayerClass(class_id) ?
						fmt::format(
							" ({})",
							GetPlayerClassBit(class_id)
						) :
						""
					)
				).c_str()
			);

			found_count++;

			if (found_count == 50) {
				break;
			}
		}

		if (found_count == 50) {
			c->Message(Chat::White, "50 Classes found, max reached.");
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"{} Class{} found.",
					found_count,
					found_count != 1 ? "es" : ""
				).c_str()
			);
		}
	}
}

