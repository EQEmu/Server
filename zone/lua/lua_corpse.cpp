#include <sol.hpp>
#include "../corpse.h"

void lua_register_corpse(sol::state *state) {
	state->new_usertype<Corpse>("Corpse",
		sol::base_classes, sol::bases<Mob, Entity>()
	);
}
