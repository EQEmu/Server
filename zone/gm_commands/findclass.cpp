#include "../client.h"

void command_findclass(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;

	if (arguments == 0) {
		c->Message(Chat::White, "Command Syntax: #findclass [search criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		int class_id = std::stoi(sep->arg[1]);
		if (class_id >= WARRIOR && class_id <= MERCERNARY_MASTER) {
			std::string class_name = GetClassIDName(class_id);
			c->Message(
				Chat::White,
				fmt::format(
					"Class {}: {}",
					class_id,
					class_name
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
	}
	else {
		std::string search_criteria = str_tolower(sep->argplus[1]);
		int         found_count     = 0;
		for (int    class_id        = WARRIOR; class_id <= MERCERNARY_MASTER; class_id++) {
			std::string class_name       = GetClassIDName(class_id);
			std::string class_name_lower = str_tolower(class_name);
			if (search_criteria.length() > 0 && class_name_lower.find(search_criteria) == std::string::npos) {
				continue;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Class {}: {}",
					class_id,
					class_name
				).c_str()
			);
			found_count++;

			if (found_count == 20) {
				break;
			}
		}

		if (found_count == 20) {
			c->Message(Chat::White, "20 Classes found... max reached.");
		}
		else {
			auto class_message = (
				found_count > 0 ?
					(
						found_count == 1 ?
							"A Class was" :
							fmt::format("{} Classes were", found_count)
					) :
					"No Classes were"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"{} found.",
					class_message
				).c_str()
			);
		}
	}
}

