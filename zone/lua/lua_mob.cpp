#include <sol.hpp>
#include "../mob.h"

void lua_register_mob(sol::state *state) {
	state->new_usertype<Mob>("Mob",
		sol::base_classes, sol::bases<Entity>()
	);
}
