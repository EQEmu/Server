#include <sol.hpp>
#include "../npc.h"

void lua_register_npc(sol::state *state) {
	state->new_usertype<NPC>("NPC",
		sol::base_classes, sol::bases<Mob, Entity>()
	);
}
