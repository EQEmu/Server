#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "../common/spdat.h"
#include "lua_spell.h"

Lua_Spell::Lua_Spell(int id) {
	if(IsValidSpell(id)) {
		SetLuaPtrData(&spells[id]);
	} else {
		SetLuaPtrData(nullptr);
	}
}

int Lua_Spell::GetID() {
	Lua_Safe_Call_Int();
	return self->id;
}

const char *Lua_Spell::GetName() {
	Lua_Safe_Call_String();
	return self->name;
}

const char *Lua_Spell::GetPlayer1() {
	Lua_Safe_Call_String();
	return self->player_1;
}

const char *Lua_Spell::GetTeleportZone() {
	Lua_Safe_Call_String();
	return self->teleport_zone;
}

const char *Lua_Spell::GetYouCast() {
	Lua_Safe_Call_String();
	return self->you_cast;
}

const char *Lua_Spell::GetOtherCasts() {
	Lua_Safe_Call_String();
	return self->other_casts;
}

const char *Lua_Spell::GetCastOnYou() {
	Lua_Safe_Call_String();
	return self->cast_on_you;
}

const char *Lua_Spell::GetCastOnOther() {
	Lua_Safe_Call_String();
	return self->cast_on_other;
}

const char *Lua_Spell::GetSpellFades() {
	Lua_Safe_Call_String();
	return self->spell_fades;
}

float Lua_Spell::GetRange() {
	Lua_Safe_Call_Real();
	return self->range;
}

float Lua_Spell::GetAoeRange() {
	Lua_Safe_Call_Real();
	return self->aoerange;
}

float Lua_Spell::GetPushBack() {
	Lua_Safe_Call_Real();
	return self->pushback;
}

float Lua_Spell::GetPushUp() {
	Lua_Safe_Call_Real();
	return self->pushup;
}

uint32 Lua_Spell::GetCastTime() {
	Lua_Safe_Call_Int();
	return self->cast_time;
}

uint32 Lua_Spell::GetRecoveryTime() {
	Lua_Safe_Call_Int();
	return self->recovery_time;
}

uint32 Lua_Spell::GetRecastTime() {
	Lua_Safe_Call_Int();
	return self->recast_time;
}

uint32 Lua_Spell::GetBuffdurationFormula() {
	Lua_Safe_Call_Int();
	return self->buffdurationformula;
}

uint32 Lua_Spell::GetBuffDuration() {
	Lua_Safe_Call_Int();
	return self->buffduration;
}

uint32 Lua_Spell::GetAEDuration() {
	Lua_Safe_Call_Int();
	return self->AEDuration;
}

int Lua_Spell::GetMana() {
	Lua_Safe_Call_Int();
	return self->mana;
}

int Lua_Spell::GetBase(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->base[i];
}

int Lua_Spell::GetBase2(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->base2[i];
}

int Lua_Spell::GetMax(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->max[i];
}

int Lua_Spell::GetComponents(int i) {
	Lua_Safe_Call_Int();

	if(i >= 4 || i < 0) {
		return 0;
	}

	return self->components[i];
}

int Lua_Spell::GetComponentCounts(int i) {
	Lua_Safe_Call_Int();

	if(i >= 4 || i < 0) {
		return 0;
	}

	return self->component_counts[i];
}

int Lua_Spell::GetNoexpendReagent(int i) {
	Lua_Safe_Call_Int();

	if(i >= 4 || i < 0) {
		return 0;
	}

	return self->NoexpendReagent[i];
}

int Lua_Spell::GetFormula(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->formula[i];
}

int Lua_Spell::GetGoodEffect() {
	Lua_Safe_Call_Int();
	return self->goodEffect;
}

int Lua_Spell::GetActivated() {
	Lua_Safe_Call_Int();
	return self->Activated;
}

int Lua_Spell::GetResistType() {
	Lua_Safe_Call_Int();
	return self->resisttype;
}

int Lua_Spell::GetEffectID(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->effectid[i];
}

int Lua_Spell::GetTargetType() {
	Lua_Safe_Call_Int();
	return self->targettype;
}

int Lua_Spell::GetBaseDiff() {
	Lua_Safe_Call_Int();
	return self->basediff;
}

int Lua_Spell::GetSkill() {
	Lua_Safe_Call_Int();
	return self->skill;
}

int Lua_Spell::GetZoneType() {
	Lua_Safe_Call_Int();
	return self->zonetype;
}

int Lua_Spell::GetEnvironmentType() {
	Lua_Safe_Call_Int();
	return self->EnvironmentType;
}

int Lua_Spell::GetTimeOfDay() {
	Lua_Safe_Call_Int();
	return self->TimeOfDay;
}

int Lua_Spell::GetClasses(int i) {
	Lua_Safe_Call_Int();

	if(i >= 16 || i < 0) {
		return 0;
	}

	return self->classes[i];
}

int Lua_Spell::GetCastingAnim() {
	Lua_Safe_Call_Int();
	return self->CastingAnim;
}

int Lua_Spell::GetSpellAffectIndex() {
	Lua_Safe_Call_Int();
	return self->SpellAffectIndex;
}

int Lua_Spell::GetDisallowSit() {
	Lua_Safe_Call_Int();
	return self->disallow_sit;
}

int Lua_Spell::GetDeities(int i) {
	Lua_Safe_Call_Int();

	if(i >= 16 || i < 0) {
		return 0;
	}

	return self->deities[i];
}

int Lua_Spell::GetUninterruptable() {
	Lua_Safe_Call_Int();
	return self->uninterruptable;
}

int Lua_Spell::GetResistDiff() {
	Lua_Safe_Call_Int();
	return self->ResistDiff;
}

int Lua_Spell::GetRecourseLink() {
	Lua_Safe_Call_Int();
	return self->RecourseLink;
}

int Lua_Spell::GetShortBuffBox() {
	Lua_Safe_Call_Int();
	return self->short_buff_box;
}

int Lua_Spell::GetDescNum() {
	Lua_Safe_Call_Int();
	return self->descnum;
}

int Lua_Spell::GetEffectDescNum() {
	Lua_Safe_Call_Int();
	return self->effectdescnum;
}

int Lua_Spell::GetBonusHate() {
	Lua_Safe_Call_Int();
	return self->bonushate;
}

int Lua_Spell::GetEndurCost() {
	Lua_Safe_Call_Int();
	return self->EndurCost;
}

int Lua_Spell::GetEndurTimerIndex() {
	Lua_Safe_Call_Int();
	return self->EndurUpkeep;
}

int Lua_Spell::GetHateAdded() {
	Lua_Safe_Call_Int();
	return self->HateAdded;
}

int Lua_Spell::GetEndurUpkeep() {
	Lua_Safe_Call_Int();
	return self->EndurUpkeep;
}

int Lua_Spell::GetNumHits() {
	Lua_Safe_Call_Int();
	return self->numhits;
}

int Lua_Spell::GetPVPResistBase() {
	Lua_Safe_Call_Int();
	return self->pvpresistbase;
}

int Lua_Spell::GetPVPResistCalc() {
	Lua_Safe_Call_Int();
	return self->pvpresistcalc;
}

int Lua_Spell::GetPVPResistCap() {
	Lua_Safe_Call_Int();
	return self->pvpresistcap;
}

int Lua_Spell::GetSpellCategory() {
	Lua_Safe_Call_Int();
	return self->spell_category;
}

int Lua_Spell::GetPVPDuration() {
	Lua_Safe_Call_Int();
	return self->pvp_duration;
}

int Lua_Spell::GetPVPDurationCap() {
	Lua_Safe_Call_Int();
	return self->pvp_duration_cap;
}

int Lua_Spell::GetCanMGB() {
	Lua_Safe_Call_Int();
	return self->can_mgb;
}

int Lua_Spell::GetDispelFlag() {
	Lua_Safe_Call_Int();
	return self->dispel_flag;
}

int Lua_Spell::GetMinResist() {
	Lua_Safe_Call_Int();
	return self->MinResist;
}

int Lua_Spell::GetMaxResist() {
	Lua_Safe_Call_Int();
	return self->MaxResist;
}

int Lua_Spell::GetViralTargets() {
	Lua_Safe_Call_Int();
	return self->viral_targets;
}

int Lua_Spell::GetViralTimer() {
	Lua_Safe_Call_Int();
	return self->viral_timer;
}

int Lua_Spell::GetNimbusEffect() {
	Lua_Safe_Call_Int();
	return self->NimbusEffect;
}

float Lua_Spell::GetDirectionalStart() {
	Lua_Safe_Call_Real();
	return self->directional_start;
}

float Lua_Spell::GetDirectionalEnd() {
	Lua_Safe_Call_Real();
	return self->directional_end;
}

int Lua_Spell::GetSpellGroup() {
	Lua_Safe_Call_Int();
	return self->spellgroup;
}

int Lua_Spell::GetPowerfulFlag() {
	Lua_Safe_Call_Int();
	return self->no_resist;
}

int Lua_Spell::GetCastRestriction() {
	Lua_Safe_Call_Int();
	return self->CastRestriction;
}

bool Lua_Spell::GetAllowRest() {
	Lua_Safe_Call_Bool();
	return self->AllowRest;
}

bool Lua_Spell::GetInCombat() {
	Lua_Safe_Call_Bool();
	return self->InCombat;
}

bool Lua_Spell::GetOutOfCombat() {
	Lua_Safe_Call_Bool();
	return self->OutofCombat;
}

int Lua_Spell::GetAEMaxTargets() {
	Lua_Safe_Call_Int();
	return self->aemaxtargets;
}

int Lua_Spell::GetMaxTargets() {
	Lua_Safe_Call_Int();
	return self->no_heal_damage_item_mod;
}

bool Lua_Spell::GetPersistDeath() {
	Lua_Safe_Call_Bool();
	return self->persistdeath;
}

float Lua_Spell::GetMinDist() {
	Lua_Safe_Call_Real();
	return self->min_dist;
}

float Lua_Spell::GetMinDistMod() {
	Lua_Safe_Call_Real();
	return self->min_dist_mod;
}

float Lua_Spell::GetMaxDist() {
	Lua_Safe_Call_Real();
	return self->max_dist;
}

float Lua_Spell::GetMaxDistMod() {
	Lua_Safe_Call_Real();
	return self->max_dist_mod;
}

float Lua_Spell::GetMinRange() {
	Lua_Safe_Call_Real();
	return self->min_range;
}

int Lua_Spell::GetDamageShieldType() {
	Lua_Safe_Call_Int();
	return self->DamageShieldType;
}

int Lua_Spell::GetRank() {
	Lua_Safe_Call_Int();
	return self->rank;
}

luabind::scope lua_register_spell() {
	return luabind::class_<Lua_Spell>("Spell")
		.def(luabind::constructor<>())
		.def(luabind::constructor<int>())
		.property("null", &Lua_Spell::Null)
		.property("valid", &Lua_Spell::Valid)
		.def("ID", &Lua_Spell::GetID)
		.def("Name", &Lua_Spell::GetName)
		.def("Player1", &Lua_Spell::GetPlayer1)
		.def("TeleportZone", &Lua_Spell::GetTeleportZone)
		.def("YouCast", &Lua_Spell::GetYouCast)
		.def("OtherCasts", &Lua_Spell::GetOtherCasts)
		.def("CastOnYou", &Lua_Spell::GetCastOnYou)
		.def("CastOnOther", &Lua_Spell::GetCastOnOther)
		.def("SpellFades", &Lua_Spell::GetSpellFades)
		.def("Range", &Lua_Spell::GetRange)
		.def("AoeRange", &Lua_Spell::GetAoeRange)
		.def("PushBack", &Lua_Spell::GetPushBack)
		.def("PushUp", &Lua_Spell::GetPushUp)
		.def("CastTime", &Lua_Spell::GetCastTime)
		.def("RecoveryTime", &Lua_Spell::GetRecoveryTime)
		.def("RecastTime", &Lua_Spell::GetRecastTime)
		.def("BuffdurationFormula", &Lua_Spell::GetBuffdurationFormula)
		.def("BuffDuration", &Lua_Spell::GetBuffDuration)
		.def("AEDuration", &Lua_Spell::GetAEDuration)
		.def("Mana", &Lua_Spell::GetMana)
		.def("Base", &Lua_Spell::GetBase)
		.def("Base2", &Lua_Spell::GetBase2)
		.def("Max", &Lua_Spell::GetMax)
		.def("Components", &Lua_Spell::GetComponents)
		.def("ComponentCounts", &Lua_Spell::GetComponentCounts)
		.def("NoexpendReagent", &Lua_Spell::GetNoexpendReagent)
		.def("Formula", &Lua_Spell::GetFormula)
		.def("GoodEffect", &Lua_Spell::GetGoodEffect)
		.def("Activated", &Lua_Spell::GetActivated)
		.def("ResistType", &Lua_Spell::GetResistType)
		.def("EffectID", &Lua_Spell::GetEffectID)
		.def("TargetType", &Lua_Spell::GetTargetType)
		.def("BaseDiff", &Lua_Spell::GetBaseDiff)
		.def("Skill", &Lua_Spell::GetSkill)
		.def("ZoneType", &Lua_Spell::GetZoneType)
		.def("EnvironmentType", &Lua_Spell::GetEnvironmentType)
		.def("TimeOfDay", &Lua_Spell::GetTimeOfDay)
		.def("Classes", &Lua_Spell::GetClasses)
		.def("CastingAnim", &Lua_Spell::GetCastingAnim)
		.def("SpellAffectIndex", &Lua_Spell::GetSpellAffectIndex)
		.def("DisallowSit", &Lua_Spell::GetDisallowSit)
		.def("Deities", &Lua_Spell::GetDeities)
		.def("Uninterruptable", &Lua_Spell::GetUninterruptable)
		.def("ResistDiff", &Lua_Spell::GetResistDiff)
		.def("RecourseLink", &Lua_Spell::GetRecourseLink)
		.def("ShortBuffBox", &Lua_Spell::GetShortBuffBox)
		.def("DescNum", &Lua_Spell::GetDescNum)
		.def("EffectDescNum", &Lua_Spell::GetEffectDescNum)
		.def("BonusHate", &Lua_Spell::GetBonusHate)
		.def("EndurCost", &Lua_Spell::GetEndurCost)
		.def("EndurTimerIndex", &Lua_Spell::GetEndurTimerIndex)
		.def("HateAdded", &Lua_Spell::GetHateAdded)
		.def("EndurUpkeep", &Lua_Spell::GetEndurUpkeep)
		.def("NumHits", &Lua_Spell::GetNumHits)
		.def("PVPResistBase", &Lua_Spell::GetPVPResistBase)
		.def("PVPResistCalc", &Lua_Spell::GetPVPResistCalc)
		.def("PVPResistCap", &Lua_Spell::GetPVPResistCap)
		.def("SpellCategory", &Lua_Spell::GetSpellCategory)
		.def("PVPDuration", &Lua_Spell::GetPVPDuration)
		.def("PVPDurationCap", &Lua_Spell::GetPVPDurationCap)
		.def("CanMGB", &Lua_Spell::GetCanMGB)
		.def("DispelFlag", &Lua_Spell::GetDispelFlag)
		.def("MinResist", &Lua_Spell::GetMinResist)
		.def("MaxResist", &Lua_Spell::GetMaxResist)
		.def("ViralTargets", &Lua_Spell::GetViralTargets)
		.def("ViralTimer", &Lua_Spell::GetViralTimer)
		.def("NimbusEffect", &Lua_Spell::GetNimbusEffect)
		.def("DirectionalStart", &Lua_Spell::GetDirectionalStart)
		.def("DirectionalEnd", &Lua_Spell::GetDirectionalEnd)
		.def("SpellGroup", &Lua_Spell::GetSpellGroup)
		.def("PowerfulFlag", &Lua_Spell::GetPowerfulFlag)
		.def("CastRestriction", &Lua_Spell::GetCastRestriction)
		.def("AllowRest", &Lua_Spell::GetAllowRest)
		.def("InCombat", &Lua_Spell::GetInCombat)
		.def("OutOfCombat", &Lua_Spell::GetOutOfCombat)
		.def("AEMaxTargets", &Lua_Spell::GetAEMaxTargets)
		.def("MaxTargets", &Lua_Spell::GetMaxTargets)
		.def("PersistDeath", &Lua_Spell::GetPersistDeath)
		.def("MinDist", &Lua_Spell::GetMinDist)
		.def("MinDistMod", &Lua_Spell::GetMinDistMod)
		.def("MaxDist", &Lua_Spell::GetMaxDist)
		.def("MaxDistMod", &Lua_Spell::GetMaxDistMod)
		.def("MinRange", &Lua_Spell::GetMinRange)
		.def("DamageShieldType", &Lua_Spell::GetDamageShieldType)
		.def("Rank", &Lua_Spell::GetRank);
}

#endif
