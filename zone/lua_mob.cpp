#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_mob.h"

const char *Lua_Mob::GetName() {
	Lua_Safe_Call_String(Mob);
	return self->GetName();
}

void Lua_Mob::Depop() {
	Lua_Safe_Call_Void(Mob);
	return self->Depop();
}

void Lua_Mob::Depop(bool start_spawn_timer) {
	Lua_Safe_Call_Void(Mob);
	return self->Depop(start_spawn_timer);
}

void Lua_Mob::RogueAssassinate(Lua_Mob other) {
	Lua_Safe_Call_Void(Mob);
	self->RogueAssassinate(other);
}

bool Lua_Mob::BehindMob() {
	Lua_Safe_Call_Bool(Mob);
	return self->BehindMob();
}

bool Lua_Mob::BehindMob(Lua_Mob other) {
	Lua_Safe_Call_Bool(Mob);
	return self->BehindMob(other);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x) {
	Lua_Safe_Call_Bool(Mob);
	return self->BehindMob(other, x);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x, float y) {
	Lua_Safe_Call_Bool(Mob);
	return self->BehindMob(other, x, y);
}

void Lua_Mob::SetLevel(int level) {
	Lua_Safe_Call_Void(Mob);
	self->SetLevel(level);
}

void Lua_Mob::SetLevel(int level, bool command) {
	Lua_Safe_Call_Void(Mob);
	self->SetLevel(level, command);
}

void Lua_Mob::SendWearChange(int material_slot) {
	Lua_Safe_Call_Void(Mob);
	self->SendWearChange(material_slot);
}

uint32 Lua_Mob::GetEquipment(int material_slot) {
	Lua_Safe_Call_Int(Mob);
	return self->GetEquipment(material_slot);
}

int32 Lua_Mob::GetEquipmentMaterial(int material_slot) {
	Lua_Safe_Call_Int(Mob);
	return self->GetEquipmentMaterial(material_slot);
}

uint32 Lua_Mob::GetEquipmentColor(int material_slot) {
	Lua_Safe_Call_Int(Mob);
	return self->GetEquipmentColor(material_slot);
}

uint32 Lua_Mob::GetArmorTint(int i) {
	Lua_Safe_Call_Int(Mob);
	return self->GetArmorTint(i);
}

bool Lua_Mob::IsMoving() {
	Lua_Safe_Call_Bool(Mob);
	return self->IsMoving();
}

void Lua_Mob::GotoBind() {
	Lua_Safe_Call_Void(Mob);
	self->GoToBind();
}

void Lua_Mob::Gate() {
	Lua_Safe_Call_Void(Mob);
	self->Gate();
}

bool Lua_Mob::Attack(Lua_Mob other) {
	Lua_Safe_Call_Bool(Mob);
	return self->Attack(other);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand) {
	Lua_Safe_Call_Bool(Mob);
	return self->Attack(other, hand);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte) {
	Lua_Safe_Call_Bool(Mob);
	return self->Attack(other, hand, from_riposte);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough) {
	Lua_Safe_Call_Bool(Mob);
	return self->Attack(other, hand, from_riposte, is_strikethrough);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell) {
	Lua_Safe_Call_Bool(Mob);
	return self->Attack(other, hand, from_riposte, is_strikethrough, is_from_spell);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill) {
	Lua_Safe_Call_Void(Mob);
	return self->Damage(from, damage, spell_id, static_cast<SkillType>(attack_skill));
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable) {
	Lua_Safe_Call_Void(Mob);
	return self->Damage(from, damage, spell_id, static_cast<SkillType>(attack_skill), avoidable);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot) {
	Lua_Safe_Call_Void(Mob);
	return self->Damage(from, damage, spell_id, static_cast<SkillType>(attack_skill), avoidable, buffslot);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot, bool buff_tic) {
	Lua_Safe_Call_Void(Mob);
	return self->Damage(from, damage, spell_id, static_cast<SkillType>(attack_skill), avoidable, buffslot, buff_tic);
}

void Lua_Mob::RangedAttack(Lua_Mob other) {
	Lua_Safe_Call_Void(Mob);
	self->RangedAttack(other);
}

void Lua_Mob::ThrowingAttack(Lua_Mob other) {
	Lua_Safe_Call_Void(Mob);
	self->ThrowingAttack(other);
}

void Lua_Mob::Heal() {
	Lua_Safe_Call_Void(Mob);
	self->Heal();
}

void Lua_Mob::HealDamage(uint32 amount) {
	Lua_Safe_Call_Void(Mob)
	self->HealDamage(amount);
}

void Lua_Mob::HealDamage(uint32 amount, Lua_Mob other) {
	Lua_Safe_Call_Void(Mob)
	self->HealDamage(amount, other);
}

uint32 Lua_Mob::GetLevelCon(int other) {
	Lua_Safe_Call_Int(Mob)
	return self->GetLevelCon(other);
}

uint32 Lua_Mob::GetLevelCon(int my, int other) {
	Lua_Safe_Call_Int(Mob)
	return self->GetLevelCon(my, other);
}

void Lua_Mob::SetHP(int hp) {
	Lua_Safe_Call_Void(Mob)
	self->SetHP(hp);
}

void Lua_Mob::DoAnim(int anim_num) {
	Lua_Safe_Call_Void(Mob)
	self->DoAnim(anim_num);
}

void Lua_Mob::DoAnim(int anim_num, int type) {
	Lua_Safe_Call_Void(Mob)
	self->DoAnim(anim_num, type);
}

void Lua_Mob::DoAnim(int anim_num, int type, bool ackreq) {
	Lua_Safe_Call_Void(Mob)
	self->DoAnim(anim_num, type, ackreq);
}

void Lua_Mob::DoAnim(int anim_num, int type, bool ackreq, int filter) {
	Lua_Safe_Call_Void(Mob)
	self->DoAnim(anim_num, type, ackreq, static_cast<eqFilterType>(filter));
}

void Lua_Mob::ChangeSize(double in_size) {
	Lua_Safe_Call_Void(Mob)
	self->ChangeSize(in_size);
}

void Lua_Mob::ChangeSize(double in_size, bool no_restriction) {
	Lua_Safe_Call_Void(Mob)
	self->ChangeSize(in_size, no_restriction);
}

void Lua_Mob::GMMove(double x, double y, double z) {
	Lua_Safe_Call_Void(Mob)
	self->GMMove(x, y, z);
}

void Lua_Mob::GMMove(double x, double y, double z, double heading) {
	Lua_Safe_Call_Void(Mob)
	self->GMMove(x, y, z, heading);
}

void Lua_Mob::GMMove(double x, double y, double z, double heading, bool send_update) {
	Lua_Safe_Call_Void(Mob)
	self->GMMove(x, y, z, heading, send_update);
}

void Lua_Mob::SendPosUpdate() {
	Lua_Safe_Call_Void(Mob)
	self->SendPosUpdate();
}

void Lua_Mob::SendPosUpdate(bool send_to_self) {
	Lua_Safe_Call_Void(Mob)
	self->SendPosUpdate(send_to_self ? 1 : 0);
}

void Lua_Mob::SendPosition() {
	Lua_Safe_Call_Void(Mob)
	self->SendPosition();
}

bool Lua_Mob::HasProcs() {
	Lua_Safe_Call_Bool(Mob)
	return self->HasProcs();
}

bool Lua_Mob::IsInvisible() {
	Lua_Safe_Call_Bool(Mob)
	return self->IsInvisible();
}

bool Lua_Mob::IsInvisible(Lua_Mob other) {
	Lua_Safe_Call_Bool(Mob)
	return self->IsInvisible(other);
}

void Lua_Mob::SetInvisible(int state) {
	Lua_Safe_Call_Void(Mob)
	self->SetInvisible(state);
}

bool Lua_Mob::FindBuff(int spell_id) {
	Lua_Safe_Call_Bool(Mob)
	return self->FindBuff(spell_id);
}

bool Lua_Mob::FindType(int type) {
	Lua_Safe_Call_Bool(Mob)
	return self->FindType(type);
}

bool Lua_Mob::FindType(int type, bool offensive) {
	Lua_Safe_Call_Bool(Mob)
	return self->FindType(type, offensive);
}

bool Lua_Mob::FindType(int type, bool offensive, int threshold) {
	Lua_Safe_Call_Bool(Mob)
	return self->FindType(type, offensive, threshold);
}

int Lua_Mob::GetBuffSlotFromType(int slot) {
	Lua_Safe_Call_Bool(Mob)
	return self->GetBuffSlotFromType(slot);
}

void Lua_Mob::MakePet(int spell_id, const char* pet_type) {
	Lua_Safe_Call_Void(Mob)
	self->MakePet(spell_id, pet_type);
}

void Lua_Mob::MakePet(int spell_id, const char* pet_type, const char *pet_name) {
	Lua_Safe_Call_Void(Mob)
	self->MakePet(spell_id, pet_type, pet_name);
}

void Lua_Mob::MakePoweredPet(int spell_id, const char* pet_type, int pet_power) {
	Lua_Safe_Call_Void(Mob)
	self->MakePoweredPet(spell_id, pet_type, pet_power);
}

void Lua_Mob::MakePoweredPet(int spell_id, const char* pet_type, int pet_power, const char *pet_name) {
	Lua_Safe_Call_Void(Mob)
	self->MakePoweredPet(spell_id, pet_type, pet_power, pet_name);
}

int Lua_Mob::GetBaseRace() {
	Lua_Safe_Call_Int(Mob)
	return self->GetBaseRace();
}

int Lua_Mob::GetBaseGender() {
	Lua_Safe_Call_Int(Mob)
	return self->GetBaseGender();
}

int Lua_Mob::GetDeity() {
	Lua_Safe_Call_Int(Mob)
	return self->GetDeity();
}

int Lua_Mob::GetRace() {
	Lua_Safe_Call_Int(Mob)
	return self->GetRace();
}

int Lua_Mob::GetGender() {
	Lua_Safe_Call_Int(Mob)
	return self->GetGender();
}

int Lua_Mob::GetTexture() {
	Lua_Safe_Call_Int(Mob)
	return self->GetTexture();
}

int Lua_Mob::GetHelmTexture() {
	Lua_Safe_Call_Int(Mob)
	return self->GetHelmTexture();
}

int Lua_Mob::GetHairColor() {
	Lua_Safe_Call_Int(Mob)
	return self->GetHairColor();
}

int Lua_Mob::GetBeardColor() {
	Lua_Safe_Call_Int(Mob)
	return self->GetBeardColor();
}

int Lua_Mob::GetEyeColor1() {
	Lua_Safe_Call_Int(Mob)
	return self->GetEyeColor1();
}

int Lua_Mob::GetEyeColor2() {
	Lua_Safe_Call_Int(Mob)
	return self->GetEyeColor2();
}

int Lua_Mob::GetHairStyle() {
	Lua_Safe_Call_Int(Mob)
	return self->GetHairStyle();
}

int Lua_Mob::GetLuclinFace() {
	Lua_Safe_Call_Int(Mob)
	return self->GetLuclinFace();
}

int Lua_Mob::GetBeard() {
	Lua_Safe_Call_Int(Mob)
	return self->GetBeard();
}

int Lua_Mob::GetDrakkinHeritage() {
	Lua_Safe_Call_Int(Mob)
	return self->GetDrakkinHeritage();
}

int Lua_Mob::GetDrakkinTattoo() {
	Lua_Safe_Call_Int(Mob)
	return self->GetDrakkinTattoo();
}

int Lua_Mob::GetDrakkinDetails() {
	Lua_Safe_Call_Int(Mob)
	return self->GetDrakkinDetails();
}

int Lua_Mob::GetClass() {
	Lua_Safe_Call_Int(Mob)
	return self->GetClass();
}

int Lua_Mob::GetLevel() {
	Lua_Safe_Call_Int(Mob)
	return self->GetLevel();
}

const char *Lua_Mob::GetCleanName() {
	Lua_Safe_Call_String(Mob)
	return self->GetCleanName();
}

Lua_Mob Lua_Mob::GetTarget() {
	Lua_Safe_Call_Mob(Mob)
	return Lua_Mob(self->GetTarget());
}

void Lua_Mob::SetTarget(Lua_Mob t) {
	Lua_Safe_Call_Void(Mob)
	self->SetTarget(t);
}

double Lua_Mob::GetHPRatio() {
	Lua_Safe_Call_Real(Mob)
	return self->GetHPRatio();
}

bool Lua_Mob::IsWarriorClass() {
	Lua_Safe_Call_Bool(Mob)
	return self->IsWarriorClass();
}

int Lua_Mob::GetHP() {
	Lua_Safe_Call_Int(Mob)
	return self->GetHP();
}

int Lua_Mob::GetMaxHP() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxHP();
}

int Lua_Mob::GetItemHPBonuses() {
	Lua_Safe_Call_Int(Mob)
	return self->GetItemHPBonuses();
}

int Lua_Mob::GetSpellHPBonuses() {
	Lua_Safe_Call_Int(Mob)
	return self->GetSpellHPBonuses();
}

double Lua_Mob::GetWalkspeed() {
	Lua_Safe_Call_Real(Mob)
	return self->GetWalkspeed();
}

double Lua_Mob::GetRunspeed() {
	Lua_Safe_Call_Real(Mob)
	return self->GetRunspeed();
}

int Lua_Mob::GetCasterLevel(int spell_id) {
	Lua_Safe_Call_Int(Mob)
	return self->GetCasterLevel(spell_id);
}

int Lua_Mob::GetMaxMana() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxMana();
}

int Lua_Mob::GetMana() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMana();
}

int Lua_Mob::SetMana(int mana) {
	Lua_Safe_Call_Int(Mob)
	return self->SetMana(mana);
}

double Lua_Mob::GetManaRatio() {
	Lua_Safe_Call_Real(Mob)
	return self->GetManaRatio();
}

int Lua_Mob::GetAC() {
	Lua_Safe_Call_Int(Mob)
	return self->GetAC();
}

int Lua_Mob::GetATK() {
	Lua_Safe_Call_Int(Mob)
	return self->GetATK();
}

int Lua_Mob::GetSTR() {
	Lua_Safe_Call_Int(Mob)
	return self->GetSTR();
}

int Lua_Mob::GetSTA() {
	Lua_Safe_Call_Int(Mob)
	return self->GetSTA();
}

int Lua_Mob::GetDEX() {
	Lua_Safe_Call_Int(Mob)
	return self->GetDEX();
}

int Lua_Mob::GetAGI() {
	Lua_Safe_Call_Int(Mob)
	return self->GetAGI();
}

int Lua_Mob::GetINT() {
	Lua_Safe_Call_Int(Mob)
	return self->GetINT();
}

int Lua_Mob::GetWIS() {
	Lua_Safe_Call_Int(Mob)
	return self->GetWIS();
}

int Lua_Mob::GetCHA() {
	Lua_Safe_Call_Int(Mob)
	return self->GetCHA();
}

int Lua_Mob::GetMR() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMR();
}

int Lua_Mob::GetFR() {
	Lua_Safe_Call_Int(Mob)
	return self->GetFR();
}

int Lua_Mob::GetDR() {
	Lua_Safe_Call_Int(Mob)
	return self->GetDR();
}

int Lua_Mob::GetPR() {
	Lua_Safe_Call_Int(Mob)
	return self->GetPR();
}

int Lua_Mob::GetCR() {
	Lua_Safe_Call_Int(Mob)
	return self->GetCR();
}

int Lua_Mob::GetCorruption() {
	Lua_Safe_Call_Int(Mob)
	return self->GetCorrup();
}

int Lua_Mob::GetMaxSTR() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxSTR();
}

int Lua_Mob::GetMaxSTA() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxSTA();
}

int Lua_Mob::GetMaxDEX() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxDEX();
}

int Lua_Mob::GetMaxAGI() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxAGI();
}

int Lua_Mob::GetMaxINT() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxINT();
}

int Lua_Mob::GetMaxWIS() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxWIS();
}

int Lua_Mob::GetMaxCHA() {
	Lua_Safe_Call_Int(Mob)
	return self->GetMaxCHA();
}

double Lua_Mob::GetActSpellRange(int spell_id, double range) {
	Lua_Safe_Call_Real(Mob)
	return self->GetActSpellRange(spell_id, range);
}

double Lua_Mob::GetActSpellRange(int spell_id, double range, bool is_bard) {
	Lua_Safe_Call_Real(Mob)
	return self->GetActSpellRange(spell_id, range, is_bard);
}

int Lua_Mob::GetActSpellDamage(int spell_id, int value) {
	Lua_Safe_Call_Int(Mob);
	return self->GetActSpellDamage(spell_id, value);
}

int Lua_Mob::GetActSpellHealing(int spell_id, int value) {
	Lua_Safe_Call_Int(Mob);
	return self->GetActSpellHealing(spell_id, value);
}

int Lua_Mob::GetActSpellCost(int spell_id, int cost) {
	Lua_Safe_Call_Int(Mob);
	return self->GetActSpellCost(spell_id, cost);
}

int Lua_Mob::GetActSpellDuration(int spell_id, int duration) {
	Lua_Safe_Call_Int(Mob);
	return self->GetActSpellDuration(spell_id, duration);
}

int Lua_Mob::GetActSpellCasttime(int spell_id, int cast_time) {
	Lua_Safe_Call_Int(Mob);
	return self->GetActSpellCasttime(spell_id, cast_time);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster) {
	Lua_Safe_Call_Real(Mob);
	return self->ResistSpell(resist_type, spell_id, caster);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override) {
	Lua_Safe_Call_Real(Mob);
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override) {
	Lua_Safe_Call_Real(Mob);
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override, resist_override);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override, 
	bool charisma_check) {
	Lua_Safe_Call_Real(Mob);
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override, resist_override, charisma_check);
}

int Lua_Mob::GetSpecializeSkillValue(int spell_id) {
	Lua_Safe_Call_Int(Mob);
	return self->GetSpecializeSkillValue(spell_id);
}

int Lua_Mob::GetNPCTypeID() {
	Lua_Safe_Call_Int(Mob);
	return self->GetNPCTypeID();
}

bool Lua_Mob::IsTargeted() {
	Lua_Safe_Call_Bool(Mob);
	return self->IsTargeted();
}

double Lua_Mob::GetX() {
	Lua_Safe_Call_Real(Mob);
	return self->GetX();
}

double Lua_Mob::GetY() {
	Lua_Safe_Call_Real(Mob);
	return self->GetY();
}

double Lua_Mob::GetZ() {
	Lua_Safe_Call_Real(Mob);
	return self->GetZ();
}

double Lua_Mob::GetHeading() {
	Lua_Safe_Call_Real(Mob);
	return self->GetHeading();
}

double Lua_Mob::GetWaypointX() {
	Lua_Safe_Call_Real(Mob);
	return self->GetCWPX();
}

double Lua_Mob::GetWaypointY() {
	Lua_Safe_Call_Real(Mob);
	return self->GetCWPY();
}

double Lua_Mob::GetWaypointZ() {
	Lua_Safe_Call_Real(Mob);
	return self->GetCWPZ();
}

double Lua_Mob::GetWaypointH() {
	Lua_Safe_Call_Real(Mob);
	return self->GetCWPH();
}

double Lua_Mob::GetWaypointPause() {
	Lua_Safe_Call_Real(Mob);
	return self->GetCWPP();
}

int Lua_Mob::GetWaypointID() {
	Lua_Safe_Call_Int(Mob);
	return self->GetCWP();
}

void Lua_Mob::SetCurrentWP(int wp) {
	Lua_Safe_Call_Void(Mob);
	self->SetCurrentWP(wp);
}

double Lua_Mob::GetSize() {
	Lua_Safe_Call_Real(Mob);
	return self->GetSize();
}

void Lua_Mob::SetFollowID(int id) {
	Lua_Safe_Call_Void(Mob);
	self->SetFollowID(id);
}

int Lua_Mob::GetFollowID() {
	Lua_Safe_Call_Int(Mob);
	return self->GetFollowID();
}

void Lua_Mob::Message(int type, const char *message) {
	Lua_Safe_Call_Void(Mob);
	self->Message(type, message);
}

void Lua_Mob::Message_StringID(int type, int string_id, uint32 distance) {
	Lua_Safe_Call_Void(Mob);
	self->Message_StringID(type, string_id, distance);
}

void Lua_Mob::Say(const char *message) {
	Lua_Safe_Call_Void(Mob);
	self->Say(message);
}

void Lua_Mob::Shout(const char *message) {
	Lua_Safe_Call_Void(Mob);
	self->Shout(message);
}

void Lua_Mob::Emote(const char *message) {
	Lua_Safe_Call_Void(Mob);
	self->Emote(message);
}

void Lua_Mob::InterruptSpell() {
	Lua_Safe_Call_Void(Mob);
	self->InterruptSpell();
}

void Lua_Mob::InterruptSpell(int spell_id) {
	Lua_Safe_Call_Void(Mob);
	self->InterruptSpell(spell_id);
}

//bool Lua_Mob::CastSpell(int spell_id, int target_id) {
//	Lua_Safe_Call_Void(Mob)
//	return self->CastSpell(spell_id, target_id);
//}
//
//bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot) {
//	Lua_Safe_Call_Void(Mob)
//	return self->CastSpell(spell_id, target_id, slot);
//}
//
//bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time) {
//	Lua_Safe_Call_Void(Mob)
//	return self->CastSpell(spell_id, target_id, slot, cast_time);
//}
//
//bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost) {
//	Lua_Safe_Call_Void(Mob)
//	return self->CastSpell(spell_id, target_id, slot, cast_time, mana_cost);
//}
//
//bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot) {
//	Lua_Safe_Call_Void(Mob)
//	return self->CastSpell(spell_id, target_id, slot, cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot));
//}
//
//bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot, int timer, 
//	int timer_duration) {
//	Lua_Safe_Call_Void(Mob)
//	return self->CastSpell(spell_id, target_id, slot, cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot), 
//		static_cast<uint32>(timer), static_cast<uint32>(timer_duration));
//}
//
//bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot, int timer, 
//	int timer_duration, int resist_adjust) {
//	Lua_Safe_Call_Void(Mob)
//	int16 res = resist_adjust;
//
//	return self->CastSpell(spell_id, target_id, slot, cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot), 
//		static_cast<uint32>(timer), static_cast<uint32>(timer_duration), 0, &res);
//}
//
//bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target) {
//	return SpellFinished(spell_id, target, 10, 0, 0xFFFFFFFF, 0, false); 
//}
//
//bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot) {
//	return SpellFinished(spell_id, target, slot, 0, 0xFFFFFFFF, 0, false); 
//}
//
//bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used) {
//	return SpellFinished(spell_id, target, slot, mana_used, 0xFFFFFFFF, 0, false); 
//}
//
//bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot) {
//	return SpellFinished(spell_id, target, slot, mana_used, inventory_slot, 0, false); 
//}
//
//bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot, int resist_adjust) {
//	return SpellFinished(spell_id, target, slot, mana_used, inventory_slot, resist_adjust, false); 
//}
//
//bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot, int resist_adjust, bool proc) {
//	Lua_Safe_Call_Void(Mob)
//	Lua_Safe_Cast(Mob, t, target);
//	return self->SpellFinished(spell_id, t, slot, mana_used, inventory_slot, resist_adjust, proc);
//}
//
//void Lua_Mob::SpellEffect(Lua_Mob caster, int spell_id, double partial) {
//	Lua_Safe_Call_Void(Mob)
//	Lua_Safe_Cast(Mob, c, caster);
//
//	self->SpellEffect(c, spell_id, partial);
//	Lua_Safe_Cast(Mob, c, caster);
//}
//
//bool Lua_Mob::IsImmuneToSpell(int spell_id, Lua_Mob caster) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::BuffFadeBySpellID(int spell_id) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::BuffFadeByEffect(int effect_id, int skip_slot) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::BuffFadeAll() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::BuffFadeDetrimental() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::BuffFadeBySlot(int slot, bool recalc_bonuses) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::BuffFadeDetrimentalByCaster(Lua_Mob caster) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::BuffFadeBySitModifier() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::BuffModifyDurationBySpellID(int spell_id, int new_duration) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::CanBuffStack(int spell_id, int caster_level, bool fail_if_overwrite) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsCasting() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::CastingSpellID() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetAppearance(int app) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetAppearance(int app, bool ignore_self) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetAppearance() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetPetID() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetOwnerID() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetPetType(int type) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetPetType() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetBodyType() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetBodyType(int type) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::Stun(int duration) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::UnStun() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::Spin() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::Kill() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetInvul(bool invul) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::GetInvul() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetExtraHaste(int haste) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetHaste() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetMonkHandToHandDamage() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetMonkHandToHandDelay() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CanThisClassDoubleAttack() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CanThisClassDualWield() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CanThisClassRiposte() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CanThisClassDodge() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CanThisClassParry() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CanThisClassBlock() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetClassLevelFactor() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::Mesmerize() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsMezzed() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsStunned() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::StartEnrage() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsEnraged() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetReverseFactionCon(Lua_Mob other) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsAIControlled() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//double Lua_Mob::GetAggroRange() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//double Lua_Mob::GetAssistRange() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetPetOrder(int order) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetPetOrder() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsRoamer() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsRooted() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::AddToHateList(Lua_Mob other) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::AddToHateList(Lua_Mob other, int hate) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::AddToHateList(Lua_Mob other, int hate, int damage) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help, bool frenzy) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help, bool frenzy, bool buff_tic) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::RemoveFromHateList(Lua_Mob mob) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetHate(Lua_Mob other) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetHate(Lua_Mob other, int hate) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetHate(Lua_Mob other, int hate, int damage) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//uint32 Lua_Mob::GetHateAmount(Lua_Mob tmob) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//uint32 Lua_Mob::GetHateAmount(Lua_Mob tmob, bool is_damage) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//uint32 Lua_Mob::GetDamageAmount(Lua_Mob tmob) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//Lua_Mob Lua_Mob::GetHateTop() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//Lua_Mob Lua_Mob::GetHateDamageTop(Lua_Mob other) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//Lua_Mob Lua_Mob::GetHateRandom() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//Lua_Mob Lua_Mob::GetHateMost() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsEngaged() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::HateSummon() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::FaceTarget() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::FaceTarget(Lua_Mob mob_to_face) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetHeading(double new_h) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::WipeHateList() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//double Lua_Mob::CalculateHeadingToTarget(double x, double y) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//double Lua_Mob::CalculateDistance(double x, double y, double z) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SendTo(double new_x, double new_y, double new_z) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SendToFixZ(double new_x, double new_y, double new_z) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::NPCSpecialAttacks(const char* parse, int permtag) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::NPCSpecialAttacks(const char* parse, int permtag, bool reset) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::NPCSpecialAttacks(const char* parse, int permtag, bool reset, bool remove) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetResist(int type) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::Charmed() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//uint32 Lua_Mob::GetLevelHP(int level) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//uint32 Lua_Mob::GetAA(int level) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::CheckAggroAmount(int spell_id) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::CheckAggroAmount(int spell_id, bool is_proc) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::CheckHealAggroAmount(int spell_id) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::CheckHealAggroAmount(int spell_id, uint32 heal_possible) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::DivineAura() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetOOCRegen(int new_regen) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//const Lua_Mob::char* GetEntityVariable(const char *id) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetEntityVariable(const char *id, const char *m_var) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::EntityVariableExists(const char *id) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CombatRange(Lua_Mob other) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoSpecialAttackDamage(Lua_Mob who, int skill, int max_damage, int min_damage) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoSpecialAttackDamage(Lua_Mob who, int skill, int max_damage, int min_damage, int hate_override) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoSpecialAttackDamage(Lua_Mob who, int skill, int max_damage, int min_damage, int hate_override, int reuse_time) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoSpecialAttackDamage(Lua_Mob who, int skill, int max_damage, int min_damage, int hate_override, int reuse_time, bool hit_chance) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skillinuse) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skillinuse, int chance_mod) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skillinuse, int chance_mod, int focus) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skillinuse, int chance_mod, int focus, bool can_riposte) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CheckLoS(Lua_Mob other) {
//	Lua_Safe_Call_Void(Mob)
//}
// // make sure this is CheckLoSFN
//bool Lua_Mob::CheckLoSToLoc(double x, double y, double z) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::CheckLoSToLoc(double x, double y, double z, double mob_size) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//double Lua_Mob::FindGroundZ(double new_x, double new_y) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//double Lua_Mob::FindGroundZ(double new_x, double new_y, double z_offset) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle, double tilt) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle, double tilt, double arc) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::HasNPCSpecialAtk(const char* parse) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5, Lua_Client client) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetFlyMode(int value) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetTexture(int value) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetRace(int value) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetGender(int value) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SendIllusionPacket(int in_race, int in_gender, int in_texture, int in_helmtexture, int in_haircolor, int in_beardcolor, int in_eyecolor1, int in_eyecolor2, int in_hairstyle, int in_luclinface, int in_beard, int in_aa_title, uint32 in_drakkin_heritage, uint32 in_drakkin_tattoo, uint32 in_drakkin_details, double in_size) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::QuestReward(Lua_Client c) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::QuestReward(Lua_Client c, uint32 silver) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::QuestReward(Lua_Client c, uint32 silver, uint32 gold) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::QuestReward(Lua_Client c, uint32 silver, uint32 gold, uint32 platinum) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::CameraEffect(uint32 duration, uint32 intensity) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SendSpellEffect(uint32 effect, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk20, bool perm_effect, Lua_Client client) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::TempName(const char *new_name) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetGlobal(const char *var_name, const char *new_value, int options, const char *duration) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetGlobal(const char *var_name, const char *new_value, int options, const char *duration, Lua_Mob other) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::TarGlobal(const char *var_name, const char *value, const char *duration, int npc_id, int char_id, int zone_id) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DelGlobal(const char *var_name) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetSlotTint(int slot, int red, int green, int blue) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::WearChange(int material_slot, int texture, uint32 color) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::DoKnockback(Lua_Mob caster, uint32 pushback, uint32 pushup) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::RemoveNimbusEffect(int effect_id) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsRunning() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetRunning(bool v) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetTargetable(bool v) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::ModSkillDmgTaken(int skill_num, int value) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetModSkillDmgTaken(int skill_num) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetSkillDmgTaken(int skill_num) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetAllowBeneficial(bool v) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::GetAllowBeneficial() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsBeneficialAllowed(Lua_Mob target) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::ModVulnerability(int resist, int value) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetModVulnerability(int resist) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetDisableMelee(bool v) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//bool Lua_Mob::IsMeleeDisabled() {
//	Lua_Safe_Call_Void(Mob)
//}
//
//void Lua_Mob::SetFlurryChance(int v) {
//	Lua_Safe_Call_Void(Mob)
//}
//
//int Lua_Mob::GetFlurryChance() {
//	Lua_Safe_Call_Void(Mob)
//}



#endif
