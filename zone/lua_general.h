#ifndef EQEMU_LUA_GENERAL_H
#define EQEMU_LUA_GENERAL_H
#ifdef LUA_EQEMU

luabind::scope lua_register_general();
luabind::scope lua_register_random();
luabind::scope lua_register_events();
luabind::scope lua_register_faction();
luabind::scope lua_register_slot();
luabind::scope lua_register_material();
luabind::scope lua_register_client_version();
luabind::scope lua_register_appearance();
luabind::scope lua_register_classes();
luabind::scope lua_register_skills();
luabind::scope lua_register_bodytypes();
luabind::scope lua_register_filters();
luabind::scope lua_register_message_types();
luabind::scope lua_register_rules_const();
luabind::scope lua_register_rulei();
luabind::scope lua_register_ruler();
luabind::scope lua_register_ruleb();

#endif
#endif
