#include "../bot_command.h"

void bot_command_resurrect(Client *c, const Seperator *sep)
{
	// Obscure bot spell code prohibits the aoe portion from working correctly...

	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Resurrect];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Resurrect) || helper_command_alias_fail(c, "bot_command_resurrect", sep->arg[0], "resurrect"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		//c->Message(Chat::White, "usage: <corpse_target> %s ([option: aoe])", sep->arg[0]);
		c->Message(Chat::White, "usage: <corpse_target> %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Resurrect);
		return;
	}

	bool aoe = false;
	//std::string aoe_arg = sep->arg[1];
	//if (!aoe_arg.compare("aoe"))
	//	aoe = true;

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToResurrect();
		if (helper_spell_check_fail(local_entry))
			continue;
		//if (local_entry->aoe != aoe)
		//	continue;
		if (local_entry->aoe)
			continue;

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		//if (!target_mob && !local_entry->aoe)
		//	continue;
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		//if (local_entry->aoe)
		//	target_mob = my_bot;

		uint32 dont_root_before = 0;
		if (helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id, true, &dont_root_before))
			target_mob->SetDontRootMeBefore(dont_root_before);

		break;
	}

	helper_no_available_bots(c, my_bot);
}
