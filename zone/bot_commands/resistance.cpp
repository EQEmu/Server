#include "../bot_command.h"

void bot_command_resistance(Client *c, const Seperator *sep)
{
	bcst_list* local_list = &bot_command_spells[BCEnum::SpT_Resistance];
	if (helper_spell_list_fail(c, local_list, BCEnum::SpT_Resistance) || helper_command_alias_fail(c, "bot_command_resistance", sep->arg[0], "resistance"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: (<friendly_target>) %s [resistance: fire | cold | poison | disease | magic | corruption]", sep->arg[0]);
		helper_send_usage_required_bots(c, BCEnum::SpT_Resistance);
		return;
	}

	std::string resistance_arg = sep->arg[1];

	auto resistance_type = BCEnum::RT_None;
	if (!resistance_arg.compare("fire"))
		resistance_type = BCEnum::RT_Fire;
	else if (!resistance_arg.compare("cold"))
		resistance_type = BCEnum::RT_Cold;
	else if (!resistance_arg.compare("poison"))
		resistance_type = BCEnum::RT_Poison;
	else if (!resistance_arg.compare("disease"))
		resistance_type = BCEnum::RT_Disease;
	else if (!resistance_arg.compare("magic"))
		resistance_type = BCEnum::RT_Magic;
	else if (!resistance_arg.compare("corruption"))
		resistance_type = BCEnum::RT_Corruption;

	if (resistance_type == BCEnum::RT_None) {
		c->Message(Chat::White, "You must specify a [resistance]");
		return;
	}

	local_list->sort([resistance_type](STBaseEntry* l, STBaseEntry* r) {
		auto _l = l->SafeCastToResistance(), _r = r->SafeCastToResistance();
		if (_l->resist_value[RESISTANCEIDTOINDEX(resistance_type)] > _r->resist_value[RESISTANCEIDTOINDEX(resistance_type)])
			return true;
		if (_l->resist_value[RESISTANCEIDTOINDEX(resistance_type)] == _r->resist_value[RESISTANCEIDTOINDEX(resistance_type)] && spells[_l->spell_id].mana < spells[_r->spell_id].mana)
			return true;
		if (_l->resist_value[RESISTANCEIDTOINDEX(resistance_type)] == _r->resist_value[RESISTANCEIDTOINDEX(resistance_type)] && spells[_l->spell_id].mana == spells[_r->spell_id].mana && _l->resist_total > _r->resist_total)
			return true;

		return false;
	});

	ActionableTarget::Types actionable_targets;
	Bot* my_bot = nullptr;
	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	bool cast_success = false;
	for (auto list_iter : *local_list) {
		auto local_entry = list_iter->SafeCastToResistance();
		if (helper_spell_check_fail(local_entry))
			continue;
		if (!local_entry->resist_value[RESISTANCEIDTOINDEX(resistance_type)])
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
