#include "../client.h"
#include "../../common/languages.h"

void command_setlanguage(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto language_id    = sep->IsNumber(1) ? std::stoi(sep->arg[1]) : -1;
	auto language_value = sep->IsNumber(2) ? std::stoi(sep->arg[2]) : -1;
	if (!strcasecmp(sep->arg[1], "list")) {
		for (int language = LANG_COMMON_TONGUE; language <= LANG_UNKNOWN; language++) {
			c->Message(
				Chat::White,
				fmt::format(
					"Language {}: {}",
					language,
					EQ::constants::GetLanguageName(language)
				).c_str()
			);
		}
	}
	else if (
		language_id < LANG_COMMON_TONGUE ||
		language_id > LANG_UNKNOWN ||
		language_value < 0 ||
		language_value > 100
		) {
		c->Message(Chat::White, "Usage: #setlanguage [Language ID] [Language Value]");
		c->Message(Chat::White, "Usage: #setlanguage [List]");
		c->Message(Chat::White, "Language ID = 0 to 27", LANG_UNKNOWN);
		c->Message(Chat::White, "Language Value = 0 to 100", HIGHEST_CAN_SET_SKILL);
	}
	else {
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

