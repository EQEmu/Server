#ifdef LUA_EQEMU
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
#endif
