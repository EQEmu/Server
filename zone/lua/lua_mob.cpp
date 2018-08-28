#include <sol.hpp>
#include "../mob.h"

void RegisterMobTypesA(sol::simple_usertype<Mob> &type);
void RegisterMobTypesB(sol::simple_usertype<Mob> &type);
void RegisterMobTypesC(sol::simple_usertype<Mob> &type);

void lua_register_mob(sol::state *state) {
	//Because Mob is such a large usertype trying to pass everything at once is stressful on the template compiler (lots of memory / cpu)
	//(esp on windows which uses a 32bit compiler by default and often runs out of allocated heap trying to compile it)
	//So instead of creating/registering the usertype in one go we'll create a simple usertype and fill it in steps before registering it with lua
	auto mob = state->create_simple_usertype<Mob>(sol::base_classes, sol::bases<Entity>());
	RegisterMobTypesA(mob);
	RegisterMobTypesB(mob);
	RegisterMobTypesC(mob);

	state->set_usertype("Mob", mob);
}
