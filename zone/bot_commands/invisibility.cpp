#include "../bot_command.h"

void bot_command_invisibility(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Invisibility];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Invisibility) || helper_command_alias_fail(c, "bot_command_invisibility", sep->arg[0], "invisibility"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<friendly_target>) %s [invisibility: living | undead | animal | see]", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Invisibility);
		return;
	}

	std::string invisibility = sep->arg[1];

	BCEnum::IType invisibility_type = BCEnum::IT_None;
	if (!invisibility.compare("living"))
		invisibility_type = BCEnum::IT_Living;
	else if (!invisibility.compare("undead"))
		invisibility_type = BCEnum::IT_Undead;
	else if (!invisibility.compare("animal"))
		invisibility_type = BCEnum::IT_Animal;
	else if (!invisibility.compare("see"))
		invisibility_type = BCEnum::IT_See;

	if (invisibility_type == BCEnum::IT_None) {
		c->Message(Chat::White, "You must specify an [invisibility]");
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToInvisibility();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->invis_type != invisibility_type)
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
