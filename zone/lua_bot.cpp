#ifdef BOTS
#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "bot.h"
#include "lua_bot.h"
#include "lua_mob.h"

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id);
}

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id, int16 charges) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id, charges);
}

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id, charges, attuned);
}

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id, charges, attuned, augment_one);
}

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id, charges, attuned, augment_one, augment_two);
}

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three);
}

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four);
}

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four, augment_five);
}

void Lua_Bot::AddItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five, uint32 augment_six) {
	Lua_Safe_Call_Void();
	self->AddItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six);
}

uint32 Lua_Bot::CountItem(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountItem(item_id);
}

Lua_Mob Lua_Bot::GetOwner() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetOwner());
}

bool Lua_Bot::HasItem(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->HasItem(item_id);
}

void Lua_Bot::RemoveItem(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id);
}

luabind::scope lua_register_bot() {
	return luabind::class_<Lua_Bot, Lua_Mob>("Bot")
	.def(luabind::constructor<>())
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32))&Lua_Bot::AddItem)
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32,int16))&Lua_Bot::AddItem)
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool))&Lua_Bot::AddItem)
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32))&Lua_Bot::AddItem)
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32))&Lua_Bot::AddItem)
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32))&Lua_Bot::AddItem)
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32))&Lua_Bot::AddItem)
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32,uint32))&Lua_Bot::AddItem)
	.def("AddItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32,uint32,uint32))&Lua_Bot::AddItem)
	.def("CountItem", (uint32(Lua_Bot::*)(uint32))&Lua_Bot::CountItem)
	.def("GetOwner", (Lua_Mob(Lua_Bot::*)(void))&Lua_Bot::GetOwner)
	.def("HasItem", (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasItem)
	.def("RemoveItem", (void(Lua_Bot::*)(uint32))&Lua_Bot::RemoveItem);
}

#endif
#endif
