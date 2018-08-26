#include <sol.hpp>
#include "../common.h"

void lua_register_extra_attack_options(sol::state *state) {
	state->new_usertype<ExtraAttackOptions>("ExtraAttackOptions",
		sol::constructors<ExtraAttackOptions()>(),
		"damage_percent", &ExtraAttackOptions::damage_percent,
		"damage_flat", &ExtraAttackOptions::damage_flat,
		"armor_pen_percent", &ExtraAttackOptions::armor_pen_percent,
		"armor_pen_flat", &ExtraAttackOptions::armor_pen_flat,
		"crit_percent", &ExtraAttackOptions::crit_percent,
		"crit_flat", &ExtraAttackOptions::crit_flat,
		"hate_percent", &ExtraAttackOptions::hate_percent,
		"hate_flat", &ExtraAttackOptions::hate_flat,
		"hit_chance", &ExtraAttackOptions::hit_chance,
		"melee_damage_bonus_flat", &ExtraAttackOptions::melee_damage_bonus_flat,
		"skilldmgtaken_bonus_flat", &ExtraAttackOptions::skilldmgtaken_bonus_flat
	);
}
