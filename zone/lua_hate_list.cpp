#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include <luabind/iterator_policy.hpp>

#include "masterentity.h"
#include "hate_list.h"
#include "lua_mob.h"
#include "lua_hate_list.h"

Lua_Mob Lua_HateEntry::GetEnt() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->mMOB);
}

void Lua_HateEntry::SetEnt(Lua_Mob e) {
	Lua_Safe_Call_Void();
	self->mMOB = e;
}

int Lua_HateEntry::GetDamage() {
	Lua_Safe_Call_Int();
	return self->mDamage;
}

void Lua_HateEntry::SetDamage(int value) {
	Lua_Safe_Call_Void();
	self->mDamage = value;
}

int Lua_HateEntry::GetHate() {
	Lua_Safe_Call_Int();
	return self->mHate;
}

void Lua_HateEntry::SetHate(int value) {
	Lua_Safe_Call_Void();
	self->mHate = value;
}

int Lua_HateEntry::GetFrenzy() {
	Lua_Safe_Call_Int();
	return self->mFrenzy;
}

void Lua_HateEntry::SetFrenzy(bool value) {
	Lua_Safe_Call_Void();
	self->mFrenzy = value;
}

luabind::scope lua_register_hate_entry() {

	return luabind::class_<Lua_HateEntry>("HateEntry")
			.property("null", &Lua_HateEntry::Null)
			.property("valid", &Lua_HateEntry::Valid)
			.property("ent", &Lua_HateEntry::GetEnt, &Lua_HateEntry::SetEnt)
			.property("damage", &Lua_HateEntry::GetDamage, &Lua_HateEntry::SetDamage)
			.property("hate", &Lua_HateEntry::GetHate, &Lua_HateEntry::SetHate)
			.property("frenzy", &Lua_HateEntry::GetFrenzy, &Lua_HateEntry::SetFrenzy);
}

luabind::scope lua_register_hate_list() {
	return luabind::class_<Lua_HateList>("HateList")
			.def_readwrite("entries", &Lua_HateList::entries, luabind::return_stl_iterator);
}

#endif
