#include <sol.hpp>
#include "../client.h"

void lua_register_client(sol::state *state) {
	state->new_usertype<Client>("Client",
		sol::base_classes, sol::bases<Mob, Entity>()
	);
}
