#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "bot.h"
#include "lua_bot.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_group.h"
#include "lua_item.h"

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

int16 Lua_Bot::HasBotItem(uint32 item_id) {
	Lua_Safe_Call_Int();
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
	return DataBucket::GetDataBuckets(self);
}

bool Lua_Bot::ReloadBotOwnerDataBuckets() {
	Lua_Safe_Call_Bool();
	return self->HasOwner() && DataBucket::GetDataBuckets(self->GetBotOwner());
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

uint32 Lua_Bot::GetBotID() {
	Lua_Safe_Call_Int();
	return self->GetBotID();
}

void Lua_Bot::Camp() {
	Lua_Safe_Call_Void();
	self->Camp();
}

void Lua_Bot::Camp(bool save_to_database) {
	Lua_Safe_Call_Void();
	self->Camp(save_to_database);
}

Lua_ItemInst Lua_Bot::GetAugmentAt(int16 slot_id, uint8 augment_index)
{
	Lua_Safe_Call_Class(Lua_ItemInst);

	auto* inst = self->GetInv().GetItem(slot_id);
	if (inst) {
		return Lua_ItemInst(inst->GetAugment(augment_index));
	}

	return Lua_ItemInst();
}

int Lua_Bot::GetAugmentIDAt(int16 slot_id, uint8 augment_index) {
	Lua_Safe_Call_Int();
	return self->GetAugmentIDAt(slot_id, augment_index);
}

int Lua_Bot::GetItemIDAt(int16 slot_id) {
	Lua_Safe_Call_Int();
	return self->GetItemIDAt(slot_id);
}

Lua_ItemInst Lua_Bot::GetItemAt(int16 slot_id) // @categories Inventory and Items
{
	Lua_Safe_Call_Class(Lua_ItemInst);
	return Lua_ItemInst(self->GetInv().GetItem(slot_id));
}

void Lua_Bot::SendSpellAnim(uint16 target_id, uint16 spell_id)
{
	Lua_Safe_Call_Void();
	self->SendSpellAnim(target_id, spell_id);
}

luabind::object Lua_Bot::GetAugmentIDsBySlotID(lua_State* L, int16 slot_id) const {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto augments = self->GetInv().GetAugmentIDsBySlotID(slot_id);
		int index = 1;
		for (auto item_id : augments) {
			lua_table[index] = item_id;
			index++;
		}
	}
	return lua_table;
}

void Lua_Bot::AddItem(const luabind::object& item_table) {
	Lua_Safe_Call_Void();
	if (luabind::type(item_table) != LUA_TTABLE) {
		return;
	}

	auto item_id = luabind::object_cast<uint32>(item_table["item_id"]);
	int16 charges = luabind::object_cast<uint32>(item_table["charges"]);
	uint32 augment_one = luabind::type(item_table["augment_one"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_one"]) : 0;
	uint32 augment_two = luabind::type(item_table["augment_two"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_two"]) : 0;
	uint32 augment_three = luabind::type(item_table["augment_three"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_three"]) : 0;
	uint32 augment_four = luabind::type(item_table["augment_four"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_four"]) : 0;
	uint32 augment_five = luabind::type(item_table["augment_five"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_five"]) : 0;
	uint32 augment_six = luabind::type(item_table["augment_six"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_six"]) : 0;
	bool attuned = luabind::type(item_table["attuned"]) != LUA_TNIL ? luabind::object_cast<bool>(item_table["attuned"]) : false;
	uint16 slot_id = luabind::type(item_table["slot_id"]) != LUA_TNIL ? luabind::object_cast<uint16>(item_table["slot_id"]) : EQ::invslot::slotCursor;

	if (slot_id <= EQ::invslot::slotAmmo) {
		self->AddBotItem(
			slot_id,
			item_id,
			charges,
			attuned,
			augment_one,
			augment_two,
			augment_three,
			augment_four,
			augment_five,
			augment_six
		);
	} else {
		self->GetOwner()->CastToClient()->SummonItem(
			item_id,
			charges,
			augment_one,
			augment_two,
			augment_three,
			augment_four,
			augment_five,
			augment_six,
			attuned,
			slot_id
		);
	}
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
	.def("AddItem", (void(Lua_Bot::*)(luabind::adl::object))&Lua_Bot::AddItem)
	.def("ApplySpell", (void(Lua_Bot::*)(int))&Lua_Bot::ApplySpell)
	.def("ApplySpell", (void(Lua_Bot::*)(int,int))&Lua_Bot::ApplySpell)
	.def("ApplySpell", (void(Lua_Bot::*)(int,int,bool))&Lua_Bot::ApplySpell)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int,int))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int,int,bool))&Lua_Bot::ApplySpellGroup)
	.def("Camp", (void(Lua_Bot::*)(void))&Lua_Bot::Camp)
	.def("Camp", (void(Lua_Bot::*)(bool))&Lua_Bot::Camp)
	.def("CountBotItem", (uint32(Lua_Bot::*)(uint32))&Lua_Bot::CountBotItem)
	.def("CountItemEquippedByID", (int(Lua_Bot::*)(uint32))&Lua_Bot::CountItemEquippedByID)
	.def("Escape", (void(Lua_Bot::*)(void))&Lua_Bot::Escape)
	.def("Fling", (void(Lua_Bot::*)(float,float,float))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,bool,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float,bool,bool))&Lua_Bot::Fling)
	.def("GetAugmentAt", (Lua_ItemInst(Lua_Bot::*)(int16,uint8))&Lua_Bot::GetAugmentAt)
	.def("GetAugmentIDAt", (int(Lua_Bot::*)(int16,uint8))&Lua_Bot::GetAugmentIDAt)
	.def("GetAugmentIDsBySlotID", (luabind::object(Lua_Bot::*)(lua_State* L,int16))&Lua_Bot::GetAugmentIDsBySlotID)
	.def("GetBaseAGI", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseAGI)
	.def("GetBaseCHA", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseCHA)
	.def("GetBaseDEX", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseDEX)
	.def("GetBaseINT", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseINT)
	.def("GetBaseSTA", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseSTA)
	.def("GetBaseSTR", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseSTR)
	.def("GetBaseWIS", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseWIS)
	.def("GetBotID", (uint32(Lua_Bot::*)(void))&Lua_Bot::GetBotID)
	.def("GetBotItem", (Lua_ItemInst(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItem)
	.def("GetBotItemIDBySlot", (uint32(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItemIDBySlot)
	.def("GetExpansionBitmask", (int(Lua_Bot::*)(void))&Lua_Bot::GetExpansionBitmask)
	.def("GetGroup", (Lua_Group(Lua_Bot::*)(void))&Lua_Bot::GetGroup)
	.def("GetHealAmount", (int(Lua_Bot::*)(void))&Lua_Bot::GetHealAmount)
	.def("GetInstrumentMod", (int(Lua_Bot::*)(int))&Lua_Bot::GetInstrumentMod)
	.def("GetItemAt", (Lua_ItemInst(Lua_Bot::*)(int16))&Lua_Bot::GetItemAt)
	.def("GetItemIDAt", (int(Lua_Bot::*)(int16))&Lua_Bot::GetItemIDAt)
	.def("GetOwner", (Lua_Mob(Lua_Bot::*)(void))&Lua_Bot::GetOwner)
	.def("GetRawItemAC", (int(Lua_Bot::*)(void))&Lua_Bot::GetRawItemAC)
	.def("GetSpellDamage", (int(Lua_Bot::*)(void))&Lua_Bot::GetSpellDamage)
	.def("HasAugmentEquippedByID", (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasAugmentEquippedByID)
	.def("HasBotItem", (int16(Lua_Bot::*)(uint32))&Lua_Bot::HasBotItem)
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
	.def("SendSpellAnim", (void(Lua_Bot::*)(uint16,uint16))&Lua_Bot::SendSpellAnim)
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
