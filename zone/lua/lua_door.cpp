#include <sol.hpp>
#include "../doors.h"

void lua_register_doors(sol::state *state) {
	auto door = state->create_simple_usertype<Doors>(sol::base_classes, sol::bases<Entity>());
	
	state->set_usertype("Door", door);
}
