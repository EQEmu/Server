#ifdef LUA_EQEMU
#include <sol/sol.hpp>
#include "lua_client.h"

#include "client.h"
#include "dynamic_zone.h"
#include "expedition_request.h"
#include "lua_client.h"
#include "lua_expedition.h"
#include "lua_npc.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_inventory.h"
#include "lua_group.h"
#include "lua_raid.h"
#include "lua_packet.h"
#include "dialogue_window.h"
#include "titles.h"
#include "../common/expedition_lockout_timer.h"

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

int Lua_Client::GetAFK() {
	Lua_Safe_Call_Int();
	return self->GetAFK();
}

void Lua_Client::SetAFK(uint8 afk_flag) {
	Lua_Safe_Call_Void();
	self->SetAFK(afk_flag);
}

int Lua_Client::GetAnon() {
	Lua_Safe_Call_Int();
	return self->GetAnon();
}

void Lua_Client::SetAnon(uint8 anon_flag) {
	Lua_Safe_Call_Void();
	self->SetAnon(anon_flag);
}

void Lua_Client::Duck() {
	Lua_Safe_Call_Void();
	self->Duck();
}

void Lua_Client::Sit() {
	Lua_Safe_Call_Void();
	self->Sit();
}

void Lua_Client::DyeArmorBySlot(uint8 slot, uint8 red, uint8 green, uint8 blue) {
	Lua_Safe_Call_Void();
	self->DyeArmorBySlot(slot, red, green, blue);
}

void Lua_Client::DyeArmorBySlot(uint8 slot, uint8 red, uint8 green, uint8 blue, uint8 use_tint) {
	Lua_Safe_Call_Void();
	self->DyeArmorBySlot(slot, red, green, blue, use_tint);
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

void Lua_Client::SendToGuildHall() {
	Lua_Safe_Call_Void();
	self->SendToGuildHall();
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

int Lua_Client::GetClassBitmask() {
	Lua_Safe_Call_Int();
	return GetPlayerClassBit(self->GetClass());
}

int Lua_Client::GetRaceBitmask() {
	Lua_Safe_Call_Int();
	return GetPlayerRaceBit(self->GetBaseRace());
}

int Lua_Client::GetBaseFace() {
	Lua_Safe_Call_Int();
	return self->GetBaseFace();
}

int Lua_Client::GetLanguageSkill(int skill_id) {
	Lua_Safe_Call_Int();
	return self->GetLanguageSkill(skill_id);
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

void Lua_Client::UpdateLDoNPoints(uint32 theme_id, int points) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNPoints(theme_id, points);
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

void Lua_Client::SetBindPoint(int to_zone, int to_instance, float new_x, float new_y, float new_z, float new_heading) {
	Lua_Safe_Call_Void();
	self->SetBindPoint2(0, to_zone, to_instance, glm::vec4(new_x, new_y, new_z, new_heading));
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

float Lua_Client::GetTargetRingX() {
	Lua_Safe_Call_Real();
	return self->GetTargetRingX();
}

float Lua_Client::GetTargetRingY() {
	Lua_Safe_Call_Real();
	return self->GetTargetRingY();
}

float Lua_Client::GetTargetRingZ() {
	Lua_Safe_Call_Real();
	return self->GetTargetRingZ();
}

void Lua_Client::MovePC(int zone, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MovePC(zone, x, y, z, heading);
}

void Lua_Client::MovePCInstance(int zone, int instance, float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->MovePC(zone, instance, x, y, z, heading);
}

void Lua_Client::MoveZone(const char *zone_short_name) {
	Lua_Safe_Call_Void();
	self->MoveZone(zone_short_name);
}

void Lua_Client::MoveZoneGroup(const char *zone_short_name) {
	Lua_Safe_Call_Void();
	self->MoveZoneGroup(zone_short_name);
}

void Lua_Client::MoveZoneRaid(const char *zone_short_name) {
	Lua_Safe_Call_Void();
	self->MoveZoneRaid(zone_short_name);
}

void Lua_Client::MoveZoneInstance(uint16 instance_id) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstance(instance_id);
}

void Lua_Client::MoveZoneInstanceGroup(uint16 instance_id) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstanceGroup(instance_id);
}

void Lua_Client::MoveZoneInstanceRaid(uint16 instance_id) {
	Lua_Safe_Call_Void();
	self->MoveZoneInstanceRaid(instance_id);
}

void Lua_Client::ChangeLastName(std::string last_name) {
	Lua_Safe_Call_Void();
	self->ChangeLastName(last_name);
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

void Lua_Client::RewardFaction(int id, int amount) {
	Lua_Safe_Call_Void();
	self->RewardFaction(id, amount);
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

int16 Lua_Client::Admin() {
	Lua_Safe_Call_Int();
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

void Lua_Client::AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->AddMoneyToPP(copper, silver, gold, platinum);
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
	return self->GetRawSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

bool Lua_Client::HasSkill(int skill_id) {
	Lua_Safe_Call_Bool();
	return self->HasSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

bool Lua_Client::CanHaveSkill(int skill_id) {
	Lua_Safe_Call_Bool();
	return self->CanHaveSkill(static_cast<EQ::skills::SkillType>(skill_id));
}

void Lua_Client::SetSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->SetSkill(static_cast<EQ::skills::SkillType>(skill_id), value);
}

void Lua_Client::AddSkill(int skill_id, int value) {
	Lua_Safe_Call_Void();
	self->AddSkill(static_cast<EQ::skills::SkillType>(skill_id), value);
}

void Lua_Client::CheckSpecializeIncrease(int spell_id) {
	Lua_Safe_Call_Void();
	self->CheckSpecializeIncrease(spell_id);
}

void Lua_Client::CheckIncreaseSkill(int skill_id, Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->CheckIncreaseSkill(static_cast<EQ::skills::SkillType>(skill_id), target);
}

void Lua_Client::CheckIncreaseSkill(int skill_id, Lua_Mob target, int chance_mod) {
	Lua_Safe_Call_Void();
	self->CheckIncreaseSkill(static_cast<EQ::skills::SkillType>(skill_id), target, chance_mod);
}

void Lua_Client::SetLanguageSkill(int language, int value) {
	Lua_Safe_Call_Void();
	self->SetLanguageSkill(language, value);
}

int Lua_Client::MaxSkill(int skill_id) {
	Lua_Safe_Call_Int();
	return self->MaxSkill(static_cast<EQ::skills::SkillType>(skill_id));
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

int Lua_Client::FindMemmedSpellBySpellID(uint16 spell_id) {
	Lua_Safe_Call_Int();
	return self->FindMemmedSpellBySpellID(spell_id);
}

int Lua_Client::FindEmptyMemSlot() {
	Lua_Safe_Call_Int();
	return self->FindEmptyMemSlot();
}

int Lua_Client::MemmedCount() {
	Lua_Safe_Call_Int();
	return self->MemmedCount();
}

sol::table Lua_Client::GetLearnableDisciplines(sol::this_state s) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto learnable_disciplines = self->GetLearnableDisciplines();
		int index = 0;
		for (auto spell_id : learnable_disciplines) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetLearnableDisciplines(sol::this_state s, uint8 min_level) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto learnable_disciplines = self->GetLearnableDisciplines(min_level);
		int index = 0;
		for (auto spell_id : learnable_disciplines) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetLearnableDisciplines(sol::this_state s, uint8 min_level, uint8 max_level) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto learnable_disciplines = self->GetLearnableDisciplines(min_level, max_level);
		int index = 0;
		for (auto spell_id : learnable_disciplines) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetLearnedDisciplines(sol::this_state s) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto learned_disciplines = self->GetLearnedDisciplines();
		int index = 0;
		for (auto spell_id : learned_disciplines) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetMemmedSpells(sol::this_state s) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto memmed_spells = self->GetMemmedSpells();
		int index = 0;
		for (auto spell_id : memmed_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetScribeableSpells(sol::this_state s) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto scribeable_spells = self->GetScribeableSpells();
		int index = 0;
		for (auto spell_id : scribeable_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetScribeableSpells(sol::this_state s, uint8 min_level) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto scribeable_spells = self->GetScribeableSpells(min_level);
		int index = 0;
		for (auto spell_id : scribeable_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetScribeableSpells(sol::this_state s, uint8 min_level, uint8 max_level) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto scribeable_spells = self->GetScribeableSpells(min_level, max_level);
		int index = 0;
		for (auto spell_id : scribeable_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetScribedSpells(sol::this_state s) {
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto scribed_spells = self->GetScribedSpells();
		int index = 0;
		for (auto spell_id : scribed_spells) {
			lua_table[index] = spell_id;
			index++;
		}
	}
	return lua_table;
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

uint32 Lua_Client::GetDisciplineTimer(uint32 timer_id) {
	Lua_Safe_Call_Int();
	return self->GetDisciplineTimer(timer_id);
}

void Lua_Client::ResetDisciplineTimer(uint32 timer_id) {
	Lua_Safe_Call_Void();
	self->ResetDisciplineTimer(timer_id);
}

bool Lua_Client::UseDiscipline(int spell_id, int target_id) {
	Lua_Safe_Call_Bool();
	return self->UseDiscipline(spell_id, target_id);
}

bool Lua_Client::HasDisciplineLearned(uint16 spell_id) {
	Lua_Safe_Call_Bool();
	return self->HasDisciplineLearned(spell_id);
}

int Lua_Client::GetCharacterFactionLevel(int faction_id) {
	Lua_Safe_Call_Int();
	return self->GetCharacterFactionLevel(faction_id);
}

void Lua_Client::SetZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Void();
	self->SetZoneFlag(zone_id);
}

void Lua_Client::ClearZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Void();
	self->ClearZoneFlag(zone_id);
}

bool Lua_Client::HasZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Bool();
	return self->HasZoneFlag(zone_id);
}

void Lua_Client::SendZoneFlagInfo(Lua_Client to) {
	Lua_Safe_Call_Void();
	self->SendZoneFlagInfo(to);
}

void Lua_Client::SetAATitle(std::string title) {
	Lua_Safe_Call_Void();
	self->SetAATitle(title);
}

void Lua_Client::SetAATitle(std::string title, bool save_to_database) {
	Lua_Safe_Call_Void();
	if (!save_to_database) {
		self->SetAATitle(title);
	} else {
		title_manager.CreateNewPlayerTitle(self, title);
	}
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

void Lua_Client::SetEbonCrystals(uint32 value) {
	Lua_Safe_Call_Void();
	self->SetEbonCrystals(value);
}

void Lua_Client::SetRadiantCrystals(uint32 value) {
	Lua_Safe_Call_Void();
	self->SetRadiantCrystals(value);
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

uint32 Lua_Client::GetSpellIDByBookSlot(int slot_id) {
	Lua_Safe_Call_Int();
	return self->GetSpellIDByBookSlot(slot_id);
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

void Lua_Client::AssignTask(int task_id) {
	Lua_Safe_Call_Void();
	self->AssignTask(task_id);
}

void Lua_Client::AssignTask(int task_id, int npc_id) {
	Lua_Safe_Call_Void();
	self->AssignTask(task_id, npc_id);
}

void Lua_Client::AssignTask(int task_id, int npc_id, bool enforce_level_requirement) {
	Lua_Safe_Call_Void();
	self->AssignTask(task_id, npc_id, enforce_level_requirement);
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

void Lua_Client::LockSharedTask(bool lock) {
	Lua_Safe_Call_Void();
	return self->LockSharedTask(lock);
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

void Lua_Client::NotifyNewTitlesAvailable() {
	Lua_Safe_Call_Void();
	self->NotifyNewTitlesAvailable();
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
	EQ::ItemInstance *rinst = inst;
	return self->PutItemInInventory(slot_id, *rinst, true);
}

bool Lua_Client::PushItemOnCursor(Lua_ItemInst inst) {
	Lua_Safe_Call_Bool();
	EQ::ItemInstance *rinst = inst;
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

void Lua_Client::QuestReward(Lua_Mob target, sol::table reward) {
	Lua_Safe_Call_Void();

	QuestReward_Struct quest_reward;
	quest_reward.mob_id = 0;
	quest_reward.target_id = self->GetID();
	quest_reward.copper = reward.get_or("copper", 0);
	quest_reward.silver = reward.get_or("silver", 0);
	quest_reward.gold = reward.get_or("gold", 0);
	quest_reward.platinum = reward.get_or("platinum", 0);
	quest_reward.exp_reward = reward.get_or("exp", 0);
	quest_reward.faction = 0;
	quest_reward.faction_mod = 0;
	bool faction = false;
	std::fill(std::begin(quest_reward.item_id), std::end(quest_reward.item_id), -1);

	quest_reward.item_id[0] = reward.get_or("itemid", -1);

	// if you define both an itemid and items table, the itemid is thrown away
	// should we error?
	auto items = reward.get<sol::optional<sol::table>>("items");
	if (items) {
		// assume they defined a compatible table
		auto table = items.value();
		for (int i = 1; i <= QUESTREWARD_COUNT; ++i) {
			quest_reward.item_id[i - 1] = table.get_or(i, -1);
		}
	}

	// faction can either be a table for faction association or just a bool for backwards compatibility
	auto lua_faction = reward.get<sol::optional<sol::object>>("faction");
	if (lua_faction) {
		auto object = lua_faction.value();
		// if it's a table it will be {faction, faction_mod}
		if (object.get_type() == sol::type::table) {
			auto table = object.as<sol::table>();
			quest_reward.faction = table.get_or(1, 0);
			quest_reward.faction_mod = table.get_or(2, 0);
		} else if (object.get_type() == sol::type::boolean) {
			faction = object.as<bool>();
		}
	}

	self->QuestReward(target, quest_reward, faction);
}

void Lua_Client::CashReward(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->CashReward(copper, silver, gold, platinum);
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

void Lua_Client::SetClientMaxLevel(uint8 max_level) {
	Lua_Safe_Call_Void();
	self->SetClientMaxLevel(max_level);
}

uint8 Lua_Client::GetClientMaxLevel() {
	Lua_Safe_Call_Int();
	return self->GetClientMaxLevel();
}

void Lua_Client::DialogueWindow(std::string markdown) {
	Lua_Safe_Call_Void();
	DialogueWindow::Render(self, std::move(markdown));
}

DynamicZoneLocation GetDynamicZoneLocationFromTable(const sol::table& lua_table)
{
	DynamicZoneLocation zone_location;

	auto lua_zone = lua_table.get<sol::optional<sol::object>>("zone");

	// default invalid/missing args to 0
	uint32_t zone_id = 0;
	if (lua_zone) {
		auto object = lua_zone.value();
		if (object.get_type() == sol::type::string) {
			zone_id = ZoneID(object.as<std::string>());
		} else if (object.get_type() == sol::type::number) {
			zone_id = object.as<uint32_t>();
		}
	}

	float x = lua_table.get_or("x", 0.0f);
	float y = lua_table.get_or("y", 0.0f);
	float z = lua_table.get_or("z", 0.0f);
	float h = lua_table.get_or("h", 0.0f);

	zone_location = { zone_id, x, y, z, h };

	return zone_location;
}

Lua_Expedition Lua_Client::CreateExpedition(sol::table expedition_table)
{
	Lua_Safe_Call_Class(Lua_Expedition);

	auto instance_info = expedition_table.get<sol::table>("instance");
	auto expedition_info = expedition_table.get<sol::table>("expedition");
	auto zone = instance_info.get<sol::optional<sol::object>>("zone");

	uint32_t zone_id = 0;
	if (zone) {
		auto object = zone.value();
		if (object.get_type() == sol::type::string) {
			zone_id = ZoneID(object.as<std::string>());
		} else if (object.get_type() == sol::type::number) {
			zone_id = object.as<uint32_t>();
		}
	}

	auto zone_version  = instance_info.get<uint32_t>("version");
	auto zone_duration = instance_info.get<uint32_t>("duration");

	DynamicZone dz{ zone_id, zone_version, zone_duration, DynamicZoneType::Expedition };
	dz.SetName(expedition_info.get<std::string>("name"));
	dz.SetMinPlayers(expedition_info.get<uint32_t>("min_players"));
	dz.SetMaxPlayers(expedition_info.get<uint32_t>("max_players"));

	// the dz_info table supports optional hash entries for 'compass', 'safereturn', and 'zonein' data
	auto compass = expedition_info.get<sol::optional<sol::table>>("compass");
	if (compass) {
		auto compass_loc = GetDynamicZoneLocationFromTable(compass.value());
		dz.SetCompass(compass_loc);
	}

	auto safereturn = expedition_info.get<sol::optional<sol::table>>("safereturn");
	if (safereturn) {
		auto safereturn_loc = GetDynamicZoneLocationFromTable(safereturn.value());
		dz.SetSafeReturn(safereturn_loc);
	}

	auto zonein = expedition_info.get<sol::optional<sol::table>>("zonein");
	if (zonein) {
		auto zonein_loc = GetDynamicZoneLocationFromTable(zonein.value());
		dz.SetZoneInLocation(zonein_loc);
	}

	auto switchid = expedition_info.get<sol::optional<int>>("switchid");
	if (switchid) {
		dz.SetSwitchID(switchid.value());
	}

	bool disable_messages = expedition_info.get_or("disable_messages", false);

	return self->CreateExpedition(dz, disable_messages);
}

Lua_Expedition Lua_Client::CreateExpedition(std::string zone_name, uint32 version, uint32 duration, std::string expedition_name, uint32 min_players, uint32 max_players) {
	Lua_Safe_Call_Class(Lua_Expedition);
	return self->CreateExpedition(zone_name, version, duration, expedition_name, min_players, max_players);
}

Lua_Expedition Lua_Client::CreateExpedition(std::string zone_name, uint32 version, uint32 duration, std::string expedition_name, uint32 min_players, uint32 max_players, bool disable_messages) {
	Lua_Safe_Call_Class(Lua_Expedition);
	return self->CreateExpedition(zone_name, version, duration, expedition_name, min_players, max_players, disable_messages);
}

Lua_Expedition Lua_Client::CreateExpeditionFromTemplate(uint32_t dz_template_id) {
	Lua_Safe_Call_Class(Lua_Expedition);
	return self->CreateExpeditionFromTemplate(dz_template_id);
}

Lua_Expedition Lua_Client::GetExpedition() {
	Lua_Safe_Call_Class(Lua_Expedition);
	return self->GetExpedition();
}

sol::table Lua_Client::GetExpeditionLockouts(sol::this_state s)
{
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto lockouts = self->GetExpeditionLockouts();

		for (const auto& lockout : lockouts) {
			auto lockout_table = lua_table[lockout.GetExpeditionName()].get_or_create<sol::table>();
			lockout_table[lockout.GetEventName()] = lockout.GetSecondsRemaining();
		}
	}
	return lua_table;
}

sol::table Lua_Client::GetExpeditionLockouts(sol::this_state s, std::string expedition_name)
{
	sol::state_view sv(s);
	auto lua_table = sv.create_table();
	if (d_)
	{
		auto self = reinterpret_cast<NativeType*>(d_);
		auto lockouts = self->GetExpeditionLockouts();

		for (const auto& lockout : lockouts)
		{
			if (lockout.GetExpeditionName() == expedition_name)
			{
				lua_table[lockout.GetEventName()] = lockout.GetSecondsRemaining();
			}
		}
	}
	return lua_table;
}

std::string Lua_Client::GetLockoutExpeditionUUID(std::string expedition_name, std::string event_name) {
	Lua_Safe_Call_String();
	std::string uuid;
	auto lockout = self->GetExpeditionLockout(expedition_name, event_name);
	if (lockout)
	{
		uuid = lockout->GetExpeditionUUID();
	}
	return uuid;
}

void Lua_Client::AddExpeditionLockout(std::string expedition_name, std::string event_name, uint32 seconds) {
	Lua_Safe_Call_Void();
	self->AddNewExpeditionLockout(expedition_name, event_name, seconds);
}

void Lua_Client::AddExpeditionLockout(std::string expedition_name, std::string event_name, uint32 seconds, std::string uuid) {
	Lua_Safe_Call_Void();
	self->AddNewExpeditionLockout(expedition_name, event_name, seconds, uuid);
}

void Lua_Client::AddExpeditionLockoutDuration(std::string expedition_name, std::string event_name, int seconds) {
	Lua_Safe_Call_Void();
	self->AddExpeditionLockoutDuration(expedition_name, event_name, seconds, {}, true);
}

void Lua_Client::AddExpeditionLockoutDuration(std::string expedition_name, std::string event_name, int seconds, std::string uuid) {
	Lua_Safe_Call_Void();
	self->AddExpeditionLockoutDuration(expedition_name, event_name, seconds, uuid, true);
}

void Lua_Client::RemoveAllExpeditionLockouts() {
	Lua_Safe_Call_Void();
	self->RemoveAllExpeditionLockouts({}, true);
}

void Lua_Client::RemoveAllExpeditionLockouts(std::string expedition_name) {
	Lua_Safe_Call_Void();
	self->RemoveAllExpeditionLockouts(expedition_name, true);
}

void Lua_Client::RemoveExpeditionLockout(std::string expedition_name, std::string event_name) {
	Lua_Safe_Call_Void();
	self->RemoveExpeditionLockout(expedition_name, event_name, true);
}

bool Lua_Client::HasExpeditionLockout(std::string expedition_name, std::string event_name) {
	Lua_Safe_Call_Bool();
	return self->HasExpeditionLockout(expedition_name, event_name);
}

void Lua_Client::MovePCDynamicZone(uint32 zone_id) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_id);
}

void Lua_Client::MovePCDynamicZone(uint32 zone_id, int zone_version) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_id, zone_version);
}

void Lua_Client::MovePCDynamicZone(uint32 zone_id, int zone_version, bool msg_if_invalid) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_id, zone_version, msg_if_invalid);
}

void Lua_Client::MovePCDynamicZone(std::string zone_name) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_name);
}

void Lua_Client::MovePCDynamicZone(std::string zone_name, int zone_version) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_name, zone_version);
}

void Lua_Client::MovePCDynamicZone(std::string zone_name, int zone_version, bool msg_if_invalid) {
	Lua_Safe_Call_Void();
	return self->MovePCDynamicZone(zone_name, zone_version, msg_if_invalid);
}

void Lua_Client::CreateTaskDynamicZone(int task_id, sol::table dz_table) {
	Lua_Safe_Call_Void();

	auto instance_info = dz_table.get<sol::table>("instance");
	auto zone = instance_info.get<sol::optional<sol::object>>("zone");

	uint32_t zone_id = 0;
	if (zone) {
		auto object = zone.value();
		if (object.get_type() == sol::type::string) {
			zone_id = ZoneID(object.as<std::string>());
		} else if (object.get_type() == sol::type::number) {
			zone_id = object.as<uint32_t>();
		}
	}

	auto zone_version = instance_info.get<uint32_t>("version");

	// tasks override dz duration so duration is ignored here
	DynamicZone dz{ zone_id, zone_version, 0, DynamicZoneType::None };

	// the dz_info table supports optional hash entries for 'compass', 'safereturn', and 'zonein' data
	auto compass = dz_table.get<sol::optional<sol::table>>("compass");
	if (compass) {
		auto compass_loc = GetDynamicZoneLocationFromTable(compass.value());
		dz.SetCompass(compass_loc);
	}

	auto safereturn = dz_table.get<sol::optional<sol::table>>("safereturn");
	if (safereturn) {
		auto safereturn_loc = GetDynamicZoneLocationFromTable(safereturn.value());
		dz.SetSafeReturn(safereturn_loc);
	}

	auto zonein = dz_table.get<sol::optional<sol::table>>("zonein");
	if (zonein) {
		auto zonein_loc = GetDynamicZoneLocationFromTable(zonein.value());
		dz.SetZoneInLocation(zonein_loc);
	}

	auto switchid = dz_table.get<sol::optional<int>>("switchid");
	if (switchid) {
		dz.SetSwitchID(switchid.value());
	}

	self->CreateTaskDynamicZone(task_id, dz);
}

void Lua_Client::Fling(float value, float target_x, float target_y, float target_z) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z);
}

void Lua_Client::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z, ignore_los);
}

void Lua_Client::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clipping) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z, ignore_los, clipping);
}

double Lua_Client::GetAAEXPModifier(uint32 zone_id) {
	Lua_Safe_Call_Real();
	return self->GetAAEXPModifier(zone_id);
}

double Lua_Client::GetAAEXPModifier(uint32 zone_id, int16 instance_version) {
	Lua_Safe_Call_Real();
	return self->GetAAEXPModifier(zone_id, instance_version);
}

double Lua_Client::GetEXPModifier(uint32 zone_id) {
	Lua_Safe_Call_Real();
	return self->GetEXPModifier(zone_id);
}

double Lua_Client::GetEXPModifier(uint32 zone_id, int16 instance_version) {
	Lua_Safe_Call_Real();
	return self->GetEXPModifier(zone_id, instance_version);
}

void Lua_Client::SetAAEXPModifier(uint32 zone_id, double aa_modifier) {
	Lua_Safe_Call_Void();
	self->SetAAEXPModifier(zone_id, aa_modifier);
}

void Lua_Client::SetAAEXPModifier(uint32 zone_id, double aa_modifier, int16 instance_version) {
	Lua_Safe_Call_Void();
	self->SetAAEXPModifier(zone_id, aa_modifier, instance_version);
}

void Lua_Client::SetEXPModifier(uint32 zone_id, double exp_modifier) {
	Lua_Safe_Call_Void();
	self->SetEXPModifier(zone_id, exp_modifier);
}

void Lua_Client::SetEXPModifier(uint32 zone_id, double exp_modifier, int16 instance_version) {
	Lua_Safe_Call_Void();
	self->SetEXPModifier(zone_id, exp_modifier, instance_version);
}

void Lua_Client::AddLDoNLoss(uint32 theme_id) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNWinLoss(theme_id);
}

void Lua_Client::AddLDoNWin(uint32 theme_id) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNWinLoss(theme_id, true);
}

void Lua_Client::SetHideMe(bool hide_me_state) {
	Lua_Safe_Call_Void();
	self->SetHideMe(hide_me_state);
}

void Lua_Client::Popup(const char* title, const char* text) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 button_type) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id, button_type);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 button_type, uint32 duration) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id, button_type, duration);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 button_type, uint32 duration, const char* button_name_one, const char* button_name_two) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id, button_type, duration, button_name_one, button_name_two);
}

void Lua_Client::Popup(const char* title, const char* text, uint32 popup_id, uint32 negative_id, uint32 button_type, uint32 duration, const char* button_name_one, const char* button_name_two, uint32 sound_controls) {
	Lua_Safe_Call_Void();
	self->SendFullPopup(title, text, popup_id, negative_id, button_type, duration, button_name_one, button_name_two, sound_controls);
}

void Lua_Client::ResetAllDisciplineTimers() {
	Lua_Safe_Call_Void();
	self->ResetAllDisciplineTimers();
}

void Lua_Client::SendToInstance(std::string instance_type, std::string zone_short_name, uint32 instance_version, float x, float y, float z, float heading, std::string instance_identifier, uint32 duration) {
	Lua_Safe_Call_Void();
	self->SendToInstance(instance_type, zone_short_name, instance_version, x, y, z, heading, instance_identifier, duration);
}

int Lua_Client::CountItem(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountItem(item_id);
}

void Lua_Client::RemoveItem(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id);
}

void Lua_Client::RemoveItem(uint32 item_id, uint32 quantity) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id, quantity);
}

void Lua_Client::SetGMStatus(int16 new_status) {
	Lua_Safe_Call_Void();
	self->SetGMStatus(new_status);
}

int16 Lua_Client::GetGMStatus() {
	Lua_Safe_Call_Int();
	return self->Admin();
}

void Lua_Client::UntrainDiscBySpellID(uint16 spell_id) {
	Lua_Safe_Call_Void();
	self->UntrainDiscBySpellID(spell_id);
}

void Lua_Client::UntrainDiscBySpellID(uint16 spell_id, bool update_client) {
	Lua_Safe_Call_Void();
	self->UntrainDiscBySpellID(spell_id, update_client);
}

int Lua_Client::GetIPExemption() {
	Lua_Safe_Call_Int();
	return self->GetIPExemption();
}

std::string Lua_Client::GetIPString() {
	Lua_Safe_Call_String();
	return self->GetIPString();
}

void Lua_Client::SetIPExemption(int exemption_amount) {
	Lua_Safe_Call_Void();
	self->SetIPExemption(exemption_amount);
}

void Lua_Client::ReadBookByName(std::string book_name, uint8 book_type) {
	Lua_Safe_Call_Void();
	self->ReadBookByName(book_name, book_type);
}

void Lua_Client::SummonBaggedItems(uint32 bag_item_id, sol::table bag_items_table) {
	Lua_Safe_Call_Void();

	std::vector<ServerLootItem_Struct> bagged_items;

	for (const auto& kv : bag_items_table) {
		auto value = kv.second;
		// we need a table ...
		if (value.get_type() == sol::type::table) {
			auto inner = value.as<sol::table>();
			ServerLootItem_Struct item{};
			item.item_id = inner.get<uint32_t>("item_id");
			item.charges = inner.get<int16_t>("charges");
			// these are optional
			item.attuned = inner.get_or("attuned", 0);
			item.aug_1 = inner.get_or("augment_one", 0);
			item.aug_2 = inner.get_or("augment_two", 0);
			item.aug_3 = inner.get_or("augment_three", 0);
			item.aug_4 = inner.get_or("augment_four", 0);
			item.aug_5 = inner.get_or("augment_five", 0);
			item.aug_6 = inner.get_or("augment_six", 0);
			bagged_items.emplace_back(item);
		}
	}

	self->SummonBaggedItems(bag_item_id, bagged_items);
}

void Lua_Client::RemoveLDoNLoss(uint32 theme_id) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNWinLoss(theme_id, false, true);
}

void Lua_Client::RemoveLDoNWin(uint32 theme_id) {
	Lua_Safe_Call_Void();
	self->UpdateLDoNWinLoss(theme_id, true, true);
}

uint16 Lua_Client::ScribeSpells(uint8 min_level, uint8 max_level) {
	Lua_Safe_Call_Int();
	return self->ScribeSpells(min_level, max_level);
}

uint16 Lua_Client::LearnDisciplines(uint8 min_level, uint8 max_level) {
	Lua_Safe_Call_Int();
	return self->LearnDisciplines(min_level, max_level);
}

int Lua_Client::GetNextAvailableDisciplineSlot() {
	Lua_Safe_Call_Int();
	return self->GetNextAvailableDisciplineSlot();
}

int Lua_Client::GetNextAvailableDisciplineSlot(int starting_slot) {
	Lua_Safe_Call_Int();
	return self->GetNextAvailableDisciplineSlot(starting_slot);
}

void Lua_Client::ResetCastbarCooldownBySlot(int slot) {
	Lua_Safe_Call_Void();
	self->ResetCastbarCooldownBySlot(slot);
}

void Lua_Client::ResetAllCastbarCooldowns() {
	Lua_Safe_Call_Void();
	self->ResetAllCastbarCooldowns();
}

void Lua_Client::ResetCastbarCooldownBySpellID(uint32 spell_id) {
	Lua_Safe_Call_Void();
	self->ResetCastbarCooldownBySpellID(spell_id);
}

void Lua_Client::UnscribeSpellBySpellID(uint16 spell_id) {
	Lua_Safe_Call_Void();
	self->UnscribeSpellBySpellID(spell_id);
}

void Lua_Client::UnscribeSpellBySpellID(uint16 spell_id, bool update_client) {
	Lua_Safe_Call_Void();
	self->UnscribeSpellBySpellID(spell_id, update_client);
}

int Lua_Client::GetEnvironmentDamageModifier() {
	Lua_Safe_Call_Int();
	return self->GetEnvironmentDamageModifier();
}

void Lua_Client::SetEnvironmentDamageModifier(int value) {
	Lua_Safe_Call_Void();
	self->SetEnvironmentDamageModifier(value);
}

bool Lua_Client::GetInvulnerableEnvironmentDamage() {
	Lua_Safe_Call_Bool();
	return self->GetInvulnerableEnvironmentDamage();
}

void Lua_Client::SetInvulnerableEnvironmentDamage(bool value) {
	Lua_Safe_Call_Void();
	self->SetInvulnerableEnvironmentDamage(value);
}

void Lua_Client::AddItem(sol::table item_table) {
	Lua_Safe_Call_Void();

	uint32 item_id = item_table.get<uint32_t>("item_id");
	int16 charges = item_table.get<uint32>("charges");
	// optional
	uint32 augment_one = item_table.get_or("augment_one", 0);
	uint32 augment_two = item_table.get_or("augment_two", 0);
	uint32 augment_three = item_table.get_or("augment_three", 0);
	uint32 augment_four = item_table.get_or("augment_four", 0);
	uint32 augment_five = item_table.get_or("augment_five", 0);
	uint32 augment_six = item_table.get_or("augment_six", 0);
	bool attuned = item_table.get_or("attuned", false);
	uint16 slot_id = item_table.get_or("slot_id", EQ::invslot::slotCursor);

	self->SummonItem(
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

int Lua_Client::CountAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetInv().CountAugmentEquippedByID(item_id);
}

bool Lua_Client::HasAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->GetInv().HasAugmentEquippedByID(item_id);
}

int Lua_Client::CountItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetInv().CountItemEquippedByID(item_id);
}

bool Lua_Client::HasItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->GetInv().HasItemEquippedByID(item_id);
}

void Lua_Client::AddPlatinum(uint32 platinum) {
	Lua_Safe_Call_Void();
	self->AddPlatinum(platinum);
}

void Lua_Client::AddPlatinum(uint32 platinum, bool update_client) {
	Lua_Safe_Call_Void();
	self->AddPlatinum(platinum, update_client);
}

uint32 Lua_Client::GetCarriedPlatinum() {
	Lua_Safe_Call_Int();
	return self->GetCarriedPlatinum();
}

bool Lua_Client::TakePlatinum(uint32 platinum) {
	Lua_Safe_Call_Bool();
	return self->TakePlatinum(platinum);
}

bool Lua_Client::TakePlatinum(uint32 platinum, bool update_client) {
	Lua_Safe_Call_Bool();
	return self->TakePlatinum(platinum, update_client);
}

void Lua_Client::LoadZoneFlags() {
	Lua_Safe_Call_Void();
	self->LoadZoneFlags();
}

void Lua_Client::ClearPEQZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Void();
	self->ClearPEQZoneFlag(zone_id);
}

bool Lua_Client::HasPEQZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Bool();
	return self->HasPEQZoneFlag(zone_id);
}

void Lua_Client::LoadPEQZoneFlags() {
	Lua_Safe_Call_Void();
	self->LoadPEQZoneFlags();
}

void Lua_Client::SendPEQZoneFlagInfo(Lua_Client to) {
	Lua_Safe_Call_Void();
	self->SendPEQZoneFlagInfo(to);
}

void Lua_Client::SetPEQZoneFlag(uint32 zone_id) {
	Lua_Safe_Call_Void();
	self->SetPEQZoneFlag(zone_id);
}

int Lua_Client::GetHealAmount() {
	Lua_Safe_Call_Int();
	return self->GetHealAmt();
}

int Lua_Client::GetSpellDamage() {
	Lua_Safe_Call_Int();
	return self->GetSpellDmg();
}

void Lua_Client::TaskSelector(sol::table table) {
	TaskSelector(table, false);
}

void Lua_Client::TaskSelector(sol::table table, bool ignore_cooldown) {
	Lua_Safe_Call_Void();

	std::vector<int> tasks;
	for(int i = 1; i <= MAXCHOOSERENTRIES; ++i) {
		auto optional = table.get<sol::optional<int>>(i);
		if (optional) {
			tasks.push_back(optional.value());
		}
	}

	self->TaskQuestSetSelector(self, tasks, ignore_cooldown);
}

bool Lua_Client::TeleportToPlayerByCharID(uint32 character_id) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayer(database.GetCharNameByID(character_id));
}

bool Lua_Client::TeleportToPlayerByName(std::string player_name) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayer(player_name);
}

bool Lua_Client::TeleportGroupToPlayerByCharID(uint32 character_id) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayerGroup(database.GetCharNameByID(character_id));
}

bool Lua_Client::TeleportGroupToPlayerByName(std::string player_name) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayerGroup(player_name);
}

bool Lua_Client::TeleportRaidToPlayerByCharID(uint32 character_id) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayerRaid(database.GetCharNameByID(character_id));
}

bool Lua_Client::TeleportRaidToPlayerByName(std::string player_name) {
	Lua_Safe_Call_Bool();
	return self->GotoPlayerRaid(player_name);
}

int Lua_Client::GetRecipeMadeCount(uint32 recipe_id) {
	Lua_Safe_Call_Int();
	return self->GetRecipeMadeCount(recipe_id);
}

bool Lua_Client::HasRecipeLearned(uint32 recipe_id) {
	Lua_Safe_Call_Bool();
	return self->HasRecipeLearned(recipe_id);
}

#endif
