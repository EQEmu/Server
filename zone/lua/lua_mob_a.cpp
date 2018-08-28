#include <sol.hpp>
#include "../mob.h"

void mob_depop(Mob &m) {
	m.Depop();
}

void mob_depop1(Mob &m, bool start_spawn_timer) {
	m.Depop(start_spawn_timer);
}

bool mob_behind_mob(Mob &m) {
	return m.BehindMob();
}

bool mob_behind_mob1(Mob &m, Mob *other) {
	return m.BehindMob(other);
}

bool mob_behind_mob2(Mob &m, Mob *other, float ourx) {
	return m.BehindMob(other, ourx);
}

bool mob_behind_mob3(Mob &m, Mob *other, float ourx, float oury) {
	return m.BehindMob(other, ourx, oury);
}

void mob_set_level(Mob &m, uint8 level) {
	m.SetLevel(level);
}

void mob_set_level1(Mob &m, uint8 level, bool command) {
	m.SetLevel(level, command);
}

bool mob_attack(Mob &m, Mob *other) {
	return m.Attack(other);
}

bool mob_attack1(Mob &m, Mob *other, int hand) {
	return m.Attack(other, hand);
}

bool mob_attack2(Mob &m, Mob *other, int hand, bool from_riposte) {
	return m.Attack(other, hand, from_riposte);
}

bool mob_attack3(Mob &m, Mob *other, int hand, bool from_riposte, bool is_strikethrough) {
	return m.Attack(other, hand, from_riposte, is_strikethrough);
}

bool mob_attack4(Mob &m, Mob *other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell) {
	return m.Attack(other, hand, from_riposte, is_strikethrough, is_from_spell);
}

bool mob_attack5(Mob &m, Mob *other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell, ExtraAttackOptions *opts) {
	return m.Attack(other, hand, from_riposte, is_strikethrough, is_from_spell, opts);
}

void mob_damage(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill));
}

void mob_damage1(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill, bool avoidable) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable);
}

void mob_damage2(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill, bool avoidable, int8 buffslot) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable, buffslot);
}

void mob_damage3(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill, bool avoidable, int8 buffslot, bool bufftic) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable, buffslot, bufftic);
}

void mob_damage4(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill, bool avoidable, int8 buffslot, bool bufftic, int special) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable, buffslot, bufftic, static_cast<eSpecialAttacks>(special));
}

void mob_heal_damage(Mob &m, uint32 amount) {
	m.HealDamage(amount);
}

void mob_heal_damage1(Mob &m, uint32 amount, Mob *caster) {
	m.HealDamage(amount, caster);
}

void mob_heal_damage2(Mob &m, uint32 amount, Mob *caster, uint16 spell_id) {
	m.HealDamage(amount, caster, spell_id);
}

uint32 mob_get_level_con(Mob &m, uint8 other) {
	return m.GetLevelCon(other);
}

uint32 mob_get_level_con1(Mob &m, uint8 mylevel, uint8 other) {
	return m.GetLevelCon(mylevel, other);
}

void mob_do_anim(Mob &m, int animnum) {
	m.DoAnim(animnum);
}

void mob_do_anim1(Mob &m, int animnum, int type) {
	m.DoAnim(animnum, type);
}

void mob_do_anim2(Mob &m, int animnum, int type, bool ackreq) {
	m.DoAnim(animnum, type, ackreq);
}

void mob_do_anim3(Mob &m, int animnum, int type, bool ackreq, int filter) {
	m.DoAnim(animnum, type, ackreq, static_cast<eqFilterType>(filter));
}

void mob_change_size(Mob &m, float sz) {
	m.ChangeSize(sz);
}

void mob_change_size1(Mob &m, float sz, bool no_restriction) {
	m.ChangeSize(sz, no_restriction);
}

void mob_gm_move(Mob &m, float x, float y, float z) {
	m.GMMove(x, y, z);
}

void mob_gm_move1(Mob &m, float x, float y, float z, float heading) {
	m.GMMove(x, y, z, heading);
}

void mob_gm_move2(Mob &m, float x, float y, float z, float heading, bool send_update) {
	m.GMMove(x, y, z, heading, send_update);
}

void mob_try_move_along(Mob &m, float distance, float angle) {
	m.TryMoveAlong(distance, angle);
}

void mob_try_move_along1(Mob &m, float distance, float angle, bool send) {
	m.TryMoveAlong(distance, angle, send);
}

bool mob_is_invisible(Mob &m) {
	return m.IsInvisible();
}

bool mob_is_invisible1(Mob &m, Mob *other) {
	return m.IsInvisible(other);
}

bool mob_find_type(Mob &m, uint16 type) {
	return m.FindType(type);
}

bool mob_find_type1(Mob &m, uint16 type, bool offensive) {
	return m.FindType(type, offensive);
}

bool mob_find_type2(Mob &m, uint16 type, bool offensive, uint16 threshold) {
	return m.FindType(type, offensive, threshold);
}

float mob_resist_spell(Mob &m, uint8 type, uint16 spell_id, Mob *caster) {
	return m.ResistSpell(type, spell_id, caster);
}

float mob_resist_spell1(Mob &m, uint8 type, uint16 spell_id, Mob *caster, bool use_resist_override) {
	return m.ResistSpell(type, spell_id, caster, use_resist_override);
}

float mob_resist_spell2(Mob &m, uint8 type, uint16 spell_id, Mob *caster, bool use_resist_override, int resist_override) {
	return m.ResistSpell(type, spell_id, caster, use_resist_override, resist_override);
}

float mob_resist_spell3(Mob &m, uint8 type, uint16 spell_id, Mob *caster, bool use_resist_override, int resist_override, bool chr_check) {
	return m.ResistSpell(type, spell_id, caster, use_resist_override, resist_override, chr_check);
}

float mob_resist_spell4(Mob &m, uint8 type, uint16 spell_id, Mob *caster, bool use_resist_override, int resist_override, bool chr_check, bool chrm_tick) {
	return m.ResistSpell(type, spell_id, caster, use_resist_override, resist_override, chr_check, chrm_tick);
}

float mob_resist_spell5(Mob &m, uint8 type, uint16 spell_id, Mob *caster, bool use_resist_override, int resist_override, bool chr_check, bool chrm_tick, bool is_root) {
	return m.ResistSpell(type, spell_id, caster, use_resist_override, resist_override, chr_check, chrm_tick, is_root);
}

float mob_resist_spell6(Mob &m, uint8 type, uint16 spell_id, Mob *caster, bool use_resist_override, int resist_override, bool chr_check, bool chrm_tick, bool is_root, int level_override) {
	return m.ResistSpell(type, spell_id, caster, use_resist_override, resist_override, chr_check, chrm_tick, is_root, level_override);
}

void mob_interrupt_spell(Mob &m) {
	m.InterruptSpell();
}

void mob_interrupt_spell1(Mob &m, uint16 spell_id) {
	m.InterruptSpell(spell_id);
}

float mob_get_waypoint_x(Mob& m) {
	return m.GetCurrentWayPoint().x;
}

float mob_get_waypoint_y(Mob& m) {
	return m.GetCurrentWayPoint().y;
}

float mob_get_waypoint_z(Mob& m) {
	return m.GetCurrentWayPoint().z;
}

float mob_get_waypoint_h(Mob& m) {
	return m.GetCurrentWayPoint().w;
}

void mob_message(Mob &m, int type, const std::string &msg) {
	m.Message(type, "%s", msg.c_str());
}

void mob_say(Mob &m, const std::string &msg) {
	m.Say("%s", msg.c_str());
}

void mob_shout(Mob &m, const std::string &msg) {
	m.Shout("%s", msg.c_str());
}

void mob_emote(Mob &m, const std::string &msg) {
	m.Emote("%s", msg.c_str());
}

bool mob_cast_spell(Mob &m, uint16 spell_id, uint16 target_id) {
	return m.CastSpell(spell_id, target_id);
}

bool mob_cast_spell1(Mob &m, uint16 spell_id, uint16 target_id, int slot) {
	return m.CastSpell(spell_id, target_id, static_cast<EQEmu::CastingSlot>(slot));
}

bool mob_cast_spell2(Mob &m, uint16 spell_id, uint16 target_id, int slot, int32 cast_time) {
	return m.CastSpell(spell_id, target_id, static_cast<EQEmu::CastingSlot>(slot), cast_time);
}

bool mob_cast_spell3(Mob &m, uint16 spell_id, uint16 target_id, int slot, int32 cast_time, int32 mana_cost) {
	return m.CastSpell(spell_id, target_id, static_cast<EQEmu::CastingSlot>(slot), cast_time, mana_cost, nullptr);
}

bool mob_cast_spell4(Mob &m, uint16 spell_id, uint16 target_id, int slot, int32 cast_time, int32 mana_cost, uint32 item_slot) {
	return m.CastSpell(spell_id, target_id, static_cast<EQEmu::CastingSlot>(slot), cast_time, mana_cost, nullptr, item_slot);
}

bool mob_cast_spell5(Mob &m, uint16 spell_id, uint16 target_id, int slot, int32 cast_time, int32 mana_cost, uint32 item_slot, uint32 timer) {
	return m.CastSpell(spell_id, target_id, static_cast<EQEmu::CastingSlot>(slot), cast_time, mana_cost, nullptr, item_slot, timer);
}

bool mob_cast_spell6(Mob &m, uint16 spell_id, uint16 target_id, int slot, int32 cast_time, int32 mana_cost, uint32 item_slot, uint32 timer, uint32 timer_duration) {
	return m.CastSpell(spell_id, target_id, static_cast<EQEmu::CastingSlot>(slot), cast_time, mana_cost, nullptr, item_slot, timer, timer_duration);
}

bool mob_cast_spell7(Mob &m, uint16 spell_id, uint16 target_id, int slot, int32 cast_time, int32 mana_cost, uint32 item_slot, uint32 timer, uint32 timer_duration, int16 resist_adjust) {
	int16 res = resist_adjust;
	return m.CastSpell(spell_id, target_id, static_cast<EQEmu::CastingSlot>(slot), cast_time, mana_cost, nullptr, item_slot, timer, timer_duration, &res);
}

bool mob_cast_spell8(Mob &m, uint16 spell_id, uint16 target_id, int slot, int32 cast_time, int32 mana_cost, uint32 item_slot, uint32 timer, uint32 timer_duration, int16 resist_adjust, uint32 aa_id) {
	int16 res = resist_adjust;
	return m.CastSpell(spell_id, target_id, static_cast<EQEmu::CastingSlot>(slot), cast_time, mana_cost, nullptr, item_slot, timer, timer_duration, &res, aa_id);
}

bool mob_spell_finished(Mob &m, uint16 spell_id, Mob *target) {
	return m.SpellFinished(spell_id, target);
}

bool mob_spell_finished1(Mob &m, uint16 spell_id, Mob *target, int slot) {
	return m.SpellFinished(spell_id, target, static_cast<EQEmu::CastingSlot>(slot));
}

bool mob_spell_finished2(Mob &m, uint16 spell_id, Mob *target, int slot, uint16 mana_used) {
	return m.SpellFinished(spell_id, target, static_cast<EQEmu::CastingSlot>(slot), mana_used);
}

bool mob_spell_finished3(Mob &m, uint16 spell_id, Mob *target, int slot, uint16 mana_used, uint32 inventory_slot) {
	return m.SpellFinished(spell_id, target, static_cast<EQEmu::CastingSlot>(slot), mana_used, inventory_slot);
}

bool mob_spell_finished4(Mob &m, uint16 spell_id, Mob *target, int slot, uint16 mana_used, uint32 inventory_slot, int16 resist_adjust) {
	return m.SpellFinished(spell_id, target, static_cast<EQEmu::CastingSlot>(slot), mana_used, inventory_slot, resist_adjust);
}

bool mob_spell_finished5(Mob &m, uint16 spell_id, Mob *target, int slot, uint16 mana_used, uint32 inventory_slot, int16 resist_adjust, bool isproc) {
	return m.SpellFinished(spell_id, target, static_cast<EQEmu::CastingSlot>(slot), mana_used, inventory_slot, resist_adjust, isproc);
}

bool mob_spell_finished6(Mob &m, uint16 spell_id, Mob *target, int slot, uint16 mana_used, uint32 inventory_slot, int16 resist_adjust, bool isproc, int level_override) {
	return m.SpellFinished(spell_id, target, static_cast<EQEmu::CastingSlot>(slot), mana_used, inventory_slot, resist_adjust, isproc, level_override);
}

void mob_add_to_hate_list(Mob &m, Mob *other) {
	m.AddToHateList(other);
}

void mob_add_to_hate_list1(Mob &m, Mob *other, uint32 hate) {
	m.AddToHateList(other, hate);
}

void mob_add_to_hate_list2(Mob &m, Mob *other, uint32 hate, int32 damage) {
	m.AddToHateList(other, hate, damage);
}

void mob_add_to_hate_list3(Mob &m, Mob *other, uint32 hate, int32 damage, bool yell_for_help) {
	m.AddToHateList(other, hate, damage, yell_for_help);
}

void mob_add_to_hate_list4(Mob &m, Mob *other, uint32 hate, int32 damage, bool yell_for_help, bool frenzy) {
	m.AddToHateList(other, hate, damage, yell_for_help, frenzy);
}

void mob_add_to_hate_list5(Mob &m, Mob *other, uint32 hate, int32 damage, bool yell_for_help, bool frenzy, bool bufftic) {
	m.AddToHateList(other, hate, damage, yell_for_help, frenzy, bufftic);
}

void mob_add_to_hate_list6(Mob &m, Mob *other, uint32 hate, int32 damage, bool yell_for_help, bool frenzy, bool bufftic, uint16 spell_id) {
	m.AddToHateList(other, hate, damage, yell_for_help, frenzy, bufftic, spell_id);
}

void mob_add_to_hate_list7(Mob &m, Mob *other, uint32 hate, int32 damage, bool yell_for_help, bool frenzy, bool bufftic, uint16 spell_id, bool pet_command) {
	m.AddToHateList(other, hate, damage, yell_for_help, frenzy, bufftic, spell_id, pet_command);
}

void mob_set_hate(Mob &m, Mob *other) {
	m.SetHateAmountOnEnt(other);
}

void mob_set_hate1(Mob &m, Mob *other, int hate) {
	m.SetHateAmountOnEnt(other, hate);
}

void mob_set_hate2(Mob &m, Mob *other, int hate, int damage) {
	m.SetHateAmountOnEnt(other, hate, damage);
}

void RegisterMobTypesA(sol::simple_usertype<Mob> &mob) {
	mob.set("GetName", &Mob::GetName);
	mob.set("Depop", sol::overload(
		&mob_depop,
		&mob_depop1));
	mob.set("BehindMob", sol::overload(
		&mob_behind_mob,
		&mob_behind_mob1,
		&mob_behind_mob2,
		&mob_behind_mob3));
	mob.set("SetLevel", sol::overload(
		&mob_set_level,
		&mob_set_level1));
	mob.set("IsMoving", &Mob::IsMoving);
	mob.set("GotoBind", &Mob::GoToBind);
	mob.set("Attack", sol::overload(
		&mob_attack,
		&mob_attack1,
		&mob_attack2,
		&mob_attack3,
		&mob_attack4,
		&mob_attack5));
	mob.set("Damage", sol::overload(
		&mob_damage,
		&mob_damage1,
		&mob_damage2,
		&mob_damage3,
		&mob_damage4));
	mob.set("RangedAttack", &Mob::RangedAttack);
	mob.set("ThrowingAttack", &Mob::ThrowingAttack);
	mob.set("Heal", &Mob::Heal);
	mob.set("HealDamage", sol::overload(
		&mob_heal_damage,
		&mob_heal_damage1,
		&mob_heal_damage2));
	mob.set("GetLevelCon", sol::overload(
		&mob_get_level_con,
		&mob_get_level_con1));
	mob.set("SetHP", &Mob::SetHP);
	mob.set("DoAnim", sol::overload(
		&mob_do_anim,
		&mob_do_anim1,
		&mob_do_anim2,
		&mob_do_anim3));
	mob.set("ChangeSize", sol::overload(
		&mob_change_size,
		&mob_change_size1));
	mob.set("GMMove", sol::overload(
		&mob_gm_move,
		&mob_gm_move1,
		&mob_gm_move2));
	mob.set("GMMove", sol::overload(
		&mob_try_move_along,
		&mob_try_move_along1));
	mob.set("HasProcs", &Mob::HasProcs);
	mob.set("IsInvisible", sol::overload(
		&mob_is_invisible,
		&mob_is_invisible1));
	mob.set("SetInvisible", &Mob::SetInvisible);
	mob.set("FindBuff", &Mob::FindBuff);
	mob.set("FindType", sol::overload(
		&mob_find_type,
		&mob_find_type1,
		&mob_find_type2));
	mob.set("GetBuffSlotFromType", &Mob::GetBuffSlotFromType);
	mob.set("GetBaseRace", &Mob::GetBaseRace);
	mob.set("GetBaseGender", &Mob::GetBaseGender);
	mob.set("GetDeity", &Mob::GetDeity);
	mob.set("GetRace", &Mob::GetRace);
	mob.set("GetGender", &Mob::GetGender);
	mob.set("GetTexture", &Mob::GetTexture);
	mob.set("GetHelmTexture", &Mob::GetHelmTexture);
	mob.set("GetHairColor", &Mob::GetHairColor);
	mob.set("GetBeardColor", &Mob::GetBeardColor);
	mob.set("GetEyeColor1", &Mob::GetEyeColor1);
	mob.set("GetEyeColor2", &Mob::GetEyeColor2);
	mob.set("GetHairStyle", &Mob::GetHairStyle);
	mob.set("GetLuclinFace", &Mob::GetLuclinFace);
	mob.set("GetBeard", &Mob::GetBeard);
	mob.set("GetDrakkinHeritage", &Mob::GetDrakkinHeritage);
	mob.set("GetDrakkinTattoo", &Mob::GetDrakkinTattoo);
	mob.set("GetDrakkinDetails", &Mob::GetDrakkinDetails);
	mob.set("GetClass", &Mob::GetClass);
	mob.set("GetLevel", &Mob::GetLevel);
	mob.set("GetCleanName", &Mob::GetCleanName);
	mob.set("GetTarget", &Mob::GetTarget);
	mob.set("SetTarget", &Mob::SetTarget);
	mob.set("GetHPRatio", &Mob::GetHPRatio);
	mob.set("IsWarriorClass", &Mob::IsWarriorClass);
	mob.set("GetHP", &Mob::GetHP);
	mob.set("GetMaxHP", &Mob::GetMaxHP);
	mob.set("GetItemHPBonuses", &Mob::GetItemHPBonuses);
	mob.set("GetSpellHPBonuses", &Mob::GetSpellHPBonuses);
	mob.set("GetWalkspeed", &Mob::GetWalkspeed);
	mob.set("GetRunspeed", &Mob::GetRunspeed);
	mob.set("GetCasterLevel", &Mob::GetCasterLevel);
	mob.set("GetMaxMana", &Mob::GetMaxMana);
	mob.set("GetMana", &Mob::GetMana);
	mob.set("SetMana", &Mob::SetMana);
	mob.set("GetManaRatio", &Mob::GetManaRatio);
	mob.set("GetAC", &Mob::GetAC);
	mob.set("GetATK", &Mob::GetATK);
	mob.set("GetSTR", &Mob::GetSTR);
	mob.set("GetSTA", &Mob::GetSTA);
	mob.set("GetDEX", &Mob::GetDEX);
	mob.set("GetAGI", &Mob::GetAGI);
	mob.set("GetINT", &Mob::GetINT);
	mob.set("GetWIS", &Mob::GetWIS);
	mob.set("GetCHA", &Mob::GetCHA);
	mob.set("GetMR", &Mob::GetMR);
	mob.set("GetFR", &Mob::GetFR);
	mob.set("GetDR", &Mob::GetDR);
	mob.set("GetPR", &Mob::GetPR);
	mob.set("GetCR", &Mob::GetCR);
	mob.set("GetCorruption", &Mob::GetCorrup);
	mob.set("GetPhR", &Mob::GetPhR);
	mob.set("GetMaxSTR", &Mob::GetMaxSTR);
	mob.set("GetMaxSTA", &Mob::GetMaxSTA);
	mob.set("GetMaxDEX", &Mob::GetMaxDEX);
	mob.set("GetMaxAGI", &Mob::GetMaxAGI);
	mob.set("GetMaxINT", &Mob::GetMaxINT);
	mob.set("GetMaxWIS", &Mob::GetMaxWIS);
	mob.set("GetMaxCHA", &Mob::GetMaxCHA);
	mob.set("ResistSpell", sol::overload(
		&mob_resist_spell,
		&mob_resist_spell1,
		&mob_resist_spell2,
		&mob_resist_spell3,
		&mob_resist_spell4,
		&mob_resist_spell5,
		&mob_resist_spell6));
	mob.set("GetSpecializeSkillValue", &Mob::GetSpecializeSkillValue);
	mob.set("GetNPCTypeID", &Mob::GetNPCTypeID);
	mob.set("IsTargeted", (bool(Mob::*)() const)&Mob::IsTargeted);
	mob.set("GetX", &Mob::GetX);
	mob.set("GetY", &Mob::GetY);
	mob.set("GetZ", &Mob::GetZ);
	mob.set("GetHeading", &Mob::GetHeading);
	mob.set("GetWaypointX", &mob_get_waypoint_x);
	mob.set("GetWaypointY", &mob_get_waypoint_y);
	mob.set("GetWaypointZ", &mob_get_waypoint_z);
	mob.set("GetWaypointH", &mob_get_waypoint_h);
	mob.set("GetWaypointPause", &Mob::GetCWPP);
	mob.set("GetWaypointID", &Mob::GetCWP);
	mob.set("SetCurrentWP", &Mob::SetCurrentWP);
	mob.set("GetSize", &Mob::GetSize);
	mob.set("Message", &mob_message);
	mob.set("Message_StringID", (void(Mob::*)(uint32, uint32, uint32))&Mob::Message_StringID);
	mob.set("Say", &mob_say);
	mob.set("QuestSay", &Mob::QuestJournalledSay);
	mob.set("Shout", &mob_shout);
	mob.set("Emote", &mob_emote);
	mob.set("InterruptSpell", sol::overload(
		&mob_interrupt_spell,
		&mob_interrupt_spell1));
	mob.set("CastSpell", sol::overload(
		&mob_cast_spell,
		&mob_cast_spell1,
		&mob_cast_spell2,
		&mob_cast_spell3,
		&mob_cast_spell4,
		&mob_cast_spell5,
		&mob_cast_spell6,
		&mob_cast_spell7,
		&mob_cast_spell8));
	mob.set("SpellFinished", sol::overload(
		&mob_spell_finished,
		&mob_spell_finished1,
		&mob_spell_finished2,
		&mob_spell_finished3,
		&mob_spell_finished4,
		&mob_spell_finished5,
		&mob_spell_finished6));
	mob.set("SendBeginCast", &Mob::SendBeginCast);
	mob.set("SpellEffect", &Mob::SpellEffect);
	mob.set("GetPet", &Mob::GetPet);
	mob.set("GetOwner", &Mob::GetOwner);
	mob.set("GetHateList", &Mob::GetHateList);
	mob.set("GetHateTop", &Mob::GetHateTop);
	mob.set("GetHateDamageTop", &Mob::GetHateDamageTop);
	mob.set("GetHateRandom", &Mob::GetHateRandom);
	mob.set("AddToHateList", sol::overload(
		&mob_add_to_hate_list,
		&mob_add_to_hate_list1,
		&mob_add_to_hate_list2,
		&mob_add_to_hate_list3,
		&mob_add_to_hate_list4,
		&mob_add_to_hate_list5,
		&mob_add_to_hate_list6,
		&mob_add_to_hate_list7));
	mob.set("SetHate", sol::overload(
		&mob_set_hate,
		&mob_set_hate1,
		&mob_set_hate2));
	mob.set("HalveAggro", &Mob::HalveAggro);
	mob.set("DoubleAggro", &Mob::DoubleAggro);
	mob.set("GetHateAmount", &Mob::GetHateAmount);
	mob.set("GetHateAmount", &Mob::GetHateAmount);
	mob.set("GetDamageAmount", &Mob::GetDamageAmount);
	mob.set("WipeHateList", &Mob::WipeHateList);
	mob.set("CheckAggro", &Mob::CheckAggro);
}
