#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "masterentity.h"
#include "lua_iteminst.h"
#include "lua_item.h"

Lua_ItemInst::Lua_ItemInst(int item_id) {
	SetLuaPtrData(database.CreateItem(item_id));
	cloned_ = true;
}

Lua_ItemInst::Lua_ItemInst(int item_id, int charges) {
	SetLuaPtrData(database.CreateItem(item_id, charges));
	cloned_ = true;
}

Lua_ItemInst& Lua_ItemInst::operator=(const Lua_ItemInst& o) {
	if(o.cloned_) {
		cloned_ = true;
		d_ = new EQ::ItemInstance(*o.d_);
	} else {
		cloned_ = false;
		d_ = o.d_;
	}
	return *this;
}

Lua_ItemInst::Lua_ItemInst(const Lua_ItemInst& o) {
	if(o.cloned_) {
		cloned_ = true;
		d_ = new EQ::ItemInstance(*o.d_);
	} else {
		cloned_ = false;
		d_ = o.d_;
	}
}

bool Lua_ItemInst::IsType(int item_class) {
	Lua_Safe_Call_Bool();
	return self->IsType(static_cast<EQ::item::ItemClass>(item_class));
}

bool Lua_ItemInst::IsStackable() {
	Lua_Safe_Call_Bool();
	return self->IsStackable();
}

bool Lua_ItemInst::IsEquipable(int race, int class_) {
	Lua_Safe_Call_Bool();
	return self->IsEquipable(race, class_);
}

bool Lua_ItemInst::IsEquipable(int slot_id) {
	Lua_Safe_Call_Bool();
	return self->IsEquipable(slot_id);
}

bool Lua_ItemInst::IsAugmentable() {
	Lua_Safe_Call_Bool();
	return self->IsAugmentable();
}

int Lua_ItemInst::GetAugmentType() {
	Lua_Safe_Call_Int();
	return self->GetAugmentType();
}

bool Lua_ItemInst::IsExpendable() {
	Lua_Safe_Call_Bool();
	return self->IsExpendable();
}

Lua_ItemInst Lua_ItemInst::GetItem(int slot) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return Lua_ItemInst(self->GetItem(slot));
}

Lua_Item Lua_ItemInst::GetItem() {
	Lua_Safe_Call_Class(Lua_Item);
	return Lua_Item(self->GetItem());
}

Lua_Item Lua_ItemInst::GetUnscaledItem(int slot) {
	Lua_Safe_Call_Class(Lua_Item);
	return self->GetUnscaledItem();
}

uint32 Lua_ItemInst::GetItemID(int slot) {
	Lua_Safe_Call_Int();
	return self->GetItemID(slot);
}

int Lua_ItemInst::GetTotalItemCount() {
	Lua_Safe_Call_Int();
	return self->GetTotalItemCount();
}

Lua_ItemInst Lua_ItemInst::GetAugment(int slot) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return self->GetAugment(slot);
}

uint32 Lua_ItemInst::GetAugmentItemID(int slot) {
	Lua_Safe_Call_Int();
	return self->GetAugmentItemID(slot);
}

bool Lua_ItemInst::IsAugmented() {
	Lua_Safe_Call_Bool();
	return self->IsAugmented();
}

bool Lua_ItemInst::IsWeapon() {
	Lua_Safe_Call_Bool();
	return self->IsWeapon();
}

bool Lua_ItemInst::IsAmmo() {
	Lua_Safe_Call_Bool();
	return self->IsAmmo();
}

uint32 Lua_ItemInst::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

uint32 Lua_ItemInst::GetItemScriptID() {
	Lua_Safe_Call_Int();
	return self->GetItemScriptID();
}

int Lua_ItemInst::GetCharges() {
	Lua_Safe_Call_Int();
	return self->GetCharges();
}

void Lua_ItemInst::SetCharges(int charges) {
	Lua_Safe_Call_Void();
	return self->SetCharges(charges);
}

uint32 Lua_ItemInst::GetPrice() {
	Lua_Safe_Call_Int();
	return self->GetPrice();
}

void Lua_ItemInst::SetPrice(uint32 price) {
	Lua_Safe_Call_Void();
	return self->SetPrice(price);
}

void Lua_ItemInst::SetColor(uint32 color) {
	Lua_Safe_Call_Void();
	return self->SetColor(color);
}

uint32 Lua_ItemInst::GetColor() {
	Lua_Safe_Call_Int();
	return self->GetColor();
}

bool Lua_ItemInst::IsInstNoDrop() {
	Lua_Safe_Call_Bool();
	return self->IsAttuned();
}

void Lua_ItemInst::SetInstNoDrop(bool flag) {
	Lua_Safe_Call_Void();
	return self->SetAttuned(flag);
}

std::string Lua_ItemInst::GetCustomDataString() {
	Lua_Safe_Call_String();
	return self->GetCustomDataString();
}

void Lua_ItemInst::SetCustomData(std::string identifier, std::string value) {
	Lua_Safe_Call_Void();
	self->SetCustomData(identifier, value);
}

void Lua_ItemInst::SetCustomData(std::string identifier, int value) {
	Lua_Safe_Call_Void();
	self->SetCustomData(identifier, value);
}

void Lua_ItemInst::SetCustomData(std::string identifier, float value) {
	Lua_Safe_Call_Void();
	self->SetCustomData(identifier, value);
}

void Lua_ItemInst::SetCustomData(std::string identifier, bool value) {
	Lua_Safe_Call_Void();
	self->SetCustomData(identifier, value);
}

std::string Lua_ItemInst::GetCustomData(std::string identifier) {
	Lua_Safe_Call_String();
	return self->GetCustomData(identifier);
}

void Lua_ItemInst::DeleteCustomData(std::string identifier) {
	Lua_Safe_Call_Void();
	self->DeleteCustomData(identifier);
}

void Lua_ItemInst::SetScale(double scale_factor) {
	Lua_Safe_Call_Void();
	self->SetExp((int)(scale_factor*10000+.5));
}

void Lua_ItemInst::SetScaling(bool v) {
	Lua_Safe_Call_Void();
	self->SetScaling(v);
}

uint32 Lua_ItemInst::GetExp() {
	Lua_Safe_Call_Int();
	return self->GetExp();
}

void Lua_ItemInst::SetExp(uint32 exp) {
	Lua_Safe_Call_Void();
	self->SetExp(exp);
}

void Lua_ItemInst::AddExp(uint32 exp) {
	Lua_Safe_Call_Void();
	self->AddExp(exp);
}

int Lua_ItemInst::GetMaxEvolveLvl() {
	Lua_Safe_Call_Int();
	return self->GetMaxEvolveLvl();
}

uint32 Lua_ItemInst::GetKillsNeeded(int current_level) {
	Lua_Safe_Call_Int();
	return self->GetKillsNeeded(current_level);
}

Lua_ItemInst Lua_ItemInst::Clone() {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return Lua_ItemInst(self->Clone(), true);
}

void Lua_ItemInst::SetTimer(std::string name, uint32 time) {
	Lua_Safe_Call_Void();
	self->SetTimer(name, time);
}

void Lua_ItemInst::StopTimer(std::string name) {
	Lua_Safe_Call_Void();
	self->StopTimer(name);
}

void Lua_ItemInst::ClearTimers() {
	Lua_Safe_Call_Void();
	self->ClearTimers();
}

bool Lua_ItemInst::ContainsAugmentByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->ContainsAugmentByID(item_id);
}

int Lua_ItemInst::CountAugmentByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountAugmentByID(item_id);
}

luabind::scope lua_register_iteminst() {
	return luabind::class_<Lua_ItemInst>("ItemInst")
	.def(luabind::constructor<>())
	.def(luabind::constructor<int>())
	.def(luabind::constructor<int,int>())
	.property("null", &Lua_ItemInst::Null)
	.property("valid", &Lua_ItemInst::Valid)
	.def("AddExp", (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::AddExp)
	.def("ClearTimers", (void(Lua_ItemInst::*)(void))&Lua_ItemInst::ClearTimers)
	.def("Clone", (Lua_ItemInst(Lua_ItemInst::*)(void))&Lua_ItemInst::Clone)
	.def("ContainsAugmentByID", (bool(Lua_ItemInst::*)(uint32))&Lua_ItemInst::ContainsAugmentByID)
	.def("CountAugmentByID", (int(Lua_ItemInst::*)(uint32))&Lua_ItemInst::CountAugmentByID)
	.def("DeleteCustomData", (void(Lua_ItemInst::*)(std::string))&Lua_ItemInst::DeleteCustomData)
	.def("GetAugment", (Lua_ItemInst(Lua_ItemInst::*)(int))&Lua_ItemInst::GetAugment)
	.def("GetAugmentItemID", (uint32(Lua_ItemInst::*)(int))&Lua_ItemInst::GetAugmentItemID)
	.def("GetAugmentType", (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetAugmentType)
	.def("GetCharges", (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetCharges)
	.def("GetColor", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetColor)
	.def("GetCustomData", (std::string(Lua_ItemInst::*)(std::string))&Lua_ItemInst::GetCustomData)
	.def("GetCustomDataString", (std::string(Lua_ItemInst::*)(void))&Lua_ItemInst::GetCustomDataString)
	.def("GetExp", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetExp)
	.def("GetID", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetID)
	.def("GetItem", (Lua_Item(Lua_ItemInst::*)(void))&Lua_ItemInst::GetItem)
	.def("GetItem", (Lua_ItemInst(Lua_ItemInst::*)(int))&Lua_ItemInst::GetItem)
	.def("GetItemID", (uint32(Lua_ItemInst::*)(int))&Lua_ItemInst::GetItemID)
	.def("GetItemScriptID", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetItemScriptID)
	.def("GetKillsNeeded", (uint32(Lua_ItemInst::*)(int))&Lua_ItemInst::GetKillsNeeded)
	.def("GetMaxEvolveLvl", (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetMaxEvolveLvl)
	.def("GetPrice", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetPrice)
	.def("GetTotalItemCount", (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetTotalItemCount)
	.def("GetUnscaledItem", (Lua_ItemInst(Lua_ItemInst::*)(int))&Lua_ItemInst::GetUnscaledItem)
	.def("IsAmmo", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAmmo)
	.def("IsAugmentable", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAugmentable)
	.def("IsAugmented", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAugmented)
	.def("IsEquipable", (bool(Lua_ItemInst::*)(int))&Lua_ItemInst::IsEquipable)
	.def("IsEquipable", (bool(Lua_ItemInst::*)(int,int))&Lua_ItemInst::IsEquipable)
	.def("IsExpendable", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsExpendable)
	.def("IsInstNoDrop", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsInstNoDrop)
	.def("IsStackable", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsStackable)
	.def("IsType", (bool(Lua_ItemInst::*)(int))&Lua_ItemInst::IsType)
	.def("IsWeapon", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsWeapon)
	.def("SetCharges", (void(Lua_ItemInst::*)(int))&Lua_ItemInst::SetCharges)
	.def("SetColor", (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetColor)
	.def("SetCustomData", (void(Lua_ItemInst::*)(std::string,bool))&Lua_ItemInst::SetCustomData)
	.def("SetCustomData", (void(Lua_ItemInst::*)(std::string,float))&Lua_ItemInst::SetCustomData)
	.def("SetCustomData", (void(Lua_ItemInst::*)(std::string,int))&Lua_ItemInst::SetCustomData)
	.def("SetCustomData", (void(Lua_ItemInst::*)(std::string,std::string))&Lua_ItemInst::SetCustomData)
	.def("SetExp", (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetExp)
	.def("SetInstNoDrop", (void(Lua_ItemInst::*)(bool))&Lua_ItemInst::SetInstNoDrop)
	.def("SetPrice", (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetPrice)
	.def("SetScale", (void(Lua_ItemInst::*)(double))&Lua_ItemInst::SetScale)
	.def("SetScaling", (void(Lua_ItemInst::*)(bool))&Lua_ItemInst::SetScaling)
	.def("SetTimer", (void(Lua_ItemInst::*)(std::string,uint32))&Lua_ItemInst::SetTimer)
	.def("StopTimer", (void(Lua_ItemInst::*)(std::string))&Lua_ItemInst::StopTimer);
}

#endif
