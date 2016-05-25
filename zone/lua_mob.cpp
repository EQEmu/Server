#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "client.h"
#include "npc.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_hate_list.h"
#include "lua_client.h"

struct SpecialAbilities { };

const char *Lua_Mob::GetName() {
	Lua_Safe_Call_String();
	return self->GetName();
}

void Lua_Mob::Depop() {
	Lua_Safe_Call_Void();
	return self->Depop();
}

void Lua_Mob::Depop(bool start_spawn_timer) {
	Lua_Safe_Call_Void();
	return self->Depop(start_spawn_timer);
}

bool Lua_Mob::BehindMob() {
	Lua_Safe_Call_Bool();
	return self->BehindMob();
}

bool Lua_Mob::BehindMob(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->BehindMob(other);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x) {
	Lua_Safe_Call_Bool();
	return self->BehindMob(other, x);
}

bool Lua_Mob::BehindMob(Lua_Mob other, float x, float y) {
	Lua_Safe_Call_Bool();
	return self->BehindMob(other, x, y);
}

void Lua_Mob::SetLevel(int level) {
	Lua_Safe_Call_Void();
	self->SetLevel(level);
}

void Lua_Mob::SetLevel(int level, bool command) {
	Lua_Safe_Call_Void();
	self->SetLevel(level, command);
}

void Lua_Mob::SendWearChange(int material_slot) {
	Lua_Safe_Call_Void();
	self->SendWearChange(material_slot);
}

bool Lua_Mob::IsMoving() {
	Lua_Safe_Call_Bool();
	return self->IsMoving();
}

void Lua_Mob::GotoBind() {
	Lua_Safe_Call_Void();
	self->GoToBind();
}

void Lua_Mob::Gate() {
	Lua_Safe_Call_Void();
	self->Gate();
}

bool Lua_Mob::Attack(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->Attack(other);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand) {
	Lua_Safe_Call_Bool();
	return self->Attack(other, hand);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte) {
	Lua_Safe_Call_Bool();
	return self->Attack(other, hand, from_riposte);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough) {
	Lua_Safe_Call_Bool();
	return self->Attack(other, hand, from_riposte, is_strikethrough);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell) {
	Lua_Safe_Call_Bool();
	return self->Attack(other, hand, from_riposte, is_strikethrough, is_from_spell);
}

bool Lua_Mob::Attack(Lua_Mob other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell, luabind::adl::object opts) {
	Lua_Safe_Call_Bool();

	ExtraAttackOptions options;
	if(luabind::type(opts) == LUA_TTABLE) {
		auto cur = opts["armor_pen_flat"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.armor_pen_flat = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = opts["crit_flat"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.crit_flat = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = opts["damage_flat"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.damage_flat = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = opts["hate_flat"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.hate_flat = luabind::object_cast<int>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = opts["armor_pen_percent"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.armor_pen_percent = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = opts["crit_percent"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.crit_percent = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = opts["damage_percent"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.damage_percent = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
			}
		}

		cur = opts["hate_percent"];
		if(luabind::type(cur) != LUA_TNIL) {
			try {
				options.hate_percent = luabind::object_cast<float>(cur);
			} catch(luabind::cast_failed) {
			}
		}
	}

	return self->Attack(other, hand, from_riposte, is_strikethrough, is_from_spell, &options);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill));
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable, buffslot);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot, bool buff_tic) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable, buffslot, buff_tic);
}

void Lua_Mob::RangedAttack(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->RangedAttack(other);
}

void Lua_Mob::ThrowingAttack(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->ThrowingAttack(other);
}

void Lua_Mob::Heal() {
	Lua_Safe_Call_Void();
	self->Heal();
}

void Lua_Mob::HealDamage(uint32 amount) {
	Lua_Safe_Call_Void();
	self->HealDamage(amount);
}

void Lua_Mob::HealDamage(uint32 amount, Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->HealDamage(amount, other);
}

uint32 Lua_Mob::GetLevelCon(int other) {
	Lua_Safe_Call_Int();
	return self->GetLevelCon(other);
}

uint32 Lua_Mob::GetLevelCon(int my, int other) {
	Lua_Safe_Call_Int();
	return self->GetLevelCon(my, other);
}

void Lua_Mob::SetHP(int hp) {
	Lua_Safe_Call_Void();
	self->SetHP(hp);
}

void Lua_Mob::DoAnim(int anim_num) {
	Lua_Safe_Call_Void();
	self->DoAnim(anim_num);
}

void Lua_Mob::DoAnim(int anim_num, int type) {
	Lua_Safe_Call_Void();
	self->DoAnim(anim_num, type);
}

void Lua_Mob::DoAnim(int anim_num, int type, bool ackreq) {
	Lua_Safe_Call_Void();
	self->DoAnim(anim_num, type, ackreq);
}

void Lua_Mob::DoAnim(int anim_num, int type, bool ackreq, int filter) {
	Lua_Safe_Call_Void();
	self->DoAnim(anim_num, type, ackreq, static_cast<eqFilterType>(filter));
}

void Lua_Mob::ChangeSize(double in_size) {
	Lua_Safe_Call_Void();
	self->ChangeSize(static_cast<float>(in_size));
}

void Lua_Mob::ChangeSize(double in_size, bool no_restriction) {
	Lua_Safe_Call_Void();
	self->ChangeSize(static_cast<float>(in_size), no_restriction);
}

void Lua_Mob::GMMove(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->GMMove(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::GMMove(double x, double y, double z, double heading) {
	Lua_Safe_Call_Void();
	self->GMMove(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(heading));
}

void Lua_Mob::GMMove(double x, double y, double z, double heading, bool send_update) {
	Lua_Safe_Call_Void();
	self->GMMove(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(heading), send_update);
}

bool Lua_Mob::HasProcs() {
	Lua_Safe_Call_Bool();
	return self->HasProcs();
}

bool Lua_Mob::IsInvisible() {
	Lua_Safe_Call_Bool();
	return self->IsInvisible();
}

bool Lua_Mob::IsInvisible(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->IsInvisible(other);
}

void Lua_Mob::SetInvisible(int state) {
	Lua_Safe_Call_Void();
	self->SetInvisible(state);
}

bool Lua_Mob::FindBuff(int spell_id) {
	Lua_Safe_Call_Bool();
	return self->FindBuff(spell_id);
}

bool Lua_Mob::FindType(int type) {
	Lua_Safe_Call_Bool();
	return self->FindType(type);
}

bool Lua_Mob::FindType(int type, bool offensive) {
	Lua_Safe_Call_Bool();
	return self->FindType(type, offensive);
}

bool Lua_Mob::FindType(int type, bool offensive, int threshold) {
	Lua_Safe_Call_Bool();
	return self->FindType(type, offensive, threshold);
}

int Lua_Mob::GetBuffSlotFromType(int slot) {
	Lua_Safe_Call_Bool();
	return self->GetBuffSlotFromType(slot);
}

int Lua_Mob::GetBaseRace() {
	Lua_Safe_Call_Int();
	return self->GetBaseRace();
}

int Lua_Mob::GetBaseGender() {
	Lua_Safe_Call_Int();
	return self->GetBaseGender();
}

int Lua_Mob::GetDeity() {
	Lua_Safe_Call_Int();
	return self->GetDeity();
}

int Lua_Mob::GetRace() {
	Lua_Safe_Call_Int();
	return self->GetRace();
}

int Lua_Mob::GetGender() {
	Lua_Safe_Call_Int();
	return self->GetGender();
}

int Lua_Mob::GetTexture() {
	Lua_Safe_Call_Int();
	return self->GetTexture();
}

int Lua_Mob::GetHelmTexture() {
	Lua_Safe_Call_Int();
	return self->GetHelmTexture();
}

int Lua_Mob::GetHairColor() {
	Lua_Safe_Call_Int();
	return self->GetHairColor();
}

int Lua_Mob::GetBeardColor() {
	Lua_Safe_Call_Int();
	return self->GetBeardColor();
}

int Lua_Mob::GetEyeColor1() {
	Lua_Safe_Call_Int();
	return self->GetEyeColor1();
}

int Lua_Mob::GetEyeColor2() {
	Lua_Safe_Call_Int();
	return self->GetEyeColor2();
}

int Lua_Mob::GetHairStyle() {
	Lua_Safe_Call_Int();
	return self->GetHairStyle();
}

int Lua_Mob::GetLuclinFace() {
	Lua_Safe_Call_Int();
	return self->GetLuclinFace();
}

int Lua_Mob::GetBeard() {
	Lua_Safe_Call_Int();
	return self->GetBeard();
}

int Lua_Mob::GetDrakkinHeritage() {
	Lua_Safe_Call_Int();
	return self->GetDrakkinHeritage();
}

int Lua_Mob::GetDrakkinTattoo() {
	Lua_Safe_Call_Int();
	return self->GetDrakkinTattoo();
}

int Lua_Mob::GetDrakkinDetails() {
	Lua_Safe_Call_Int();
	return self->GetDrakkinDetails();
}

int Lua_Mob::GetClass() {
	Lua_Safe_Call_Int();
	return self->GetClass();
}

int Lua_Mob::GetLevel() {
	Lua_Safe_Call_Int();
	return self->GetLevel();
}

const char *Lua_Mob::GetCleanName() {
	Lua_Safe_Call_String();
	return self->GetCleanName();
}

Lua_Mob Lua_Mob::GetTarget() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetTarget());
}

void Lua_Mob::SetTarget(Lua_Mob t) {
	Lua_Safe_Call_Void();
	self->SetTarget(t);
}

double Lua_Mob::GetHPRatio() {
	Lua_Safe_Call_Real();
	return self->GetHPRatio();
}

bool Lua_Mob::IsWarriorClass() {
	Lua_Safe_Call_Bool();
	return self->IsWarriorClass();
}

int Lua_Mob::GetHP() {
	Lua_Safe_Call_Int();
	return self->GetHP();
}

int Lua_Mob::GetMaxHP() {
	Lua_Safe_Call_Int();
	return self->GetMaxHP();
}

int Lua_Mob::GetItemHPBonuses() {
	Lua_Safe_Call_Int();
	return self->GetItemHPBonuses();
}

int Lua_Mob::GetSpellHPBonuses() {
	Lua_Safe_Call_Int();
	return self->GetSpellHPBonuses();
}

double Lua_Mob::GetWalkspeed() {
	Lua_Safe_Call_Real();
	return self->GetWalkspeed();
}

double Lua_Mob::GetRunspeed() {
	Lua_Safe_Call_Real();
	return self->GetRunspeed();
}

int Lua_Mob::GetCasterLevel(int spell_id) {
	Lua_Safe_Call_Int();
	return self->GetCasterLevel(spell_id);
}

int Lua_Mob::GetMaxMana() {
	Lua_Safe_Call_Int();
	return self->GetMaxMana();
}

int Lua_Mob::GetMana() {
	Lua_Safe_Call_Int();
	return self->GetMana();
}

int Lua_Mob::SetMana(int mana) {
	Lua_Safe_Call_Int();
	return self->SetMana(mana);
}

double Lua_Mob::GetManaRatio() {
	Lua_Safe_Call_Real();
	return self->GetManaRatio();
}

int Lua_Mob::GetAC() {
	Lua_Safe_Call_Int();
	return self->GetAC();
}

int Lua_Mob::GetATK() {
	Lua_Safe_Call_Int();
	return self->GetATK();
}

int Lua_Mob::GetSTR() {
	Lua_Safe_Call_Int();
	return self->GetSTR();
}

int Lua_Mob::GetSTA() {
	Lua_Safe_Call_Int();
	return self->GetSTA();
}

int Lua_Mob::GetDEX() {
	Lua_Safe_Call_Int();
	return self->GetDEX();
}

int Lua_Mob::GetAGI() {
	Lua_Safe_Call_Int();
	return self->GetAGI();
}

int Lua_Mob::GetINT() {
	Lua_Safe_Call_Int();
	return self->GetINT();
}

int Lua_Mob::GetWIS() {
	Lua_Safe_Call_Int();
	return self->GetWIS();
}

int Lua_Mob::GetCHA() {
	Lua_Safe_Call_Int();
	return self->GetCHA();
}

int Lua_Mob::GetMR() {
	Lua_Safe_Call_Int();
	return self->GetMR();
}

int Lua_Mob::GetFR() {
	Lua_Safe_Call_Int();
	return self->GetFR();
}

int Lua_Mob::GetDR() {
	Lua_Safe_Call_Int();
	return self->GetDR();
}

int Lua_Mob::GetPR() {
	Lua_Safe_Call_Int();
	return self->GetPR();
}

int Lua_Mob::GetCR() {
	Lua_Safe_Call_Int();
	return self->GetCR();
}

int Lua_Mob::GetCorruption() {
	Lua_Safe_Call_Int();
	return self->GetCorrup();
}

int Lua_Mob::GetPhR() {
	Lua_Safe_Call_Int();
	return self->GetPhR();
}

int Lua_Mob::GetMaxSTR() {
	Lua_Safe_Call_Int();
	return self->GetMaxSTR();
}

int Lua_Mob::GetMaxSTA() {
	Lua_Safe_Call_Int();
	return self->GetMaxSTA();
}

int Lua_Mob::GetMaxDEX() {
	Lua_Safe_Call_Int();
	return self->GetMaxDEX();
}

int Lua_Mob::GetMaxAGI() {
	Lua_Safe_Call_Int();
	return self->GetMaxAGI();
}

int Lua_Mob::GetMaxINT() {
	Lua_Safe_Call_Int();
	return self->GetMaxINT();
}

int Lua_Mob::GetMaxWIS() {
	Lua_Safe_Call_Int();
	return self->GetMaxWIS();
}

int Lua_Mob::GetMaxCHA() {
	Lua_Safe_Call_Int();
	return self->GetMaxCHA();
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster) {
	Lua_Safe_Call_Real();
	return self->ResistSpell(resist_type, spell_id, caster);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override) {
	Lua_Safe_Call_Real();
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override) {
	Lua_Safe_Call_Real();
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override, resist_override);
}

double Lua_Mob::ResistSpell(int resist_type, int spell_id, Lua_Mob caster, bool use_resist_override, int resist_override,
	bool charisma_check) {
	Lua_Safe_Call_Real();
	return self->ResistSpell(resist_type, spell_id, caster, use_resist_override, resist_override, charisma_check);
}

int Lua_Mob::GetSpecializeSkillValue(int spell_id) {
	Lua_Safe_Call_Int();
	return self->GetSpecializeSkillValue(spell_id);
}

int Lua_Mob::GetNPCTypeID() {
	Lua_Safe_Call_Int();
	return self->GetNPCTypeID();
}

bool Lua_Mob::IsTargeted() {
	Lua_Safe_Call_Bool();
	return self->IsTargeted();
}

double Lua_Mob::GetX() {
	Lua_Safe_Call_Real();
	return self->GetX();
}

double Lua_Mob::GetY() {
	Lua_Safe_Call_Real();
	return self->GetY();
}

double Lua_Mob::GetZ() {
	Lua_Safe_Call_Real();
	return self->GetZ();
}

double Lua_Mob::GetHeading() {
	Lua_Safe_Call_Real();
	return self->GetHeading();
}

double Lua_Mob::GetWaypointX() {
	Lua_Safe_Call_Real();
	return self->GetCurrentWayPoint().x;
}

double Lua_Mob::GetWaypointY() {
	Lua_Safe_Call_Real();
	return self->GetCurrentWayPoint().y;
}

double Lua_Mob::GetWaypointZ() {
	Lua_Safe_Call_Real();
	return self->GetCurrentWayPoint().z;
}

double Lua_Mob::GetWaypointH() {
	Lua_Safe_Call_Real();
	return self->GetCurrentWayPoint().w;
}

double Lua_Mob::GetWaypointPause() {
	Lua_Safe_Call_Real();
	return self->GetCWPP();
}

int Lua_Mob::GetWaypointID() {
	Lua_Safe_Call_Int();
	return self->GetCWP();
}

void Lua_Mob::SetCurrentWP(int wp) {
	Lua_Safe_Call_Void();
	self->SetCurrentWP(wp);
}

double Lua_Mob::GetSize() {
	Lua_Safe_Call_Real();
	return self->GetSize();
}

void Lua_Mob::Message(int type, const char *message) {
	Lua_Safe_Call_Void();
	self->Message(type, message);
}

void Lua_Mob::Message_StringID(int type, int string_id, uint32 distance) {
	Lua_Safe_Call_Void();
	self->Message_StringID(type, string_id, distance);
}

void Lua_Mob::Say(const char *message) {
	Lua_Safe_Call_Void();
	self->Say(message);
}

void Lua_Mob::QuestSay(Lua_Client client, const char *message) {
	Lua_Safe_Call_Void();
	self->QuestJournalledSay(client, message);
}

void Lua_Mob::Shout(const char *message) {
	Lua_Safe_Call_Void();
	self->Shout(message);
}

void Lua_Mob::Emote(const char *message) {
	Lua_Safe_Call_Void();
	self->Emote(message);
}

void Lua_Mob::InterruptSpell() {
	Lua_Safe_Call_Void();
	self->InterruptSpell();
}

void Lua_Mob::InterruptSpell(int spell_id) {
	Lua_Safe_Call_Void();
	self->InterruptSpell(spell_id);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, slot);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, slot, cast_time);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, slot, cast_time, mana_cost);
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, slot, cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot));
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot, int timer,
	int timer_duration) {
	Lua_Safe_Call_Bool();
	return self->CastSpell(spell_id, target_id, slot, cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot),
		static_cast<uint32>(timer), static_cast<uint32>(timer_duration));
}

bool Lua_Mob::CastSpell(int spell_id, int target_id, int slot, int cast_time, int mana_cost, int item_slot, int timer,
	int timer_duration, int resist_adjust) {
	Lua_Safe_Call_Bool();
	int16 res = resist_adjust;

	return self->CastSpell(spell_id, target_id, slot, cast_time, mana_cost, nullptr, static_cast<uint32>(item_slot),
		static_cast<uint32>(timer), static_cast<uint32>(timer_duration), &res);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, slot);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, slot, mana_used);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, slot, mana_used, inventory_slot);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot, int resist_adjust) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, slot, mana_used, inventory_slot, resist_adjust);
}

bool Lua_Mob::SpellFinished(int spell_id, Lua_Mob target, int slot, int mana_used, uint32 inventory_slot, int resist_adjust, bool proc) {
	Lua_Safe_Call_Bool();
	return self->SpellFinished(spell_id, target, slot, mana_used, inventory_slot, resist_adjust, proc);
}

void Lua_Mob::SpellEffect(Lua_Mob caster, int spell_id, double partial) {
	Lua_Safe_Call_Void();
	self->SpellEffect(caster, spell_id, static_cast<float>(partial));
}

Lua_Mob Lua_Mob::GetPet() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetPet());
}

Lua_Mob Lua_Mob::GetOwner() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetOwner());
}

Lua_HateList Lua_Mob::GetHateList() {
	Lua_Safe_Call_Class(Lua_HateList);
	Lua_HateList ret;

	auto h_list = self->GetHateList();
	auto iter = h_list.begin();
	while(iter != h_list.end()) {
		Lua_HateEntry e(*iter);
		ret.entries.push_back(e);
		++iter;
	}

	return ret;
}

Lua_Mob Lua_Mob::GetHateTop() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetHateTop());
}

Lua_Mob Lua_Mob::GetHateDamageTop(Lua_Mob other) {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetHateDamageTop(other));
}

Lua_Mob Lua_Mob::GetHateRandom() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetHateRandom());
}

void Lua_Mob::AddToHateList(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int hate) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int hate, int damage) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate, damage);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate, damage, yell_for_help);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help, bool frenzy) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate, damage, yell_for_help, frenzy);
}

void Lua_Mob::AddToHateList(Lua_Mob other, int hate, int damage, bool yell_for_help, bool frenzy, bool buff_tic) {
	Lua_Safe_Call_Void();
	self->AddToHateList(other, hate, damage, yell_for_help, frenzy, buff_tic);
}

void Lua_Mob::SetHate(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->SetHateAmountOnEnt(other);
}

void Lua_Mob::SetHate(Lua_Mob other, int hate) {
	Lua_Safe_Call_Void();
	self->SetHateAmountOnEnt(other, hate);
}

void Lua_Mob::SetHate(Lua_Mob other, int hate, int damage) {
	Lua_Safe_Call_Void();
	self->SetHateAmountOnEnt(other, hate, damage);
}

void Lua_Mob::HalveAggro(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->HalveAggro(other);
}

void Lua_Mob::DoubleAggro(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->DoubleAggro(other);
}

uint32 Lua_Mob::GetHateAmount(Lua_Mob target) {
	Lua_Safe_Call_Int();
	return self->GetHateAmount(target);
}

uint32 Lua_Mob::GetHateAmount(Lua_Mob target, bool is_damage) {
	Lua_Safe_Call_Int();
	return self->GetHateAmount(target, is_damage);
}

uint32 Lua_Mob::GetDamageAmount(Lua_Mob target) {
	Lua_Safe_Call_Int();
	return self->GetDamageAmount(target);
}

void Lua_Mob::WipeHateList() {
	Lua_Safe_Call_Void();
	self->WipeHateList();
}

bool Lua_Mob::CheckAggro(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->CheckAggro(other);
}

void Lua_Mob::Stun(int duration) {
	Lua_Safe_Call_Void();
	self->Stun(duration);
}

void Lua_Mob::UnStun() {
	Lua_Safe_Call_Void();
	self->UnStun();
}

bool Lua_Mob::IsStunned() {
	Lua_Safe_Call_Bool();
	return self->IsStunned();
}

void Lua_Mob::Spin() {
	Lua_Safe_Call_Void();
	self->Spin();
}

void Lua_Mob::Kill() {
	Lua_Safe_Call_Void();
	self->Kill();
}

bool Lua_Mob::CanThisClassDoubleAttack() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassDoubleAttack();
}

bool Lua_Mob::CanThisClassDualWield() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassDualWield();
}

bool Lua_Mob::CanThisClassRiposte() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassRiposte();
}

bool Lua_Mob::CanThisClassDodge() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassDodge();
}

bool Lua_Mob::CanThisClassParry() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassParry();
}

bool Lua_Mob::CanThisClassBlock() {
	Lua_Safe_Call_Bool();
	return self->CanThisClassBlock();
}

void Lua_Mob::SetInvul(bool value) {
	Lua_Safe_Call_Void();
	self->SetInvul(value);
}

bool Lua_Mob::GetInvul() {
	Lua_Safe_Call_Bool();
	return self->GetInvul();
}

void Lua_Mob::SetExtraHaste(int haste) {
	Lua_Safe_Call_Void();
	self->SetExtraHaste(haste);
}

int Lua_Mob::GetHaste() {
	Lua_Safe_Call_Int();
	return self->GetHaste();
}

int Lua_Mob::GetHandToHandDamage() {
	Lua_Safe_Call_Int();
	return self->GetHandToHandDamage();
}

int Lua_Mob::GetHandToHandDelay() {
	Lua_Safe_Call_Int();
	return self->GetHandToHandDelay();
}

void Lua_Mob::Mesmerize() {
	Lua_Safe_Call_Void();
	self->Mesmerize();
}

bool Lua_Mob::IsMezzed() {
	Lua_Safe_Call_Bool();
	return self->IsMezzed();
}

bool Lua_Mob::IsEnraged() {
	Lua_Safe_Call_Bool();
	return self->IsEnraged();
}

int Lua_Mob::GetReverseFactionCon(Lua_Mob other) {
	Lua_Safe_Call_Int();
	return self->GetReverseFactionCon(other);
}

bool Lua_Mob::IsAIControlled() {
	Lua_Safe_Call_Bool();
	return self->IsAIControlled();
}

float Lua_Mob::GetAggroRange() {
	Lua_Safe_Call_Real();
	return self->GetAggroRange();
}

float Lua_Mob::GetAssistRange() {
	Lua_Safe_Call_Real();
	return self->GetAssistRange();
}

void Lua_Mob::SetPetOrder(int order) {
	Lua_Safe_Call_Void();
	self->SetPetOrder(static_cast<Mob::eStandingPetOrder>(order));
}

int Lua_Mob::GetPetOrder() {
	Lua_Safe_Call_Int();
	return self->GetPetOrder();
}

bool Lua_Mob::IsRoamer() {
	Lua_Safe_Call_Bool();
	return self->IsRoamer();
}

bool Lua_Mob::IsRooted() {
	Lua_Safe_Call_Bool();
	return self->IsRooted();
}

bool Lua_Mob::IsEngaged() {
	Lua_Safe_Call_Bool();
	return self->IsEngaged();
}

void Lua_Mob::FaceTarget(Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->FaceTarget(target);
}

void Lua_Mob::SetHeading(double in) {
	Lua_Safe_Call_Void();
	self->SetHeading(static_cast<float>(in));
}

double Lua_Mob::CalculateHeadingToTarget(double in_x, double in_y) {
	Lua_Safe_Call_Real();
	return self->CalculateHeadingToTarget(static_cast<float>(in_x), static_cast<float>(in_y));
}

bool Lua_Mob::CalculateNewPosition(double x, double y, double z, double speed) {
	Lua_Safe_Call_Bool();
	return self->CalculateNewPosition(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(speed));
}

bool Lua_Mob::CalculateNewPosition(double x, double y, double z, double speed, bool check_z) {
	Lua_Safe_Call_Bool();
	return self->CalculateNewPosition(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(speed),
		check_z);
}

bool Lua_Mob::CalculateNewPosition2(double x, double y, double z, double speed) {
	Lua_Safe_Call_Bool();
	return self->CalculateNewPosition2(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(speed));
}

bool Lua_Mob::CalculateNewPosition2(double x, double y, double z, double speed, bool check_z) {
	Lua_Safe_Call_Bool();
	return self->CalculateNewPosition2(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(speed),
		check_z);
}

float Lua_Mob::CalculateDistance(double x, double y, double z) {
	Lua_Safe_Call_Real();
	return self->CalculateDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::SendTo(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->SendTo(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::SendToFixZ(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->SendToFixZ(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::NPCSpecialAttacks(const char *parse, int perm) {
	Lua_Safe_Call_Void();
	self->NPCSpecialAttacks(parse, perm);
}

void Lua_Mob::NPCSpecialAttacks(const char *parse, int perm, bool reset) {
	Lua_Safe_Call_Void();
	self->NPCSpecialAttacks(parse, perm, reset);
}

void Lua_Mob::NPCSpecialAttacks(const char *parse, int perm, bool reset, bool remove) {
	Lua_Safe_Call_Void();
	self->NPCSpecialAttacks(parse, perm, reset, remove);
}

int Lua_Mob::GetResist(int type) {
	Lua_Safe_Call_Int();
	return self->GetResist(type);
}

bool Lua_Mob::Charmed() {
	Lua_Safe_Call_Bool();
	return self->Charmed();
}

int Lua_Mob::CheckAggroAmount(int spell_id) {
	Lua_Safe_Call_Int();
	return self->CheckAggroAmount(spell_id, nullptr);
}

int Lua_Mob::CheckAggroAmount(int spell_id, bool is_proc) {
	Lua_Safe_Call_Int();
	return self->CheckAggroAmount(spell_id, nullptr, is_proc);
}

int Lua_Mob::CheckHealAggroAmount(int spell_id) {
	Lua_Safe_Call_Int();
	return self->CheckHealAggroAmount(spell_id, nullptr);
}

int Lua_Mob::CheckHealAggroAmount(int spell_id, uint32 heal_possible) {
	Lua_Safe_Call_Int();
	return self->CheckHealAggroAmount(spell_id, nullptr, heal_possible);
}

int Lua_Mob::GetAA(int id) {
	Lua_Safe_Call_Int();
	return self->GetAA(id);
}

int Lua_Mob::GetAAByAAID(int id) {
	Lua_Safe_Call_Int();
	return self->GetAAByAAID(id);
}

bool Lua_Mob::SetAA(int rank_id, int new_value) {
	Lua_Safe_Call_Bool();
	return self->SetAA(rank_id, new_value);
}

bool Lua_Mob::SetAA(int rank_id, int new_value, int charges) {
	Lua_Safe_Call_Bool();
	return self->SetAA(rank_id, new_value, charges);
}

bool Lua_Mob::DivineAura() {
	Lua_Safe_Call_Bool();
	return self->DivineAura();
}

void Lua_Mob::SetOOCRegen(int regen) {
	Lua_Safe_Call_Void();
	self->SetOOCRegen(regen);
}

const char* Lua_Mob::GetEntityVariable(const char *name) {
	Lua_Safe_Call_String();
	return self->GetEntityVariable(name);
}

void Lua_Mob::SetEntityVariable(const char *name, const char *value) {
	Lua_Safe_Call_Void();
	self->SetEntityVariable(name, value);
}

bool Lua_Mob::EntityVariableExists(const char *name) {
	Lua_Safe_Call_Bool();
	return self->EntityVariableExists(name);
}

void Lua_Mob::Signal(uint32 id) {
	Lua_Safe_Call_Void();

	if(self->IsClient()) {
		self->CastToClient()->Signal(id);
	} else if(self->IsNPC()) {
		self->CastToNPC()->SignalNPC(id);
	}
}

bool Lua_Mob::CombatRange(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->CombatRange(other);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQEmu::skills::SkillType>(skill), max_damage);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage, int min_damage) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQEmu::skills::SkillType>(skill), max_damage, min_damage);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage, int min_damage, int hate_override) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQEmu::skills::SkillType>(skill), max_damage, min_damage, hate_override);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage, int min_damage, int hate_override, int reuse_time) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQEmu::skills::SkillType>(skill), max_damage, min_damage, hate_override, reuse_time);
}

void Lua_Mob::DoSpecialAttackDamage(Lua_Mob other, int skill, int max_damage, int min_damage, int hate_override, int reuse_time,
									bool hit_chance) {
	Lua_Safe_Call_Void();
	self->DoSpecialAttackDamage(other, static_cast<EQEmu::skills::SkillType>(skill), max_damage, min_damage, hate_override, reuse_time, hit_chance);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_Item item) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon, item);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_Item item, int weapon_damage) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon, item, weapon_damage);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_Item item, int weapon_damage, int chance_mod) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon, item, weapon_damage, chance_mod);
}

void Lua_Mob::DoThrowingAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_Item item, int weapon_damage, int chance_mod,
								  int focus) {
	Lua_Safe_Call_Void();
	self->DoThrowingAttackDmg(other, range_weapon, item, weapon_damage, chance_mod, focus);
}

void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skill) {
	Lua_Safe_Call_Void();
	self->DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skill));
}

void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skill, int chance_mod) {
	Lua_Safe_Call_Void();
	self->DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skill), chance_mod);
}

void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skill, int chance_mod, int focus) {
	Lua_Safe_Call_Void();
	self->DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skill), chance_mod, focus);
}

void Lua_Mob::DoMeleeSkillAttackDmg(Lua_Mob other, int weapon_damage, int skill, int chance_mod, int focus, bool can_riposte) {
	Lua_Safe_Call_Void();
	self->DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skill), chance_mod, focus, can_riposte);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_ItemInst ammo) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon, ammo);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_ItemInst ammo, int weapon_damage) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon, ammo, weapon_damage);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_ItemInst ammo, int weapon_damage, int chance_mod) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon, ammo, weapon_damage, chance_mod);
}

void Lua_Mob::DoArcheryAttackDmg(Lua_Mob other, Lua_ItemInst range_weapon, Lua_ItemInst ammo, int weapon_damage, int chance_mod,
								 int focus) {
	Lua_Safe_Call_Void();
	self->DoArcheryAttackDmg(other, range_weapon, ammo, weapon_damage, chance_mod, focus);
}

bool Lua_Mob::CheckLoS(Lua_Mob other) {
	Lua_Safe_Call_Bool();
	return self->CheckLosFN(other);
}

bool Lua_Mob::CheckLoSToLoc(double x, double y, double z) {
	Lua_Safe_Call_Bool();
	return self->CheckLosFN(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 6.0f);
}

bool Lua_Mob::CheckLoSToLoc(double x, double y, double z, double mob_size) {
	Lua_Safe_Call_Bool();
	return self->CheckLosFN(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(mob_size));
}

double Lua_Mob::FindGroundZ(double x, double y) {
	Lua_Safe_Call_Real();
	return self->GetGroundZ(static_cast<float>(x), static_cast<float>(y));
}

double Lua_Mob::FindGroundZ(double x, double y, double z) {
	Lua_Safe_Call_Real();
	return self->GetGroundZ(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id);
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow);
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow, static_cast<float>(speed));
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow, static_cast<float>(speed), static_cast<float>(angle));
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle, double tilt) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow, static_cast<float>(speed), static_cast<float>(angle), static_cast<float>(tilt));
}

void Lua_Mob::ProjectileAnimation(Lua_Mob to, int item_id, bool is_arrow, double speed, double angle, double tilt, double arc) {
	Lua_Safe_Call_Void();
	self->ProjectileAnimation(to, item_id, is_arrow, static_cast<float>(speed), static_cast<float>(angle), static_cast<float>(tilt),
		static_cast<float>(arc));
}

bool Lua_Mob::HasNPCSpecialAtk(const char *parse) {
	Lua_Safe_Call_Bool();
	return self->HasNPCSpecialAtk(parse);
}

void Lua_Mob::SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5) {
	Lua_Safe_Call_Void();
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5);
}

void Lua_Mob::SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5, Lua_Client specific_target) {
	Lua_Safe_Call_Void();
	self->SendAppearanceEffect(parm1, parm2, parm3, parm4, parm5, specific_target);
}

void Lua_Mob::SetFlyMode(int in) {
	Lua_Safe_Call_Void();
	self->SetFlyMode(in);
}

void Lua_Mob::SetTexture(int in) {
	Lua_Safe_Call_Void();
	self->SendIllusionPacket(self->GetRace(), 0xFF, in);
}

void Lua_Mob::SetRace(int in) {
	Lua_Safe_Call_Void();
	self->SendIllusionPacket(in);
}

void Lua_Mob::SetGender(int in) {
	Lua_Safe_Call_Void();
	self->SendIllusionPacket(self->GetRace(), in);
}

void Lua_Mob::SendIllusionPacket(luabind::adl::object illusion) {
	Lua_Safe_Call_Void();

	if(luabind::type(illusion) != LUA_TTABLE) {
		return;
	}

	int race = 0;
	int gender = 255;
	int texture = 255;
	int helmtexture = 255;
	int haircolor = 255;
	int beardcolor = 255;
	int eyecolor1 = 255;
	int eyecolor2 = 255;
	int hairstyle = 255;
	int luclinface = 255;
	int beard = 255;
	int aa_title = 255;
	uint32 drakkin_heritage = 4294967295;
	uint32 drakkin_tattoo = 4294967295;
	uint32 drakkin_details = 4294967295;
	float size = -1.0f;

	auto cur = illusion["race"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			race = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["gender"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			gender = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["texture"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			texture = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["helmtexture"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			helmtexture = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["haircolor"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			haircolor = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["beardcolor"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			beardcolor = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["eyecolor1"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			eyecolor1 = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["eyecolor2"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			eyecolor2 = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["hairstyle"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			hairstyle = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["luclinface"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			luclinface = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["beard"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			beard = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["aa_title"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			aa_title = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["drakkin_heritage"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			drakkin_heritage = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["drakkin_tattoo"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			drakkin_tattoo = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["drakkin_details"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			drakkin_details = luabind::object_cast<int>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	cur = illusion["size"];
	if(luabind::type(cur) != LUA_TNIL) {
		try {
			size = luabind::object_cast<float>(cur);
		} catch(luabind::cast_failed) {
		}
	}

	self->SendIllusionPacket(race, gender, texture, helmtexture, haircolor, beardcolor, eyecolor1, eyecolor2, hairstyle, luclinface,
		beard, aa_title, drakkin_heritage, drakkin_tattoo, drakkin_details, size);
}

void Lua_Mob::CameraEffect(uint32 duration, uint32 intensity) {
	Lua_Safe_Call_Void();
	self->CameraEffect(duration, intensity);
}

void Lua_Mob::CameraEffect(uint32 duration, uint32 intensity, Lua_Client c) {
	Lua_Safe_Call_Void();
	self->CameraEffect(duration, intensity, c);
}

void Lua_Mob::CameraEffect(uint32 duration, uint32 intensity, Lua_Client c, bool global) {
	Lua_Safe_Call_Void();
	self->CameraEffect(duration, intensity, c, global);
}

void Lua_Mob::SendSpellEffect(uint32 effect_id, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020) {
	Lua_Safe_Call_Void();
	self->SendSpellEffect(effect_id, duration, finish_delay, zone_wide, unk020);
}

void Lua_Mob::SendSpellEffect(uint32 effect_id, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect) {
	Lua_Safe_Call_Void();
	self->SendSpellEffect(effect_id, duration, finish_delay, zone_wide, unk020, perm_effect);
}

void Lua_Mob::SendSpellEffect(uint32 effect_id, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect,
							  Lua_Client c) {
	Lua_Safe_Call_Void();
	self->SendSpellEffect(effect_id, duration, finish_delay, zone_wide, unk020, perm_effect, c);
}

void Lua_Mob::TempName() {
	Lua_Safe_Call_Void();
	self->TempName();
}

void Lua_Mob::TempName(const char *newname) {
	Lua_Safe_Call_Void();
	self->TempName(newname);
}

std::string Lua_Mob::GetGlobal(const char *varname) {
	Lua_Safe_Call_String();
	return self->GetGlobal(varname);
}

void Lua_Mob::SetGlobal(const char *varname, const char *newvalue, int options, const char *duration) {
	Lua_Safe_Call_Void();
	self->SetGlobal(varname, newvalue, options, duration);
}

void Lua_Mob::SetGlobal(const char *varname, const char *newvalue, int options, const char *duration, Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->SetGlobal(varname, newvalue, options, duration, other);
}

void Lua_Mob::TarGlobal(const char *varname, const char *value, const char *duration, int npc_id, int char_id, int zone_id) {
	Lua_Safe_Call_Void();
	self->TarGlobal(varname, value, duration, npc_id, char_id, zone_id);
}

void Lua_Mob::DelGlobal(const char *varname) {
	Lua_Safe_Call_Void();
	self->DelGlobal(varname);
}

void Lua_Mob::SetSlotTint(int material_slot, int red_tint, int green_tint, int blue_tint) {
	Lua_Safe_Call_Void();
	self->SetSlotTint(material_slot, red_tint, green_tint, blue_tint);
}

void Lua_Mob::WearChange(int material_slot, int texture, uint32 color) {
	Lua_Safe_Call_Void();
	self->WearChange(material_slot, texture, color);
}

void Lua_Mob::DoKnockback(Lua_Mob caster, uint32 pushback, uint32 pushup) {
	Lua_Safe_Call_Void();
	self->DoKnockback(caster, pushback, pushup);
}

void Lua_Mob::RemoveNimbusEffect(int effect_id) {
	Lua_Safe_Call_Void();
	self->RemoveNimbusEffect(effect_id);
}

bool Lua_Mob::IsRunning() {
	Lua_Safe_Call_Bool();
	return self->IsRunning();
}

void Lua_Mob::SetRunning(bool running) {
	Lua_Safe_Call_Void();
	self->SetRunning(running);
}

void Lua_Mob::SetBodyType(int new_body, bool overwrite_orig) {
	Lua_Safe_Call_Void();
	self->SetBodyType(static_cast<bodyType>(new_body), overwrite_orig);
}

void Lua_Mob::SetTargetable(bool on) {
	Lua_Safe_Call_Void();
	self->SetTargetable(on);
}

void Lua_Mob::ModSkillDmgTaken(int skill, int value) {
	Lua_Safe_Call_Void();
	self->ModSkillDmgTaken(static_cast<EQEmu::skills::SkillType>(skill), value);
}

int Lua_Mob::GetModSkillDmgTaken(int skill) {
	Lua_Safe_Call_Int();
	return self->GetModSkillDmgTaken(static_cast<EQEmu::skills::SkillType>(skill));
}

int Lua_Mob::GetSkillDmgTaken(int skill) {
	Lua_Safe_Call_Int();
	return self->GetSkillDmgTaken(static_cast<EQEmu::skills::SkillType>(skill));
}

void Lua_Mob::SetAllowBeneficial(bool value) {
	Lua_Safe_Call_Void();
	self->SetAllowBeneficial(value);
}

bool Lua_Mob::GetAllowBeneficial() {
	Lua_Safe_Call_Bool();
	return self->GetAllowBeneficial();
}

bool Lua_Mob::IsBeneficialAllowed(Lua_Mob target) {
	Lua_Safe_Call_Bool();
	return self->IsBeneficialAllowed(target);
}

void Lua_Mob::ModVulnerability(int resist, int value) {
	Lua_Safe_Call_Void();
	self->ModVulnerability(resist, value);
}

int Lua_Mob::GetModVulnerability(int resist) {
	Lua_Safe_Call_Int();
	return self->GetModVulnerability(resist);
}

void Lua_Mob::SetDisableMelee(bool disable) {
	Lua_Safe_Call_Void();
	self->SetDisableMelee(disable);
}

bool Lua_Mob::IsMeleeDisabled() {
	Lua_Safe_Call_Bool();
	return IsMeleeDisabled();
}

void Lua_Mob::SetFlurryChance(int value) {
	Lua_Safe_Call_Void();
	self->SetFlurryChance(value);
}

int Lua_Mob::GetFlurryChance() {
	Lua_Safe_Call_Int();
	return self->GetFlurryChance();
}

int Lua_Mob::GetSkill(int skill) {
	Lua_Safe_Call_Int();
	return self->GetSkill(static_cast<EQEmu::skills::SkillType>(skill));
}

int Lua_Mob::GetSpecialAbility(int ability) {
	Lua_Safe_Call_Int();
	return self->GetSpecialAbility(ability);
}

int Lua_Mob::GetSpecialAbilityParam(int ability, int param) {
	Lua_Safe_Call_Int();
	return self->GetSpecialAbilityParam(ability, param);
}

void Lua_Mob::SetSpecialAbility(int ability, int level) {
	Lua_Safe_Call_Void();
	self->SetSpecialAbility(ability, level);
}

void Lua_Mob::SetSpecialAbilityParam(int ability, int param, int value) {
	Lua_Safe_Call_Void();
	self->SetSpecialAbilityParam(ability, param, value);
}

void Lua_Mob::ClearSpecialAbilities() {
	Lua_Safe_Call_Void();
	self->ClearSpecialAbilities();
}

void Lua_Mob::ProcessSpecialAbilities(std::string str) {
	Lua_Safe_Call_Void();
	self->ProcessSpecialAbilities(str);
}

void Lua_Mob::SetAppearance(int app) {
	Lua_Safe_Call_Void();
	self->SetAppearance(static_cast<EmuAppearance>(app));
}

void Lua_Mob::SetAppearance(int app, bool ignore_self) {
	Lua_Safe_Call_Void();
	self->SetAppearance(static_cast<EmuAppearance>(app), ignore_self);
}

void Lua_Mob::SetDestructibleObject(bool set) {
	Lua_Safe_Call_Void();
	self->SetDestructibleObject(set);
}

bool Lua_Mob::IsImmuneToSpell(int spell_id, Lua_Mob caster) {
	Lua_Safe_Call_Bool();
	return self->IsImmuneToSpell(spell_id, caster);
}

void Lua_Mob::BuffFadeBySpellID(int spell_id) {
	Lua_Safe_Call_Void();
	self->BuffFadeBySpellID(spell_id);
}

void Lua_Mob::BuffFadeByEffect(int effect_id) {
	Lua_Safe_Call_Void();
	self->BuffFadeByEffect(effect_id);
}

void Lua_Mob::BuffFadeByEffect(int effect_id, int skipslot) {
	Lua_Safe_Call_Void();
	self->BuffFadeByEffect(effect_id, skipslot);
}

void Lua_Mob::BuffFadeAll() {
	Lua_Safe_Call_Void();
	self->BuffFadeAll();
}

void Lua_Mob::BuffFadeBySlot(int slot) {
	Lua_Safe_Call_Void();
	self->BuffFadeBySlot(slot);
}

void Lua_Mob::BuffFadeBySlot(int slot, bool recalc_bonuses) {
	Lua_Safe_Call_Void();
	self->BuffFadeBySlot(slot, recalc_bonuses);
}

int Lua_Mob::CanBuffStack(int spell_id, int caster_level) {
	Lua_Safe_Call_Int();
	return self->CanBuffStack(spell_id, caster_level);
}

int Lua_Mob::CanBuffStack(int spell_id, int caster_level, bool fail_if_overwrite) {
	Lua_Safe_Call_Int();
	return self->CanBuffStack(spell_id, caster_level, fail_if_overwrite);
}

void Lua_Mob::SetPseudoRoot(bool in) {
	Lua_Safe_Call_Void();
	self->SetPseudoRoot(in);
}

bool Lua_Mob::IsFeared() {
	Lua_Safe_Call_Bool();
	return self->IsFeared();
}

bool Lua_Mob::IsBlind() {
	Lua_Safe_Call_Bool();
	return self->IsBlind();
}

uint8 Lua_Mob::SeeInvisible() {
	Lua_Safe_Call_Int();
	return self->SeeInvisible();
}

bool Lua_Mob::SeeInvisibleUndead() {
	Lua_Safe_Call_Bool();
	return self->SeeInvisibleUndead();
}

bool Lua_Mob::SeeHide() {
	Lua_Safe_Call_Bool();
	return self->SeeHide();
}

bool Lua_Mob::SeeImprovedHide() {
	Lua_Safe_Call_Bool();
	return self->SeeImprovedHide();
}

uint8 Lua_Mob::GetNimbusEffect1() {
	Lua_Safe_Call_Int();
	return self->GetNimbusEffect1();
}

uint8 Lua_Mob::GetNimbusEffect2() {
	Lua_Safe_Call_Int();
	return self->GetNimbusEffect2();
}

uint8 Lua_Mob::GetNimbusEffect3() {
	Lua_Safe_Call_Int();
	return self->GetNimbusEffect3();
}

bool Lua_Mob::IsTargetable() {
	Lua_Safe_Call_Bool();
	return self->IsTargetable();
}

bool Lua_Mob::HasShieldEquiped() {
	Lua_Safe_Call_Bool();
	return self->HasShieldEquiped();
}

bool Lua_Mob::HasTwoHandBluntEquiped() {
	Lua_Safe_Call_Bool();
	return self->HasTwoHandBluntEquiped();
}

bool Lua_Mob::HasTwoHanderEquipped() {
	Lua_Safe_Call_Bool();
	return self->HasTwoHanderEquipped();
}

uint32 Lua_Mob::GetHerosForgeModel(uint8 material_slot) {
	Lua_Safe_Call_Int();
	return self->GetHerosForgeModel(material_slot);
}

uint32 Lua_Mob::IsEliteMaterialItem(uint8 material_slot) {
	Lua_Safe_Call_Int();
	return self->IsEliteMaterialItem(material_slot);
}

float Lua_Mob::GetBaseSize() {
	Lua_Safe_Call_Real();
	return self->GetBaseSize();
}

bool Lua_Mob::HasOwner() {
	Lua_Safe_Call_Bool();
	return self->HasOwner();
}

bool Lua_Mob::IsPet() {
	Lua_Safe_Call_Bool();
	return self->IsPet();
}

bool Lua_Mob::HasPet() {
	Lua_Safe_Call_Bool();
	return self->HasPet();
}

bool Lua_Mob::IsSilenced() {
	Lua_Safe_Call_Bool();
	return self->IsSilenced();
}

bool Lua_Mob::IsAmnesiad() {
	Lua_Safe_Call_Bool();
	return self->IsAmnesiad();
}

int32 Lua_Mob::GetMeleeMitigation() {
	Lua_Safe_Call_Int();
	return self->GetMeleeMitigation();
}

luabind::scope lua_register_mob() {
	return luabind::class_<Lua_Mob, Lua_Entity>("Mob")
		.def(luabind::constructor<>())
		.def("GetName", &Lua_Mob::GetName)
		.def("Depop", (void(Lua_Mob::*)(void))&Lua_Mob::Depop)
		.def("Depop", (void(Lua_Mob::*)(bool))&Lua_Mob::Depop)
		.def("BehindMob", (bool(Lua_Mob::*)(void))&Lua_Mob::BehindMob)
		.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::BehindMob)
		.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob,float))&Lua_Mob::BehindMob)
		.def("BehindMob", (bool(Lua_Mob::*)(Lua_Mob,float,float))&Lua_Mob::BehindMob)
		.def("SetLevel", (void(Lua_Mob::*)(int))&Lua_Mob::SetLevel)
		.def("SetLevel", (void(Lua_Mob::*)(int,bool))&Lua_Mob::SetLevel)
		.def("IsMoving", &Lua_Mob::IsMoving)
		.def("GotoBind", &Lua_Mob::GotoBind)
		.def("Attack", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::Attack)
		.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int))&Lua_Mob::Attack)
		.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool))&Lua_Mob::Attack)
		.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool,bool))&Lua_Mob::Attack)
		.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool,bool,bool))&Lua_Mob::Attack)
		.def("Attack", (bool(Lua_Mob::*)(Lua_Mob,int,bool,bool,bool,luabind::adl::object))&Lua_Mob::Attack)
		.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int,int,int))&Lua_Mob::Damage)
		.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,bool))&Lua_Mob::Damage)
		.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,bool,int))&Lua_Mob::Damage)
		.def("Damage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,bool,int,bool))&Lua_Mob::Damage)
		.def("RangedAttack", &Lua_Mob::RangedAttack)
		.def("ThrowingAttack", &Lua_Mob::ThrowingAttack)
		.def("Heal", &Lua_Mob::Heal)
		.def("HealDamage", (void(Lua_Mob::*)(uint32))&Lua_Mob::HealDamage)
		.def("HealDamage", (void(Lua_Mob::*)(uint32,Lua_Mob))&Lua_Mob::HealDamage)
		.def("GetLevelCon", (uint32(Lua_Mob::*)(int))&Lua_Mob::GetLevelCon)
		.def("GetLevelCon", (uint32(Lua_Mob::*)(int,int))&Lua_Mob::GetLevelCon)
		.def("SetHP", &Lua_Mob::SetHP)
		.def("DoAnim", (void(Lua_Mob::*)(int))&Lua_Mob::DoAnim)
		.def("DoAnim", (void(Lua_Mob::*)(int,int))&Lua_Mob::DoAnim)
		.def("DoAnim", (void(Lua_Mob::*)(int,int,bool))&Lua_Mob::DoAnim)
		.def("DoAnim", (void(Lua_Mob::*)(int,int,bool,int))&Lua_Mob::DoAnim)
		.def("ChangeSize", (void(Lua_Mob::*)(double))&Lua_Mob::ChangeSize)
		.def("ChangeSize", (void(Lua_Mob::*)(double,bool))&Lua_Mob::ChangeSize)
		.def("GMMove", (void(Lua_Mob::*)(double,double,double))&Lua_Mob::GMMove)
		.def("GMMove", (void(Lua_Mob::*)(double,double,double,double))&Lua_Mob::GMMove)
		.def("GMMove", (void(Lua_Mob::*)(double,double,double,double,bool))&Lua_Mob::GMMove)
		.def("HasProcs", &Lua_Mob::HasProcs)
		.def("IsInvisible", (bool(Lua_Mob::*)(void))&Lua_Mob::IsInvisible)
		.def("IsInvisible", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::IsInvisible)
		.def("SetInvisible", &Lua_Mob::SetInvisible)
		.def("FindBuff", &Lua_Mob::FindBuff)
		.def("FindType", (bool(Lua_Mob::*)(int))&Lua_Mob::FindType)
		.def("FindType", (bool(Lua_Mob::*)(int,bool))&Lua_Mob::FindType)
		.def("FindType", (bool(Lua_Mob::*)(int,bool,int))&Lua_Mob::FindType)
		.def("GetBuffSlotFromType", &Lua_Mob::GetBuffSlotFromType)
		.def("GetBaseRace", &Lua_Mob::GetBaseRace)
		.def("GetBaseGender", &Lua_Mob::GetBaseGender)
		.def("GetDeity", &Lua_Mob::GetDeity)
		.def("GetRace", &Lua_Mob::GetRace)
		.def("GetGender", &Lua_Mob::GetGender)
		.def("GetTexture", &Lua_Mob::GetTexture)
		.def("GetHelmTexture", &Lua_Mob::GetHelmTexture)
		.def("GetHairColor", &Lua_Mob::GetHairColor)
		.def("GetBeardColor", &Lua_Mob::GetBeardColor)
		.def("GetEyeColor1", &Lua_Mob::GetEyeColor1)
		.def("GetEyeColor2", &Lua_Mob::GetEyeColor2)
		.def("GetHairStyle", &Lua_Mob::GetHairStyle)
		.def("GetLuclinFace", &Lua_Mob::GetLuclinFace)
		.def("GetBeard", &Lua_Mob::GetBeard)
		.def("GetDrakkinHeritage", &Lua_Mob::GetDrakkinHeritage)
		.def("GetDrakkinTattoo", &Lua_Mob::GetDrakkinTattoo)
		.def("GetDrakkinDetails", &Lua_Mob::GetDrakkinDetails)
		.def("GetClass", &Lua_Mob::GetClass)
		.def("GetLevel", &Lua_Mob::GetLevel)
		.def("GetCleanName", &Lua_Mob::GetCleanName)
		.def("GetTarget", &Lua_Mob::GetTarget)
		.def("SetTarget", &Lua_Mob::SetTarget)
		.def("GetHPRatio", &Lua_Mob::GetHPRatio)
		.def("IsWarriorClass", &Lua_Mob::IsWarriorClass)
		.def("GetHP", &Lua_Mob::GetHP)
		.def("GetMaxHP", &Lua_Mob::GetMaxHP)
		.def("GetItemHPBonuses", &Lua_Mob::GetItemHPBonuses)
		.def("GetSpellHPBonuses", &Lua_Mob::GetSpellHPBonuses)
		.def("GetWalkspeed", &Lua_Mob::GetWalkspeed)
		.def("GetRunspeed", &Lua_Mob::GetRunspeed)
		.def("GetCasterLevel", &Lua_Mob::GetCasterLevel)
		.def("GetMaxMana", &Lua_Mob::GetMaxMana)
		.def("GetMana", &Lua_Mob::GetMana)
		.def("SetMana", &Lua_Mob::SetMana)
		.def("GetManaRatio", &Lua_Mob::GetManaRatio)
		.def("GetAC", &Lua_Mob::GetAC)
		.def("GetATK", &Lua_Mob::GetATK)
		.def("GetSTR", &Lua_Mob::GetSTR)
		.def("GetSTA", &Lua_Mob::GetSTA)
		.def("GetDEX", &Lua_Mob::GetDEX)
		.def("GetAGI", &Lua_Mob::GetAGI)
		.def("GetINT", &Lua_Mob::GetINT)
		.def("GetWIS", &Lua_Mob::GetWIS)
		.def("GetCHA", &Lua_Mob::GetCHA)
		.def("GetMR", &Lua_Mob::GetMR)
		.def("GetFR", &Lua_Mob::GetFR)
		.def("GetDR", &Lua_Mob::GetDR)
		.def("GetPR", &Lua_Mob::GetPR)
		.def("GetCR", &Lua_Mob::GetCR)
		.def("GetCorruption", &Lua_Mob::GetCorruption)
		.def("GetPhR", &Lua_Mob::GetPhR)
		.def("GetMaxSTR", &Lua_Mob::GetMaxSTR)
		.def("GetMaxSTA", &Lua_Mob::GetMaxSTA)
		.def("GetMaxDEX", &Lua_Mob::GetMaxDEX)
		.def("GetMaxAGI", &Lua_Mob::GetMaxAGI)
		.def("GetMaxINT", &Lua_Mob::GetMaxINT)
		.def("GetMaxWIS", &Lua_Mob::GetMaxWIS)
		.def("GetMaxCHA", &Lua_Mob::GetMaxCHA)
		.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob))&Lua_Mob::ResistSpell)
		.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool))&Lua_Mob::ResistSpell)
		.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool,int))&Lua_Mob::ResistSpell)
		.def("ResistSpell", (double(Lua_Mob::*)(int,int,Lua_Mob,bool,int,bool))&Lua_Mob::ResistSpell)
		.def("GetSpecializeSkillValue", &Lua_Mob::GetSpecializeSkillValue)
		.def("GetNPCTypeID", &Lua_Mob::GetNPCTypeID)
		.def("IsTargeted", &Lua_Mob::IsTargeted)
		.def("GetX", &Lua_Mob::GetX)
		.def("GetY", &Lua_Mob::GetY)
		.def("GetZ", &Lua_Mob::GetZ)
		.def("GetHeading", &Lua_Mob::GetHeading)
		.def("GetWaypointX", &Lua_Mob::GetWaypointX)
		.def("GetWaypointY", &Lua_Mob::GetWaypointY)
		.def("GetWaypointZ", &Lua_Mob::GetWaypointZ)
		.def("GetWaypointH", &Lua_Mob::GetWaypointH)
		.def("GetWaypointPause", &Lua_Mob::GetWaypointPause)
		.def("GetWaypointID", &Lua_Mob::GetWaypointID)
		.def("SetCurrentWP", &Lua_Mob::SetCurrentWP)
		.def("GetSize", &Lua_Mob::GetSize)
		.def("Message", &Lua_Mob::Message)
		.def("Message_StringID", &Lua_Mob::Message_StringID)
		.def("Say", &Lua_Mob::Say)
		.def("QuestSay", &Lua_Mob::QuestSay)
		.def("Shout", &Lua_Mob::Shout)
		.def("Emote", &Lua_Mob::Emote)
		.def("InterruptSpell", (void(Lua_Mob::*)(void))&Lua_Mob::InterruptSpell)
		.def("InterruptSpell", (void(Lua_Mob::*)(int))&Lua_Mob::InterruptSpell)
		.def("CastSpell", (bool(Lua_Mob::*)(int,int))&Lua_Mob::CastSpell)
		.def("CastSpell", (bool(Lua_Mob::*)(int,int,int))&Lua_Mob::CastSpell)
		.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int))&Lua_Mob::CastSpell)
		.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int))&Lua_Mob::CastSpell)
		.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int))&Lua_Mob::CastSpell)
		.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int,int,int))&Lua_Mob::CastSpell)
		.def("CastSpell", (bool(Lua_Mob::*)(int,int,int,int,int,int,int,int,int))&Lua_Mob::CastSpell)
		.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob))&Lua_Mob::SpellFinished)
		.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int))&Lua_Mob::SpellFinished)
		.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int))&Lua_Mob::SpellFinished)
		.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32))&Lua_Mob::SpellFinished)
		.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32,int))&Lua_Mob::SpellFinished)
		.def("SpellFinished", (bool(Lua_Mob::*)(int,Lua_Mob,int,int,uint32,int,bool))&Lua_Mob::SpellFinished)
		.def("SpellEffect", &Lua_Mob::SpellEffect)
		.def("GetHateList", &Lua_Mob::GetHateList)
		.def("GetHateTop", (Lua_Mob(Lua_Mob::*)(void))&Lua_Mob::GetHateTop)
		.def("GetHateDamageTop", (Lua_Mob(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetHateDamageTop)
		.def("GetHateRandom", (Lua_Mob(Lua_Mob::*)(void))&Lua_Mob::GetHateRandom)
		.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::AddToHateList)
		.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int))&Lua_Mob::AddToHateList)
		.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int,int))&Lua_Mob::AddToHateList)
		.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int,int,bool))&Lua_Mob::AddToHateList)
		.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int,int,bool,bool))&Lua_Mob::AddToHateList)
		.def("AddToHateList", (void(Lua_Mob::*)(Lua_Mob,int,int,bool,bool,bool))&Lua_Mob::AddToHateList)
		.def("SetHate", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::SetHate)
		.def("SetHate", (void(Lua_Mob::*)(Lua_Mob,int))&Lua_Mob::SetHate)
		.def("SetHate", (void(Lua_Mob::*)(Lua_Mob,int,int))&Lua_Mob::SetHate)
		.def("HalveAggro", &Lua_Mob::HalveAggro)
		.def("DoubleAggro", &Lua_Mob::DoubleAggro)
		.def("GetHateAmount", (uint32(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetHateAmount)
		.def("GetHateAmount", (uint32(Lua_Mob::*)(Lua_Mob,bool))&Lua_Mob::GetHateAmount)
		.def("GetDamageAmount", (uint32(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetDamageAmount)
		.def("WipeHateList", (void(Lua_Mob::*)(void))&Lua_Mob::WipeHateList)
		.def("CheckAggro", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CheckAggro)
		.def("Stun", (void(Lua_Mob::*)(int))&Lua_Mob::Stun)
		.def("UnStun", (void(Lua_Mob::*)(void))&Lua_Mob::UnStun)
		.def("IsStunned", (bool(Lua_Mob::*)(void))&Lua_Mob::IsStunned)
		.def("Spin", (void(Lua_Mob::*)(void))&Lua_Mob::Spin)
		.def("Kill", (void(Lua_Mob::*)(void))&Lua_Mob::Kill)
		.def("CanThisClassDoubleAttack", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDoubleAttack)
		.def("CanThisClassDualWield", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDualWield)
		.def("CanThisClassRiposte", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassRiposte)
		.def("CanThisClassDodge", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDodge)
		.def("CanThisClassParry", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassParry)
		.def("CanThisClassBlock", (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassBlock)
		.def("SetInvul", (void(Lua_Mob::*)(bool))&Lua_Mob::SetInvul)
		.def("GetInvul", (bool(Lua_Mob::*)(void))&Lua_Mob::GetInvul)
		.def("SetExtraHaste", (void(Lua_Mob::*)(int))&Lua_Mob::SetExtraHaste)
		.def("GetHaste", (int(Lua_Mob::*)(void))&Lua_Mob::GetHaste)
		.def("GetHandToHandDamage", (int(Lua_Mob::*)(void))&Lua_Mob::GetHandToHandDamage)
		.def("GetHandToHandDelay", (int(Lua_Mob::*)(void))&Lua_Mob::GetHandToHandDelay)
		.def("Mesmerize", (void(Lua_Mob::*)(void))&Lua_Mob::Mesmerize)
		.def("IsMezzed", (bool(Lua_Mob::*)(void))&Lua_Mob::IsMezzed)
		.def("IsEnraged", (bool(Lua_Mob::*)(void))&Lua_Mob::IsEnraged)
		.def("GetReverseFactionCon", (int(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetReverseFactionCon)
		.def("IsAIControlled", (bool(Lua_Mob::*)(void))&Lua_Mob::IsAIControlled)
		.def("GetAggroRange", (float(Lua_Mob::*)(void))&Lua_Mob::GetAggroRange)
		.def("GetAssistRange", (float(Lua_Mob::*)(void))&Lua_Mob::GetAssistRange)
		.def("SetPetOrder", (void(Lua_Mob::*)(int))&Lua_Mob::SetPetOrder)
		.def("GetPetOrder", (int(Lua_Mob::*)(void))&Lua_Mob::GetPetOrder)
		.def("IsRoamer", (bool(Lua_Mob::*)(void))&Lua_Mob::IsRoamer)
		.def("IsRooted", (bool(Lua_Mob::*)(void))&Lua_Mob::IsRooted)
		.def("IsEngaged", (bool(Lua_Mob::*)(void))&Lua_Mob::IsEngaged)
		.def("FaceTarget", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::FaceTarget)
		.def("SetHeading", (void(Lua_Mob::*)(double))&Lua_Mob::SetHeading)
		.def("CalculateHeadingToTarget", (double(Lua_Mob::*)(double,double))&Lua_Mob::CalculateHeadingToTarget)
		.def("CalculateNewPosition", (bool(Lua_Mob::*)(double,double,double,double))&Lua_Mob::CalculateNewPosition)
		.def("CalculateNewPosition", (bool(Lua_Mob::*)(double,double,double,double,bool))&Lua_Mob::CalculateNewPosition)
		.def("CalculateNewPosition2", (bool(Lua_Mob::*)(double,double,double,double))&Lua_Mob::CalculateNewPosition2)
		.def("CalculateNewPosition2", (bool(Lua_Mob::*)(double,double,double,double,bool))&Lua_Mob::CalculateNewPosition2)
		.def("CalculateDistance", (float(Lua_Mob::*)(double,double,double))&Lua_Mob::CalculateDistance)
		.def("SendTo", (void(Lua_Mob::*)(double,double,double))&Lua_Mob::SendTo)
		.def("SendToFixZ", (void(Lua_Mob::*)(double,double,double))&Lua_Mob::SendToFixZ)
		.def("NPCSpecialAttacks", (void(Lua_Mob::*)(const char*,int))&Lua_Mob::NPCSpecialAttacks)
		.def("NPCSpecialAttacks", (void(Lua_Mob::*)(const char*,int,bool))&Lua_Mob::NPCSpecialAttacks)
		.def("NPCSpecialAttacks", (void(Lua_Mob::*)(const char*,int,bool,bool))&Lua_Mob::NPCSpecialAttacks)
		.def("GetResist", (int(Lua_Mob::*)(int))&Lua_Mob::GetResist)
		.def("Charmed", (bool(Lua_Mob::*)(void))&Lua_Mob::Charmed)
		.def("CheckAggroAmount", (int(Lua_Mob::*)(int))&Lua_Mob::CheckAggroAmount)
		.def("CheckAggroAmount", (int(Lua_Mob::*)(int,bool))&Lua_Mob::CheckAggroAmount)
		.def("CheckHealAggroAmount", (int(Lua_Mob::*)(int))&Lua_Mob::CheckHealAggroAmount)
		.def("CheckHealAggroAmount", (int(Lua_Mob::*)(int,uint32))&Lua_Mob::CheckHealAggroAmount)
		.def("GetAA", (int(Lua_Mob::*)(int))&Lua_Mob::GetAA)
		.def("GetAAByAAID", (int(Lua_Mob::*)(int))&Lua_Mob::GetAAByAAID)
		.def("SetAA", (bool(Lua_Mob::*)(int,int))&Lua_Mob::SetAA)
		.def("SetAA", (bool(Lua_Mob::*)(int,int,int))&Lua_Mob::SetAA)
		.def("DivineAura", (bool(Lua_Mob::*)(void))&Lua_Mob::DivineAura)
		.def("SetOOCRegen", (void(Lua_Mob::*)(int))&Lua_Mob::SetOOCRegen)
		.def("GetEntityVariable", (const char*(Lua_Mob::*)(const char*))&Lua_Mob::GetEntityVariable)
		.def("SetEntityVariable", (void(Lua_Mob::*)(const char*,const char*))&Lua_Mob::SetEntityVariable)
		.def("EntityVariableExists", (bool(Lua_Mob::*)(const char*))&Lua_Mob::EntityVariableExists)
		.def("Signal", (void(Lua_Mob::*)(uint32))&Lua_Mob::Signal)
		.def("CombatRange", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CombatRange)
		.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int))&Lua_Mob::DoSpecialAttackDamage)
		.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int,int))&Lua_Mob::DoSpecialAttackDamage)
		.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int))&Lua_Mob::DoSpecialAttackDamage)
		.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int,int))&Lua_Mob::DoSpecialAttackDamage)
		.def("DoSpecialAttackDamage", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int,int,bool))&Lua_Mob::DoSpecialAttackDamage)
		.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::DoThrowingAttackDmg)
		.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst))&Lua_Mob::DoThrowingAttackDmg)
		.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_Item))&Lua_Mob::DoThrowingAttackDmg)
		.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_Item,int))&Lua_Mob::DoThrowingAttackDmg)
		.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_Item,int,int))&Lua_Mob::DoThrowingAttackDmg)
		.def("DoThrowingAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_Item,int,int,int))&Lua_Mob::DoThrowingAttackDmg)
		.def("DoMeleeSkillAttackDmg", (void(Lua_Mob::*)(Lua_Mob,int,int))&Lua_Mob::DoMeleeSkillAttackDmg)
		.def("DoMeleeSkillAttackDmg", (void(Lua_Mob::*)(Lua_Mob,int,int,int))&Lua_Mob::DoMeleeSkillAttackDmg)
		.def("DoMeleeSkillAttackDmg", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int))&Lua_Mob::DoMeleeSkillAttackDmg)
		.def("DoMeleeSkillAttackDmg", (void(Lua_Mob::*)(Lua_Mob,int,int,int,int,bool))&Lua_Mob::DoMeleeSkillAttackDmg)
		.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::DoArcheryAttackDmg)
		.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst))&Lua_Mob::DoArcheryAttackDmg)
		.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_ItemInst))&Lua_Mob::DoArcheryAttackDmg)
		.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_ItemInst,int))&Lua_Mob::DoArcheryAttackDmg)
		.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_ItemInst,int,int))&Lua_Mob::DoArcheryAttackDmg)
		.def("DoArcheryAttackDmg", (void(Lua_Mob::*)(Lua_Mob,Lua_ItemInst,Lua_ItemInst,int,int,int))&Lua_Mob::DoArcheryAttackDmg)
		.def("CheckLoS", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CheckLoS)
		.def("CheckLoSToLoc", (bool(Lua_Mob::*)(double,double,double))&Lua_Mob::CheckLoSToLoc)
		.def("CheckLoSToLoc", (bool(Lua_Mob::*)(double,double,double,double))&Lua_Mob::CheckLoSToLoc)
		.def("FindGroundZ", (double(Lua_Mob::*)(double,double))&Lua_Mob::FindGroundZ)
		.def("FindGroundZ", (double(Lua_Mob::*)(double,double,double))&Lua_Mob::FindGroundZ)
		.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int))&Lua_Mob::ProjectileAnimation)
		.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool))&Lua_Mob::ProjectileAnimation)
		.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool,double))&Lua_Mob::ProjectileAnimation)
		.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool,double,double))&Lua_Mob::ProjectileAnimation)
		.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool,double,double,double))&Lua_Mob::ProjectileAnimation)
		.def("ProjectileAnimation", (void(Lua_Mob::*)(Lua_Mob,int,bool,double,double,double,double))&Lua_Mob::ProjectileAnimation)
		.def("HasNPCSpecialAtk", (bool(Lua_Mob::*)(const char*))&Lua_Mob::HasNPCSpecialAtk)
		.def("SendAppearanceEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,uint32,uint32))&Lua_Mob::SendAppearanceEffect)
		.def("SendAppearanceEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,uint32,uint32,Lua_Client))&Lua_Mob::SendAppearanceEffect)
		.def("SetFlyMode", (void(Lua_Mob::*)(int))&Lua_Mob::SetFlyMode)
		.def("SetTexture", (void(Lua_Mob::*)(int))&Lua_Mob::SetTexture)
		.def("SetRace", (void(Lua_Mob::*)(int))&Lua_Mob::SetRace)
		.def("SetGender", (void(Lua_Mob::*)(int))&Lua_Mob::SetGender)
		.def("SendIllusionPacket", (void(Lua_Mob::*)(luabind::adl::object))&Lua_Mob::SendIllusionPacket)
		.def("CameraEffect", (void(Lua_Mob::*)(uint32,uint32))&Lua_Mob::CameraEffect)
		.def("CameraEffect", (void(Lua_Mob::*)(uint32,uint32,Lua_Client))&Lua_Mob::CameraEffect)
		.def("CameraEffect", (void(Lua_Mob::*)(uint32,uint32,Lua_Client,bool))&Lua_Mob::CameraEffect)
		.def("SendSpellEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,bool,uint32))&Lua_Mob::SendSpellEffect)
		.def("SendSpellEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,bool,uint32,bool))&Lua_Mob::SendSpellEffect)
		.def("SendSpellEffect", (void(Lua_Mob::*)(uint32,uint32,uint32,bool,uint32,bool,Lua_Client))&Lua_Mob::SendSpellEffect)
		.def("TempName", (void(Lua_Mob::*)(void))&Lua_Mob::TempName)
		.def("TempName", (void(Lua_Mob::*)(const char*))&Lua_Mob::TempName)
		.def("GetGlobal", (std::string(Lua_Mob::*)(const char*))&Lua_Mob::GetGlobal)
		.def("SetGlobal", (void(Lua_Mob::*)(const char*,const char*,int,const char*))&Lua_Mob::SetGlobal)
		.def("SetGlobal", (void(Lua_Mob::*)(const char*,const char*,int,const char*,Lua_Mob))&Lua_Mob::SetGlobal)
		.def("TarGlobal", (void(Lua_Mob::*)(const char*,const char*,const char*,int,int,int))&Lua_Mob::TarGlobal)
		.def("DelGlobal", (void(Lua_Mob::*)(const char*))&Lua_Mob::DelGlobal)
		.def("SetSlotTint", (void(Lua_Mob::*)(int,int,int,int))&Lua_Mob::SetSlotTint)
		.def("WearChange", (void(Lua_Mob::*)(int,int,uint32))&Lua_Mob::WearChange)
		.def("DoKnockback", (void(Lua_Mob::*)(Lua_Mob,uint32,uint32))&Lua_Mob::DoKnockback)
		.def("RemoveNimbusEffect", (void(Lua_Mob::*)(int))&Lua_Mob::RemoveNimbusEffect)
		.def("IsFeared", (bool(Lua_Mob::*)(void))&Lua_Mob::IsFeared)
		.def("IsBlind", (bool(Lua_Mob::*)(void))&Lua_Mob::IsBlind)
		.def("IsRunning", (bool(Lua_Mob::*)(void))&Lua_Mob::IsRunning)
		.def("SetRunning", (void(Lua_Mob::*)(bool))&Lua_Mob::SetRunning)
		.def("SetBodyType", (void(Lua_Mob::*)(int,bool))&Lua_Mob::SetBodyType)
		.def("SetTargetable", (void(Lua_Mob::*)(bool))&Lua_Mob::SetTargetable)
		.def("ModSkillDmgTaken", (void(Lua_Mob::*)(int,int))&Lua_Mob::ModSkillDmgTaken)
		.def("GetModSkillDmgTaken", (int(Lua_Mob::*)(int))&Lua_Mob::GetModSkillDmgTaken)
		.def("GetSkillDmgTaken", (int(Lua_Mob::*)(int))&Lua_Mob::GetSkillDmgTaken)
		.def("SetAllowBeneficial", (void(Lua_Mob::*)(bool))&Lua_Mob::SetAllowBeneficial)
		.def("GetAllowBeneficial", (bool(Lua_Mob::*)(void))&Lua_Mob::GetAllowBeneficial)
		.def("IsBeneficialAllowed", (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::IsBeneficialAllowed)
		.def("ModVulnerability", (void(Lua_Mob::*)(int,int))&Lua_Mob::ModVulnerability)
		.def("GetModVulnerability", (int(Lua_Mob::*)(int))&Lua_Mob::GetModVulnerability)
		.def("SetDisableMelee", (void(Lua_Mob::*)(bool))&Lua_Mob::SetDisableMelee)
		.def("IsMeleeDisabled", (bool(Lua_Mob::*)(void))&Lua_Mob::IsMeleeDisabled)
		.def("SetFlurryChance", (void(Lua_Mob::*)(int))&Lua_Mob::SetFlurryChance)
		.def("GetFlurryChance", (int(Lua_Mob::*)(void))&Lua_Mob::GetFlurryChance)
		.def("GetSkill", (int(Lua_Mob::*)(int))&Lua_Mob::GetSkill)
		.def("GetSpecialAbility", (int(Lua_Mob::*)(int))&Lua_Mob::GetSpecialAbility)
		.def("GetSpecialAbilityParam", (int(Lua_Mob::*)(int,int))&Lua_Mob::GetSpecialAbilityParam)
		.def("SetSpecialAbility", (void(Lua_Mob::*)(int,int))&Lua_Mob::SetSpecialAbility)
		.def("SetSpecialAbilityParam", (void(Lua_Mob::*)(int,int,int))&Lua_Mob::SetSpecialAbilityParam)
		.def("ClearSpecialAbilities", (void(Lua_Mob::*)(void))&Lua_Mob::ClearSpecialAbilities)
		.def("ProcessSpecialAbilities", (void(Lua_Mob::*)(std::string))&Lua_Mob::ProcessSpecialAbilities)
		.def("SetAppearance", (void(Lua_Mob::*)(int))&Lua_Mob::SetAppearance)
		.def("SetAppearance", (void(Lua_Mob::*)(int,bool))&Lua_Mob::SetAppearance)
		.def("SetDestructibleObject", (void(Lua_Mob::*)(bool))&Lua_Mob::SetDestructibleObject)
		.def("IsImmuneToSpell", (bool(Lua_Mob::*)(int,Lua_Mob))&Lua_Mob::IsImmuneToSpell)
		.def("BuffFadeBySpellID", (void(Lua_Mob::*)(int))&Lua_Mob::BuffFadeBySpellID)
		.def("BuffFadeByEffect", (void(Lua_Mob::*)(int))&Lua_Mob::BuffFadeByEffect)
		.def("BuffFadeByEffect", (void(Lua_Mob::*)(int,int))&Lua_Mob::BuffFadeByEffect)
		.def("BuffFadeAll", (void(Lua_Mob::*)(void))&Lua_Mob::BuffFadeAll)
		.def("BuffFadeBySlot", (void(Lua_Mob::*)(int))&Lua_Mob::BuffFadeBySlot)
		.def("BuffFadeBySlot", (void(Lua_Mob::*)(int,bool))&Lua_Mob::BuffFadeBySlot)
		.def("CanBuffStack", (int(Lua_Mob::*)(int,int))&Lua_Mob::CanBuffStack)
		.def("CanBuffStack", (int(Lua_Mob::*)(int,int,bool))&Lua_Mob::CanBuffStack)
		.def("SetPseudoRoot", (void(Lua_Mob::*)(bool))&Lua_Mob::SetPseudoRoot)
		.def("SeeInvisible", (uint8(Lua_Mob::*)(void))&Lua_Mob::SeeInvisible)
		.def("SeeInvisibleUndead", (bool(Lua_Mob::*)(void))&Lua_Mob::SeeInvisibleUndead)
		.def("SeeHide", (bool(Lua_Mob::*)(void))&Lua_Mob::SeeHide)
		.def("SeeImprovedHide", (bool(Lua_Mob::*)(bool))&Lua_Mob::SeeImprovedHide)
		.def("GetNimbusEffect1", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect1)
		.def("GetNimbusEffect2", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect2)
		.def("GetNimbusEffect3", (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect3)
		.def("IsTargetable", (bool(Lua_Mob::*)(void))&Lua_Mob::IsTargetable)
		.def("HasShieldEquiped", (bool(Lua_Mob::*)(void))&Lua_Mob::HasShieldEquiped)		
		.def("HasTwoHandBluntEquiped", (bool(Lua_Mob::*)(void))&Lua_Mob::HasTwoHandBluntEquiped)
		.def("HasTwoHanderEquipped", (bool(Lua_Mob::*)(void))&Lua_Mob::HasTwoHanderEquipped)
		.def("GetHerosForgeModel", (int32(Lua_Mob::*)(uint8))&Lua_Mob::GetHerosForgeModel)
		.def("IsEliteMaterialItem", (uint32(Lua_Mob::*)(uint8))&Lua_Mob::IsEliteMaterialItem)
		.def("GetBaseSize", (double(Lua_Mob::*)(void))&Lua_Mob::GetBaseSize)
		.def("HasOwner", (bool(Lua_Mob::*)(void))&Lua_Mob::HasOwner)
		.def("IsPet", (bool(Lua_Mob::*)(void))&Lua_Mob::IsPet)
		.def("HasPet", (bool(Lua_Mob::*)(void))&Lua_Mob::HasPet)
		.def("IsSilenced", (bool(Lua_Mob::*)(void))&Lua_Mob::IsSilenced)
		.def("IsAmnesiad", (bool(Lua_Mob::*)(void))&Lua_Mob::IsAmnesiad)
		.def("GetMeleeMitigation", (int32(Lua_Mob::*)(void))&Lua_Mob::GetMeleeMitigation);
}

luabind::scope lua_register_special_abilities() {
	return luabind::class_<SpecialAbilities>("SpecialAbility")
		.enum_("constants")
		[
				luabind::value("summon", static_cast<int>(SPECATK_SUMMON)),
				luabind::value("enrage", static_cast<int>(SPECATK_ENRAGE)),
				luabind::value("rampage", static_cast<int>(SPECATK_RAMPAGE)),
				luabind::value("area_rampage", static_cast<int>(SPECATK_AREA_RAMPAGE)),
				luabind::value("flurry", static_cast<int>(SPECATK_FLURRY)),
				luabind::value("triple_attack", static_cast<int>(SPECATK_TRIPLE)),
				luabind::value("quad_attack", static_cast<int>(SPECATK_QUAD)),
				luabind::value("innate_dual_wield", static_cast<int>(SPECATK_INNATE_DW)),
				luabind::value("bane_attack", static_cast<int>(SPECATK_BANE)),
				luabind::value("magical_attack", static_cast<int>(SPECATK_MAGICAL)),
				luabind::value("ranged_attack", static_cast<int>(SPECATK_RANGED_ATK)),
				luabind::value("unslowable", static_cast<int>(UNSLOWABLE)),
				luabind::value("unmezable", static_cast<int>(UNMEZABLE)),
				luabind::value("uncharmable", static_cast<int>(UNCHARMABLE)),
				luabind::value("unstunable", static_cast<int>(UNSTUNABLE)),
				luabind::value("unsnareable", static_cast<int>(UNSNAREABLE)),
				luabind::value("unfearable", static_cast<int>(UNFEARABLE)),
				luabind::value("undispellable", static_cast<int>(UNDISPELLABLE)),
				luabind::value("immune_melee", static_cast<int>(IMMUNE_MELEE)),
				luabind::value("immune_magic", static_cast<int>(IMMUNE_MAGIC)),
				luabind::value("immune_fleeing", static_cast<int>(IMMUNE_FLEEING)),
				luabind::value("immune_melee_except_bane", static_cast<int>(IMMUNE_MELEE_EXCEPT_BANE)),
				luabind::value("immune_melee_except_magical", static_cast<int>(IMMUNE_MELEE_NONMAGICAL)),
				luabind::value("immune_aggro", static_cast<int>(IMMUNE_AGGRO)),
				luabind::value("immune_aggro_on", static_cast<int>(IMMUNE_AGGRO_ON)),
				luabind::value("immune_casting_from_range", static_cast<int>(IMMUNE_CASTING_FROM_RANGE)),
				luabind::value("immune_feign_death", static_cast<int>(IMMUNE_FEIGN_DEATH)),
				luabind::value("immune_taunt", static_cast<int>(IMMUNE_TAUNT)),
				luabind::value("tunnelvision", static_cast<int>(NPC_TUNNELVISION)),
				luabind::value("dont_buff_friends", static_cast<int>(NPC_NO_BUFFHEAL_FRIENDS)),
				luabind::value("immune_pacify", static_cast<int>(IMMUNE_PACIFY)),
				luabind::value("leash", static_cast<int>(LEASH)),
				luabind::value("tether", static_cast<int>(TETHER)),
				luabind::value("destructible_object", static_cast<int>(DESTRUCTIBLE_OBJECT)),
				luabind::value("no_harm_from_client", static_cast<int>(NO_HARM_FROM_CLIENT)),
				luabind::value("always_flee", static_cast<int>(ALWAYS_FLEE)),
				luabind::value("flee_percent", static_cast<int>(FLEE_PERCENT)),
				luabind::value("allow_beneficial", static_cast<int>(ALLOW_BENEFICIAL)),
				luabind::value("disable_melee", static_cast<int>(DISABLE_MELEE)),
				luabind::value("npc_chase_distance", static_cast<int>(NPC_CHASE_DISTANCE)),
				luabind::value("allow_to_tank", static_cast<int>(ALLOW_TO_TANK)),
				luabind::value("ignore_root_aggro_rules", static_cast<int>(IGNORE_ROOT_AGGRO_RULES)),
				luabind::value("casting_resist_diff", static_cast<int>(CASTING_RESIST_DIFF)),
				luabind::value("counter_avoid_damage", static_cast<int>(COUNTER_AVOID_DAMAGE))
		];
}

#endif
