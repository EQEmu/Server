#include "../bot_command.h"

void bot_command_cure(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Cure];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Cure) || helper_command_alias_fail(c, "bot_command_cure", sep->arg[0], "cure"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<friendly_target>) %s [ailment: blindness | disease | poison | curse | corruption]", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Cure);
		return;
	}

	std::string ailment_arg = sep->arg[1];

	auto ailment_type = BCEnum::AT_None;
	if (!ailment_arg.compare("blindness"))
		ailment_type = BCEnum::AT_Blindness;
	else if (!ailment_arg.compare("disease"))
		ailment_type = BCEnum::AT_Disease;
	else if (!ailment_arg.compare("poison"))
		ailment_type = BCEnum::AT_Poison;
	else if (!ailment_arg.compare("curse"))
		ailment_type = BCEnum::AT_Curse;
	else if (!ailment_arg.compare("corruption"))
		ailment_type = BCEnum::AT_Corruption;

	if (ailment_type == BCEnum::AT_None) {
		c->Message(Chat::White, "You must specify a cure [ailment] to use this command");
		return;
	}

	local_list->sort([ailment_type](STBaseEntry* l, STBaseEntry* r) {
		auto _l = l->SafeCastToCure(), _r = r->SafeCastToCure();
		if (_l->cure_value[AILMENTIDTOINDEX(ailment_type)] < _r->cure_value[AILMENTIDTOINDEX(ailment_type)])
			return true;
		if (_l->cure_value[AILMENTIDTOINDEX(ailment_type)] == _r->cure_value[AILMENTIDTOINDEX(ailment_type)] && spells[_l->spell_id].mana < spells[_r->spell_id].mana)
			return true;
		if (_l->cure_value[AILMENTIDTOINDEX(ailment_type)] == _r->cure_value[AILMENTIDTOINDEX(ailment_type)] && spells[_l->spell_id].mana == spells[_r->spell_id].mana && _l->cure_total < _r->cure_total)
			return true;

		return false;
	});

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToCure();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (!local_entry->cure_value[AILMENTIDTOINDEX(ailment_type)])
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
