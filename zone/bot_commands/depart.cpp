#include "../bot_command.h"

void bot_command_depart(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Depart];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Depart) || helper_command_alias_fail(c, "bot_command_depart", sep->arg[0], "depart"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [list | destination] ([option: single])", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Depart);
		return;
	}

	bool single = false;
	std::string single_arg = sep->arg[2];
	if (!single_arg.compare("single"))
		single = true;

	std::string destination = sep->arg[1];
	if (!destination.compare("list")) {
		Bot* my_druid_bot = ActionableBots::AsGroupMember_ByClass(c, c, Class::Druid);
		Bot* my_wizard_bot = ActionableBots::AsGroupMember_ByClass(c, c, Class::Wizard);
		helper_command_depart_list(c, my_druid_bot, my_wizard_bot, local_list, single);
		return;
	}
	else if (destination.empty()) {
		c->Message(Chat::White, "A [destination] or [list] argument is required to use this command");
		return;
	}

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToDepart();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (local_entry->single != single)
			continue;
		if (destination.compare(spells[local_entry->spell_id].teleport_zone))
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
