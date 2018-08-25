#include <sol.hpp>
#include "../doors.h"

void lua_register_doors(sol::state *state) {
	state->new_usertype<Doors>("Doors",
		sol::base_classes, sol::bases<Entity>()
	);
}
