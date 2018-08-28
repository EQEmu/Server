#include <sol.hpp>
#include "../mob.h"

bool mob_calculate_new_position(Mob &m, float x, float y, float z, float speed) {
	return m.CalculateNewPosition(x, y, z, speed);
}

bool mob_calculate_new_position1(Mob &m, float x, float y, float z, float speed, bool check_z) {
	return m.CalculateNewPosition(x, y, z, speed, check_z);
}

bool mob_calculate_new_position2(Mob &m, float x, float y, float z, float speed, bool check_z, bool calculate_heading) {
	return m.CalculateNewPosition(x, y, z, speed, check_z, calculate_heading);
}

void mob_npc_special_attacks(Mob &m, const char *parse, int permtag) {
	m.NPCSpecialAttacks(parse, permtag);
}

void mob_npc_special_attacks1(Mob &m, const char *parse, int permtag, bool reset) {
	m.NPCSpecialAttacks(parse, permtag, reset);
}

void mob_npc_special_attacks2(Mob &m, const char *parse, int permtag, bool reset, bool remove) {
	m.NPCSpecialAttacks(parse, permtag, reset, remove);
}

int32 mob_check_aggro_amount(Mob &m, uint16 spell_id, Mob *target) {
	return m.CheckAggroAmount(spell_id, target);
}

int32 mob_check_aggro_amount1(Mob &m, uint16 spell_id, Mob *target, bool isproc) {
	return m.CheckAggroAmount(spell_id, target, isproc);
}

int32 mob_check_heal_aggro_amount(Mob &m, uint16 spell_id, Mob *target) {
	return m.CheckHealAggroAmount(spell_id, target);
}

int32 mob_check_heal_aggro_amount1(Mob &m, uint16 spell_id, Mob *target, uint32 heal_possible) {
	return m.CheckHealAggroAmount(spell_id, target, heal_possible);
}

void mob_set_aa(Mob &m, uint32 rank_id, uint32 new_value) {
	m.SetAA(rank_id, new_value);
}

void mob_set_aa1(Mob &m, uint32 rank_id, uint32 new_value, uint32 charges) {
	m.SetAA(rank_id, new_value, charges);
}

void mob_do_special_attack_damage(Mob &m, Mob *who, int skill, int base_damage) {
	m.DoSpecialAttackDamage(who, static_cast<EQEmu::skills::SkillType>(skill), base_damage);
}

void mob_do_special_attack_damage1(Mob &m, Mob *who, int skill, int base_damage, int min_damage) {
	m.DoSpecialAttackDamage(who, static_cast<EQEmu::skills::SkillType>(skill), base_damage, min_damage);
}

void mob_do_special_attack_damage2(Mob &m, Mob *who, int skill, int base_damage, int min_damage, int32 hate_override) {
	m.DoSpecialAttackDamage(who, static_cast<EQEmu::skills::SkillType>(skill), base_damage, min_damage, hate_override);
}

void mob_do_special_attack_damage3(Mob &m, Mob *who, int skill, int base_damage, int min_damage, int32 hate_override, int reuse_time) {
	m.DoSpecialAttackDamage(who, static_cast<EQEmu::skills::SkillType>(skill), base_damage, min_damage, hate_override, reuse_time);
}

void mob_do_throwing_attack_damage(Mob &m, Mob *other) {
	m.DoThrowingAttackDmg(other);
}

void mob_do_throwing_attack_damage1(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon) {
	m.DoThrowingAttackDmg(other, weapon);
}

void mob_do_throwing_attack_damage2(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemData *ammo) {
	m.DoThrowingAttackDmg(other, weapon, ammo);
}

void mob_do_throwing_attack_damage3(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemData *ammo,
	uint16 weapon_damage) {
	m.DoThrowingAttackDmg(other, weapon, ammo, weapon_damage);
}

void mob_do_throwing_attack_damage4(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemData *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus) {
	m.DoThrowingAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus);
}

void mob_do_throwing_attack_damage5(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemData *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time) {
	m.DoThrowingAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time);
}

void mob_do_throwing_attack_damage6(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemData *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time, uint32 range_id) {
	m.DoThrowingAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time, range_id);
}

void mob_do_throwing_attack_damage7(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemData *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time, uint32 range_id, int ammo_slot) {
	m.DoThrowingAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time, range_id, ammo_slot);
}

void mob_do_throwing_attack_damage8(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemData *ammo, 
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time, uint32 range_id, int ammo_slot, float speed) {
	m.DoThrowingAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time, range_id, ammo_slot, speed);
}

void mob_do_melee_skill_attack_damage(Mob &m, Mob *other, uint16 weapon_damage, int skillinuse) {
	m.DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skillinuse));
}

void mob_do_melee_skill_attack_damage1(Mob &m, Mob *other, uint16 weapon_damage, int skillinuse, int16 chance_mod) {
	m.DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skillinuse), chance_mod);
}

void mob_do_melee_skill_attack_damage2(Mob &m, Mob *other, uint16 weapon_damage, int skillinuse, int16 chance_mod, int16 focus) {
	m.DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skillinuse), chance_mod, focus);
}

void mob_do_melee_skill_attack_damage3(Mob &m, Mob *other, uint16 weapon_damage, int skillinuse, int16 chance_mod, int16 focus, bool can_riposte) {
	m.DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skillinuse), chance_mod, focus, can_riposte);
}

void mob_do_melee_skill_attack_damage4(Mob &m, Mob *other, uint16 weapon_damage, int skillinuse, int16 chance_mod, int16 focus, bool can_riposte, int reuse_time) {
	m.DoMeleeSkillAttackDmg(other, weapon_damage, static_cast<EQEmu::skills::SkillType>(skillinuse), chance_mod, focus, can_riposte, reuse_time);
}

void mob_do_archery_attack_damage(Mob &m, Mob *other) {
	m.DoArcheryAttackDmg(other);
}

void mob_do_archery_attack_damage1(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon) {
	m.DoArcheryAttackDmg(other, weapon);
}

void mob_do_archery_attack_damage2(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo) {
	m.DoArcheryAttackDmg(other, weapon, ammo);
}

void mob_do_archery_attack_damage3(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage);
}

void mob_do_archery_attack_damage4(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage, int16 chance_mod) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage, chance_mod);
}

void mob_do_archery_attack_damage5(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus);
}

void mob_do_archery_attack_damage6(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time);
}

void mob_do_archery_attack_damage7(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time, uint32 range_id) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time, range_id);
}

void mob_do_archery_attack_damage8(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time, uint32 range_id, int ammo_id) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time, range_id, ammo_id);
}

void mob_do_archery_attack_damage9(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time, uint32 range_id, int ammo_id, const EQEmu::ItemData *ammo_data) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time, range_id, ammo_id, ammo_data);
}

void mob_do_archery_attack_damage10(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time, uint32 range_id, int ammo_id, const EQEmu::ItemData *ammo_data, int ammo_slot) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time, range_id, ammo_id, ammo_data, ammo_slot);
}

void mob_do_archery_attack_damage11(Mob &m, Mob *other, const EQEmu::ItemInstance *weapon, const EQEmu::ItemInstance *ammo,
	uint16 weapon_damage, int16 chance_mod, int16 focus, int reuse_time, uint32 range_id, int ammo_id, const EQEmu::ItemData *ammo_data, int ammo_slot, float speed) {
	m.DoArcheryAttackDmg(other, weapon, ammo, weapon_damage, chance_mod, focus, reuse_time, range_id, ammo_slot, ammo_data, ammo_slot, speed);
}

bool mob_check_los(Mob &m, Mob *other) {
	return m.CheckLosFN(other);
}

bool mob_check_los_to_loc(Mob &m, float x, float y, float z) {
	return m.CheckLosFN(x, y, z, 6.0f);
}

bool mob_check_los_to_loc1(Mob &m, float x, float y, float z, float mob_size) {
	return m.CheckLosFN(x, y, z, mob_size);
}

float mob_find_ground_z(Mob &m, float x, float y) {
	return m.GetGroundZ(x, y);
}

float mob_find_ground_z1(Mob &m, float x, float y, float offset) {
	return m.GetGroundZ(x, y, offset);
}

void mob_projectile_animation(Mob &m, Mob *to, int item_id) {
	m.ProjectileAnimation(to, item_id);
}

void mob_projectile_animation1(Mob &m, Mob *to, int item_id, bool is_arrow) {
	m.ProjectileAnimation(to, item_id, is_arrow);
}

void mob_projectile_animation2(Mob &m, Mob *to, int item_id, bool is_arrow, float speed) {
	m.ProjectileAnimation(to, item_id, is_arrow, speed);
}

void mob_projectile_animation3(Mob &m, Mob *to, int item_id, bool is_arrow, float speed, float angle) {
	m.ProjectileAnimation(to, item_id, is_arrow, speed, angle);
}

void mob_projectile_animation4(Mob &m, Mob *to, int item_id, bool is_arrow, float speed, float angle, float tilt) {
	m.ProjectileAnimation(to, item_id, is_arrow, speed, angle, tilt);
}

void mob_projectile_animation5(Mob &m, Mob *to, int item_id, bool is_arrow, float speed, float angle, float tilt, float arc) {
	m.ProjectileAnimation(to, item_id, is_arrow, speed, angle, tilt, arc);
}

void mob_projectile_animation6(Mob &m, Mob *to, int item_id, bool is_arrow, float speed, float angle, float tilt, float arc, const std::string &id_file) {
	m.ProjectileAnimation(to, item_id, is_arrow, speed, angle, tilt, arc, id_file.c_str());
}

void mob_projectile_animation7(Mob &m, Mob *to, int item_id, bool is_arrow, float speed, float angle, float tilt, float arc, const std::string &id_file, int skillinuse) {
	m.ProjectileAnimation(to, item_id, is_arrow, speed, angle, tilt, arc, id_file.c_str(), static_cast<EQEmu::skills::SkillType>(skillinuse));
}

void mob_send_appearance_effect(Mob &m, uint32 param1, uint32 param2, uint32 param3, uint32 param4, uint32 param5) {
	m.SendAppearanceEffect(param1, param2, param2, param4, param5);
}

void mob_send_appearance_effect1(Mob &m, uint32 param1, uint32 param2, uint32 param3, uint32 param4, uint32 param5, Client *target) {
	m.SendAppearanceEffect(param1, param2, param2, param4, param5, target);
}

void mob_set_texture(Mob &m, int in) {
	m.SendIllusionPacket(m.GetRace(), 0xFF, in);
}

void mob_set_race(Mob &m, int in) {
	m.SendIllusionPacket(in);
}

void mob_set_gender(Mob &m, int in) {
	m.SendIllusionPacket(m.GetRace(), in);
}

void RegisterMobTypesB(sol::simple_usertype<Mob> &mob) {
	mob.set("Stun", &Mob::Stun);
	mob.set("UnStun", &Mob::UnStun);
	mob.set("IsStunned", &Mob::IsStunned);
	mob.set("Spin", &Mob::Spin);
	mob.set("Kill", &Mob::Kill);
	mob.set("CanThisClassDoubleAttack", &Mob::CanThisClassDoubleAttack);
	mob.set("CanThisClassDualWield", &Mob::CanThisClassDualWield);
	mob.set("CanThisClassRiposte", &Mob::CanThisClassRiposte);
	mob.set("CanThisClassDodge", &Mob::CanThisClassDodge);
	mob.set("CanThisClassParry", &Mob::CanThisClassParry);
	mob.set("CanThisClassBlock", &Mob::CanThisClassBlock);
	mob.set("SetInvul", &Mob::SetInvul);
	mob.set("GetInvul", &Mob::GetInvul);
	mob.set("SetExtraHaste", &Mob::SetExtraHaste);
	mob.set("GetHaste", &Mob::GetHaste);
	mob.set("GetHandToHandDamage", &Mob::GetHandToHandDamage);
	mob.set("GetHandToHandDelay", &Mob::GetHandToHandDelay);
	mob.set("Mesmerize", &Mob::Mesmerize);
	mob.set("IsMezzed", &Mob::IsMezzed);
	mob.set("IsEnraged", &Mob::IsEnraged);
	mob.set("GetReverseFactionCon", &Mob::GetReverseFactionCon);
	mob.set("IsAIControlled", &Mob::IsAIControlled);
	mob.set("GetAggroRange", &Mob::GetAggroRange);
	mob.set("GetAssistRange", &Mob::GetAssistRange);
	mob.set("SetPetOrder", &Mob::SetPetOrder);
	mob.set("GetPetOrder", &Mob::GetPetOrder);
	mob.set("IsRoamer", &Mob::IsRoamer);
	mob.set("IsRooted", &Mob::IsRooted);
	mob.set("IsEngaged", &Mob::IsEngaged);
	mob.set("FaceTarget", &Mob::FaceTarget);
	mob.set("SetHeading", &Mob::SetHeading);
	mob.set("CalculateHeadingToTarget", &Mob::CalculateHeadingToTarget);
	mob.set("CalculateNewPosition", sol::overload(
		&mob_calculate_new_position,
		&mob_calculate_new_position1,
		&mob_calculate_new_position2));
	mob.set("CalculateDistance", &Mob::CalculateDistance);
	mob.set("SendTo", &Mob::SendTo);
	mob.set("SendToFixZ", &Mob::SendToFixZ);
	mob.set("NPCSpecialAttacks", sol::overload(
		&mob_npc_special_attacks,
		&mob_npc_special_attacks1,
		&mob_npc_special_attacks2));
	mob.set("GetResist", &Mob::GetResist);
	mob.set("Charmed", &Mob::Charmed);
	mob.set("CheckAggroAmount", sol::overload(
		&mob_check_aggro_amount,
		&mob_check_aggro_amount1));
	mob.set("CheckHealAggroAmount", sol::overload(
		&mob_check_heal_aggro_amount,
		&mob_check_heal_aggro_amount1));
	mob.set("GetAA", &Mob::GetAA);
	mob.set("GetAA", &Mob::GetAA);
	mob.set("SetAA", sol::overload(
		&mob_set_aa,
		&mob_set_aa1));
	mob.set("DivineAura", &Mob::DivineAura);
	mob.set("SetOOCRegen", &Mob::SetOOCRegen);
	mob.set("GetEntityVariable", &Mob::GetEntityVariable);
	mob.set("SetEntityVariable", &Mob::SetEntityVariable);
	mob.set("EntityVariableExists", &Mob::EntityVariableExists);
	mob.set("Signal", &Mob::Signal);
	mob.set("CombatRange", &Mob::CombatRange);
	mob.set("DoSpecialAttackDamage", sol::overload(
		&mob_do_special_attack_damage,
		&mob_do_special_attack_damage1,
		&mob_do_special_attack_damage2,
		&mob_do_special_attack_damage3));
	mob.set("DoThrowingAttackDmg", sol::overload(
		&mob_do_throwing_attack_damage,
		&mob_do_throwing_attack_damage1,
		&mob_do_throwing_attack_damage2,
		&mob_do_throwing_attack_damage3,
		&mob_do_throwing_attack_damage4,
		&mob_do_throwing_attack_damage5,
		&mob_do_throwing_attack_damage6,
		&mob_do_throwing_attack_damage7,
		&mob_do_throwing_attack_damage8));
	mob.set("DoMeleeSkillAttackDmg", sol::overload(
		&mob_do_melee_skill_attack_damage,
		&mob_do_melee_skill_attack_damage1,
		&mob_do_melee_skill_attack_damage2,
		&mob_do_melee_skill_attack_damage3,
		&mob_do_melee_skill_attack_damage4));
	mob.set("DoArcheryAttackDmg", sol::overload(
		&mob_do_archery_attack_damage,
		&mob_do_archery_attack_damage1,
		&mob_do_archery_attack_damage2,
		&mob_do_archery_attack_damage3,
		&mob_do_archery_attack_damage4,
		&mob_do_archery_attack_damage5,
		&mob_do_archery_attack_damage6,
		&mob_do_archery_attack_damage7,
		&mob_do_archery_attack_damage8,
		&mob_do_archery_attack_damage9,
		&mob_do_archery_attack_damage10,
		&mob_do_archery_attack_damage11));
	mob.set("CheckLoS", &mob_check_los);
	mob.set("CheckLoSToLoc", sol::overload(
		&mob_check_los_to_loc,
		&mob_check_los_to_loc1));
	mob.set("FindGroundZ", sol::overload(
		&mob_find_ground_z,
		&mob_find_ground_z1));
	mob.set("ProjectileAnimation", sol::overload(
		&mob_projectile_animation,
		&mob_projectile_animation1,
		&mob_projectile_animation2,
		&mob_projectile_animation3,
		&mob_projectile_animation4,
		&mob_projectile_animation5,
		&mob_projectile_animation6,
		&mob_projectile_animation7));
	mob.set("HasNPCSpecialAtk", &Mob::HasNPCSpecialAtk);
	mob.set("SendAppearanceEffect", sol::overload(
		&mob_send_appearance_effect,
		&mob_send_appearance_effect1));
	mob.set("SetFlyMode", &Mob::SetFlyMode);
	mob.set("SetTexture", &mob_set_texture);
	mob.set("SetRace", &mob_set_race);
	mob.set("SetGender", &mob_set_gender);
	mob.set("SendIllusionPacket", &Mob::SendIllusionPacket);
	mob.set("ChangeRace", &Mob::ChangeRace);
	mob.set("ChangeGender", &Mob::ChangeGender);
	mob.set("ChangeTexture", &Mob::ChangeTexture);
	mob.set("ChangeHelmTexture", &Mob::ChangeHelmTexture);
	mob.set("ChangeHairColor", &Mob::ChangeHairColor);
	mob.set("ChangeBeardColor", &Mob::ChangeBeardColor);
	mob.set("ChangeEyeColor1", &Mob::ChangeEyeColor1);
	mob.set("ChangeEyeColor2", &Mob::ChangeEyeColor2);
	mob.set("ChangeHairStyle", &Mob::ChangeHairStyle);
	mob.set("ChangeLuclinFace", &Mob::ChangeLuclinFace);
	mob.set("ChangeBeard", &Mob::ChangeBeard);
	mob.set("ChangeDrakkinHeritage", &Mob::ChangeDrakkinHeritage);
	mob.set("ChangeDrakkinTattoo", &Mob::ChangeDrakkinTattoo);
	mob.set("ChangeDrakkinDetails", &Mob::ChangeDrakkinDetails);
}
