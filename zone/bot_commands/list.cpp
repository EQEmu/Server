#include "../client.h"
#include "../bot_command.h"

void bot_command_list_(Client *c, const Seperator *sep)
{
	enum {
		FilterClass,
		FilterRace,
		FilterName,
		FilterCount,
		MaskClass = (1 << FilterClass),
		MaskRace = (1 << FilterRace),
		MaskName = (1 << FilterName)
	};

	if (helper_command_alias_fail(c, "bot_command_list", sep->arg[0], "botlist")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: {} (account) ([class] [value]) ([race] [value]) ([name] [partial-full])",
				sep->arg[0]
			).c_str()
		);
		c->Message(Chat::White, "Note: Filter criteria is orderless and optional.");
		return;
	}

	bool Account = false;
	int seps = 1;
	uint32 filter_value[FilterCount];
	int name_criteria_arg = 0;
	memset(&filter_value, 0, sizeof(uint32) * FilterCount);

	int filter_mask = 0;
	if (!strcasecmp(sep->arg[1], "account")) {
		Account = true;
		seps = 2;
	}

	for (int i = seps; i < (FilterCount * 2); i += 2) {
		if (sep->arg[i][0] == '\0') {
			break;
		}

		if (!strcasecmp(sep->arg[i], "class")) {
			filter_mask |= MaskClass;
			filter_value[FilterClass] = Strings::ToInt(sep->arg[i + 1]);
			continue;
		}

		if (!strcasecmp(sep->arg[i], "race")) {
			filter_mask |= MaskRace;
			filter_value[FilterRace] = Strings::ToInt(sep->arg[i + 1]);
			continue;
		}

		if (!strcasecmp(sep->arg[i], "name")) {
			filter_mask |= MaskName;
			name_criteria_arg = (i + 1);
			continue;
		}

		c->Message(Chat::White, "A numeric value or name is required to use the filter property of this command (f: '%s', v: '%s')", sep->arg[i], sep->arg[i + 1]);
		return;
	}

	std::list<BotsAvailableList> bots_list;
	if (!database.botdb.LoadBotsList(c->CharacterID(), bots_list, Account)) {
		c->Message(Chat::White, "%s", BotDatabase::fail::LoadBotsList());
		return;
	}

	if (bots_list.empty()) {
		c->Message(Chat::White, "You have no bots.");
		return;
	}

	auto bot_count = 0;
	auto bots_owned = 0;
	auto bot_number = 1;
	for (auto bots_iter : bots_list) {
		if (filter_mask) {
			if ((filter_mask & MaskClass) && filter_value[FilterClass] != bots_iter.Class) {
				continue;
			}

			if ((filter_mask & MaskRace) && filter_value[FilterRace] != bots_iter.Race) {
				continue;
			}

			if (filter_mask & MaskName) {
				std::string name_criteria = sep->arg[name_criteria_arg];
				std::transform(name_criteria.begin(), name_criteria.end(), name_criteria.begin(), ::tolower);
				std::string name_check = bots_iter.Name;
				std::transform(name_check.begin(), name_check.end(), name_check.begin(), ::tolower);
				if (name_check.find(name_criteria) == std::string::npos) {
					continue;
				}
			}
		}

		auto* bot = entity_list.GetBotByBotName(bots_iter.Name);

		c->Message(
			Chat::White,
			fmt::format(
				"Bot {} | {} is a Level {} {} {} {} owned by {}.",
				bot_number,
				(
					(c->CharacterID() == bots_iter.Owner_ID && !bot) ?
						Saylink::Silent(
							fmt::format("^spawn {}", bots_iter.Name),
							bots_iter.Name
						) :
						bots_iter.Name
				),
				bots_iter.Level,
				GetGenderName(bots_iter.Gender),
				GetRaceIDName(bots_iter.Race),
				GetClassIDName(bots_iter.Class),
				bots_iter.Owner
			).c_str()
		);

		if (c->CharacterID() == bots_iter.Owner_ID) {
			bots_owned++;
		}

		bot_count++;
		bot_number++;
	}

	if (!bot_count) {
		c->Message(Chat::White, "You have no bots meeting this criteria.");
		return;
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} Of {} bot{} shown, {} {} owned by you.",
				bot_count,
				bots_list.size(),
				bot_count != 1 ? "s" : "",
				bots_owned,
				bots_owned != 1 ? "are" : "is"
			).c_str()
		);

		c->Message(Chat::White, "Note: You can spawn any owned bots by clicking their name if they are not already spawned.");

		c->Message(Chat::White, "Your bot creation limits are as follows:");

		const auto overall_bot_creation_limit = c->GetBotCreationLimit();

		c->Message(
			Chat::White,
			fmt::format(
				"Overall | {} Bot{}",
				overall_bot_creation_limit,
				overall_bot_creation_limit != 1 ? "s" : ""
			).c_str()
		);

		for (uint8 class_id = Class::Warrior; class_id <= Class::Berserker; class_id++) {
			auto class_creation_limit = c->GetBotCreationLimit(class_id);

			if (class_creation_limit != overall_bot_creation_limit) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} | {} Bot{}",
						GetClassIDName(class_id),
						class_creation_limit,
						class_creation_limit != 1 ? "s" : ""
					).c_str()
				);
			}
		}
	}
}
