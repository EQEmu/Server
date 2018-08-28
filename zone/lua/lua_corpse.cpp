#include <sol.hpp>
#include "../corpse.h"

void lua_register_corpse(sol::state *state) {
	auto corpse = state->create_simple_usertype<Corpse>(sol::base_classes, sol::bases<Mob, Entity>());

	state->set_usertype("Corpse", corpse);
}
