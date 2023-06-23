#include "../client.h"
#include "../../common/languages.h"

void command_findlanguage(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Command Syntax: #findlanguage [Search Criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		const auto language_id = Strings::ToInt(sep->arg[1]);
		if (EQ::ValueWithin(language_id, LANG_COMMON_TONGUE, LANG_UNKNOWN)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Language {} | {}",
					language_id,
					EQ::constants::GetLanguageName(language_id)
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Language ID {} was not found.",
				language_id
			).c_str()
		);
		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[1]);
	if (!search_criteria.empty()) {
		const auto& m = EQ::constants::GetLanguageMap();
		auto found_count = 0;
		for (const auto& l : m) {
			const auto& language_name_lower = Strings::ToLower(l.second);
			if (Strings::Contains(language_name_lower, search_criteria)) {
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

		auto language_message = (
			found_count > 0 ?
			(
				found_count == 1 ?
				"A Language was" :
				fmt::format("{} Languages were", found_count)
			) :
			"No Languages were"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} found.",
				language_message
			).c_str()
		);
	}
}

