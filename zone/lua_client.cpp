#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "client.h"
#include "lua_client.h"
#include "lua_npc.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_inventory.h"
#include "lua_group.h"
#include "lua_raid.h"
#include "lua_packet.h"

struct InventoryWhere { };

void Lua_Client::SendSound() {
	Lua_Safe_Call_Void();
	self->SendSound();
}

void Lua_Client::Save() {
	Lua_Safe_Call_Void();
	self->Save();
}

void Lua_Client::Save(int commit_now) {
	Lua_Safe_Call_Void();
	self->Save(commit_now);
}

void Lua_Client::SaveBackup() {
	Lua_Safe_Call_Void();
	self->SaveBackup();
}

bool Lua_Client::Connected() {
	Lua_Safe_Call_Bool();
	return self->Connected();
}

bool Lua_Client::InZone() {
	Lua_Safe_Call_Bool();
	return self->InZone();
}

void Lua_Client::Kick() {
	Lua_Safe_Call_Void();
	self->Kick("Lua Quest");
}

void Lua_Client::Disconnect() {
	Lua_Safe_Call_Void();
	self->Disconnect();
}

bool Lua_Client::IsLD() {
	Lua_Safe_Call_Bool();
	return self->IsLD();
}

void Lua_Client::WorldKick() {
	Lua_Safe_Call_Void();
	self->WorldKick();
}

bool Lua_Client::GetAnon() {
	Lua_Safe_Call_Bool();
	return self->GetAnon() != 0;
}

void Lua_Client::Duck() {
	Lua_Safe_Call_Void();
	self->Duck();
}

void Lua_Client::Stand() {
	Lua_Safe_Call_Void();
	self->Stand();
}

void Lua_Client::SetGM(bool v) {
	Lua_Safe_Call_Void();
	self->SetGM(v);
}

void Lua_Client::SetPVP(bool v) {
	Lua_Safe_Call_Void();
	self->SetPVP(v);
}

bool Lua_Client::GetPVP() {
	Lua_Safe_Call_Bool();
	return self->GetPVP();
}

bool Lua_Client::GetGM() {
	Lua_Safe_Call_Bool();
	return self->GetGM();
}

void Lua_Client::SetBaseClass(int v) {
	Lua_Safe_Call_Void();
	self->SetBaseClass(v);
}

void Lua_Client::SetBaseRace(int v) {
	Lua_Safe_Call_Void();
	self->SetBaseRace(v);
}

void Lua_Client::SetBaseGender(int v) {
	Lua_Safe_Call_Void();
	self->SetBaseGender(v);
}

int Lua_Client::GetBaseFace() {
	Lua_Safe_Call_Int();
	return self->GetBaseFace();
}

int Lua_Client::GetLanguageSkill(int skill_id) {
	Lua_Safe_Call_Int();
	return self->GetLanguageSkill(skill_id);
}

const char *Lua_Client::GetLastName() {
	Lua_Safe_Call_String();
	return self->GetLastName();
}

int Lua_Client::GetLDoNPointsTheme(int theme) {
	Lua_Safe_Call_Int();
	return self->GetLDoNPointsTheme(theme);
}

int Lua_Client::GetBaseSTR() {
	Lua_Safe_Call_Int();
	return self->GetBaseSTR();
}

int Lua_Client::GetBaseSTA() {
	Lua_Safe_Call_Int();
	return self->GetBaseSTA();
}

int Lua_Client::GetBaseCHA() {
	Lua_Safe_Call_Int();
	return self->GetBaseCHA();
}

int Lua_Client::GetBaseDEX() {
	Lua_Safe_Call_Int();
	return self->GetBaseDEX();
}

int Lua_Client::GetBaseINT() {
	Lua_Safe_Call_Int();
	return self->GetBaseINT();
}

int Lua_Client::GetBaseAGI() {
	Lua_Safe_Call_Int();
	return self->GetBaseAGI();
}

int Lua_Client::GetBaseWIS() {
	Lua_Safe_Call_Int();
	return self->GetBaseWIS();
}

int Lua_Client::GetWeight() {
	Lua_Safe_Call_Int();
	return self->GetWeight();
}

uint32 Lua_Client::GetEXP() {
	Lua_Safe_Call_Int();
	return self->GetEXP();
}

uint32 Lua_Client::GetAAExp() {
	Lua_Safe_Call_Int();
	return self->GetAAXP();
}

uint32 Lua_Client::GetAAPercent() {
	Lua_Safe_Call_Int();
	return self->GetAAPercent();
}

uint32 Lua_Client::GetTotalSecondsPlayed() {
	Lua_Safe_Call_Int();
	return self->GetTotalSecondsPlayed();
}

void Lua_Client::UpdateLDoNPoints(int points, uint32 theme) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNPoints(points, theme);
}

void Lua_Client::SetDeity(int v) {
	Lua_Safe_Call_Void();
	self->SetDeity(v);
}

void Lua_Client::AddEXP(uint32 add_exp) {
	Lua_Safe_Call_Void();
	self->AddEXP(add_exp);
}

void Lua_Client::AddEXP(uint32 add_exp, int conlevel) {
	Lua_Safe_Call_Void();
	self->AddEXP(add_exp, conlevel);
}

void Lua_Client::AddEXP(uint32 add_exp, int conlevel, bool resexp) {
	Lua_Safe_Call_Void();
	self->AddEXP(add_exp, conlevel, resexp);
}

void Lua_Client::SetEXP(uint32 set_exp, uint32 set_aaxp) {
	Lua_Safe_Call_Void();
	self->SetEXP(set_exp, set_aaxp);
}

void Lua_Client::SetEXP(uint32 set_exp, uint32 set_aaxp, bool resexp) {
	Lua_Safe_Call_Void();
	self->SetEXP(set_exp, set_aaxp, resexp);
}

void Lua_Client::SetBindPoint() {
	Lua_Safe_Call_Void();
	self->SetBindPoint();
}

void Lua_Client::SetBindPoint(int to_zone) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone);
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone, to_instance);
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance, float new_x) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone, to_instance, glm::vec3(new_x,0.0f,0.0f));
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance, float new_x, float new_y) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone, to_instance, glm::vec3(new_x, new_y, 0.0f));
}

void Lua_Client::SetBindPoint(int to_zone, int to_instance, float new_x, float new_y, float new_z) {
	Lua_Safe_Call_Void();
	self->SetBindPoint(0, to_zone, to_instance, glm::vec3(new_x, new_y, new_z));
}

float Lua_Client::GetBindX() {
	Lua_Safe_Call_Real();
	return self->GetBindX();
}

float Lua_Client::GetBindX(int index) {
	Lua_Safe_Call_Real();
	return self->GetBindX(index);
}

float Lua_Client::GetBindY() {
	Lua_Safe_Call_Real();
	return self->GetBindY();
}

float Lua_Client::GetBindY(int index) {
	Lua_Safe_Call_Real();
	return self->GetBindY(index);
}

float Lua_Client::GetBindZ() {
	Lua_Safe_Call_Real();
	return self->GetBindZ();
}

float Lua_Client::GetBindZ(int index) {
	Lua_Safe_Call_Real();
	return self->GetBindZ(index);
}

float Lua_Client::GetBindHeading() {
	Lua_Safe_Call_Real();
	return self->GetBindHeading();
}

float Lua_Client::GetBindHeading(int index) {
	Lua_Safe_Call_Real();
	return self->GetBindHeading(index);
}

uint32 Lua_Client::GetBindZoneID() {
	Lua_Safe_Call_Int();
	return self->GetBindZoneID();
}

uint32 Lua_Client::GetBindZoneID(int index) {
	Lua_Safe_Call_Int();
	return self->GetBindZoneID(index);
}

void Lua_Client::MovePC(int zone, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MovePC(zone, x, y, z, heading);
}

void Lua_Client::MovePCInstance(int zone, int instance, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MovePC(zone, instance, x, y, z, heading);
}

void Lua_Client::ChangeLastName(const char *in) {
	Lua_Safe_Call_Void();
	self->ChangeLastName(in);
}

int Lua_Client::GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 race, uint32 class_, uint32 deity, uint32 faction, Lua_NPC npc) {
	Lua_Safe_Call_Int();
	return static_cast<int>(self->GetFactionLevel(char_id, npc_id, race, class_, deity, faction, npc));
}

void Lua_Client::SetFactionLevel(uint32 char_id, uint32 npc_id, int char_class, int char_race, int char_deity) {
	Lua_Safe_Call_Void();
	self->SetFactionLevel(char_id, npc_id, char_class, char_race, char_deity);
}

void Lua_Client::SetFactionLevel2(uint32 char_id, int faction_id, int char_class, int char_race, int char_deity, int value, int temp) {
	Lua_Safe_Call_Void();
	self->SetFactionLevel2(char_id, faction_id, char_class, char_race, char_deity, value, temp);
}

int Lua_Client::GetRawItemAC() {
	Lua_Safe_Call_Int();
	return self->GetRawItemAC();
}

uint32 Lua_Client::AccountID() {
	Lua_Safe_Call_Int();
	return self->AccountID();
}

const char *Lua_Client::AccountName() {
	Lua_Safe_Call_String();
	return self->AccountName();
}

int Lua_Client::GetAccountAge() {
	Lua_Safe_Call_Int();
	return time(nullptr) - self->GetAccountCreation();
}

int Lua_Client::Admin() {
	Lua_Safe_Call_Bool();
	return self->Admin();
}

uint32 Lua_Client::CharacterID() {
	Lua_Safe_Call_Int();
	return self->CharacterID();
}

int Lua_Client::GuildRank() {
	Lua_Safe_Call_Int();
	return self->GuildRank();
}

uint32 Lua_Client::GuildID() {
	Lua_Safe_Call_Int();
	return self->GuildID();
}

int Lua_Client::GetFace() {
	Lua_Safe_Call_Int();
	return self->GetFace();
}

bool Lua_Client::TakeMoneyFromPP(uint64 copper) {
	Lua_Safe_Call_Bool();
	return self->TakeMoneyFromPP(copper);
}

bool Lua_Client::TakeMoneyFromPP(uint64 copper, bool update_client) {
	Lua_Safe_Call_Bool();
	return self->TakeMoneyFromPP(copper, update_client);
}

void Lua_Client::AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, bool update_client) {
	Lua_Safe_Call_Void();
	self->AddMoneyToPP(copper, silver, gold, platinum, update_client);
}

bool Lua_Client::TGB() {
	Lua_Safe_Call_Bool();
	return self->TGB();
}

int Lua_Client::GetSkillPoints() {
	Lua_Safe_Call_Int();
	return self->GetSkillPoints();
}

void Lua_Client::SetSkillPoints(int skill) {
	Lua_Safe_Call_Void();
	self->SetSkillPoints(skill);
}

void Lua_Client::IncreaseSkill(int skill_id) {
	Lua_Safe_Call_Void();
	self->IncreaseSkill(skill_id);
}

void Lua_Client::IncreaseSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->IncreaseSkill(skill_id, value);
}

void Lua_Client::IncreaseLanguageSkill(int skill_id) {
	Lua_Safe_Call_Void();
	self->IncreaseLanguageSkill(skill_id);
}

void Lua_Client::IncreaseLanguageSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->IncreaseLanguageSkill(skill_id, value);
}

int Lua_Client::GetRawSkill(int skill_id) {
	Lua_Safe_Call_Int();
	return self->GetRawSkill(static_cast<EQEmu::skills::SkillType>(skill_id));
}

bool Lua_Client::HasSkill(int skill_id) {
	Lua_Safe_Call_Bool();
	return self->HasSkill(static_cast<EQEmu::skills::SkillType>(skill_id));
}

bool Lua_Client::CanHaveSkill(int skill_id) {
	Lua_Safe_Call_Bool();
	return self->CanHaveSkill(static_cast<EQEmu::skills::SkillType>(skill_id));
}

void Lua_Client::SetSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->SetSkill(static_cast<EQEmu::skills::SkillType>(skill_id), value);
}

void Lua_Client::AddSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->AddSkill(static_cast<EQEmu::skills::SkillType>(skill_id), value);
}

void Lua_Client::CheckSpecializeIncrease(int spell_id) {
	Lua_Safe_Call_Void();
	self->CheckSpecializeIncrease(spell_id);
}

void Lua_Client::CheckIncreaseSkill(int skill_id, Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->CheckIncreaseSkill(static_cast<EQEmu::skills::SkillType>(skill_id), target);
}

void Lua_Client::CheckIncreaseSkill(int skill_id, Lua_Mob target, int chance_mod) {
	Lua_Safe_Call_Void();
	self->CheckIncreaseSkill(static_cast<EQEmu::skills::SkillType>(skill_id), target, chance_mod);
}

void Lua_Client::SetLanguageSkill(int language, int value) {
	Lua_Safe_Call_Void();
	self->SetLanguageSkill(language, value);
}

int Lua_Client::MaxSkill(int skill_id) {
	Lua_Safe_Call_Int();
	return self->MaxSkill(static_cast<EQEmu::skills::SkillType>(skill_id));
}

bool Lua_Client::IsMedding() {
	Lua_Safe_Call_Bool();
	return self->IsMedding();
}

int Lua_Client::GetDuelTarget() {
	Lua_Safe_Call_Int();
	return self->GetDuelTarget();
}

bool Lua_Client::IsDueling() {
	Lua_Safe_Call_Bool();
	return self->IsDueling();
}

void Lua_Client::SetDuelTarget(int c) {
	Lua_Safe_Call_Void();
	self->SetDuelTarget(c);
}

void Lua_Client::SetDueling(bool v) {
	Lua_Safe_Call_Void();
	self->SetDueling(v);
}

void Lua_Client::ResetAA() {
	Lua_Safe_Call_Void();
	self->ResetAA();
}

void Lua_Client::MemSpell(int spell_id, int slot) {
	Lua_Safe_Call_Void();
	self->MemSpell(spell_id, slot);
}

void Lua_Client::MemSpell(int spell_id, int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->MemSpell(spell_id, slot, update_client);
}

void Lua_Client::UnmemSpell(int slot) {
	Lua_Safe_Call_Void();
	self->UnmemSpell(slot);
}

void Lua_Client::UnmemSpell(int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->UnmemSpell(slot, update_client);
}

void Lua_Client::UnmemSpellBySpellID(int32 spell_id) {
	Lua_Safe_Call_Void();
	self->UnmemSpellBySpellID(spell_id);
}

void Lua_Client::UnmemSpellAll() {
	Lua_Safe_Call_Void();
	self->UnmemSpellAll();
}

void Lua_Client::UnmemSpellAll(bool update_client) {
	Lua_Safe_Call_Void();
	self->UnmemSpellAll(update_client);
}

uint16 Lua_Client::FindMemmedSpellBySlot(int slot) {
	Lua_Safe_Call_Int();
	return self->FindMemmedSpellBySlot(slot);
}

int Lua_Client::MemmedCount() {
	Lua_Safe_Call_Int();
	return self->MemmedCount();
}

void Lua_Client::ScribeSpell(int spell_id, int slot) {
	Lua_Safe_Call_Void();
	self->ScribeSpell(spell_id, slot);
}

void Lua_Client::ScribeSpell(int spell_id, int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->ScribeSpell(spell_id, slot, update_client);
}

void Lua_Client::UnscribeSpell(int slot) {
	Lua_Safe_Call_Void();
	self->UnscribeSpell(slot);
}

void Lua_Client::UnscribeSpell(int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->UnscribeSpell(slot, update_client);
}

void Lua_Client::UnscribeSpellAll() {
	Lua_Safe_Call_Void();
	self->UnscribeSpellAll();
}

void Lua_Client::UnscribeSpellAll(bool update_client) {
	Lua_Safe_Call_Void();
	self->UnscribeSpellAll(update_client);
}

void Lua_Client::TrainDisc(int itemid) {
	Lua_Safe_Call_Void();
	self->TrainDiscipline(itemid);
}

void Lua_Client::TrainDiscBySpellID(int32 spell_id) {
	Lua_Safe_Call_Void();
	self->TrainDiscBySpellID(spell_id);
}

int Lua_Client::GetDiscSlotBySpellID(int32 spell_id) {
	Lua_Safe_Call_Int();
	return self->GetDiscSlotBySpellID(spell_id);
}

void Lua_Client::UntrainDisc(int slot) {
	Lua_Safe_Call_Void();
	self->UntrainDisc(slot);
}

void Lua_Client::UntrainDisc(int slot, bool update_client) {
	Lua_Safe_Call_Void();
	self->UntrainDisc(slot, update_client);
}

void Lua_Client::UntrainDiscAll() {
	Lua_Safe_Call_Void();
	self->UntrainDiscAll();
}

void Lua_Client::UntrainDiscAll(bool update_client) {
	Lua_Safe_Call_Void();
	self->UntrainDiscAll(update_client);
}

bool Lua_Client::IsStanding() {
	Lua_Safe_Call_Bool();
	return self->IsStanding();
}

bool Lua_Client::IsSitting() {
	Lua_Safe_Call_Bool();
	return self->IsSitting();
}

bool Lua_Client::IsCrouching() {
	Lua_Safe_Call_Bool();
	return self->IsCrouching();
}

void Lua_Client::SetFeigned(bool v) {
	Lua_Safe_Call_Void();
	self->SetFeigned(v);
}

bool Lua_Client::GetFeigned() {
	Lua_Safe_Call_Bool();
	return self->GetFeigned();
}

bool Lua_Client::AutoSplitEnabled() {
	Lua_Safe_Call_Bool();
	return self->AutoSplitEnabled();
}

void Lua_Client::SetHorseId(int id) {
	Lua_Safe_Call_Void();
	self->SetHorseId(id);
}

int Lua_Client::GetHorseId() {
	Lua_Safe_Call_Int();
	return self->GetHorseId();
}

void Lua_Client::NukeItem(uint32 item_num) {
	Lua_Safe_Call_Void();
	self->NukeItem(item_num, 0xFF);
}

void Lua_Client::NukeItem(uint32 item_num, int where_to_check) {
	Lua_Safe_Call_Void();
	self->NukeItem(item_num, where_to_check);
}

void Lua_Client::SetTint(int slot_id, uint32 color) {
	Lua_Safe_Call_Void();
	self->SetTint(slot_id, color);
}

void Lua_Client::SetMaterial(int slot_id, uint32 item_id) {
	Lua_Safe_Call_Void();
	self->SetMaterial(slot_id, item_id);
}

void Lua_Client::Undye() {
	Lua_Safe_Call_Void();
	self->Undye();
}

int Lua_Client::GetItemIDAt(int slot_id) {
	Lua_Safe_Call_Int();
	return self->GetItemIDAt(slot_id);
}

int Lua_Client::GetAugmentIDAt(int slot_id, int aug_slot) {
	Lua_Safe_Call_Int();
	return self->GetAugmentIDAt(slot_id, aug_slot);
}

void Lua_Client::DeleteItemInInventory(int slot_id, int quantity) {
	Lua_Safe_Call_Void();
	self->DeleteItemInInventory(slot_id, quantity);
}

void Lua_Client::DeleteItemInInventory(int slot_id, int quantity, bool update_client) {
	Lua_Safe_Call_Void();
	self->DeleteItemInInventory(slot_id, quantity, update_client);
}

void Lua_Client::SummonItem(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id);
}

void Lua_Client::SummonItem(uint32 item_id, int charges) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, aug5);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, bool attuned) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, aug5, 0, attuned);
}

void Lua_Client::SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, bool attuned, int to_slot) {
	Lua_Safe_Call_Void();
	self->SummonItem(item_id, charges, aug1, aug2, aug3, aug4, aug5, 0, attuned, to_slot);
}

void Lua_Client::SetStats(int type, int value) {
	Lua_Safe_Call_Void();
	self->SetStats(type, value);
}

void Lua_Client::IncStats(int type, int value) {
	Lua_Safe_Call_Void();
	self->IncStats(type, value);
}

void Lua_Client::DropItem(int slot_id) {
	Lua_Safe_Call_Void();
	self->DropItem(slot_id);
}

void Lua_Client::BreakInvis() {
	Lua_Safe_Call_Void();
	self->BreakInvis();
}

void Lua_Client::LeaveGroup() {
	Lua_Safe_Call_Void();
	self->LeaveGroup();
}

bool Lua_Client::IsGrouped() {
	Lua_Safe_Call_Bool();
	return self->IsGrouped();
}

bool Lua_Client::IsRaidGrouped() {
	Lua_Safe_Call_Bool();
	return self->IsRaidGrouped();
}

bool Lua_Client::Hungry() {
	Lua_Safe_Call_Bool();
	return self->Hungry();
}

bool Lua_Client::Thirsty() {
	Lua_Safe_Call_Bool();
	return self->Thirsty();
}

int Lua_Client::GetInstrumentMod(int spell_id) {
	Lua_Safe_Call_Int();
	return self->GetInstrumentMod(spell_id);
}

bool Lua_Client::DecreaseByID(uint32 type, int amt) {
	Lua_Safe_Call_Bool();
	return self->DecreaseByID(type, amt);
}

void Lua_Client::Escape() {
	Lua_Safe_Call_Void();
	self->Escape();
}

void Lua_Client::GoFish() {
	Lua_Safe_Call_Void();
	self->GoFish();
}

void Lua_Client::ForageItem() {
	Lua_Safe_Call_Void();
	self->ForageItem();
}

void Lua_Client::ForageItem(bool guarantee) {
	Lua_Safe_Call_Void();
	self->ForageItem(guarantee);
}

float Lua_Client::CalcPriceMod(Lua_Mob other, bool reverse) {
	Lua_Safe_Call_Real();
	return self->CalcPriceMod(other, reverse);
}

void Lua_Client::ResetTrade() {
	Lua_Safe_Call_Void();
	self->ResetTrade();
}

bool Lua_Client::UseDiscipline(int spell_id, int target_id) {
	Lua_Safe_Call_Bool();
	return self->UseDiscipline(spell_id, target_id);
}

int Lua_Client::GetCharacterFactionLevel(int faction_id) {
	Lua_Safe_Call_Int();
	return self->GetCharacterFactionLevel(faction_id);
}

void Lua_Client::SetZoneFlag(int zone_id) {
	Lua_Safe_Call_Void();
	self->SetZoneFlag(zone_id);
}

void Lua_Client::ClearZoneFlag(int zone_id) {
	Lua_Safe_Call_Void();
	self->ClearZoneFlag(zone_id);
}

bool Lua_Client::HasZoneFlag(int zone_id) {
	Lua_Safe_Call_Bool();
	return self->HasZoneFlag(zone_id);
}

void Lua_Client::SendZoneFlagInfo(Lua_Client to) {
	Lua_Safe_Call_Void();
	self->SendZoneFlagInfo(to);
}

void Lua_Client::SetAATitle(const char *title) {
	Lua_Safe_Call_Void();
	self->SetAATitle(title);
}

int Lua_Client::GetClientVersion() {
	Lua_Safe_Call_Int();
	return static_cast<unsigned int>(self->ClientVersion());
}

uint32 Lua_Client::GetClientVersionBit() {
	Lua_Safe_Call_Int();
	return self->ClientVersionBit();
}

void Lua_Client::SetTitleSuffix(const char *text) {
	Lua_Safe_Call_Void();
	self->SetTitleSuffix(text);
}

void Lua_Client::SetAAPoints(int points) {
	Lua_Safe_Call_Void();
	self->SetAAPoints(points);
}

int Lua_Client::GetAAPoints() {
	Lua_Safe_Call_Int();
	return self->GetAAPoints();
}

int Lua_Client::GetSpentAA() {
	Lua_Safe_Call_Int();
	return self->GetSpentAA();
}

void Lua_Client::AddAAPoints(int points) {
	Lua_Safe_Call_Void();
	self->AddAAPoints(points);
}

void Lua_Client::RefundAA() {
	Lua_Safe_Call_Void();
	self->RefundAA();
}

int Lua_Client::GetModCharacterFactionLevel(int faction) {
	Lua_Safe_Call_Int();
	return self->GetModCharacterFactionLevel(faction);
}

int Lua_Client::GetLDoNWins() {
	Lua_Safe_Call_Int();
	return self->GetLDoNWins();
}

int Lua_Client::GetLDoNLosses() {
	Lua_Safe_Call_Int();
	return self->GetLDoNLosses();
}

int Lua_Client::GetLDoNWinsTheme(int theme) {
	Lua_Safe_Call_Int();
	return self->GetLDoNWinsTheme(theme);
}

int Lua_Client::GetLDoNLossesTheme(int theme) {
	Lua_Safe_Call_Int();
	return self->GetLDoNLossesTheme(theme);
}

int Lua_Client::GetStartZone() {
	Lua_Safe_Call_Int();
	return self->GetStartZone();
}

void Lua_Client::SetStartZone(int zone_id) {
	Lua_Safe_Call_Void();
	self->SetStartZone(zone_id);
}

void Lua_Client::SetStartZone(int zone_id, float x) {
	Lua_Safe_Call_Void();
	self->SetStartZone(zone_id, x);
}

void Lua_Client::SetStartZone(int zone_id, float x, float y) {
	Lua_Safe_Call_Void();
	self->SetStartZone(zone_id, x, y);
}

void Lua_Client::SetStartZone(int zone_id, float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->SetStartZone(zone_id, x, y, z);
}

void Lua_Client::KeyRingAdd(uint32 item) {
	Lua_Safe_Call_Void();
	self->KeyRingAdd(item);
}

bool Lua_Client::KeyRingCheck(uint32 item) {
	Lua_Safe_Call_Bool();
	return self->KeyRingCheck(item);
}

void Lua_Client::AddPVPPoints(uint32 points) {
	Lua_Safe_Call_Void();
	self->AddPVPPoints(points);
}

void Lua_Client::AddCrystals(uint32 radiant, uint32 ebon) {
	Lua_Safe_Call_Void();
	self->AddCrystals(radiant, ebon);
}

uint32 Lua_Client::GetPVPPoints() {
	Lua_Safe_Call_Int();
	return self->GetPVPPoints();
}

uint32 Lua_Client::GetRadiantCrystals() {
	Lua_Safe_Call_Int();
	return self->GetRadiantCrystals();
}

uint32 Lua_Client::GetEbonCrystals() {
	Lua_Safe_Call_Int();
	return self->GetEbonCrystals();
}

void Lua_Client::QuestReadBook(const char *text, int type) {
	Lua_Safe_Call_Void();
	self->QuestReadBook(text, type);
}

void Lua_Client::UpdateGroupAAs(int points, uint32 type) {
	Lua_Safe_Call_Void();
	self->UpdateGroupAAs(points, type);
}

uint32 Lua_Client::GetGroupPoints() {
	Lua_Safe_Call_Int();
	return self->GetGroupPoints();
}

uint32 Lua_Client::GetRaidPoints() {
	Lua_Safe_Call_Int();
	return self->GetRaidPoints();
}

void Lua_Client::LearnRecipe(uint32 recipe) {
	Lua_Safe_Call_Void();
	self->LearnRecipe(recipe);
}

int Lua_Client::GetEndurance() {
	Lua_Safe_Call_Int();
	return self->GetEndurance();
}

int Lua_Client::GetMaxEndurance() {
	Lua_Safe_Call_Int();
	return self->GetMaxEndurance();
}

int Lua_Client::GetEndurancePercent() {
	Lua_Safe_Call_Int();
	return self->GetEndurancePercent();
}

void Lua_Client::SetEndurance(int endur) {
	Lua_Safe_Call_Void();
	self->SetEndurance(endur);
}

void Lua_Client::SendOPTranslocateConfirm(Lua_Mob caster, int spell_id) {
	Lua_Safe_Call_Void();
	self->SendOPTranslocateConfirm(caster, spell_id);
}

uint32 Lua_Client::GetIP() {
	Lua_Safe_Call_Int();
	return self->GetIP();
}

void Lua_Client::AddLevelBasedExp(int exp_pct) {
	Lua_Safe_Call_Void();
	self->AddLevelBasedExp(exp_pct);
}

void Lua_Client::AddLevelBasedExp(int exp_pct, int max_level) {
	Lua_Safe_Call_Void();
	self->AddLevelBasedExp(exp_pct, max_level);
}

void Lua_Client::AddLevelBasedExp(int exp_pct, int max_level, bool ignore_mods) {
	Lua_Safe_Call_Void();
	self->AddLevelBasedExp(exp_pct, max_level, ignore_mods);
}

void Lua_Client::IncrementAA(int aa) {
	Lua_Safe_Call_Void();
	self->IncrementAlternateAdvancementRank(aa);
}

bool Lua_Client::GrantAlternateAdvancementAbility(int aa_id, int points) {
	Lua_Safe_Call_Bool();
	return self->GrantAlternateAdvancementAbility(aa_id, points);
}

bool Lua_Client::GrantAlternateAdvancementAbility(int aa_id, int points, bool ignore_cost) {
	Lua_Safe_Call_Bool();
	return self->GrantAlternateAdvancementAbility(aa_id, points, ignore_cost);
}

void Lua_Client::MarkSingleCompassLoc(float in_x, float in_y, float in_z) {
	Lua_Safe_Call_Void();
	self->MarkSingleCompassLoc(in_x, in_y, in_z);
}

void Lua_Client::MarkSingleCompassLoc(float in_x, float in_y, float in_z, int count) {
	Lua_Safe_Call_Void();
	self->MarkSingleCompassLoc(in_x, in_y, in_z, count);
}

void Lua_Client::ClearCompassMark() {
	Lua_Safe_Call_Void();
	self->MarkSingleCompassLoc(0,0,0,0);
}

int Lua_Client::GetNextAvailableSpellBookSlot() {
	Lua_Safe_Call_Int();
	return self->GetNextAvailableSpellBookSlot();
}

int Lua_Client::GetNextAvailableSpellBookSlot(int start) {
	Lua_Safe_Call_Int();
	return self->GetNextAvailableSpellBookSlot(start);
}

int Lua_Client::FindSpellBookSlotBySpellID(int spell_id) {
	Lua_Safe_Call_Int();
	return self->FindSpellBookSlotBySpellID(spell_id);
}

void Lua_Client::UpdateTaskActivity(int task, int activity, int count) {
	Lua_Safe_Call_Void();
	self->UpdateTaskActivity(task, activity, count);
}

void Lua_Client::AssignTask(int task, int npc_id) {
	Lua_Safe_Call_Void();
	self->AssignTask(task, npc_id);
}

void Lua_Client::AssignTask(int task, int npc_id, bool enforce_level_requirement) {
	Lua_Safe_Call_Void();
	self->AssignTask(task, npc_id, enforce_level_requirement);
}

void Lua_Client::FailTask(int task) {
	Lua_Safe_Call_Void();
	self->FailTask(task);
}

bool Lua_Client::IsTaskCompleted(int task) {
	Lua_Safe_Call_Bool();
	return self->IsTaskCompleted(task) != 0;
}

bool Lua_Client::IsTaskActive(int task) {
	Lua_Safe_Call_Bool();
	return self->IsTaskActive(task);
}

bool Lua_Client::IsTaskActivityActive(int task, int activity) {
	Lua_Safe_Call_Bool();
	return self->IsTaskActivityActive(task, activity);
}

int Lua_Client::GetCorpseCount() {
	Lua_Safe_Call_Int();
	return self->GetCorpseCount();
}

int Lua_Client::GetCorpseID(int corpse) {
	Lua_Safe_Call_Int();
	return self->GetCorpseID(corpse);
}

int Lua_Client::GetCorpseItemAt(int corpse, int slot) {
	Lua_Safe_Call_Int();
	return self->GetCorpseItemAt(corpse, slot);
}

void Lua_Client::AssignToInstance(int instance_id) {
	Lua_Safe_Call_Void();
	self->AssignToInstance(instance_id);
}

void Lua_Client::Freeze() {
	Lua_Safe_Call_Void();
	self->SendAppearancePacket(AT_Anim, ANIM_FREEZE);
}

void Lua_Client::UnFreeze() {
	Lua_Safe_Call_Void();
	self->SendAppearancePacket(AT_Anim, ANIM_STAND);
}

int Lua_Client::GetAggroCount() {
	Lua_Safe_Call_Int();
	return self->GetAggroCount();
}

uint64 Lua_Client::GetCarriedMoney() {
	Lua_Safe_Call_Int();
	return self->GetCarriedMoney();
}

uint64 Lua_Client::GetAllMoney() {
	Lua_Safe_Call_Int();
	return self->GetAllMoney();
}

uint32 Lua_Client::GetMoney(uint8 type, uint8 subtype) {
	Lua_Safe_Call_Int();
	return self->GetMoney(type, subtype);
}

void Lua_Client::OpenLFGuildWindow() {
	Lua_Safe_Call_Void();
	self->OpenLFGuildWindow();
}

void Lua_Client::Signal(uint32 id) {
	Lua_Safe_Call_Void();
	self->Signal(id);
}

void Lua_Client::AddAlternateCurrencyValue(uint32 currency, int amount) {
	Lua_Safe_Call_Void();
	self->AddAlternateCurrencyValue(currency, amount, 1);
}

void Lua_Client::SetAlternateCurrencyValue(uint32 currency, int amount) {
	Lua_Safe_Call_Void();
	self->SetAlternateCurrencyValue(currency, amount);
}

int Lua_Client::GetAlternateCurrencyValue(uint32 currency) {
	Lua_Safe_Call_Int();
	return self->GetAlternateCurrencyValue(currency);
}

void Lua_Client::SendWebLink(const char *site) {
	Lua_Safe_Call_Void();
	self->SendWebLink(site);
}

bool Lua_Client::HasSpellScribed(int spell_id) {
	Lua_Safe_Call_Bool();
	return self->HasSpellScribed(spell_id);
}

void Lua_Client::SetAccountFlag(std::string flag, std::string val) {
	Lua_Safe_Call_Void();
	self->SetAccountFlag(flag, val);
}

std::string Lua_Client::GetAccountFlag(std::string flag) {
	Lua_Safe_Call_String();
	return self->GetAccountFlag(flag);
}

Lua_Group Lua_Client::GetGroup() {
	Lua_Safe_Call_Class(Lua_Group);
	return self->GetGroup();
}

Lua_Raid Lua_Client::GetRaid() {
	Lua_Safe_Call_Class(Lua_Raid);
	return self->GetRaid();
}

bool Lua_Client::PutItemInInventory(int slot_id, Lua_ItemInst inst) {
	Lua_Safe_Call_Bool();
	EQEmu::ItemInstance *rinst = inst;
	return self->PutItemInInventory(slot_id, *rinst, true);
}

bool Lua_Client::PushItemOnCursor(Lua_ItemInst inst) {
	Lua_Safe_Call_Bool();
	EQEmu::ItemInstance *rinst = inst;
	return self->PushItemOnCursor(*rinst, true);
}

Lua_Inventory Lua_Client::GetInventory() {
	Lua_Safe_Call_Class(Lua_Inventory);
	return &self->GetInv();
}

void Lua_Client::SendItemScale(Lua_ItemInst inst) {
	Lua_Safe_Call_Void();
	self->SendItemScale(inst);
}

void Lua_Client::QueuePacket(Lua_Packet app) {
	Lua_Safe_Call_Void();
	self->QueuePacket(app);
}

void Lua_Client::QueuePacket(Lua_Packet app, bool ack_req) {
	Lua_Safe_Call_Void();
	self->QueuePacket(app, ack_req);
}

void Lua_Client::QueuePacket(Lua_Packet app, bool ack_req, int client_connection_status) {
	Lua_Safe_Call_Void();
	self->QueuePacket(app, ack_req, static_cast<Mob::CLIENT_CONN_STATUS>(client_connection_status));
}

void Lua_Client::QueuePacket(Lua_Packet app, bool ack_req, int client_connection_status, int filter) {
	Lua_Safe_Call_Void();
	self->QueuePacket(app, ack_req, static_cast<Mob::CLIENT_CONN_STATUS>(client_connection_status), static_cast<eqFilterType>(filter));
}

int Lua_Client::GetHunger() {
	Lua_Safe_Call_Int();
	return self->GetHunger();
}

int Lua_Client::GetThirst() {
	Lua_Safe_Call_Int();
	return self->GetThirst();
}

void Lua_Client::SetHunger(int in_hunger) {
	Lua_Safe_Call_Void();
	self->SetHunger(in_hunger);
}

void Lua_Client::SetThirst(int in_thirst) {
	Lua_Safe_Call_Void();
	self->SetThirst(in_thirst);
}

void Lua_Client::SetConsumption(int in_hunger, int in_thirst) {
	Lua_Safe_Call_Void();
	self->SetConsumption(in_hunger, in_thirst);
}

void Lua_Client::SendMarqueeMessage(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string msg) {
	Lua_Safe_Call_Void();
	self->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, msg);
}

void Lua_Client::SendColoredText(uint32 type, std::string msg) {
	Lua_Safe_Call_Void();
	self->SendColoredText(type, msg);
}

void Lua_Client::PlayMP3(std::string file)
{
	Lua_Safe_Call_Void();
	self->PlayMP3(file.c_str());
}

void Lua_Client::QuestReward(Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->QuestReward(target);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold, platinum);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 itemid) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold, platinum, itemid);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 itemid, uint32 exp) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold, platinum, itemid, exp);
}

void Lua_Client::QuestReward(Lua_Mob target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 itemid, uint32 exp, bool faction) {
	Lua_Safe_Call_Void();
	self->QuestReward(target, copper, silver, gold, platinum, itemid, exp, faction);
}

void Lua_Client::QuestReward(Lua_Mob target, luabind::adl::object reward) {
	Lua_Safe_Call_Void();

	if (luabind::type(reward) != LUA_TTABLE) {
		return;
	}

	QuestReward_Struct quest_reward;
	quest_reward.mob_id = 0;
	quest_reward.target_id = self->GetID();
	quest_reward.copper = 0;
	quest_reward.silver = 0;
	quest_reward.gold = 0;
	quest_reward.platinum = 0;
	quest_reward.exp_reward = 0;
	quest_reward.faction = 0;
	quest_reward.faction_mod = 0;
	bool faction = false;
	std::fill(std::begin(quest_reward.item_id), std::end(quest_reward.item_id), -1);

	auto cur = reward["copper"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			quest_reward.copper = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = reward["silver"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			quest_reward.silver = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = reward["gold"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			quest_reward.gold = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = reward["platinum"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			quest_reward.platinum = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = reward["itemid"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			quest_reward.item_id[0] = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	// if you define both an itemid and items table, the itemid is thrown away
	// should we error?
	cur = reward["items"];
	if (luabind::type(cur) == LUA_TTABLE) {
		try {
			// assume they defined a compatible table
			for (int i = 1; i <= QUESTREWARD_COUNT; ++i) {
				auto item = cur[i];
				int cur_value = -1;
				if (luabind::type(item) != LUA_TNIL) {
					try {
						cur_value = luabind::object_cast<uint32>(item);
					} catch (luabind::cast_failed &) {
					}
				} else {
					break;
				}
				quest_reward.item_id[i - 1] = cur_value;
			}
		} catch (luabind::cast_failed &) {
		}
	}

	cur = reward["exp"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			quest_reward.exp_reward = luabind::object_cast<uint32>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	cur = reward["faction"];
	if (luabind::type(cur) != LUA_TNIL) {
		try {
			faction = luabind::object_cast<bool>(cur);
		} catch (luabind::cast_failed &) {
		}
	}

	self->QuestReward(target, quest_reward, faction);
}

bool Lua_Client::IsDead() {
	Lua_Safe_Call_Bool();
	return self->IsDead();
}

int Lua_Client::CalcCurrentWeight() {
	Lua_Safe_Call_Int();
	return self->CalcCurrentWeight();
}

int Lua_Client::CalcATK() {
	Lua_Safe_Call_Int();
	return self->CalcATK();
}

void Lua_Client::FilteredMessage(Mob *sender, uint32 type, int filter, const char *message)
{
	Lua_Safe_Call_Void();
	self->FilteredMessage(sender, type, (eqFilterType)filter, message);
}

void Lua_Client::EnableAreaHPRegen(int value)
{
	Lua_Safe_Call_Void();
	self->EnableAreaHPRegen(value);
}

void Lua_Client::DisableAreaHPRegen()
{
	Lua_Safe_Call_Void();
	self->DisableAreaHPRegen();
}

void Lua_Client::EnableAreaManaRegen(int value)
{
	Lua_Safe_Call_Void();
	self->EnableAreaManaRegen(value);
}

void Lua_Client::DisableAreaManaRegen()
{
	Lua_Safe_Call_Void();
	self->DisableAreaManaRegen();
}

void Lua_Client::EnableAreaEndRegen(int value)
{
	Lua_Safe_Call_Void();
	self->EnableAreaEndRegen(value);
}

void Lua_Client::DisableAreaEndRegen()
{
	Lua_Safe_Call_Void();
	self->DisableAreaEndRegen();
}

void Lua_Client::EnableAreaRegens(int value)
{
	Lua_Safe_Call_Void();
	self->EnableAreaRegens(value);
}

void Lua_Client::DisableAreaRegens()
{
	Lua_Safe_Call_Void();
	self->DisableAreaRegens();
}

void Lua_Client::SetPrimaryWeaponOrnamentation(uint32 model_id) {
	Lua_Safe_Call_Void();
	self->SetPrimaryWeaponOrnamentation(model_id);
}

void Lua_Client::SetSecondaryWeaponOrnamentation(uint32 model_id) {
	Lua_Safe_Call_Void();
	self->SetSecondaryWeaponOrnamentation(model_id);
}

void Lua_Client::SetClientMaxLevel(int value) {
	Lua_Safe_Call_Void();
	self->SetClientMaxLevel(value);
}

int Lua_Client::GetClientMaxLevel() {
	Lua_Safe_Call_Int();
	return self->GetClientMaxLevel();
}



luabind::scope lua_register_client() {
	return luabind::class_<Lua_Client, Lua_Mob>("Client")
		.def(luabind::constructor<>())
		.def("SendSound", (void(Lua_Client::*)(void))&Lua_Client::SendSound)
		.def("Save", (void(Lua_Client::*)(void))&Lua_Client::Save)
		.def("Save", (void(Lua_Client::*)(int))&Lua_Client::Save)
		.def("SaveBackup", (void(Lua_Client::*)(void))&Lua_Client::SaveBackup)
		.def("Connected", (bool(Lua_Client::*)(void))&Lua_Client::Connected)
		.def("InZone", (bool(Lua_Client::*)(void))&Lua_Client::InZone)
		.def("Kick", (void(Lua_Client::*)(void))&Lua_Client::Kick)
		.def("Disconnect", (void(Lua_Client::*)(void))&Lua_Client::Disconnect)
		.def("IsLD", (bool(Lua_Client::*)(void))&Lua_Client::IsLD)
		.def("WorldKick", (void(Lua_Client::*)(void))&Lua_Client::WorldKick)
		.def("GetAnon", (bool(Lua_Client::*)(void))&Lua_Client::GetAnon)
		.def("Duck", (void(Lua_Client::*)(void))&Lua_Client::Duck)
		.def("Stand", (void(Lua_Client::*)(void))&Lua_Client::Stand)
		.def("SetGM", (void(Lua_Client::*)(bool))&Lua_Client::SetGM)
		.def("SetPVP", (void(Lua_Client::*)(bool))&Lua_Client::SetPVP)
		.def("GetPVP", (bool(Lua_Client::*)(void))&Lua_Client::GetPVP)
		.def("GetGM", (bool(Lua_Client::*)(void))&Lua_Client::GetGM)
		.def("SetBaseClass", (void(Lua_Client::*)(int))&Lua_Client::SetBaseClass)
		.def("SetBaseRace", (void(Lua_Client::*)(int))&Lua_Client::SetBaseRace)
		.def("SetBaseGender", (void(Lua_Client::*)(int))&Lua_Client::SetBaseGender)
		.def("GetBaseFace", (int(Lua_Client::*)(void))&Lua_Client::GetBaseFace)
		.def("GetLanguageSkill", (int(Lua_Client::*)(int))&Lua_Client::GetLanguageSkill)
		.def("GetLastName", (const char *(Lua_Client::*)(void))&Lua_Client::GetLastName)
		.def("GetLDoNPointsTheme", (int(Lua_Client::*)(int))&Lua_Client::GetLDoNPointsTheme)
		.def("GetBaseSTR", (int(Lua_Client::*)(void))&Lua_Client::GetBaseSTR)
		.def("GetBaseSTA", (int(Lua_Client::*)(void))&Lua_Client::GetBaseSTA)
		.def("GetBaseCHA", (int(Lua_Client::*)(void))&Lua_Client::GetBaseCHA)
		.def("GetBaseDEX", (int(Lua_Client::*)(void))&Lua_Client::GetBaseDEX)
		.def("GetBaseINT", (int(Lua_Client::*)(void))&Lua_Client::GetBaseINT)
		.def("GetBaseAGI", (int(Lua_Client::*)(void))&Lua_Client::GetBaseAGI)
		.def("GetBaseWIS", (int(Lua_Client::*)(void))&Lua_Client::GetBaseWIS)
		.def("GetWeight", (int(Lua_Client::*)(void))&Lua_Client::GetWeight)
		.def("GetEXP", (uint32(Lua_Client::*)(void))&Lua_Client::GetEXP)
		.def("GetAAExp", (uint32(Lua_Client::*)(void))&Lua_Client::GetAAExp)
		.def("GetAAPercent", (uint32(Lua_Client::*)(void))&Lua_Client::GetAAPercent)
		.def("GetTotalSecondsPlayed", (uint32(Lua_Client::*)(void))&Lua_Client::GetTotalSecondsPlayed)
		.def("UpdateLDoNPoints", (void(Lua_Client::*)(int,uint32))&Lua_Client::UpdateLDoNPoints)
		.def("SetDeity", (void(Lua_Client::*)(int))&Lua_Client::SetDeity)
		.def("AddEXP", (void(Lua_Client::*)(uint32))&Lua_Client::AddEXP)
		.def("AddEXP", (void(Lua_Client::*)(uint32,int))&Lua_Client::AddEXP)
		.def("AddEXP", (void(Lua_Client::*)(uint32,int,bool))&Lua_Client::AddEXP)
		.def("SetEXP", (void(Lua_Client::*)(uint32,uint32))&Lua_Client::SetEXP)
		.def("SetEXP", (void(Lua_Client::*)(uint32,uint32,bool))&Lua_Client::SetEXP)
		.def("SetBindPoint", (void(Lua_Client::*)(void))&Lua_Client::SetBindPoint)
		.def("SetBindPoint", (void(Lua_Client::*)(int))&Lua_Client::SetBindPoint)
		.def("SetBindPoint", (void(Lua_Client::*)(int,int))&Lua_Client::SetBindPoint)
		.def("SetBindPoint", (void(Lua_Client::*)(int,int,float))&Lua_Client::SetBindPoint)
		.def("SetBindPoint", (void(Lua_Client::*)(int,int,float,float))&Lua_Client::SetBindPoint)
		.def("SetBindPoint", (void(Lua_Client::*)(int,int,float,float, float))&Lua_Client::SetBindPoint)
		.def("GetBindX", (float(Lua_Client::*)(void))&Lua_Client::GetBindX)
		.def("GetBindX", (float(Lua_Client::*)(int))&Lua_Client::GetBindX)
		.def("GetBindY", (float(Lua_Client::*)(void))&Lua_Client::GetBindY)
		.def("GetBindY", (float(Lua_Client::*)(int))&Lua_Client::GetBindY)
		.def("GetBindZ", (float(Lua_Client::*)(void))&Lua_Client::GetBindZ)
		.def("GetBindZ", (float(Lua_Client::*)(int))&Lua_Client::GetBindZ)
		.def("GetBindHeading", (float(Lua_Client::*)(void))&Lua_Client::GetBindHeading)
		.def("GetBindHeading", (float(Lua_Client::*)(int))&Lua_Client::GetBindHeading)
		.def("GetBindZoneID", (uint32(Lua_Client::*)(void))&Lua_Client::GetBindZoneID)
		.def("GetBindZoneID", (uint32(Lua_Client::*)(int))&Lua_Client::GetBindZoneID)
		.def("SetPrimaryWeaponOrnamentation", (void(Lua_Client::*)(uint32))&Lua_Client::SetPrimaryWeaponOrnamentation)
		.def("SetSecondaryWeaponOrnamentation", (void(Lua_Client::*)(uint32))&Lua_Client::SetSecondaryWeaponOrnamentation)
		.def("MovePC", (void(Lua_Client::*)(int,float,float,float,float))&Lua_Client::MovePC)
		.def("MovePCInstance", (void(Lua_Client::*)(int,int,float,float,float,float))&Lua_Client::MovePCInstance)
		.def("ChangeLastName", (void(Lua_Client::*)(const char *in))&Lua_Client::ChangeLastName)
		.def("GetFactionLevel", (int(Lua_Client::*)(uint32,uint32,uint32,uint32,uint32,uint32,Lua_NPC))&Lua_Client::GetFactionLevel)
		.def("SetFactionLevel", (void(Lua_Client::*)(uint32,uint32,int,int,int))&Lua_Client::SetFactionLevel)
		.def("SetFactionLevel2", (void(Lua_Client::*)(uint32,int,int,int,int,int,int))&Lua_Client::SetFactionLevel2)
		.def("GetRawItemAC", (int(Lua_Client::*)(void))&Lua_Client::GetRawItemAC)
		.def("AccountID", (uint32(Lua_Client::*)(void))&Lua_Client::AccountID)
		.def("AccountName", (const char *(Lua_Client::*)(void))&Lua_Client::AccountName)
		.def("GetAccountAge", (int(Lua_Client::*)(void))&Lua_Client::GetAccountAge)
		.def("Admin", (int(Lua_Client::*)(void))&Lua_Client::Admin)
		.def("CharacterID", (uint32(Lua_Client::*)(void))&Lua_Client::CharacterID)
		.def("GuildRank", (int(Lua_Client::*)(void))&Lua_Client::GuildRank)
		.def("GuildID", (uint32(Lua_Client::*)(void))&Lua_Client::GuildID)
		.def("GetFace", (int(Lua_Client::*)(void))&Lua_Client::GetFace)
		.def("TakeMoneyFromPP", (bool(Lua_Client::*)(uint64))&Lua_Client::TakeMoneyFromPP)
		.def("TakeMoneyFromPP", (bool(Lua_Client::*)(uint64,bool))&Lua_Client::TakeMoneyFromPP)
		.def("AddMoneyToPP", (void(Lua_Client::*)(uint32,uint32,uint32,uint32,bool))&Lua_Client::AddMoneyToPP)
		.def("TGB", (bool(Lua_Client::*)(void))&Lua_Client::TGB)
		.def("GetSkillPoints", (int(Lua_Client::*)(void))&Lua_Client::GetSkillPoints)
		.def("SetSkillPoints", (void(Lua_Client::*)(int))&Lua_Client::SetSkillPoints)
		.def("IncreaseSkill", (void(Lua_Client::*)(int))&Lua_Client::IncreaseSkill)
		.def("IncreaseSkill", (void(Lua_Client::*)(int,int))&Lua_Client::IncreaseSkill)
		.def("IncreaseLanguageSkill", (void(Lua_Client::*)(int))&Lua_Client::IncreaseLanguageSkill)
		.def("IncreaseLanguageSkill", (void(Lua_Client::*)(int,int))&Lua_Client::IncreaseLanguageSkill)
		.def("GetRawSkill", (int(Lua_Client::*)(int))&Lua_Client::GetRawSkill)
		.def("HasSkill", (bool(Lua_Client::*)(int))&Lua_Client::HasSkill)
		.def("CanHaveSkill", (bool(Lua_Client::*)(int))&Lua_Client::CanHaveSkill)
		.def("SetSkill", (void(Lua_Client::*)(int,int))&Lua_Client::SetSkill)
		.def("AddSkill", (void(Lua_Client::*)(int,int))&Lua_Client::AddSkill)
		.def("CheckSpecializeIncrease", (void(Lua_Client::*)(int))&Lua_Client::CheckSpecializeIncrease)
		.def("CheckIncreaseSkill", (void(Lua_Client::*)(int,Lua_Mob))&Lua_Client::CheckIncreaseSkill)
		.def("CheckIncreaseSkill", (void(Lua_Client::*)(int,Lua_Mob,int))&Lua_Client::CheckIncreaseSkill)
		.def("SetLanguageSkill", (void(Lua_Client::*)(int,int))&Lua_Client::SetLanguageSkill)
		.def("MaxSkill", (int(Lua_Client::*)(int))&Lua_Client::MaxSkill)
		.def("IsMedding", (bool(Lua_Client::*)(void))&Lua_Client::IsMedding)
		.def("GetDuelTarget", (int(Lua_Client::*)(void))&Lua_Client::GetDuelTarget)
		.def("IsDueling", (bool(Lua_Client::*)(void))&Lua_Client::IsDueling)
		.def("SetDuelTarget", (void(Lua_Client::*)(int))&Lua_Client::SetDuelTarget)
		.def("SetDueling", (void(Lua_Client::*)(bool))&Lua_Client::SetDueling)
		.def("ResetAA", (void(Lua_Client::*)(void))&Lua_Client::ResetAA)
		.def("MemSpell", (void(Lua_Client::*)(int,int))&Lua_Client::MemSpell)
		.def("MemSpell", (void(Lua_Client::*)(int,int,bool))&Lua_Client::MemSpell)
		.def("UnmemSpell", (void(Lua_Client::*)(int))&Lua_Client::UnmemSpell)
		.def("UnmemSpell", (void(Lua_Client::*)(int,bool))&Lua_Client::UnmemSpell)
		.def("UnmemSpellBySpellID", (void(Lua_Client::*)(int32))&Lua_Client::UnmemSpellBySpellID)
		.def("UnmemSpellAll", (void(Lua_Client::*)(void))&Lua_Client::UnmemSpellAll)
		.def("UnmemSpellAll", (void(Lua_Client::*)(bool))&Lua_Client::UnmemSpellAll)
		.def("FindMemmedSpellBySlot", (uint16(Lua_Client::*)(int))&Lua_Client::FindMemmedSpellBySlot)
		.def("MemmedCount", (int(Lua_Client::*)(void))&Lua_Client::MemmedCount)
		.def("ScribeSpell", (void(Lua_Client::*)(int,int))&Lua_Client::ScribeSpell)
		.def("ScribeSpell", (void(Lua_Client::*)(int,int,bool))&Lua_Client::ScribeSpell)
		.def("UnscribeSpell", (void(Lua_Client::*)(int))&Lua_Client::UnscribeSpell)
		.def("UnscribeSpell", (void(Lua_Client::*)(int,bool))&Lua_Client::UnscribeSpell)
		.def("UnscribeSpellAll", (void(Lua_Client::*)(void))&Lua_Client::UnscribeSpellAll)
		.def("UnscribeSpellAll", (void(Lua_Client::*)(bool))&Lua_Client::UnscribeSpellAll)
		.def("TrainDisc", (void(Lua_Client::*)(int))&Lua_Client::TrainDisc)
		.def("TrainDiscBySpellID", (void(Lua_Client::*)(int32))&Lua_Client::TrainDiscBySpellID)
		.def("GetDiscSlotBySpellID", (int(Lua_Client::*)(int32))&Lua_Client::GetDiscSlotBySpellID)
		.def("UntrainDisc", (void(Lua_Client::*)(int))&Lua_Client::UntrainDisc)
		.def("UntrainDisc", (void(Lua_Client::*)(int,bool))&Lua_Client::UntrainDisc)
		.def("UntrainDiscAll", (void(Lua_Client::*)(void))&Lua_Client::UntrainDiscAll)
		.def("UntrainDiscAll", (void(Lua_Client::*)(bool))&Lua_Client::UntrainDiscAll)
		.def("IsStanding", (bool(Lua_Client::*)(void))&Lua_Client::IsStanding)
		.def("IsSitting", (bool(Lua_Client::*)(void))&Lua_Client::IsSitting)
		.def("IsCrouching", (bool(Lua_Client::*)(void))&Lua_Client::IsCrouching)
		.def("SetFeigned", (void(Lua_Client::*)(bool))&Lua_Client::SetFeigned)
		.def("GetFeigned", (bool(Lua_Client::*)(void))&Lua_Client::GetFeigned)
		.def("AutoSplitEnabled", (bool(Lua_Client::*)(void))&Lua_Client::AutoSplitEnabled)
		.def("SetHorseId", (void(Lua_Client::*)(int))&Lua_Client::SetHorseId)
		.def("GetHorseId", (int(Lua_Client::*)(void))&Lua_Client::GetHorseId)
		.def("NukeItem", (void(Lua_Client::*)(uint32))&Lua_Client::NukeItem)
		.def("NukeItem", (void(Lua_Client::*)(uint32,int))&Lua_Client::NukeItem)
		.def("SetTint", (void(Lua_Client::*)(int,uint32))&Lua_Client::SetTint)
		.def("SetMaterial", (void(Lua_Client::*)(int,uint32))&Lua_Client::SetMaterial)
		.def("Undye", (void(Lua_Client::*)(void))&Lua_Client::Undye)
		.def("GetItemIDAt", (int(Lua_Client::*)(int))&Lua_Client::GetItemIDAt)
		.def("GetAugmentIDAt", (int(Lua_Client::*)(int,int))&Lua_Client::GetAugmentIDAt)
		.def("DeleteItemInInventory", (void(Lua_Client::*)(int,int))&Lua_Client::DeleteItemInInventory)
		.def("DeleteItemInInventory", (void(Lua_Client::*)(int,int,bool))&Lua_Client::DeleteItemInInventory)
		.def("SummonItem", (void(Lua_Client::*)(uint32))&Lua_Client::SummonItem)
		.def("SummonItem", (void(Lua_Client::*)(uint32,int))&Lua_Client::SummonItem)
		.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32))&Lua_Client::SummonItem)
		.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32))&Lua_Client::SummonItem)
		.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32))&Lua_Client::SummonItem)
		.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32,uint32))&Lua_Client::SummonItem)
		.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32,uint32,uint32))&Lua_Client::SummonItem)
		.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32,uint32,uint32,bool))&Lua_Client::SummonItem)
		.def("SummonItem", (void(Lua_Client::*)(uint32,int,uint32,uint32,uint32,uint32,uint32,bool,int))&Lua_Client::SummonItem)
		.def("SetStats", (void(Lua_Client::*)(int,int))&Lua_Client::SetStats)
		.def("IncStats", (void(Lua_Client::*)(int,int))&Lua_Client::IncStats)
		.def("DropItem", (void(Lua_Client::*)(int))&Lua_Client::DropItem)
		.def("BreakInvis", (void(Lua_Client::*)(void))&Lua_Client::BreakInvis)
		.def("LeaveGroup", (void(Lua_Client::*)(void))&Lua_Client::LeaveGroup)
		.def("IsGrouped", (bool(Lua_Client::*)(void))&Lua_Client::IsGrouped)
		.def("IsRaidGrouped", (bool(Lua_Client::*)(void))&Lua_Client::IsRaidGrouped)
		.def("Hungry", (bool(Lua_Client::*)(void))&Lua_Client::Hungry)
		.def("Thirsty", (bool(Lua_Client::*)(void))&Lua_Client::Thirsty)
		.def("GetInstrumentMod", (int(Lua_Client::*)(int))&Lua_Client::GetInstrumentMod)
		.def("DecreaseByID", (bool(Lua_Client::*)(uint32,int))&Lua_Client::DecreaseByID)
		.def("Escape", (void(Lua_Client::*)(void))&Lua_Client::Escape)
		.def("GoFish", (void(Lua_Client::*)(void))&Lua_Client::GoFish)
		.def("ForageItem", (void(Lua_Client::*)(void))&Lua_Client::ForageItem)
		.def("ForageItem", (void(Lua_Client::*)(bool))&Lua_Client::ForageItem)
		.def("CalcPriceMod", (float(Lua_Client::*)(Lua_Mob,bool))&Lua_Client::CalcPriceMod)
		.def("ResetTrade", (void(Lua_Client::*)(void))&Lua_Client::ResetTrade)
		.def("UseDiscipline", (bool(Lua_Client::*)(int,int))&Lua_Client::UseDiscipline)
		.def("GetCharacterFactionLevel", (int(Lua_Client::*)(int))&Lua_Client::GetCharacterFactionLevel)
		.def("SetZoneFlag", (void(Lua_Client::*)(int))&Lua_Client::SetZoneFlag)
		.def("ClearZoneFlag", (void(Lua_Client::*)(int))&Lua_Client::ClearZoneFlag)
		.def("HasZoneFlag", (bool(Lua_Client::*)(int))&Lua_Client::HasZoneFlag)
		.def("SendZoneFlagInfo", (void(Lua_Client::*)(Lua_Client))&Lua_Client::SendZoneFlagInfo)
		.def("SetAATitle", (void(Lua_Client::*)(const char *))&Lua_Client::SetAATitle)
		.def("GetClientVersion", (int(Lua_Client::*)(void))&Lua_Client::GetClientVersion)
		.def("GetClientVersionBit", (uint32(Lua_Client::*)(void))&Lua_Client::GetClientVersionBit)
		.def("SetTitleSuffix", (void(Lua_Client::*)(const char *))&Lua_Client::SetTitleSuffix)
		.def("SetAAPoints", (void(Lua_Client::*)(int))&Lua_Client::SetAAPoints)
		.def("GetAAPoints", (int(Lua_Client::*)(void))&Lua_Client::GetAAPoints)
		.def("GetSpentAA", (int(Lua_Client::*)(void))&Lua_Client::GetSpentAA)
		.def("AddAAPoints", (void(Lua_Client::*)(int))&Lua_Client::AddAAPoints)
		.def("RefundAA", (void(Lua_Client::*)(void))&Lua_Client::RefundAA)
		.def("GetModCharacterFactionLevel", (int(Lua_Client::*)(int))&Lua_Client::GetModCharacterFactionLevel)
		.def("GetLDoNWins", (int(Lua_Client::*)(void))&Lua_Client::GetLDoNWins)
		.def("GetLDoNLosses", (int(Lua_Client::*)(void))&Lua_Client::GetLDoNLosses)
		.def("GetLDoNWinsTheme", (int(Lua_Client::*)(int))&Lua_Client::GetLDoNWinsTheme)
		.def("GetLDoNLossesTheme", (int(Lua_Client::*)(int))&Lua_Client::GetLDoNLossesTheme)
		.def("GetStartZone", (int(Lua_Client::*)(void))&Lua_Client::GetStartZone)
		.def("SetStartZone", (void(Lua_Client::*)(int))&Lua_Client::SetStartZone)
		.def("SetStartZone", (void(Lua_Client::*)(int,float))&Lua_Client::SetStartZone)
		.def("SetStartZone", (void(Lua_Client::*)(int,float,float))&Lua_Client::SetStartZone)
		.def("SetStartZone", (void(Lua_Client::*)(int,float,float,float))&Lua_Client::SetStartZone)
		.def("KeyRingAdd", (void(Lua_Client::*)(uint32))&Lua_Client::KeyRingAdd)
		.def("KeyRingCheck", (bool(Lua_Client::*)(uint32))&Lua_Client::KeyRingCheck)
		.def("AddPVPPoints", (void(Lua_Client::*)(uint32))&Lua_Client::AddPVPPoints)
		.def("AddCrystals", (void(Lua_Client::*)(uint32,uint32))&Lua_Client::AddCrystals)
		.def("GetPVPPoints", (uint32(Lua_Client::*)(void))&Lua_Client::GetPVPPoints)
		.def("GetRadiantCrystals", (uint32(Lua_Client::*)(void))&Lua_Client::GetRadiantCrystals)
		.def("GetEbonCrystals", (uint32(Lua_Client::*)(void))&Lua_Client::GetEbonCrystals)
		.def("QuestReadBook", (void(Lua_Client::*)(const char *,int))&Lua_Client::QuestReadBook)
		.def("UpdateGroupAAs", (void(Lua_Client::*)(int,uint32))&Lua_Client::UpdateGroupAAs)
		.def("GetGroupPoints", (uint32(Lua_Client::*)(void))&Lua_Client::GetGroupPoints)
		.def("GetRaidPoints", (uint32(Lua_Client::*)(void))&Lua_Client::GetRaidPoints)
		.def("LearnRecipe", (void(Lua_Client::*)(uint32))&Lua_Client::LearnRecipe)
		.def("GetEndurance", (int(Lua_Client::*)(void))&Lua_Client::GetEndurance)
		.def("GetMaxEndurance", (int(Lua_Client::*)(void))&Lua_Client::GetMaxEndurance)
		.def("GetEndurancePercent", (int(Lua_Client::*)(void))&Lua_Client::GetEndurancePercent)
		.def("SetEndurance", (void(Lua_Client::*)(int))&Lua_Client::SetEndurance)
		.def("SendOPTranslocateConfirm", (void(Lua_Client::*)(Lua_Mob,int))&Lua_Client::SendOPTranslocateConfirm)
		.def("GetIP", (uint32(Lua_Client::*)(void))&Lua_Client::GetIP)
		.def("AddLevelBasedExp", (void(Lua_Client::*)(int))&Lua_Client::AddLevelBasedExp)
		.def("AddLevelBasedExp", (void(Lua_Client::*)(int,int))&Lua_Client::AddLevelBasedExp)
		.def("AddLevelBasedExp", (void(Lua_Client::*)(int,int,bool))&Lua_Client::AddLevelBasedExp)
		.def("IncrementAA", (void(Lua_Client::*)(int))&Lua_Client::IncrementAA)
		.def("GrantAlternateAdvancementAbility", (bool(Lua_Client::*)(int, int))&Lua_Client::GrantAlternateAdvancementAbility)
		.def("GrantAlternateAdvancementAbility", (bool(Lua_Client::*)(int, int, bool))&Lua_Client::GrantAlternateAdvancementAbility)
		.def("MarkSingleCompassLoc", (void(Lua_Client::*)(float,float,float))&Lua_Client::MarkSingleCompassLoc)
		.def("MarkSingleCompassLoc", (void(Lua_Client::*)(float,float,float,int))&Lua_Client::MarkSingleCompassLoc)
		.def("ClearCompassMark",(void(Lua_Client::*)(void))&Lua_Client::ClearCompassMark)
		.def("GetNextAvailableSpellBookSlot", (int(Lua_Client::*)(void))&Lua_Client::GetNextAvailableSpellBookSlot)
		.def("GetNextAvailableSpellBookSlot", (int(Lua_Client::*)(int))&Lua_Client::GetNextAvailableSpellBookSlot)
		.def("FindSpellBookSlotBySpellID", (int(Lua_Client::*)(int))&Lua_Client::FindSpellBookSlotBySpellID)
		.def("UpdateTaskActivity", (void(Lua_Client::*)(int,int,int))&Lua_Client::UpdateTaskActivity)
		.def("AssignTask", (void(Lua_Client::*)(int,int))&Lua_Client::AssignTask)
		.def("AssignTask", (void(Lua_Client::*)(int,int,bool))&Lua_Client::AssignTask)
		.def("FailTask", (void(Lua_Client::*)(int))&Lua_Client::FailTask)
		.def("IsTaskCompleted", (bool(Lua_Client::*)(int))&Lua_Client::IsTaskCompleted)
		.def("IsTaskActive", (bool(Lua_Client::*)(int))&Lua_Client::IsTaskActive)
		.def("IsTaskActivityActive", (bool(Lua_Client::*)(int,int))&Lua_Client::IsTaskActivityActive)
		.def("GetCorpseCount", (int(Lua_Client::*)(void))&Lua_Client::GetCorpseCount)
		.def("GetCorpseID", (int(Lua_Client::*)(int))&Lua_Client::GetCorpseID)
		.def("GetCorpseItemAt", (int(Lua_Client::*)(int,int))&Lua_Client::GetCorpseItemAt)
		.def("AssignToInstance", (void(Lua_Client::*)(int))&Lua_Client::AssignToInstance)
		.def("Freeze", (void(Lua_Client::*)(void))&Lua_Client::Freeze)
		.def("UnFreeze", (void(Lua_Client::*)(void))&Lua_Client::UnFreeze)
		.def("GetAggroCount", (int(Lua_Client::*)(void))&Lua_Client::GetAggroCount)
		.def("GetCarriedMoney", (uint64(Lua_Client::*)(void))&Lua_Client::GetCarriedMoney)
		.def("GetAllMoney", (uint64(Lua_Client::*)(void))&Lua_Client::GetAllMoney)
		.def("GetMoney", (uint32(Lua_Client::*)(uint8, uint8))&Lua_Client::GetMoney)
		.def("OpenLFGuildWindow", (void(Lua_Client::*)(void))&Lua_Client::OpenLFGuildWindow)
		.def("Signal", (void(Lua_Client::*)(uint32))&Lua_Client::Signal)
		.def("AddAlternateCurrencyValue", (void(Lua_Client::*)(uint32,int))&Lua_Client::AddAlternateCurrencyValue)
		.def("SetAlternateCurrencyValue", (void(Lua_Client::*)(uint32,int))&Lua_Client::SetAlternateCurrencyValue)
		.def("GetAlternateCurrencyValue", (int(Lua_Client::*)(uint32))&Lua_Client::GetAlternateCurrencyValue)
		.def("SendWebLink", (void(Lua_Client::*)(const char *))&Lua_Client::SendWebLink)
		.def("HasSpellScribed", (bool(Lua_Client::*)(int))&Lua_Client::HasSpellScribed)
		.def("SetAccountFlag", (void(Lua_Client::*)(std::string,std::string))&Lua_Client::SetAccountFlag)
		.def("GetAccountFlag", (std::string(Lua_Client::*)(std::string))&Lua_Client::GetAccountFlag)
		.def("GetGroup", (Lua_Group(Lua_Client::*)(void))&Lua_Client::GetGroup)
		.def("GetRaid", (Lua_Raid(Lua_Client::*)(void))&Lua_Client::GetRaid)
		.def("PutItemInInventory", (bool(Lua_Client::*)(int,Lua_ItemInst))&Lua_Client::PutItemInInventory)
		.def("PushItemOnCursor", (bool(Lua_Client::*)(Lua_ItemInst))&Lua_Client::PushItemOnCursor)
		.def("GetInventory", (Lua_Inventory(Lua_Client::*)(void))&Lua_Client::GetInventory)
		.def("SendItemScale", (void(Lua_Client::*)(Lua_ItemInst))&Lua_Client::SendItemScale)
		.def("QueuePacket", (void(Lua_Client::*)(Lua_Packet))&Lua_Client::QueuePacket)
		.def("QueuePacket", (void(Lua_Client::*)(Lua_Packet,bool))&Lua_Client::QueuePacket)
		.def("QueuePacket", (void(Lua_Client::*)(Lua_Packet,bool,int))&Lua_Client::QueuePacket)
		.def("QueuePacket", (void(Lua_Client::*)(Lua_Packet,bool,int,int))&Lua_Client::QueuePacket)
		.def("GetHunger", (int(Lua_Client::*)(void))&Lua_Client::GetHunger)
		.def("GetThirst", (int(Lua_Client::*)(void))&Lua_Client::GetThirst)
		.def("SetHunger", (void(Lua_Client::*)(int))&Lua_Client::SetHunger)
		.def("SetThirst", (void(Lua_Client::*)(int))&Lua_Client::SetThirst)
		.def("SetConsumption", (void(Lua_Client::*)(int, int))&Lua_Client::SetConsumption)
		.def("SendMarqueeMessage", (void(Lua_Client::*)(uint32, uint32, uint32, uint32, uint32, std::string))&Lua_Client::SendMarqueeMessage)
		.def("SendColoredText", (void(Lua_Client::*)(uint32, std::string))&Lua_Client::SendColoredText)
		.def("PlayMP3", (void(Lua_Client::*)(std::string))&Lua_Client::PlayMP3)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob))&Lua_Client::QuestReward)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32))&Lua_Client::QuestReward)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32))&Lua_Client::QuestReward)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32))&Lua_Client::QuestReward)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32, uint32, bool))&Lua_Client::QuestReward)
		.def("QuestReward", (void(Lua_Client::*)(Lua_Mob, luabind::adl::object))&Lua_Client::QuestReward)
		.def("IsDead", &Lua_Client::IsDead)
		.def("CalcCurrentWeight", &Lua_Client::CalcCurrentWeight)
		.def("CalcATK", &Lua_Client::CalcATK)
		.def("FilteredMessage", &Lua_Client::FilteredMessage)
		.def("EnableAreaHPRegen", &Lua_Client::EnableAreaHPRegen)
		.def("DisableAreaHPRegen", &Lua_Client::DisableAreaHPRegen)
		.def("EnableAreaManaRegen", &Lua_Client::EnableAreaManaRegen)
		.def("DisableAreaManaRegen", &Lua_Client::DisableAreaManaRegen)
		.def("EnableAreaEndRegen", &Lua_Client::EnableAreaEndRegen)
		.def("DisableAreaEndRegen", &Lua_Client::DisableAreaEndRegen)
		.def("EnableAreaRegens", &Lua_Client::EnableAreaRegens)
		.def("DisableAreaRegens", &Lua_Client::DisableAreaRegens)
		.def("SetClientMaxLevel", (void(Lua_Client::*)(int))&Lua_Client::SetClientMaxLevel)
		.def("GetClientMaxLevel", (int(Lua_Client::*)(void))&Lua_Client::GetClientMaxLevel);
}

luabind::scope lua_register_inventory_where() {
	return luabind::class_<InventoryWhere>("InventoryWhere")
		.enum_("constants")
		[
			luabind::value("Personal", static_cast<int>(invWherePersonal)),
			luabind::value("Bank", static_cast<int>(invWhereBank)),
			luabind::value("SharedBank", static_cast<int>(invWhereSharedBank)),
			luabind::value("Trading", static_cast<int>(invWhereTrading)),
			luabind::value("Cursor", static_cast<int>(invWhereCursor))
		];
}



#endif
