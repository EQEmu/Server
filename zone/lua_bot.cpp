#ifdef BOTS
#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "bot.h"
#include "lua_bot.h"
#include "lua_iteminst.h"
#include "lua_mob.h"

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four, augment_five);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five, uint32 augment_six) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six);
}

uint32 Lua_Bot::CountBotItem(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountBotItem(item_id);
}

Lua_Mob Lua_Bot::GetOwner() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetOwner());
}

bool Lua_Bot::HasBotItem(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->HasBotItem(item_id);
}

void Lua_Bot::RemoveBotItem(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->RemoveBotItem(item_id);
}

Lua_ItemInst Lua_Bot::GetBotItem(uint16 slot_id) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return self->GetBotItem(slot_id);
}

uint32 Lua_Bot::GetBotItemIDBySlot(uint16 slot_id) {
	Lua_Safe_Call_Int();
	return self->GetBotItemBySlot(slot_id);
}

void Lua_Bot::Signal(int signal_id) {
	Lua_Safe_Call_Void();
	self->Signal(signal_id);
}

void Lua_Bot::OwnerMessage(std::string message) {
	Lua_Safe_Call_Void();
	self->OwnerMessage(message);
}

int Lua_Bot::GetExpansionBitmask() {
	Lua_Safe_Call_Int();
	return self->GetExpansionBitmask();
}

void Lua_Bot::SetExpansionBitmask(int expansion_bitmask) {
	Lua_Safe_Call_Void();
	self->SetExpansionBitmask(expansion_bitmask);
}

void Lua_Bot::SetExpansionBitmask(int expansion_bitmask, bool save) {
	Lua_Safe_Call_Void();
	self->SetExpansionBitmask(expansion_bitmask, save);
}

bool Lua_Bot::ReloadBotDataBuckets() {
	Lua_Safe_Call_Bool();
	return self->GetBotDataBuckets();
}

bool Lua_Bot::ReloadBotOwnerDataBuckets() {
	Lua_Safe_Call_Bool();
	return self->GetBotOwnerDataBuckets();
}

bool Lua_Bot::ReloadBotSpells() {
	Lua_Safe_Call_Bool();
	return self->AI_AddBotSpells(self->GetBotSpellID());
}

void Lua_Bot::ReloadBotSpellSettings() {
	Lua_Safe_Call_Void();
	self->LoadBotSpellSettings();
}

bool Lua_Bot::HasBotSpellEntry(uint16 spellid) {
	Lua_Safe_Call_Bool();
	return self->HasBotSpellEntry(spellid);
}

void Lua_Bot::SendPayload(int payload_id) {
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id);
}

void Lua_Bot::SendPayload(int payload_id, std::string payload_value) {
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id, payload_value);
}

luabind::scope lua_register_bot() {
	return luabind::class_<Lua_Bot, Lua_Mob>("Bot")
	.def(luabind::constructor<>())
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("CountBotItem", (uint32(Lua_Bot::*)(uint32))&Lua_Bot::CountBotItem)
	.def("GetBotItem", (Lua_ItemInst(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItem)
	.def("GetBotItemIDBySlot", (uint32(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItemIDBySlot)
	.def("GetExpansionBitmask", (int(Lua_Bot::*)(void))&Lua_Bot::GetExpansionBitmask)
	.def("GetOwner", (Lua_Mob(Lua_Bot::*)(void))&Lua_Bot::GetOwner)
	.def("HasBotItem", (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasBotItem)
	.def("HasBotSpellEntry", (bool(Lua_Bot::*)(uint16)) & Lua_Bot::HasBotSpellEntry)
	.def("OwnerMessage", (void(Lua_Bot::*)(std::string))&Lua_Bot::OwnerMessage)
	.def("ReloadBotDataBuckets", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotDataBuckets)
	.def("ReloadBotOwnerDataBuckets", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotOwnerDataBuckets)
	.def("ReloadBotSpells", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotSpells)
	.def("ReloadBotSpellSettings", (void(Lua_Bot::*)(void))&Lua_Bot::ReloadBotSpellSettings)
	.def("RemoveBotItem", (void(Lua_Bot::*)(uint32))&Lua_Bot::RemoveBotItem)
	.def("SetExpansionBitmask", (void(Lua_Bot::*)(int))&Lua_Bot::SetExpansionBitmask)
	.def("SetExpansionBitmask", (void(Lua_Bot::*)(int,bool))&Lua_Bot::SetExpansionBitmask)
	.def("SendPayload", (void(Lua_Bot::*)(int))&Lua_Bot::SendPayload)
	.def("SendPayload", (void(Lua_Bot::*)(int,std::string))&Lua_Bot::SendPayload)
	.def("Signal", (void(Lua_Bot::*)(int))&Lua_Bot::Signal);
}

#endif
#endif
