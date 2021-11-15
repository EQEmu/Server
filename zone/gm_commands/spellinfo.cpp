#include "../client.h"

void command_spellinfo(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #spellinfo [spell_id]");
	}
	else {
		short int                       spell_id = atoi(sep->arg[1]);
		const struct SPDat_Spell_Struct *s       = &spells[spell_id];
		c->Message(Chat::White, "Spell info for spell #%d:", spell_id);
		c->Message(Chat::White, "  name: %s", s->name);
		c->Message(Chat::White, "  player_1: %s", s->player_1);
		c->Message(Chat::White, "  teleport_zone: %s", s->teleport_zone);
		c->Message(Chat::White, "  you_cast: %s", s->you_cast);
		c->Message(Chat::White, "  other_casts: %s", s->other_casts);
		c->Message(Chat::White, "  cast_on_you: %s", s->cast_on_you);
		c->Message(Chat::White, "  spell_fades: %s", s->spell_fades);
		c->Message(Chat::White, "  range: %f", s->range);
		c->Message(Chat::White, "  aoe_range: %f", s->aoe_range);
		c->Message(Chat::White, "  push_back: %f", s->push_back);
		c->Message(Chat::White, "  push_up: %f", s->push_up);
		c->Message(Chat::White, "  cast_time: %d", s->cast_time);
		c->Message(Chat::White, "  recovery_time: %d", s->recovery_time);
		c->Message(Chat::White, "  recast_time: %d", s->recast_time);
		c->Message(Chat::White, "  buff_duration_formula: %d", s->buff_duration_formula);
		c->Message(Chat::White, "  buff_duration: %d", s->buff_duration);
		c->Message(Chat::White, "  AEDuration: %d", s->aoe_duration);
		c->Message(Chat::White, "  mana: %d", s->mana);
		c->Message(
			Chat::White,
			"  base[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			s->base_value[0],
			s->base_value[1],
			s->base_value[2],
			s->base_value[3],
			s->base_value[4],
			s->base_value[5],
			s->base_value[6],
			s->base_value[7],
			s->base_value[8],
			s->base_value[9],
			s->base_value[10],
			s->base_value[11]
		);
		c->Message(
			Chat::White,
			"  base22[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			s->limit_value[0],
			s->limit_value[1],
			s->limit_value[2],
			s->limit_value[3],
			s->limit_value[4],
			s->limit_value[5],
			s->limit_value[6],
			s->limit_value[7],
			s->limit_value[8],
			s->limit_value[9],
			s->limit_value[10],
			s->limit_value[11]
		);
		c->Message(
			Chat::White,
			"  max[12]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			s->max_value[0],
			s->max_value[1],
			s->max_value[2],
			s->max_value[3],
			s->max_value[4],
			s->max_value[5],
			s->max_value[6],
			s->max_value[7],
			s->max_value[8],
			s->max_value[9],
			s->max_value[10],
			s->max_value[11]
		);
		c->Message(
			Chat::White,
			"  components[4]: %d, %d, %d, %d",
			s->component[0],
			s->component[1],
			s->component[2],
			s->component[3]
		);
		c->Message(
			Chat::White,
			"  component_counts[4]: %d, %d, %d, %d",
			s->component_count[0],
			s->component_count[1],
			s->component_count[2],
			s->component_count[3]
		);
		c->Message(
			Chat::White,
			"  NoexpendReagent[4]: %d, %d, %d, %d",
			s->no_expend_reagent[0],
			s->no_expend_reagent[1],
			s->no_expend_reagent[2],
			s->no_expend_reagent[3]
		);
		c->Message(
			Chat::White,
			"  formula[12]: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
			s->formula[0],
			s->formula[1],
			s->formula[2],
			s->formula[3],
			s->formula[4],
			s->formula[5],
			s->formula[6],
			s->formula[7],
			s->formula[8],
			s->formula[9],
			s->formula[10],
			s->formula[11]
		);
		c->Message(Chat::White, "  goodEffect: %d", s->good_effect);
		c->Message(Chat::White, "  Activated: %d", s->activated);
		c->Message(Chat::White, "  resisttype: %d", s->resist_type);
		c->Message(
			Chat::White,
			"  effectid[12]: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
			s->effect_id[0],
			s->effect_id[1],
			s->effect_id[2],
			s->effect_id[3],
			s->effect_id[4],
			s->effect_id[5],
			s->effect_id[6],
			s->effect_id[7],
			s->effect_id[8],
			s->effect_id[9],
			s->effect_id[10],
			s->effect_id[11]
		);
		c->Message(Chat::White, "  targettype: %d", s->target_type);
		c->Message(Chat::White, "  basediff: %d", s->base_difficulty);
		c->Message(Chat::White, "  skill: %d", s->skill);
		c->Message(Chat::White, "  zonetype: %d", s->zone_type);
		c->Message(Chat::White, "  EnvironmentType: %d", s->environment_type);
		c->Message(Chat::White, "  TimeOfDay: %d", s->time_of_day);
		c->Message(
			Chat::White, "  classes[15]: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
			s->classes[0], s->classes[1], s->classes[2], s->classes[3], s->classes[4],
			s->classes[5], s->classes[6], s->classes[7], s->classes[8], s->classes[9],
			s->classes[10], s->classes[11], s->classes[12], s->classes[13], s->classes[14]
		);
		c->Message(Chat::White, "  CastingAnim: %d", s->casting_animation);
		c->Message(Chat::White, "  SpellAffectIndex: %d", s->spell_affect_index);
		c->Message(Chat::White, " RecourseLink: %d", s->recourse_link);
	}
}

