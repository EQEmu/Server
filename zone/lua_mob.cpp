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
	Mob *o = reinterpret_cast<Mob*>(other.d_);
	m->RogueAssassinate(o);
}

bool Lua_Mob::BehindMob() {
	return BehindMob(Lua_Mob(nullptr), 0.0f, 0.0f);
}

bool Lua_Mob::BehindMob(Lua_Mob other) {
	return BehindMob(other, 0.0f, 0.0f);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x) {
	return BehindMob(other, x, 0.0f);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x, float y) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Mob *o = reinterpret_cast<Mob*>(other.d_);
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
	Mob *o = reinterpret_cast<Mob*>(other.d_);
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
	Mob *f = reinterpret_cast<Mob*>(from.d_);
	m->Damage(f, damage, spell_id, static_cast<SkillType>(attack_skill), avoidable, buffslot, buff_tic);
}

void Lua_Mob::RangedAttack(Lua_Mob other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Mob *o = reinterpret_cast<Mob*>(other.d_);
	m->RangedAttack(o);
}

void Lua_Mob::ThrowingAttack(Lua_Mob other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Mob *o = reinterpret_cast<Mob*>(other.d_);
	m->ThrowingAttack(o);
}

void Lua_Mob::Heal() {
	Mob *m = reinterpret_cast<Mob*>(d_);
	m->Heal();
}

void Lua_Mob::HealDamage(uint32 amount) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	HealDamage(amount, Lua_Mob(nullptr));
}

void Lua_Mob::HealDamage(uint32 amount, Lua_Mob other) {
	Mob *m = reinterpret_cast<Mob*>(d_);
	Mob *o = reinterpret_cast<Mob*>(other.d_);
	m->HealDamage(amount, o);
}


#endif
