#include "../bot_command.h"

void bot_command_mesmerize(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Mesmerize];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Mesmerize) || helper_command_alias_fail(c, "bot_command_mesmerize", sep->arg[0], "mesmerize"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <enemy_target> %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Mesmerize);
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, ENEMY);
		if (!target_mob)
			continue;

		if (spells[local_entry->spell_id].max_value[EFFECTIDTOINDEX(1)] < target_mob->GetLevel())
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		uint32 dont_root_before = 0;
		if (helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id, true, &dont_root_before))
			target_mob->SetDontRootMeBefore(dont_root_before);

		break;
	}

	helper_no_available_bots(c, my_bot);
}
