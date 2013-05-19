#ifndef EQEMU_LUA_GENERAL_H
#define EQEMU_LUA_GENERAL_H
#ifdef LUA_EQEMU

luabind::scope lua_register_general();
luabind::scope lua_register_events();

#endif
#endif