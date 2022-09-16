#ifdef LUA_EQEMU
#include "corpse.h"
#include "lua_corpse.h"
#include "lua_client.h"

uint32 Lua_Corpse::GetCharID() {
	Lua_Safe_Call_Int();
	return self->GetCharID();
}

uint32 Lua_Corpse::GetDecayTime() {
	Lua_Safe_Call_Int();
	return self->GetDecayTime();
}

void Lua_Corpse::Lock() {
	Lua_Safe_Call_Void();
	self->Lock();
}

void Lua_Corpse::UnLock() {
	Lua_Safe_Call_Void();
	self->UnLock();
}

bool Lua_Corpse::IsLocked() {
	Lua_Safe_Call_Bool();
	return self->IsLocked();
}

void Lua_Corpse::ResetLooter() {
	Lua_Safe_Call_Void();
	self->ResetLooter();
}

uint32 Lua_Corpse::GetDBID() {
	Lua_Safe_Call_Int();
	return self->GetCorpseDBID();
}

bool Lua_Corpse::IsRezzed() {
	Lua_Safe_Call_Bool();
	return self->IsRezzed();
}

const char* Lua_Corpse::GetOwnerName() {
	Lua_Safe_Call_String();
	return self->GetOwnerName();
}

bool Lua_Corpse::Save() {
	Lua_Safe_Call_Bool();
	return self->Save();
}

void Lua_Corpse::Delete() {
	Lua_Safe_Call_Void();
	self->Delete();
}

void Lua_Corpse::Bury() {
	Lua_Safe_Call_Void();
	self->Bury();
}

void Lua_Corpse::Depop() {
	Lua_Safe_Call_Void();
	self->Depop();
}

uint32 Lua_Corpse::CountItems() {
	Lua_Safe_Call_Int();
	return self->CountItems();
}

void Lua_Corpse::AddItem(uint32 itemnum, uint16 charges, int16 slot, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) {
	Lua_Safe_Call_Void();
	self->AddItem(itemnum, charges, slot, aug1, aug2, aug3, aug4, aug5);
}

uint32 Lua_Corpse::GetWornItem(int16 equipSlot) {
	Lua_Safe_Call_Int();
	return self->GetWornItem(equipSlot);
}

void Lua_Corpse::RemoveItem(uint16 lootslot) {
	Lua_Safe_Call_Void();
	self->RemoveItem(lootslot);
}

void Lua_Corpse::SetCash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->SetCash(copper, silver, gold, platinum);
}

void Lua_Corpse::RemoveCash() {
	Lua_Safe_Call_Void();
	self->RemoveCash();
}

bool Lua_Corpse::IsEmpty() {
	Lua_Safe_Call_Bool();
	return self->IsEmpty();
}

void Lua_Corpse::SetDecayTimer(uint32 decaytime) {
	Lua_Safe_Call_Void();
	self->SetDecayTimer(decaytime);
}

bool Lua_Corpse::CanMobLoot(int charid) {
	Lua_Safe_Call_Bool();
	return self->CanPlayerLoot(charid);
}

void Lua_Corpse::AllowMobLoot(Lua_Mob them, uint8 slot) {
	Lua_Safe_Call_Void();
	self->AllowPlayerLoot(them, slot);
}

bool Lua_Corpse::Summon(Lua_Client client, bool spell, bool checkdistance) {
	Lua_Safe_Call_Bool();
	return self->Summon(client, spell, checkdistance);
}

uint32 Lua_Corpse::GetCopper() {
	Lua_Safe_Call_Int();
	return self->GetCopper();
}

uint32 Lua_Corpse::GetSilver() {
	Lua_Safe_Call_Int();
	return self->GetSilver();
}

uint32 Lua_Corpse::GetGold() {
	Lua_Safe_Call_Int();
	return self->GetGold();
}

uint32 Lua_Corpse::GetPlatinum() {
	Lua_Safe_Call_Int();
	return self->GetPlatinum();
}

void Lua_Corpse::AddLooter(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->AddLooter(who);
}

bool Lua_Corpse::HasItem(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->HasItem(item_id);
}

uint16 Lua_Corpse::CountItem(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountItem(item_id);
}

uint32 Lua_Corpse::GetItemIDBySlot(uint16 loot_slot) {
	Lua_Safe_Call_Int();
	return self->GetItemIDBySlot(loot_slot);
}

uint16 Lua_Corpse::GetFirstSlotByItemID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetFirstSlotByItemID(item_id);
}

void Lua_Corpse::RemoveItemByID(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->RemoveItemByID(item_id);
}

void Lua_Corpse::RemoveItemByID(uint32 item_id, int quantity) {
	Lua_Safe_Call_Void();
	self->RemoveItemByID(item_id, quantity);	
}
#endif
