#ifdef LUA_EQEMU

#include "masterentity.h"
#include "lua_mob.h"
#include "lua_hate_entry.h"
#include "lua_hate_list.h"
#include "lua_client.h"

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

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<SkillType>(attack_skill));
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<SkillType>(attack_skill), avoidable);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<SkillType>(attack_skill), avoidable, buffslot);
}

void Lua_Mob::Damage(Lua_Mob from, int damage, int spell_id, int attack_skill, bool avoidable, int buffslot, bool buff_tic) {
	Lua_Safe_Call_Void();
	return self->Damage(from, damage, spell_id, static_cast<SkillType>(attack_skill), avoidable, buffslot, buff_tic);
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
	self->ChangeSize(in_size);
}

void Lua_Mob::ChangeSize(double in_size, bool no_restriction) {
	Lua_Safe_Call_Void();
	self->ChangeSize(in_size, no_restriction);
}

void Lua_Mob::GMMove(double x, double y, double z) {
	Lua_Safe_Call_Void();
	self->GMMove(x, y, z);
}

void Lua_Mob::GMMove(double x, double y, double z, double heading) {
	Lua_Safe_Call_Void();
	self->GMMove(x, y, z, heading);
}

void Lua_Mob::GMMove(double x, double y, double z, double heading, bool send_update) {
	Lua_Safe_Call_Void();
	self->GMMove(x, y, z, heading, send_update);
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
	Lua_Safe_Call_Mob();
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
	return self->GetCWPX();
}

double Lua_Mob::GetWaypointY() {
	Lua_Safe_Call_Real();
	return self->GetCWPY();
}

double Lua_Mob::GetWaypointZ() {
	Lua_Safe_Call_Real();
	return self->GetCWPZ();
}

double Lua_Mob::GetWaypointH() {
	Lua_Safe_Call_Real();
	return self->GetCWPH();
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
		static_cast<uint32>(timer), static_cast<uint32>(timer_duration), 0, &res);
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
	self->SpellEffect(caster, spell_id, partial);
}

Lua_Mob Lua_Mob::GetPet() {
	Lua_Safe_Call_Mob();
	return Lua_Mob(self->GetPet());
}

Lua_Mob Lua_Mob::GetOwner() {
	Lua_Safe_Call_Mob();
	return Lua_Mob(self->GetOwner());
}

Lua_HateList Lua_Mob::GetHateList() {
	Lua_Safe_Call_HateList();
	Lua_HateList ret;
	
	std::list<tHateEntry*> h_list;
	self->GetHateList(h_list);
	auto iter = h_list.begin();
	while(iter != h_list.end()) {
		tHateEntry *ent = (*iter);
		Lua_HateEntry e;
		e.ent = Lua_Mob(ent->ent);
		e.damage = ent->damage;
		e.hate = ent->hate;
		e.frenzy = ent->bFrenzy;
		ret.entries.push_back(e);
		++iter;
	}

	return ret;
}

#endif
