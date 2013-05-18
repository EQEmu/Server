#ifdef LUA_EQEMU

#include "masterentity.h"
#include "hate_list.h"
#include "lua_mob.h"
#include "lua_hate_entry.h"

Lua_Mob Lua_HateEntry::GetEnt() {
	Lua_Safe_Call_Mob();
	return Lua_Mob(self->ent);
}

void Lua_HateEntry::SetEnt(Lua_Mob e) {
	Lua_Safe_Call_Void();
	self->ent = e;
}

int Lua_HateEntry::GetDamage() {
	Lua_Safe_Call_Int();
	return self->damage;
}

void Lua_HateEntry::SetDamage(int value) {
	Lua_Safe_Call_Void();
	self->damage = value;
}

int Lua_HateEntry::GetHate() {
	Lua_Safe_Call_Int();
	return self->hate;
}

void Lua_HateEntry::SetHate(int value) {
	Lua_Safe_Call_Void();
	self->hate = value;
}

int Lua_HateEntry::GetFrenzy() {
	Lua_Safe_Call_Int();
	return self->bFrenzy;
}

void Lua_HateEntry::SetFrenzy(bool value) {
	Lua_Safe_Call_Void();
	self->bFrenzy = value;
}

#endif
