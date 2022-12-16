#ifdef BOTS
#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "bot.h"
#include "lua_bot.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_group.h"

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

void Lua_Bot::ApplySpell(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id);
}

void Lua_Bot::ApplySpell(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration);
}

void Lua_Bot::ApplySpell(int spell_id, int duration, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, ApplySpellType::Solo, allow_pets);
}

void Lua_Bot::ApplySpellGroup(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, 0, ApplySpellType::Group);
}

void Lua_Bot::ApplySpellGroup(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, ApplySpellType::Group);
}

void Lua_Bot::ApplySpellGroup(int spell_id, int duration, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, ApplySpellType::Group, allow_pets);
}

void Lua_Bot::SetSpellDuration(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id);
}

void Lua_Bot::SetSpellDuration(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration);
}

void Lua_Bot::SetSpellDuration(int spell_id, int duration, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, ApplySpellType::Solo, allow_pets);
}

void Lua_Bot::SetSpellDurationGroup(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, 0, ApplySpellType::Group);
}

void Lua_Bot::SetSpellDurationGroup(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, ApplySpellType::Group);
}

void Lua_Bot::SetSpellDurationGroup(int spell_id, int duration, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, ApplySpellType::Group, allow_pets);
}

int Lua_Bot::CountAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetInv().CountAugmentEquippedByID(item_id);
}

bool Lua_Bot::HasAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->GetInv().HasAugmentEquippedByID(item_id);
}

int Lua_Bot::CountItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetInv().CountItemEquippedByID(item_id);
}

bool Lua_Bot::HasItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->GetInv().HasItemEquippedByID(item_id);
}

void Lua_Bot::Escape() {
	Lua_Safe_Call_Void();
	self->Escape();
}

void Lua_Bot::Fling(float target_x, float target_y, float target_z) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, false, false, true);
}

void Lua_Bot::Fling(float target_x, float target_y, float target_z, bool ignore_los) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, ignore_los, false, true);
}

void Lua_Bot::Fling(float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, ignore_los, clip_through_walls, true);
}

void Lua_Bot::Fling(float value, float target_x, float target_y, float target_z) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z);
}

void Lua_Bot::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z, ignore_los);
}

void Lua_Bot::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z, ignore_los, clip_through_walls);
}

int Lua_Bot::GetItemIDAt(int slot_id) {
	Lua_Safe_Call_Int();
	return self->GetItemIDAt(slot_id);
}

int Lua_Bot::GetAugmentIDAt(int slot_id, int aug_slot) {
	Lua_Safe_Call_Int();
	return self->GetAugmentIDAt(slot_id, aug_slot);
}

int Lua_Bot::GetBaseSTR() {
	Lua_Safe_Call_Int();
	return self->GetBaseSTR();
}

int Lua_Bot::GetBaseSTA() {
	Lua_Safe_Call_Int();
	return self->GetBaseSTA();
}

int Lua_Bot::GetBaseCHA() {
	Lua_Safe_Call_Int();
	return self->GetBaseCHA();
}

int Lua_Bot::GetBaseDEX() {
	Lua_Safe_Call_Int();
	return self->GetBaseDEX();
}

int Lua_Bot::GetBaseINT() {
	Lua_Safe_Call_Int();
	return self->GetBaseINT();
}

int Lua_Bot::GetBaseAGI() {
	Lua_Safe_Call_Int();
	return self->GetBaseAGI();
}

int Lua_Bot::GetBaseWIS() {
	Lua_Safe_Call_Int();
	return self->GetBaseWIS();
}

Lua_Group Lua_Bot::GetGroup() {
	Lua_Safe_Call_Class(Lua_Group);
	return self->GetGroup();
}

int Lua_Bot::GetHealAmount() {
	Lua_Safe_Call_Int();
	return self->GetHealAmt();
}

int Lua_Bot::GetSpellDamage() {
	Lua_Safe_Call_Int();
	return self->GetSpellDmg();
}

int Lua_Bot::GetInstrumentMod(int spell_id) {
	Lua_Safe_Call_Int();
	return self->GetInstrumentMod(spell_id);
}

int Lua_Bot::GetRawItemAC() {
	Lua_Safe_Call_Int();
	return self->GetRawItemAC();
}

bool Lua_Bot::IsGrouped() {
	Lua_Safe_Call_Bool();
	return self->IsGrouped();
}

bool Lua_Bot::IsStanding() {
	Lua_Safe_Call_Bool();
	return self->IsStanding();
}

bool Lua_Bot::IsSitting() {
	Lua_Safe_Call_Bool();
	return self->IsSitting();
}

void Lua_Bot::Sit() {
	Lua_Safe_Call_Void();
	self->Sit();
}

void Lua_Bot::Stand() {
	Lua_Safe_Call_Void();
	self->Stand();
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
	.def("ApplySpell", (void(Lua_Bot::*)(int))&Lua_Bot::ApplySpell)
	.def("ApplySpell", (void(Lua_Bot::*)(int,int))&Lua_Bot::ApplySpell)
	.def("ApplySpell", (void(Lua_Bot::*)(int,int,bool))&Lua_Bot::ApplySpell)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int,int))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int,int,bool))&Lua_Bot::ApplySpellGroup)
	.def("CountBotItem", (uint32(Lua_Bot::*)(uint32))&Lua_Bot::CountBotItem)
	.def("CountItemEquippedByID", (int(Lua_Bot::*)(uint32))&Lua_Bot::CountItemEquippedByID)
	.def("Escape", (void(Lua_Bot::*)(void))&Lua_Bot::Escape)
	.def("Fling", (void(Lua_Bot::*)(float,float,float))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,bool,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float,bool,bool))&Lua_Bot::Fling)
	.def("GetAugmentIDAt", (int(Lua_Bot::*)(int,int))&Lua_Bot::GetAugmentIDAt)
	.def("GetBaseAGI", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseAGI)
	.def("GetBaseCHA", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseCHA)
	.def("GetBaseDEX", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseDEX)
	.def("GetBaseINT", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseINT)
	.def("GetBaseSTA", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseSTA)
	.def("GetBaseSTR", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseSTR)
	.def("GetBaseWIS", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseWIS)
	.def("GetBotItem", (Lua_ItemInst(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItem)
	.def("GetBotItemIDBySlot", (uint32(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItemIDBySlot)
	.def("GetExpansionBitmask", (int(Lua_Bot::*)(void))&Lua_Bot::GetExpansionBitmask)
	.def("GetGroup", (Lua_Group(Lua_Bot::*)(void))&Lua_Bot::GetGroup)
	.def("GetHealAmount", (int(Lua_Bot::*)(void))&Lua_Bot::GetHealAmount)
	.def("GetInstrumentMod", (int(Lua_Bot::*)(int))&Lua_Bot::GetInstrumentMod)
	.def("GetOwner", (Lua_Mob(Lua_Bot::*)(void))&Lua_Bot::GetOwner)
	.def("GetRawItemAC", (int(Lua_Bot::*)(void))&Lua_Bot::GetRawItemAC)
	.def("GetSpellDamage", (int(Lua_Bot::*)(void))&Lua_Bot::GetSpellDamage)
	.def("HasAugmentEquippedByID", (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasAugmentEquippedByID)
	.def("HasBotItem", (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasBotItem)
	.def("HasBotSpellEntry", (bool(Lua_Bot::*)(uint16)) & Lua_Bot::HasBotSpellEntry)
	.def("HasItemEquippedByID", (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasItemEquippedByID)
	.def("IsGrouped", (bool(Lua_Bot::*)(void))&Lua_Bot::IsGrouped)
	.def("IsSitting", (bool(Lua_Bot::*)(void))&Lua_Bot::IsSitting)
	.def("IsStanding", (bool(Lua_Bot::*)(void))&Lua_Bot::IsStanding)
	.def("OwnerMessage", (void(Lua_Bot::*)(std::string))&Lua_Bot::OwnerMessage)
	.def("ReloadBotDataBuckets", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotDataBuckets)
	.def("ReloadBotOwnerDataBuckets", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotOwnerDataBuckets)
	.def("ReloadBotSpells", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotSpells)
	.def("ReloadBotSpellSettings", (void(Lua_Bot::*)(void))&Lua_Bot::ReloadBotSpellSettings)
	.def("RemoveBotItem", (void(Lua_Bot::*)(uint32))&Lua_Bot::RemoveBotItem)
	.def("SetExpansionBitmask", (void(Lua_Bot::*)(int))&Lua_Bot::SetExpansionBitmask)
	.def("SetExpansionBitmask", (void(Lua_Bot::*)(int,bool))&Lua_Bot::SetExpansionBitmask)
	.def("SetSpellDuration", (void(Lua_Bot::*)(int))&Lua_Bot::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Bot::*)(int,int))&Lua_Bot::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Bot::*)(int,int,bool))&Lua_Bot::SetSpellDuration)
	.def("SetSpellDurationGroup", (void(Lua_Bot::*)(int))&Lua_Bot::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Bot::*)(int,int))&Lua_Bot::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Bot::*)(int,int,bool))&Lua_Bot::SetSpellDurationGroup)
	.def("SendPayload", (void(Lua_Bot::*)(int))&Lua_Bot::SendPayload)
	.def("SendPayload", (void(Lua_Bot::*)(int,std::string))&Lua_Bot::SendPayload)
	.def("Signal", (void(Lua_Bot::*)(int))&Lua_Bot::Signal)
	.def("Sit", (void(Lua_Bot::*)(void))&Lua_Bot::Sit)
	.def("Stand", (void(Lua_Bot::*)(void))&Lua_Bot::Stand);
}

#endif
#endif
