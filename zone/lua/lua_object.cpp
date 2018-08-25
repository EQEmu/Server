#include <sol.hpp>
#include "../object.h"

void lua_register_object(sol::state *state) {
	state->new_usertype<Object>("Object",
		sol::base_classes, sol::bases<Entity>()
		);
}
