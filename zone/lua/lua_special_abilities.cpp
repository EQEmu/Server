#include <sol.hpp>
#include "../mob.h"

void lua_register_special_abilities(sol::state *state) {
	state->create_named_table("SpecialAbility");
}
