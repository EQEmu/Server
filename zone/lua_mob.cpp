#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_mob.h"

const char *Lua_Mob::GetName() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetName();
}

void Lua_Mob::Depop() {
	Depop(true);
}

void Lua_Mob::Depop(bool start_spawn_timer) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->Depop(start_spawn_timer);
}

void Lua_Mob::RogueAssassinate(Lua_Mob other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, o, other);
	m->RogueAssassinate(o);
}

bool Lua_Mob::BehindMob() {
	return BehindMob(nullptr, 0.0f, 0.0f);
}

bool Lua_Mob::BehindMob(Lua_Mob other) {
	return BehindMob(other, 0.0f, 0.0f);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x) {
	return BehindMob(other, x, 0.0f);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x, float y) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, o, other);
	return m->BehindMob(o, x, y);
}

void Lua_Mob::SetLevel(int level) {
	SetLevel(level, false);
}

void Lua_Mob::SetLevel(int level, bool command) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->SetLevel(level, command);
}

void Lua_Mob::SendWearChange(int material_slot) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->SendWearChange(material_slot);
}

uint32 Lua_Mob::GetEquipment(int material_slot) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetEquipment(material_slot);
}

int32 Lua_Mob::GetEquipmentMaterial(int material_slot) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetEquipmentMaterial(material_slot);
}

uint32 Lua_Mob::GetEquipmentColor(int material_slot) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetEquipmentColor(material_slot);
}

uint32 Lua_Mob::GetArmorTint(int i) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetArmorTint(i);
}

bool Lua_Mob::IsMoving() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->IsMoving();
}

void Lua_Mob::GotoBind() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->GoToBind();
}

void Lua_Mob::Gate() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->Gate();
}

bool Lua_Mob::Attack(Lua_Mob other) {
	return Attack(other, 13, false, false, false);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand) {
	return Attack(other, hand, false, false, false);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte) {
	return Attack(other, hand, from_riposte, false, false);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough) {
	return Attack(other, hand, from_riposte, is_strikethrough, false);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, o, other);
	return m->Attack(o, hand, from_riposte, is_strikethrough, is_from_spell);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill) {
	Damage(from, damage, spell_id, attack_skill, true, -1, false);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable) {
	Damage(from, damage, spell_id, attack_skill, avoidable, -1, false);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot) {
	Damage(from, damage, spell_id, attack_skill, avoidable, buffslot, false);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot, bool buff_tic) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, f, from);
	m->Damage(f, damage, spell_id, static_cast<SkillType>(attack_skill), avoidable, buffslot, buff_tic);
}

void Lua_Mob::RangedAttack(Lua_Mob other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, o, other);
	m->RangedAttack(o);
}

void Lua_Mob::ThrowingAttack(Lua_Mob other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, o, other);
	m->ThrowingAttack(o);
}

void Lua_Mob::Heal() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->Heal();
}

void Lua_Mob::HealDamage(uint32 amount) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	HealDamage(amount, nullptr);
}

void Lua_Mob::HealDamage(uint32 amount, Lua_Mob other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, o, other);
	m->HealDamage(amount, o);
}

uint32 Lua_Mob::GetLevelCon(int other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetLevelCon(other);
}

uint32 Lua_Mob::GetLevelCon(int my, int other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetLevelCon(my, other);
}

void Lua_Mob::SetHP(int hp) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->SetHP(hp);
}

void Lua_Mob::DoAnim(int anim_num) {
	DoAnim(anim_num, 0, true, 0);
}

void Lua_Mob::DoAnim(int anim_num, int type) {
	DoAnim(anim_num, type, true, 0);
}

void Lua_Mob::DoAnim(int anim_num, int type, bool ackreq) {
	DoAnim(anim_num, type, ackreq, 0);
}

void Lua_Mob::DoAnim(int anim_num, int type, bool ackreq, int filter) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->DoAnim(anim_num, type, ackreq, static_cast<eqFilterType>(filter));
}

void Lua_Mob::ChangeSize(double in_size) {
	ChangeSize(in_size, false);
}

void Lua_Mob::ChangeSize(double in_size, bool no_restriction) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->ChangeSize(in_size, no_restriction);
}

void Lua_Mob::GMMove(double x, double y, double z) {
	GMMove(x, y, z, 0.01, true);
}

void Lua_Mob::GMMove(double x, double y, double z, double heading) {
	GMMove(x, y, z, heading, true);
}

void Lua_Mob::GMMove(double x, double y, double z, double heading, bool send_update) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->GMMove(x, y, z, heading, send_update);
}

void Lua_Mob::SendPosUpdate() {
	SendPosUpdate(false);
}

void Lua_Mob::SendPosUpdate(bool send_to_self) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->SendPosUpdate(send_to_self ? 1 : 0);
}

void Lua_Mob::SendPosition() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->SendPosition();
}

bool Lua_Mob::HasProcs() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->HasProcs();
}

bool Lua_Mob::IsInvisible() {
	return IsInvisible(Lua_Mob(nullptr));
}

bool Lua_Mob::IsInvisible(Lua_Mob other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, o, other);

	return m->IsInvisible(o);
}

void Lua_Mob::SetInvisible(int state) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->SetInvisible(state);
}

bool Lua_Mob::FindBuff(int spell_id) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->FindBuff(spell_id);
}

bool Lua_Mob::FindType(int type) {
	return FindType(type, false, 100);
}

bool Lua_Mob::FindType(int type, bool offensive) {
	return FindType(type, offensive, 100);
}

bool Lua_Mob::FindType(int type, bool offensive, int threshold) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->FindType(type, offensive, threshold);
}

int Lua_Mob::GetBuffSlotFromType(int slot) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetBuffSlotFromType(slot);
}

void Lua_Mob::MakePet(int spell_id, const char* pet_type) {
	MakePet(spell_id, pet_type, nullptr);
}

void Lua_Mob::MakePet(int spell_id, const char* pet_type, const char *pet_name) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->MakePet(spell_id, pet_type, pet_name);
}

void Lua_Mob::MakePoweredPet(int spell_id, const char* pet_type, int pet_power) {
	MakePoweredPet(spell_id, pet_type, pet_power, nullptr);
}

void Lua_Mob::MakePoweredPet(int spell_id, const char* pet_type, int pet_power, const char *pet_name) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->MakePoweredPet(spell_id, pet_type, pet_power, pet_name);
}

int Lua_Mob::GetBaseRace() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetBaseRace();
}

int Lua_Mob::GetBaseGender() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetBaseGender();
}

int Lua_Mob::GetDeity() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetDeity();
}

int Lua_Mob::GetRace() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetRace();
}

int Lua_Mob::GetGender() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetGender();
}

int Lua_Mob::GetTexture() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetTexture();
}

int Lua_Mob::GetHelmTexture() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetHelmTexture();
}

int Lua_Mob::GetHairColor() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetHairColor();
}

int Lua_Mob::GetBeardColor() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetBeardColor();
}

int Lua_Mob::GetEyeColor1() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetEyeColor1();
}

int Lua_Mob::GetEyeColor2() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetEyeColor2();
}

int Lua_Mob::GetHairStyle() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetHairStyle();
}

int Lua_Mob::GetLuclinFace() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetLuclinFace();
}

int Lua_Mob::GetBeard() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetBeard();
}

int Lua_Mob::GetDrakkinHeritage() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetDrakkinHeritage();
}

int Lua_Mob::GetDrakkinTattoo() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetDrakkinTattoo();
}

int Lua_Mob::GetDrakkinDetails() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetDrakkinDetails();
}

int Lua_Mob::GetClass() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetClass();
}

int Lua_Mob::GetLevel() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetLevel();
}

const char *Lua_Mob::GetCleanName() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCleanName();
}

Lua_Mob Lua_Mob::GetTarget() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return Lua_Mob(m->GetTarget());
}

void Lua_Mob::SetTarget(Lua_Mob t) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, tar, t);
	m->SetTarget(tar);
}

double Lua_Mob::GetHPRatio() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetHPRatio();
}

bool Lua_Mob::IsWarriorClass() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->IsWarriorClass();
}

int Lua_Mob::GetHP() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetHP();
}

int Lua_Mob::GetMaxHP() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxHP();
}

int Lua_Mob::GetItemHPBonuses() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetItemHPBonuses();
}

int Lua_Mob::GetSpellHPBonuses() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetSpellHPBonuses();
}

double Lua_Mob::GetWalkspeed() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetWalkspeed();
}

double Lua_Mob::GetRunspeed() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetRunspeed();
}

int Lua_Mob::GetCasterLevel(int spell_id) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCasterLevel(spell_id);
}

int Lua_Mob::GetMaxMana() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxMana();
}

int Lua_Mob::GetMana() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMana();
}

int Lua_Mob::SetMana(int mana) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->SetMana(mana);
}

double Lua_Mob::GetManaRatio() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetManaRatio();
}

int Lua_Mob::GetAC() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetAC();
}

int Lua_Mob::GetATK() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetATK();
}

int Lua_Mob::GetSTR() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetSTR();
}

int Lua_Mob::GetSTA() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetSTA();
}

int Lua_Mob::GetDEX() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetDEX();
}

int Lua_Mob::GetAGI() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetAGI();
}

int Lua_Mob::GetINT() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetINT();
}

int Lua_Mob::GetWIS() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetWIS();
}

int Lua_Mob::GetCHA() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCHA();
}

int Lua_Mob::GetMR() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMR();
}

int Lua_Mob::GetFR() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetFR();
}

int Lua_Mob::GetDR() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetDR();
}

int Lua_Mob::GetPR() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetPR();
}

int Lua_Mob::GetCR() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCR();
}

int Lua_Mob::GetCorruption() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCorrup();
}

int Lua_Mob::GetMaxSTR() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxSTR();
}

int Lua_Mob::GetMaxSTA() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxSTA();
}

int Lua_Mob::GetMaxDEX() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxDEX();
}

int Lua_Mob::GetMaxAGI() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxAGI();
}

int Lua_Mob::GetMaxINT() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxINT();
}

int Lua_Mob::GetMaxWIS() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxWIS();
}

int Lua_Mob::GetMaxCHA() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetMaxCHA();
}

double Lua_Mob::GetActSpellRange(int spell_id, double range) {
	return GetActSpellRange(spell_id, range, false);
}

double Lua_Mob::GetActSpellRange(int spell_id, double range, bool is_bard) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetActSpellRange(spell_id, range, is_bard);
}

int Lua_Mob::GetActSpellDamage(int spell_id, int value) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetActSpellDamage(spell_id, value);
}

int Lua_Mob::GetActSpellHealing(int spell_id, int value) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetActSpellHealing(spell_id, value);
}

int Lua_Mob::GetActSpellCost(int spell_id, int cost) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetActSpellCost(spell_id, cost);
}

int Lua_Mob::GetActSpellDuration(int spell_id, int duration) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetActSpellDuration(spell_id, duration);
}

int Lua_Mob::GetActSpellCasttime(int spell_id, int cast_time) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetActSpellCasttime(spell_id, cast_time);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster) {
	return ResistSpell(resist_type, spell_id, caster, false, 0, false);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override) {
	return ResistSpell(resist_type, spell_id, caster, use_resist_override, 0, false);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override) {
	return ResistSpell(resist_type, spell_id, caster, use_resist_override, resist_override, false);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override, 
	bool charisma_check) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Lua_Safe_Cast(Mob, c, caster);

	return m->ResistSpell(resist_type, spell_id, c, use_resist_override, resist_override, charisma_check);
}

int Lua_Mob::GetSpecializeSkillValue(int spell_id) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetSpecializeSkillValue(spell_id);
}

int Lua_Mob::GetNPCTypeID() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetNPCTypeID();
}

bool Lua_Mob::IsTargeted() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->IsTargeted();
}

double Lua_Mob::GetX() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetX();
}

double Lua_Mob::GetY() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetY();
}

double Lua_Mob::GetZ() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetZ();
}

double Lua_Mob::GetHeading() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetHeading();
}

double Lua_Mob::GetWaypointX() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCWPX();
}

double Lua_Mob::GetWaypointY() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCWPY();
}

double Lua_Mob::GetWaypointZ() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCWPZ();
}

double Lua_Mob::GetWaypointH() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCWPH();
}

double Lua_Mob::GetWaypointPause() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCWPP();
}

int Lua_Mob::GetWaypointID() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetCWP();
}

void Lua_Mob::SetCurrentWP(int wp) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->SetCurrentWP(wp);
}

double Lua_Mob::GetSize() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetSize();
}

void Lua_Mob::SetFollowID(int id) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->SetFollowID(id);
}

int Lua_Mob::GetFollowID() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	return m->GetFollowID();
}

void Lua_Mob::Message(int type, const char *message) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->Message(type, message);
}

void Lua_Mob::Message_StringID(int type, int string_id, uint32 distance) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->Message_StringID(type, string_id, distance);
}

void Lua_Mob::Say(const char *message) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->Say(message);
}

void Lua_Mob::Shout(const char *message) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->Shout(message);
}

void Lua_Mob::Emote(const char *message) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->Emote(message);
}

void Lua_Mob::InterruptSpell() {
	InterruptSpell(65535U);
}

void Lua_Mob::InterruptSpell(int spell_id) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->InterruptSpell(spell_id);
}


#endif
