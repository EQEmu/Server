#include <sol.hpp>
#include "../mob.h"

void mob_depop(Mob &m) {
	m.Depop();
}

void mob_depop(Mob &m, bool start_spawn_timer) {
	m.Depop(start_spawn_timer);
}

bool mob_behind_mob(Mob &m) {
	return m.BehindMob();
}

bool mob_behind_mob(Mob &m, Mob *other) {
	return m.BehindMob(other);
}

bool mob_behind_mob(Mob &m, Mob *other, float ourx) {
	return m.BehindMob(other, ourx);
}

bool mob_behind_mob(Mob &m, Mob *other, float ourx, float oury) {
	return m.BehindMob(other, ourx, oury);
}

void mob_set_level(Mob &m, uint8 level) {
	m.SetLevel(level);
}

void mob_set_level(Mob &m, uint8 level, bool command) {
	m.SetLevel(level, command);
}

bool mob_attack(Mob &m, Mob *other) {
	return m.Attack(other);
}

bool mob_attack(Mob &m, Mob *other, int hand) {
	return m.Attack(other, hand);
}

bool mob_attack(Mob &m, Mob *other, int hand, bool from_riposte) {
	return m.Attack(other, hand, from_riposte);
}

bool mob_attack(Mob &m, Mob *other, int hand, bool from_riposte, bool is_strikethrough) {
	return m.Attack(other, hand, from_riposte, is_strikethrough);
}

bool mob_attack(Mob &m, Mob *other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell) {
	return m.Attack(other, hand, from_riposte, is_strikethrough, is_from_spell);
}

bool mob_attack(Mob &m, Mob *other, int hand, bool from_riposte, bool is_strikethrough, bool is_from_spell, ExtraAttackOptions *opts) {
	return m.Attack(other, hand, from_riposte, is_strikethrough, is_from_spell, opts);
}

void mob_damage(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill));
}

void mob_damage(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill, bool avoidable) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable);
}

void mob_damage(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill, bool avoidable, int8 buffslot) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable, buffslot);
}

void mob_damage(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill, bool avoidable, int8 buffslot, bool bufftic) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable, buffslot, bufftic);
}

void mob_damage(Mob &m, Mob *from, int32 damage, uint16 spell_id, int attack_skill, bool avoidable, int8 buffslot, bool bufftic, int special) {
	m.Damage(from, damage, spell_id, static_cast<EQEmu::skills::SkillType>(attack_skill), avoidable, buffslot, bufftic, static_cast<eSpecialAttacks>(special));
}

void mob_heal_damage(Mob &m, uint32 amount) {
	m.HealDamage(amount);
}

void mob_heal_damage(Mob &m, uint32 amount, Mob *caster) {
	m.HealDamage(amount, caster);
}

void mob_heal_damage(Mob &m, uint32 amount, Mob *caster, uint16 spell_id) {
	m.HealDamage(amount, caster, spell_id);
}

uint32 mob_get_level_con(Mob &m, uint8 other) {
	return m.GetLevelCon(other);
}

uint32 mob_get_level_con(Mob &m, uint8 mylevel, uint8 other) {
	return m.GetLevelCon(mylevel, other);
}

void mob_do_anim(Mob &m, int animnum) {
	m.DoAnim(animnum);
}

void mob_do_anim(Mob &m, int animnum, int type) {
	m.DoAnim(animnum, type);
}

void mob_do_anim(Mob &m, int animnum, int type, bool ackreq) {
	m.DoAnim(animnum, type, ackreq);
}

void mob_do_anim(Mob &m, int animnum, int type, bool ackreq, int filter) {
	m.DoAnim(animnum, type, ackreq, static_cast<eqFilterType>(filter));
}

void mob_change_size(Mob &m, float sz) {
	m.ChangeSize(sz);
}

void mob_change_size(Mob &m, float sz, bool no_restriction) {
	m.ChangeSize(sz, no_restriction);
}

void mob_gm_move(Mob &m, float x, float y, float z) {
	m.GMMove(x, y, z);
}

void mob_gm_move(Mob &m, float x, float y, float z, float heading) {
	m.GMMove(x, y, z, heading);
}

void mob_gm_move(Mob &m, float x, float y, float z, float heading, bool send_update) {
	m.GMMove(x, y, z, heading, send_update);
}

void mob_try_move_along(Mob &m, float distance, float angle) {
	m.TryMoveAlong(distance, angle);
}

void mob_try_move_along(Mob &m, float distance, float angle, bool send) {
	m.TryMoveAlong(distance, angle, send);
}

void lua_register_mob(sol::state *state) {
	state->new_usertype<Mob>("Mob",
		sol::base_classes, sol::bases<Entity>(),
		"GetName", &Mob::GetName,
		"Depop", sol::overload(
			(void(*)(Mob&))&mob_depop, 
			(void(*)(Mob&, bool))&mob_depop),
		"BehindMob", sol::overload(
			(bool(*)(Mob&))&mob_behind_mob, 
			(bool(*)(Mob&,Mob*))&mob_behind_mob, 
			(bool(*)(Mob&, Mob*, float))&mob_behind_mob, 
			(bool(*)(Mob&, Mob*, float, float))&mob_behind_mob),
		"SetLevel", sol::overload(
			(void(*)(Mob&, uint8))&mob_set_level,
			(void(*)(Mob&, uint8, bool))&mob_set_level),
		"IsMoving", &Mob::IsMoving,
		"GoToBind", &Mob::GoToBind,
		"Attack", sol::overload(
			(bool(*)(Mob&, Mob*))&mob_attack,
			(bool(*)(Mob&, Mob*, int))&mob_attack,
			(bool(*)(Mob&, Mob*, int, bool))&mob_attack,
			(bool(*)(Mob&, Mob*, int, bool, bool))&mob_attack,
			(bool(*)(Mob&, Mob*, int, bool, bool, bool))&mob_attack,
			(bool(*)(Mob&, Mob*, int, bool, bool, bool, ExtraAttackOptions*))&mob_attack),
		"Damage", sol::overload(
			(void(*)(Mob&, Mob*, int32, uint16, int))&mob_damage,
			(void(*)(Mob&, Mob*, int32, uint16, int, bool))&mob_damage,
			(void(*)(Mob&, Mob*, int32, uint16, int, bool, int8))&mob_damage,
			(void(*)(Mob&, Mob*, int32, uint16, int, bool, int8, bool))&mob_damage,
			(void(*)(Mob&, Mob*, int32, uint16, int, bool, int8, bool, int))&mob_damage),
		"RangedAttack", &Mob::RangedAttack,
		"ThrowingAttack", &Mob::ThrowingAttack,
		"Heal", &Mob::Heal,
		"HealDamage", sol::overload(
			(void(*)(Mob&, uint32))&mob_heal_damage,
			(void(*)(Mob&, uint32, Mob*))&mob_heal_damage,
			(void(*)(Mob&, uint32, Mob*, uint16))&mob_heal_damage),
		"GetLevelCon", sol::overload(
			(uint32(*)(Mob&, uint8))&mob_get_level_con,
			(uint32(*)(Mob&, uint8, uint8))&mob_get_level_con),
		"SetHP", &Mob::SetHP,
		"DoAnim", sol::overload(
			(void(*)(Mob&, int))&mob_do_anim,
			(void(*)(Mob&, int, int))&mob_do_anim,
			(void(*)(Mob&, int, int, bool))&mob_do_anim,
			(void(*)(Mob&, int, int, bool, int))&mob_do_anim),
		"ChangeSize", sol::overload(
			(void(*)(Mob&, float))&mob_change_size,
			(void(*)(Mob&, float, bool))&mob_change_size),
		"GMMove", sol::overload(
			(void(*)(Mob&, float, float, float))&mob_gm_move,
			(void(*)(Mob&, float, float, float, float))&mob_gm_move,
			(void(*)(Mob&, float, float, float, float, bool))&mob_gm_move),
		"GMMove", sol::overload(
			(void(*)(Mob&, float, float))&mob_try_move_along,
			(void(*)(Mob&, float, float, bool))&mob_try_move_along),
		"HasProcs", &Mob::HasProcs
		/*
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
		.def("SendBeginCast", &Lua_Mob::SendBeginCast)
		.def("SpellEffect", &Lua_Mob::SpellEffect)
		.def("GetPet", &Lua_Mob::GetPet)
		.def("GetOwner", &Lua_Mob::GetOwner)
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
		.def("ChangeRace", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeRace)
		.def("ChangeGender", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeGender)
		.def("ChangeTexture", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeTexture)
		.def("ChangeHelmTexture", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHelmTexture)
		.def("ChangeHairColor", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHairColor)
		.def("ChangeBeardColor", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeBeardColor)
		.def("ChangeEyeColor1", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeEyeColor1)
		.def("ChangeEyeColor2", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeEyeColor2)
		.def("ChangeHairStyle", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHairStyle)
		.def("ChangeLuclinFace", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeLuclinFace)
		.def("ChangeBeard", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeBeard)
		.def("ChangeDrakkinHeritage", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinHeritage)
		.def("ChangeDrakkinTattoo", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinTattoo)
		.def("ChangeDrakkinDetails", (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinDetails)
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
		.def("AddNimbusEffect", (void(Lua_Mob::*)(int))&Lua_Mob::AddNimbusEffect)
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
		.def("GetFcDamageAmtIncoming", &Lua_Mob::GetFcDamageAmtIncoming)
		.def("GetSkillDmgAmt", (int(Lua_Mob::*)(int))&Lua_Mob::GetSkillDmgAmt)
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
		.def("GetAppearance", (uint32(Lua_Mob::*)(void))&Lua_Mob::GetAppearance)
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
		.def("GetMeleeMitigation", (int32(Lua_Mob::*)(void))&Lua_Mob::GetMeleeMitigation)
		.def("GetWeaponDamageBonus", &Lua_Mob::GetWeaponDamageBonus)
		.def("GetItemBonuses", &Lua_Mob::GetItemBonuses)
		.def("GetSpellBonuses", &Lua_Mob::GetSpellBonuses)
		.def("GetAABonuses", &Lua_Mob::GetAABonuses)
		.def("GetMeleeDamageMod_SE", &Lua_Mob::GetMeleeDamageMod_SE)
		.def("GetMeleeMinDamageMod_SE", &Lua_Mob::GetMeleeMinDamageMod_SE)
		.def("IsAttackAllowed", &Lua_Mob::IsAttackAllowed)
		.def("IsCasting", &Lua_Mob::IsCasting)
		.def("AttackAnimation", &Lua_Mob::AttackAnimation)
		.def("GetWeaponDamage", &Lua_Mob::GetWeaponDamage)
		.def("IsBerserk", &Lua_Mob::IsBerserk)
		.def("TryFinishingBlow", &Lua_Mob::TryFinishingBlow)
		.def("GetBodyType", &Lua_Mob::GetBodyType)
		.def("GetOrigBodyType", &Lua_Mob::GetOrigBodyType)
		.def("CheckNumHitsRemaining", &Lua_Mob::CheckNumHitsRemaining);
		*/
	);
}
