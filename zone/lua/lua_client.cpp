#include <sol.hpp>
#include "../client.h"

void lua_register_client(sol::state *state) {
	auto client = state->create_simple_usertype<Client>(sol::base_classes, sol::bases<Mob, Entity>());

	state->set_usertype("Client", client);
}
