#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "../common/spdat.h"
#include "lua_spell.h"

void lua_register_spell(sol::state_view &sv)
{
	auto spell = sv.new_usertype<Lua_Spell>("Spell", sol::constructors<Lua_Spell(), Lua_Spell(int)>());
	spell["null"] = sol::readonly_property(&Lua_Spell::Null);
	spell["valid"] = sol::readonly_property(&Lua_Spell::Valid);
	spell["AEDuration"] = &Lua_Spell::GetAEDuration;
	spell["AEMaxTargets"] = &Lua_Spell::GetAEMaxTargets;
	spell["Activated"] = &Lua_Spell::GetActivated;
	spell["AllowRest"] = &Lua_Spell::GetAllowRest;
	spell["AoeRange"] = &Lua_Spell::GetAoeRange;
	spell["Base"] = &Lua_Spell::GetBase;
	spell["Base2"] = &Lua_Spell::GetBase2;
	spell["BaseDiff"] = &Lua_Spell::GetBaseDiff;
	spell["BonusHate"] = &Lua_Spell::GetBonusHate;
	spell["BuffDuration"] = &Lua_Spell::GetBuffDuration;
	spell["BuffdurationFormula"] = &Lua_Spell::GetBuffdurationFormula;
	spell["CanMGB"] = &Lua_Spell::GetCanMGB;
	spell["CastOnOther"] = &Lua_Spell::GetCastOnOther;
	spell["CastOnYou"] = &Lua_Spell::GetCastOnYou;
	spell["CastRestriction"] = &Lua_Spell::GetCastRestriction;
	spell["CastTime"] = &Lua_Spell::GetCastTime;
	spell["CastingAnim"] = &Lua_Spell::GetCastingAnim;
	spell["Classes"] = &Lua_Spell::GetClasses;
	spell["ComponentCounts"] = &Lua_Spell::GetComponentCounts;
	spell["Components"] = &Lua_Spell::GetComponents;
	spell["DamageShieldType"] = &Lua_Spell::GetDamageShieldType;
	spell["Deities"] = &Lua_Spell::GetDeities;
	spell["DescNum"] = &Lua_Spell::GetDescNum;
	spell["DirectionalEnd"] = &Lua_Spell::GetDirectionalEnd;
	spell["DirectionalStart"] = &Lua_Spell::GetDirectionalStart;
	spell["DisallowSit"] = &Lua_Spell::GetDisallowSit;
	spell["DispelFlag"] = &Lua_Spell::GetDispelFlag;
	spell["EffectDescNum"] = &Lua_Spell::GetEffectDescNum;
	spell["EffectID"] = &Lua_Spell::GetEffectID;
	spell["EndurCost"] = &Lua_Spell::GetEndurCost;
	spell["EndurTimerIndex"] = &Lua_Spell::GetEndurTimerIndex;
	spell["EndurUpkeep"] = &Lua_Spell::GetEndurUpkeep;
	spell["EnvironmentType"] = &Lua_Spell::GetEnvironmentType;
	spell["Formula"] = &Lua_Spell::GetFormula;
	spell["GoodEffect"] = &Lua_Spell::GetGoodEffect;
	spell["HateAdded"] = &Lua_Spell::GetHateAdded;
	spell["ID"] = &Lua_Spell::GetID;
	spell["InCombat"] = &Lua_Spell::GetInCombat;
	spell["Mana"] = &Lua_Spell::GetMana;
	spell["Max"] = &Lua_Spell::GetMax;
	spell["MaxDist"] = &Lua_Spell::GetMaxDist;
	spell["MaxDistMod"] = &Lua_Spell::GetMaxDistMod;
	spell["MaxResist"] = &Lua_Spell::GetMaxResist;
	spell["MaxTargets"] = &Lua_Spell::GetMaxTargets;
	spell["MinDist"] = &Lua_Spell::GetMinDist;
	spell["MinDistMod"] = &Lua_Spell::GetMinDistMod;
	spell["MinRange"] = &Lua_Spell::GetMinRange;
	spell["MinResist"] = &Lua_Spell::GetMinResist;
	spell["Name"] = &Lua_Spell::GetName;
	spell["NimbusEffect"] = &Lua_Spell::GetNimbusEffect;
	spell["NoexpendReagent"] = &Lua_Spell::GetNoexpendReagent;
	spell["NumHits"] = &Lua_Spell::GetNumHits;
	spell["OtherCasts"] = &Lua_Spell::GetOtherCasts;
	spell["OutOfCombat"] = &Lua_Spell::GetOutOfCombat;
	spell["PVPDuration"] = &Lua_Spell::GetPVPDuration;
	spell["PVPDurationCap"] = &Lua_Spell::GetPVPDurationCap;
	spell["PVPResistBase"] = &Lua_Spell::GetPVPResistBase;
	spell["PVPResistCalc"] = &Lua_Spell::GetPVPResistCalc;
	spell["PVPResistCap"] = &Lua_Spell::GetPVPResistCap;
	spell["PersistDeath"] = &Lua_Spell::GetPersistDeath;
	spell["Player1"] = &Lua_Spell::GetPlayer1;
	spell["PowerfulFlag"] = &Lua_Spell::GetPowerfulFlag;
	spell["PushBack"] = &Lua_Spell::GetPushBack;
	spell["PushUp"] = &Lua_Spell::GetPushUp;
	spell["Range"] = &Lua_Spell::GetRange;
	spell["Rank"] = &Lua_Spell::GetRank;
	spell["RecastTime"] = &Lua_Spell::GetRecastTime;
	spell["RecourseLink"] = &Lua_Spell::GetRecourseLink;
	spell["RecoveryTime"] = &Lua_Spell::GetRecoveryTime;
	spell["ResistDiff"] = &Lua_Spell::GetResistDiff;
	spell["ResistType"] = &Lua_Spell::GetResistType;
	spell["ShortBuffBox"] = &Lua_Spell::GetShortBuffBox;
	spell["Skill"] = &Lua_Spell::GetSkill;
	spell["SpellAffectIndex"] = &Lua_Spell::GetSpellAffectIndex;
	spell["SpellCategory"] = &Lua_Spell::GetSpellCategory;
	spell["SpellFades"] = &Lua_Spell::GetSpellFades;
	spell["SpellGroup"] = &Lua_Spell::GetSpellGroup;
	spell["TargetType"] = &Lua_Spell::GetTargetType;
	spell["TeleportZone"] = &Lua_Spell::GetTeleportZone;
	spell["TimeOfDay"] = &Lua_Spell::GetTimeOfDay;
	spell["Uninterruptable"] = &Lua_Spell::GetUninterruptable;
	spell["ViralTargets"] = &Lua_Spell::GetViralTargets;
	spell["ViralTimer"] = &Lua_Spell::GetViralTimer;
	spell["YouCast"] = &Lua_Spell::GetYouCast;
	spell["ZoneType"] = &Lua_Spell::GetZoneType;
}

#endif
