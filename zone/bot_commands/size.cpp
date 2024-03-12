#include "../bot_command.h"

void bot_command_size(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Size];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Size) || helper_command_alias_fail(c, "bot_command_size", sep->arg[0], "size"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<friendly_target>) %s [grow | shrink]", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Size);
		return;
	}

	std::string size_arg = sep->arg[1];
	auto size_type = BCEnum::SzT_Reduce;
	if (!size_arg.compare("grow")) {
		size_type = BCEnum::SzT_Enlarge;
	}
	else if (size_arg.compare("shrink")) {
		c->Message(Chat::White, "This command requires a [grow | shrink] argument");
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToSize();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->size_type != size_type)
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);
		break;
	}

	helper_no_available_bots(c, my_bot);
}
