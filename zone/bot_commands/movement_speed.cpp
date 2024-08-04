#include "../bot_command.h"

void bot_command_movement_speed(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_MovementSpeed];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_MovementSpeed) || helper_command_alias_fail(c, "bot_command_movement_speed", sep->arg[0], "movementspeed"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<friendly_target>) %s ([group | sow])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_MovementSpeed);
		return;
	}

	bool group = false;
	bool sow = false;
	std::string arg1 = sep->arg[1];
	if (!arg1.compare("group"))
		group = true;
	else if (!arg1.compare("sow"))
		sow = true;

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToMovementSpeed();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (!sow && (local_entry->group != group))
			continue;
		if (sow && (local_entry->spell_id != 278)) // '278' = single-target "Spirit of Wolf"
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
