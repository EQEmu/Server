#ifdef LUA_EQEMU

#include "masterentity.h"
#include "hate_list.h"
#include "lua_mob.h"
#include "lua_hate_list.h"

void lua_register_hate_entry(sol::state_view &sv)
{
	auto hate_entry = sv.new_usertype<Lua_HateEntry>("HateEntry");
	hate_entry["null"] = sol::readonly_property(&Lua_HateEntry::Null);
	hate_entry["valid"] = sol::readonly_property(&Lua_HateEntry::Valid);
	hate_entry["damage"] = sol::property(&Lua_HateEntry::GetDamage, &Lua_HateEntry::SetDamage);
	hate_entry["ent"] = sol::property(&Lua_HateEntry::GetEnt, &Lua_HateEntry::SetEnt);
	hate_entry["frenzy"] = sol::property(&Lua_HateEntry::GetFrenzy, &Lua_HateEntry::SetFrenzy);
	hate_entry["hate"] = sol::property(&Lua_HateEntry::GetHate, &Lua_HateEntry::SetHate);
}

void lua_register_hate_list(sol::state_view &sv)
{
	auto hate_list = sv.new_usertype<Lua_HateList>("HateList");
	hate_list["entries"] = sol::readonly_property(&Lua_HateList::get_entries);
}

#endif
