#include <sol.hpp>
#include "../mob.h"

void mob_camera_effect(Mob &m, uint32 duration, uint32 intensity) {
	m.CameraEffect(duration, intensity);
}

void mob_camera_effect1(Mob &m, uint32 duration, uint32 intensity, Client *c) {
	m.CameraEffect(duration, intensity, c);
}

void mob_camera_effect2(Mob &m, uint32 duration, uint32 intensity, Client *c, bool global) {
	m.CameraEffect(duration, intensity, c, global);
}

void mob_send_spell_effect(Mob &m, uint32 effectid, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020) {
	m.SendSpellEffect(effectid, duration, finish_delay, zone_wide, unk020);
}

void mob_send_spell_effect1(Mob &m, uint32 effectid, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect) {
	m.SendSpellEffect(effectid, duration, finish_delay, zone_wide, unk020, perm_effect);
}

void mob_send_spell_effect2(Mob &m, uint32 effectid, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect, Client *c) {
	m.SendSpellEffect(effectid, duration, finish_delay, zone_wide, unk020, perm_effect, c);
}

void mob_temp_name(Mob &m) {
	m.TempName();
}

void mob_temp_name1(Mob &m, const std::string &name) {
	m.TempName(name.c_str());
}

void mob_set_appearance(Mob &m, int app) {
	m.SetAppearance(static_cast<EmuAppearance>(app));
}

void mob_set_appearance1(Mob &m, int app, bool ignore_self) {
	m.SetAppearance(static_cast<EmuAppearance>(app), ignore_self);
}

void mob_buff_fade_by_effect(Mob &m, int effect_id) {
	m.BuffFadeByEffect(effect_id);
}

void mob_buff_fade_by_effect1(Mob &m, int effect_id, int skipslot) {
	m.BuffFadeByEffect(effect_id, skipslot);
}

void mob_buff_fade_by_slot(Mob &m, int slot) {
	m.BuffFadeBySlot(slot);
}

void mob_buff_fade_by_slot1(Mob &m, int slot, bool recalc_bonuses) {
	m.BuffFadeBySlot(slot, recalc_bonuses);
}

int mob_can_buff_stack(Mob &m, uint16 spell_id, uint8 caster_level) {
	return m.CanBuffStack(spell_id, caster_level);
}

int mob_can_buff_stack1(Mob &m, uint16 spell_id, uint8 caster_level, bool fail_if_overwrite) {
	return m.CanBuffStack(spell_id, caster_level, fail_if_overwrite);
}

int mob_get_weapon_damage(Mob &m, Mob *against, EQEmu::ItemInstance *weapon) {
	return m.GetWeaponDamage(against, weapon);
}

std::tuple<bool, int> mob_try_finishing_blow(Mob &m, Mob *defender, int damage) {
	auto r = m.TryFinishingBlow(defender, damage);

	return std::make_tuple(r, damage);
}

void RegisterMobTypesC(sol::simple_usertype<Mob> &mob) {
	mob.set("CameraEffect", sol::overload(
		&mob_camera_effect,
		&mob_camera_effect1,
		&mob_camera_effect2));
	mob.set("SendSpellEffect", sol::overload(
		&mob_send_spell_effect,
		&mob_send_spell_effect1,
		&mob_send_spell_effect2));
	mob.set("TempName", sol::overload(
		&mob_temp_name,
		&mob_temp_name1));
	mob.set("GetGlobal", &Mob::GetGlobal);
	mob.set("SetGlobal", &Mob::SetGlobal);
	mob.set("SetGlobal", &Mob::SetGlobal);
	mob.set("TarGlobal", &Mob::TarGlobal);
	mob.set("DelGlobal", &Mob::DelGlobal);
	mob.set("SetSlotTint", &Mob::SetSlotTint);
	mob.set("WearChange", &Mob::WearChange);
	mob.set("DoKnockback", &Mob::DoKnockback);
	mob.set("AddNimbusEffect", &Mob::AddNimbusEffect);
	mob.set("RemoveNimbusEffect", &Mob::RemoveNimbusEffect);
	mob.set("IsFeared", &Mob::IsFeared);
	mob.set("IsBlind", &Mob::IsBlind);
	mob.set("IsRunning", &Mob::IsRunning);
	mob.set("SetRunning", &Mob::SetRunning);
	mob.set("SetBodyType", &Mob::SetBodyType);
	mob.set("SetTargetable", &Mob::SetTargetable);
	mob.set("ModSkillDmgTaken", &Mob::ModSkillDmgTaken);
	mob.set("GetModSkillDmgTaken", &Mob::GetModSkillDmgTaken);
	mob.set("GetSkillDmgTaken", &Mob::GetSkillDmgTaken);
	mob.set("GetFcDamageAmtIncoming", &Mob::GetFcDamageAmtIncoming);
	mob.set("GetSkillDmgAmt", &Mob::GetSkillDmgAmt);
	mob.set("SetAllowBeneficial", &Mob::SetAllowBeneficial);
	mob.set("GetAllowBeneficial", &Mob::GetAllowBeneficial);
	mob.set("IsBeneficialAllowed", &Mob::IsBeneficialAllowed);
	mob.set("ModVulnerability", &Mob::ModVulnerability);
	mob.set("GetModVulnerability", &Mob::GetModVulnerability);
	mob.set("SetDisableMelee", &Mob::SetDisableMelee);
	mob.set("IsMeleeDisabled", &Mob::IsMeleeDisabled);
	mob.set("SetFlurryChance", &Mob::SetFlurryChance);
	mob.set("GetFlurryChance", &Mob::GetFlurryChance);
	mob.set("GetSkill", &Mob::GetSkill);
	mob.set("GetSpecialAbility", &Mob::GetSpecialAbility);
	mob.set("GetSpecialAbilityParam", &Mob::GetSpecialAbilityParam);
	mob.set("SetSpecialAbility", &Mob::SetSpecialAbility);
	mob.set("SetSpecialAbilityParam", &Mob::SetSpecialAbilityParam);
	mob.set("ClearSpecialAbilities", &Mob::ClearSpecialAbilities);
	mob.set("ProcessSpecialAbilities", &Mob::ProcessSpecialAbilities);
	mob.set("GetAppearance", &Mob::GetAppearance);
	mob.set("SetAppearance", sol::overload(
		&mob_set_appearance,
		&mob_set_appearance1));
	mob.set("SetDestructibleObject", &Mob::SetDestructibleObject);
	mob.set("IsImmuneToSpell", &Mob::IsImmuneToSpell);
	mob.set("BuffFadeBySpellID", &Mob::BuffFadeBySpellID);
	mob.set("BuffFadeAll", &Mob::BuffFadeAll);
	mob.set("BuffFadeByEffect", sol::overload(
		&mob_buff_fade_by_effect,
		&mob_buff_fade_by_effect1));
	mob.set("BuffFadeBySlot", sol::overload(
		&mob_buff_fade_by_slot,
		&mob_buff_fade_by_slot1));
	mob.set("CanBuffStack", sol::overload(
		&mob_can_buff_stack,
		&mob_can_buff_stack1));
	mob.set("SetPseudoRoot", &Mob::SetPseudoRoot);
	mob.set("SeeInvisible", &Mob::SeeInvisible);
	mob.set("SeeInvisibleUndead", &Mob::SeeInvisibleUndead);
	mob.set("SeeHide", &Mob::SeeHide);
	mob.set("SeeImprovedHide", &Mob::SeeImprovedHide);
	mob.set("GetNimbusEffect1", &Mob::GetNimbusEffect1);
	mob.set("GetNimbusEffect2", &Mob::GetNimbusEffect2);
	mob.set("GetNimbusEffect3", &Mob::GetNimbusEffect3);
	mob.set("IsTargetable", &Mob::IsTargetable);
	mob.set("HasShieldEquiped", &Mob::HasShieldEquiped);
	mob.set("HasTwoHandBluntEquiped", &Mob::HasTwoHandBluntEquiped);
	mob.set("HasTwoHanderEquipped", &Mob::HasTwoHanderEquipped);
	mob.set("GetHerosForgeModel", &Mob::GetHerosForgeModel);
	mob.set("IsEliteMaterialItem", &Mob::IsEliteMaterialItem);
	mob.set("GetBaseSize", &Mob::GetBaseSize);
	mob.set("HasOwner", &Mob::HasOwner);
	mob.set("IsPet", &Mob::IsPet);
	mob.set("HasPet", &Mob::HasPet);
	mob.set("IsSilenced", &Mob::IsSilenced);
	mob.set("IsAmnesiad", &Mob::IsAmnesiad);
	mob.set("GetMeleeMitigation", &Mob::GetMeleeMitigation);
	mob.set("GetWeaponDamageBonus", &Mob::GetWeaponDamageBonus);
	mob.set("GetItemBonuses", &Mob::GetItemBonuses);
	mob.set("GetSpellBonuses", &Mob::GetSpellBonuses);
	mob.set("GetAABonuses", &Mob::GetAABonuses);
	mob.set("GetMeleeDamageMod_SE", &Mob::GetMeleeDamageMod_SE);
	mob.set("GetMeleeMinDamageMod_SE", &Mob::GetMeleeMinDamageMod_SE);
	mob.set("IsAttackAllowed", &Mob::IsAttackAllowed);
	mob.set("IsCasting", &Mob::IsCasting);
	mob.set("AttackAnimation", &Mob::AttackAnimation);
	mob.set("GetWeaponDamage", &mob_get_weapon_damage);
	mob.set("IsBerserk", &Mob::IsBerserk);
	mob.set("TryFinishingBlow", &mob_try_finishing_blow);
	mob.set("GetBodyType", &Mob::GetBodyType);
	mob.set("GetOrigBodyType", &Mob::GetOrigBodyType);
	mob.set("CheckNumHitsRemaining", &Mob::CheckNumHitsRemaining);
}
