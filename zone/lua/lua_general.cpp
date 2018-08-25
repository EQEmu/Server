#include <sol.hpp>

void lua_register_general(sol::state *state) {
	auto table = state->create_named_table("eqemu");
}
