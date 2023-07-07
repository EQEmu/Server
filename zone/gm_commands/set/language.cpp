#include "../../client.h"
#include "../../../common/languages.h"
#include "../../../common/data_verification.h"

void SetLanguage(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set language [Language ID] [Language Value]");
		c->Message(Chat::White, "Language ID = 0 to 27");
		c->Message(Chat::White, "Language Value = 0 to 100");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const int language_id = Strings::ToInt(sep->arg[2]);
	const int language_value = Strings::ToInt(sep->arg[3]);
	if (
		!EQ::ValueWithin(language_id, LANG_COMMON_TONGUE, LANG_UNKNOWN) ||
		!EQ::ValueWithin(language_value, 0, MAX_LANGUAGE_SKILL)
	) {
		c->Message(Chat::White, "Usage: #set language [Language ID] [Language Value]");
		c->Message(Chat::White, "Language ID = 0 to 27");
		c->Message(Chat::White, "Language Value = 0 to 100");
		return;
	}

	LogInfo(
		"Set language request from [{}], Target: [{}] Language ID: [{}] Language Value: [{}]",
		c->GetCleanName(),
		c->GetTargetDescription(t),
		language_id,
		language_value
	);

	t->SetLanguageSkill(language_id, language_value);

	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"Set {} ({}) to {} for {}.",
				EQ::constants::GetLanguageName(language_id),
				language_id,
				language_value,
				c->GetTargetDescription(t)
			).c_str()
		);
	}
}
