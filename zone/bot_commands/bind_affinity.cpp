#include "../bot_command.h"

void bot_command_bind_affinity(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_BindAffinity];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_BindAffinity) || helper_command_alias_fail(c, "bot_command_bind_affinity", sep->arg[0], "bindaffinity"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<friendly_target>) %s", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_BindAffinity);
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

		auto target_mob = actionable_targets.Select(c, local_entry->target_type, FRIENDLY);
		if (!target_mob)
			continue;

		my_bot = ActionableBots::Select_ByMinLevelAndClass(c, local_entry->target_type, sbl, local_entry->spell_level, local_entry->caster_class, target_mob);
		if (!my_bot)
			continue;

		// Cast effect message is not being generated
		if (helper_cast_standard_spell(my_bot, target_mob, local_entry->spell_id))
			c->Message(Chat::White, "Successfully bound %s to this location", target_mob->GetCleanName());
		else
			c->Message(Chat::White, "Failed to bind %s to this location", target_mob->GetCleanName());
		break;
	}

	helper_no_available_bots(c, my_bot);
}
