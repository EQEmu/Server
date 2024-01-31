#include "../bot_command.h"

void bot_command_summon_corpse(Client *c, const Seperator *sep)
{
	// Same methodology as old command..but, does not appear to work... (note: didn't work there, either...)

	// temp
	c->Message(Chat::White, "This command is currently unavailable...");
	return;

	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_SummonCorpse];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_SummonCorpse) || helper_command_alias_fail(c, "bot_command_summon_corpse", sep->arg[0], "summoncorpse"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: <friendly_target> %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_SummonCorpse);
		return;
	}

	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter;
		if (helper_spell_check_fail(local_entry))
			continue;

		auto target_mob = ActionableTarget::AsSingle_ByPlayer(c);
		if (!target_mob)
			continue;

		if (spells[local_entry->spell_id].base_value[EFFECTIDTOINDEX(1)] < target_mob->GetLevel())
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		cast_success = helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id);

		break;
	}

	helper_no_available_bots(c, my_bot);
}
