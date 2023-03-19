#include "../client.h"
#include "../../common/languages.h"
#include "../../common/data_verification.h"

void command_setlanguage(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto language_id = sep->IsNumber(1) ? Strings::ToInt(sep->arg[1]) : -1;
	auto language_value = sep->IsNumber(2) ? Strings::ToInt(sep->arg[2]) : -1;
	if (!strcasecmp(sep->arg[1], "list")) {
		for (const auto& language : EQ::constants::GetLanguageMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Language {}: {}",
					language.first,
					language.second
				).c_str()
			);
		}
	} else if (
		!EQ::ValueWithin(language_id, LANG_COMMON_TONGUE, LANG_UNKNOWN) ||
		!EQ::ValueWithin(language_value, 0, 100)
	) {
		c->Message(Chat::White, "Usage: #setlanguage [Language ID] [Language Value]");
		c->Message(Chat::White, "Usage: #setlanguage [List]");
		c->Message(Chat::White, "Language ID = 0 to 27");
		c->Message(Chat::White, "Language Value = 0 to 100");
	} else {
		LogInfo(
			"Set language request from [{}], Target: [{}] Language ID: [{}] Language Value: [{}]",
			c->GetCleanName(),
			c->GetTargetDescription(target),
			language_id,
			language_value
		);

		target->SetLanguageSkill(language_id, language_value);

		if (c != target) {
			c->Message(
				Chat::White,
				fmt::format(
					"Set {} ({}) to {} for {}.",
					EQ::constants::GetLanguageName(language_id),
					language_id,
					language_value,
					c->GetTargetDescription(target)
				).c_str()
			);
		}
	}
}
